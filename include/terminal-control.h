#ifndef __TERMINAL_CONTROL_H__
#define __TERMINAL_CONTROL_H__

#include <stdint.h>
#include <stdlib.h>

void init_terminal();
void reset_terminal();
int32_t select_option(char *options[], size_t option_size, int32_t header_offset);
char *get_str(char *header, size_t max_len, int32_t (*filter)(int), char *placeholder);
uint32_t get_int(char *prompt, size_t max_length, char *placeholder);
void popup(char *h1, char *h2, char *h3);
void press_any_key();

#endif /* __TERMINAL_CONTROL_H__ */