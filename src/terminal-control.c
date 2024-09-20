#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"
#include "terminal-control.h"

char Print_Buffer[BUFSIZ];
static struct termios Original_Termios;
static bool_t Termios_Initialized = false;
unsigned short int Rows = 0, Cols = 0;

static inline void check_initialization();
static bool_t terminal_size_changed();
static uint32_t get_keypress();
static void print_menu(char *[], size_t, int32_t, int32_t);
static void print_border(char *start, char *middle, size_t middle_count, char *end);
static void print_input_dialog(char *header, char *input, size_t max_len, int32_t selected,
                               char *help);

enum
{
    KEY_BACKSPACE_ALT = 0x8,
    KEY_ENTER = 0xA,
    KEY_BACKSPACE = 0x7F,
    KEY_UP_ARROW = 0x415B1B,
    KEY_DOWN_ARROW = 0x425B1B,
    KEY_LEFT_ARROW = 0x445B1B,
    KEY_RIGHT_ARROW = 0x435B1B,
};

static inline void check_initialization()
{
    if (Termios_Initialized == false)
    {
        perror("termios not initialized");
        cleanup_and_exit();
    }
    return;
}

static bool_t terminal_size_changed()
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        perror("ioctl error");
        cleanup_and_exit();
    }

    if (Rows != ws.ws_row || Cols != ws.ws_col)
    {
        Rows = ws.ws_row;
        Cols = ws.ws_col;
        return true;
    }

    return false;
}

static void print_border(char *start, char *middle, size_t middle_count, char *end)
{
    size_t pos = 0;
    size_t start_length = 0;
    size_t middle_length = 0;
    size_t end_length = 0;
    size_t total_length = 0;
    char *line = NULL;

    if (start == NULL || middle == NULL || end == NULL)
    {
        fprintf(stderr, "start, middle or end can not be null.\n");
        return;
    }

    start_length = strnlen(start, 10);
    middle_length = strnlen(middle, 10);
    end_length = strnlen(end, 10);
    total_length = start_length + middle_count * middle_length + end_length;

    line = (char *)malloc(total_length + 1);
    if (line == NULL)
    {
        perror("Failed to allocate memory");
        return;
    }

    memcpy(line + pos, start, start_length);
    pos += start_length;

    for (size_t i = 0; i < middle_count; ++i)
    {
        memcpy(line + pos, middle, middle_length);
        pos += middle_length;
    }

    memcpy(line + pos, end, end_length);
    pos += end_length;
    line[total_length] = '\0';

    puts(line);
    free(line);

    return;
}

/****************************************************************************
 * Name: print_menu
 * Input:
 *   char *options[]       An array of strings representing the header and menu options.
 *   size_t option_size    The size of options.
 *   int32_t header_offset The number of header lines to be displayed.
 *   int32_t selected      The index of the currently selected menu option.
 * Return:
 *   None
 * Description:
 *   This function displays a menu in the terminal with the given options. It prints
 *   the options in a bordered box, highlighting the currently selected option.
 *   The use of Unicode characters for borders is conditional on the USE_UNICODE macro.
 ****************************************************************************/
static void print_menu(char *options[], size_t option_size, int32_t header_offset, int32_t selected)
{
    int32_t i = 0;
    size_t max_length = 0;
    size_t str_length = 0;

    int32_t start_index = 0;
    int32_t end_index = 0;

    if (Cols < TERMINAL_MIN_SIZE || Rows < TERMINAL_MIN_SIZE)
    {
        printf(CLEAR_SCREEN "Window Size too small (%dx%d), resize window", Cols, Rows);
        fflush(stdout);
        return;
    }

    start_index = header_offset;
    end_index = (option_size < Rows - 4) ? option_size : Rows - 4;
    while (selected >= end_index - 3)
    {
        end_index++;
        start_index++;
    }

    while (end_index > option_size)
    {
        end_index--;
        start_index--;
    }

    for (i = 0; i < option_size; i++)
    {
        str_length = strnlen(options[i], Cols - 4);
        max_length = (max_length > str_length) ? max_length : str_length;
    }
    max_length = (max_length < WINDOW_MIN_WIDTH) ? WINDOW_MIN_WIDTH : max_length;

    printf(CLEAR_SCREEN);
#ifdef USE_UNICODE
    print_border("╔═", "═", max_length, "═╗");
#else
    print_border("+-", "-", max_length, "-+");
#endif

    int32_t padding;
    for (i = 0; i < header_offset; i++)
    {
        print_centered(PIPE " ", options[i], max_length, " " PIPE "\n");
    }

#ifdef USE_UNICODE
    print_border("╠═", "═", max_length, "═╣");
#else
    print_border("+-", "-", max_length, "-+");
#endif

    for (i = start_index; i < end_index; i++)
    {
        if (i == header_offset + selected)
        {
            printf(PIPE " " HIGHLIGHT_START "%-*.*s" HIGHLIGHT_END " " PIPE "\n", max_length,
                   max_length, options[i]);
        }
        else
        {
            printf(PIPE " %-*.*s " PIPE "\n", max_length, max_length, options[i]);
        }
    }

#ifdef USE_UNICODE
    print_border("╚═", "═", max_length, "═╝");
#else
    print_border("+-", "-", max_length, "-+");
#endif

    fflush(stdout);
    return;
}

