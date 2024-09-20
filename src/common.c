#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"
#include "dept.h"
#include "grade.h"
#include "menu.h"
#include "student.h"
#include "terminal-control.h"

char **Menu_Options = NULL;

/****************************************************************************
 * Name: string_alloc
 * Input:
 *   const char *literal  Pointer to a null-terminated string to be copied.
 *   size_t max_size      Maximum size for the allocated string buffer.
 * Return:
 *   char *               Pointer to a dynamically allocated string containing
 *                        a copy of the input `literal`, truncated to fit within
 *                        `max_size - 1` characters. Returns NULL if `literal` is
 *                        NULL or if memory allocation fails.
 * Description:
 *   This function allocates memory for a new string and copies a portion of the
 *   input `literal` string into the newly allocated buffer. The length of the
 *   copied string is limited to `max_size - 1` characters.
 ****************************************************************************/
char *string_alloc(const char *literal, size_t max_size)
{
    if (literal == NULL)
    {
        return NULL;
    }
    size_t str_length = strnlen(literal, max_size - 1);
    char *str = (char *)malloc(str_length + 1);
    if (str == NULL)
    {
        return NULL;
    }
    strncpy(str, literal, str_length);
    str[str_length] = '\0';
    return str;
}

void save_database()
{
    save_depts("data/departments.dat");
    save_students("data/students.dat");
    save_grades("data/grades.dat");
}

void load_database()
{
    /* create directory 'data' */
    struct stat st = {0};
    if (stat("data", &st) == -1)
    {
        if (mkdir("data", 0700) == -1)
        {
            fprintf(stderr, "Unable to create 'data' folder.");
            press_any_key();
            return;
        }
    }
    load_depts("data/departments.dat");
    load_students("data/students.dat");
    load_grades("data/grades.dat");
}

void cleanup_and_exit()
{
    reset_terminal();
    release_menu_resources();
    cleanup_dept();
    cleanup_student();
    if (Menu_Options != NULL)
    {
        free(Menu_Options);
    }
    printf(ENABLE_CURSOR);
    fflush(stdout);
    exit(0);
}

void print_centered(char *before, char *str, size_t max_len, char *after)
{
    size_t str_len = strnlen(str, max_len);
    size_t pad = (max_len - str_len) / 2;
    printf("%s%*.*s%*s%s", before, (int)(pad + str_len), (int)(pad + str_len), str,
           max_len - pad - str_len, "", after);
    return;
}

int32_t compare_uint32(uint32_t a, uint32_t b)
{
    if (a > b)
    {
        return 1;
    }
    if (a < b)
    {
        return -1;
    }
    return 0;
}

int32_t rollover(int32_t current_val, int32_t max_val, int32_t increment)
{
    current_val += increment;
    if (current_val > max_val)
    {
        return 0;
    }
    else if (current_val < 0)
    {
        return max_val;
    }
    return current_val;
}

void catch_exit_command(int n)
{
    cleanup_and_exit();
}