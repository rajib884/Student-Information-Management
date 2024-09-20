#ifndef __MENU_H__
#define __MENU_H__

#include <stddef.h>

typedef struct menu_t
{
    char *menu_text;
    void (*menu_fun)(void);
    struct menu_t *sub_menu;
    struct menu_t *next;
} menu_t;

extern menu_t *Main_Menu;

void init_menu();
void release_menu_resources();
menu_t *select_menu(menu_t *const menu, char *const menu_header);

#endif