/****************************************************************************
 * Name: get_keypress
 * Input:
 *   None
 * Return:
 *   uint32_t   The value of the key pressed as a 32-bit unsigned integer.
 *              If no key is pressed within the timeout period, returns UINT32_MAX.
 * Description:
 *   This function waits for a keypress from the standard input (stdin) and
 *   returns the key's value as a 32-bit unsigned integer. If no key is pressed
 *   within a predefined timeout period (100 milliseconds), it returns UINT32_MAX
 *   to indicate that no input was received.
 ****************************************************************************/
static uint32_t get_keypress()
{
    check_initialization();
    union {
        unsigned char c[4];
        uint32_t i;
    } buf = {0};
    ssize_t result = 0;

    fd_set read_fds;
    struct timeval timeout = {0, 100000};

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    /**
     * Use select() to monitor the file descriptors. It will wait for input on
     * STDIN or until the timeout occurs. FD_ISSET checks if STDIN_FILENO
     * (standard input) is ready for reading.
     */
    if (select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout) > 0 &&
        FD_ISSET(STDIN_FILENO, &read_fds))
    {
        size_t bytes_read = 0;
        while (bytes_read < sizeof(buf.c))
        {
            result = read(STDIN_FILENO, &buf.c[bytes_read], 1);
            if (result < 1)
            {
                return UINT32_MAX;
            }
            bytes_read++;

            if (buf.c[0] != 0x1b || bytes_read >= 4)
            {
                break;
            }

            if (buf.c[1] == 0x5B && buf.c[2] >= 0x40 && buf.c[2] <= 0x7E)
            {
                break;
            }
        }
        return buf.i;
    }
    return UINT32_MAX;
}

void init_terminal()
{
    struct termios raw;
    if (Termios_Initialized == false)
    {
        setvbuf(stdout, Print_Buffer, _IOFBF, BUFSIZ);
        tcgetattr(STDIN_FILENO, &Original_Termios);
        memcpy(&raw, &Original_Termios, sizeof(Original_Termios));
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        Termios_Initialized = true;
    }
    return;
}

void reset_terminal()
{
    if (Termios_Initialized == true)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &Original_Termios);
        setvbuf(stdout, NULL, _IOLBF, 0);
    }
    return;
}

/****************************************************************************
 * Name: select_option
 * Input: options        Pointer to an array of strings representing the menu options.
 *        option_size    Total number of options in the array.
 *        header_offset  Offset to account for any header text in the menu.
 * Return: On success, returns the index of the selected option.
 *         If the user exits the menu or no valid selection is made, returns -1.
 * Description: Displays a menu to the user based on the provided options and allows
 *              the user to navigate and select an option using keyboard input.
 *              The function handles arrow key inputs for navigation and the Enter
 *              key for selection. If the user presses 'q' or 'Q', the menu exits
 *              without making a selection.
 ****************************************************************************/
int32_t select_option(char *options[], size_t option_size, int32_t header_offset)
{
    bool_t running = true;
    bool_t reprint = true;
    uint32_t keypress = 0;
    int32_t selected = 0;
    int32_t selected_max = 0;

    selected_max = option_size - header_offset - 1;

    check_initialization();

    while (running)
    {
        if (terminal_size_changed() || reprint)
        {
            print_menu(options, option_size, header_offset, selected);
        }

        reprint = true;
        keypress = get_keypress();
        switch (keypress)
        {
            case UINT32_MAX: /* Timeout occured */
                reprint = false;
                break;
            case KEY_UP_ARROW:
            case KEY_LEFT_ARROW:
                selected = rollover(selected, selected_max, -1);
                break;
            case KEY_DOWN_ARROW:
            case KEY_RIGHT_ARROW:
                selected = rollover(selected, selected_max, +1);
                break;
            case KEY_ENTER:
                running = false;
                return selected;
                break;
            case 'q':
            case 'Q':
                running = false;
                break;
            default:
                /**
                 * printf("0x%x", keypress);
                 * fflush(stdout);
                 * sleep(1);
                 */
                break;
        }
    }

    return -1;
}

