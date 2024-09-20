#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>
#include <stdint.h>

/* #define USE_UNICODE */

#define DEPT_NAME_SIZE 20
#define INT_DEPT_LENGTH 10

#define STUDENT_NAME_SIZE 20
#define INT_STUDENT_LENGTH 3

#define MAX_GRADE 100
#define INT_GRADE_LENGTH 3

#define WINDOW_MIN_WIDTH 30
#define TERMINAL_MIN_SIZE 10

#define CLEAR_SCREEN "\033[H\033[J"
#define DISABLE_CURSOR "\e[?25l"
#define ENABLE_CURSOR "\e[?25h"
#ifdef USE_UNICODE
    #define PIPE "║"
    #define PIPE2 "│"
    #define HIGHLIGHT_START "\x1b[48;5;46m\x1b[30m"
    #define HIGHLIGHT_RED "\x1b[41m\x1b[37m"
#else
    #define PIPE "|"
    #define PIPE2 "|"
    #define HIGHLIGHT_START "\x1b[7m"
    #define HIGHLIGHT_RED ""
#endif
#define HIGHLIGHT_END "\x1b[0m"

typedef enum bool_t
{
    false = 0,
    true = 1
} bool_t;

extern char **Menu_Options;

void cleanup_and_exit();
char *string_alloc(const char *literal, size_t max_size);
void print_centered(char *before, char *str, size_t max_len, char *after);
int32_t compare_uint32(uint32_t a, uint32_t b);
int32_t rollover(int32_t current_val, int32_t max_val, int32_t increment);
void save_database();
void load_database();
void catch_exit_command(int n);

#endif /* __UTILS_H__ */