/****************************************************************************
 * Name: popup
 * Input:
 *   char *h1     Pointer to the first header string to be displayed.
 *   char *h2     Pointer to the second header string to be displayed.
 *   char *option Pointer to the selected option string.
 * Return:
 *   None.
 * Description:
 *   This function displays a popup menu with two header strings and an option
 *   string provided as input parameters and waits for the user to press enter
 *   or 'Q'/'q'.
 ****************************************************************************/
void popup(char *h1, char *h2, char *option)
{
    if (h1 == NULL || h2 == NULL || option == NULL)
    {
        return;
    }
    char *msg[3] = {h1, h2, option};
    select_option(msg, 3, 2);

    return;
}

/****************************************************************************
 * Name: get_str
 * Input:
 *   char *header            Pointer to a string to be displayed as the prompt/
 *                           header for user input.
 *   size_t max_len          Maximum length of the input string, including the
 *                           null terminator.
 *   int32_t (*filter)(int)  Function pointer to a filter function that determines
 *                           whether a character is valid. This function takes an
 *                           int (character) and returns a non-zero value if the
 *                           character is valid.
 *   char *placeholder       Pointer to a string to be used as initial input or
 *                           placeholder text. This string is pre-filled into the
 *                           input buffer if not NULL.
 * Return:
 *   char *           Pointer to a dynamically allocated string containing the user input.
 *                    Returns NULL if memory allocation fails or if the user exits without
 *                    providing valid input.
 * Description:
 *   This function prompts the user to enter a string, with options for handling
 *   special characters and user input constraints. It manages the input buffer, validates
 *   input characters using the provided filter function, and supports basic text editing
 *   commands such as backspace.
 ****************************************************************************/
char *get_str(char *header, size_t max_len, int32_t (*filter)(int), char *placeholder)
{
    bool_t reprint = true;
    bool_t running = true;
    uint32_t keypress = 0;
    int32_t selected = 0;
    size_t current_pos = 0;
    char *input = NULL;
    char *help = NULL;

    check_initialization();

    input = (char *)calloc(max_len, sizeof(char));
    if (input == NULL)
    {
        fprintf(
            stderr,
            "Memory allocation failed\nCould not allocate memory to store user input string.\n");
        press_any_key();
        return NULL;
    }

    if (placeholder != NULL)
    {
        for (size_t i = 0; i < max_len - 1; i++)
        {
            if (filter(placeholder[i]) && placeholder[i] != '\0')
            {
                input[current_pos++] = placeholder[i];
            }
            else
            {
                break;
            }
        }
    }

    printf(ENABLE_CURSOR);
    while (running)
    {
        if (terminal_size_changed() || reprint)
        {
            print_input_dialog(header, input, max_len - 1, selected, help);
        }
        help = NULL;
        reprint = true;
        keypress = get_keypress();
        if (selected == 0)
        {
            if (filter((char)keypress))
            {
                if (current_pos < max_len - 1)
                {
                    input[current_pos++] = (char)keypress;
                }
                else
                {
                    help = "Input limit reached";
                }
            }
            else
            {
                help = "Enter valid character";
            }
        }

        switch (keypress)
        {
            case KEY_UP_ARROW:
            case KEY_LEFT_ARROW:
                selected = rollover(selected, 2, -1);
                help = NULL;
                break;
            case KEY_DOWN_ARROW:
            case KEY_RIGHT_ARROW:
                selected = rollover(selected, 2, 1);
                help = NULL;
                break;
            case KEY_ENTER:
                running = false;
                break;
            case KEY_BACKSPACE:
            case KEY_BACKSPACE_ALT:
                help = "Nothing in buffer to remove";
                if (selected == 0 && current_pos > 0)
                {
                    input[--current_pos] = '\0';
                    help = NULL;
                }
                break;
            case UINT32_MAX:
                reprint = false;
                break;
            default:
                break;
        }
    }
    printf(DISABLE_CURSOR);

    if (selected == 0 || selected == 1)
    {
        return input;
    }
    if (input != NULL)
    {
        free(input);
        input = NULL;
    }
    return NULL;
}

/****************************************************************************
 * Name: print_input_dialog
 * Input:
 *   char *header     Pointer to a string to be displayed as the header or title
 *                    of the dialog box.
 *   char *input      Pointer to a string representing the current user input.
 *   size_t max_len   Maximum length of the input field and dialog box width.
 *   int32_t selected Index indicating the currently selected option
 *                        0: input field,
 *                        1: OK button,
 *                        2: Cancel button.
 *   char *help       Pointer to a string containing help or error messages to be displayed.
 * Return:
 *   None.
 * Description:
 *   This function displays a dialog box in the terminal with a header, an input field,
 *   and two options (OK and Cancel).
 ****************************************************************************/
static void print_input_dialog(char *header, char *input, size_t max_len, int32_t selected,
                               char *help)
{
    size_t terminal_width = 0;
    size_t str_len = 0;

    terminal_width = Cols - 4;
    str_len = strnlen(header, (max_len > terminal_width) ? max_len : terminal_width);
    max_len = (max_len > str_len) ? max_len : str_len;
    max_len = (max_len > WINDOW_MIN_WIDTH) ? max_len : WINDOW_MIN_WIDTH;

    printf(CLEAR_SCREEN);
#ifdef USE_UNICODE
    print_border("╔═", "═", max_len, "═╗");
    print_centered("║ ", header, max_len, " ║\n");
    print_border("╠═", "═", max_len, "═╣");
#else
    print_border("+-", "-", max_len, "-+");
    print_centered("| ", header, max_len, " |\n");
    print_border("+-", "-", max_len, "-+");
#endif

    if (selected == 0)
    {
        printf(PIPE " " HIGHLIGHT_START "%-*.*s" HIGHLIGHT_END " " PIPE "\n", max_len, max_len,
               input);
    }
    else
    {
        printf(PIPE " %-*.*s " PIPE "\n", max_len, max_len, input);
    }

#ifdef USE_UNICODE
    print_border("╚═", "═", max_len, "═╝");
#else
    print_border("+-", "-", max_len, "-+");
#endif

    if (selected == 1)
    {
        print_centered("  " HIGHLIGHT_START, "[  OK  ]", max_len / 2, HIGHLIGHT_END);
    }
    else
    {
        print_centered("  ", "[  OK  ]", max_len / 2, "");
    }

    if (selected == 2)
    {
        print_centered(HIGHLIGHT_START, "[Cancel]", max_len - max_len / 2, HIGHLIGHT_END "  ");
    }
    else
    {
        print_centered("", "[Cancel]", max_len - max_len / 2, "  ");
    }

    if (help != NULL)
    {
        printf("\n\n*%-30s", help);
    }

    printf("\033[%zu;%zuH", 4, strnlen(input, max_len) + 3);

    fflush(stdout);
    return;
}

/****************************************************************************
 * Name: get_int
 * Input:
 *   char *prompt       Pointer to a string to be displayed as a prompt to the user.
 *   size_t max_length  Maximum length of the input digit.
 *   char *placeholder  Pointer to a string to be used as placeholder
 *                      in the input field. This may be NULL.
 * Return:
 *   uint32_t           The integer value parsed from the user input, or UINT32_MAX
 *                      if the input is invalid or conversion fails.
 * Description:
 *   This function prompts the user to enter an integer value. It uses the `get_str`
 *   function to obtain a string input from the user, and then attempts to parse it as
 *   an unsigned integer.
 ****************************************************************************/
uint32_t get_int(char *prompt, size_t max_length, char *placeholder)
{
    char *str = NULL;
    char *endptr = NULL;
    unsigned long result = 0;

    result = UINT32_MAX;
    str = get_str(prompt, max_length, &isdigit, placeholder);
    if (str != NULL)
    {
        if (str[0] != '\0')
        {
            errno = 0;
            result = strtoul(str, &endptr, 10);
            if (errno != 0 || *endptr != '\0' || result > UINT32_MAX)
            {
                result = UINT32_MAX;
            }
        }
        free(str);
    }

    return (uint32_t)result;
}

void press_any_key()
{
    printf("\nPress any key to continue...\n");
    fflush(stdout);
    while (get_keypress() == UINT32_MAX)
    {
        usleep(10000);
    }
    return;
}