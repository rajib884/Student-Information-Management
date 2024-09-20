#include <signal.h>
#include <stdio.h>

#include "common.h"
#include "menu.h"
#include "terminal-control.h"

int main()
{
    signal(SIGINT, &catch_exit_command); /* Catch Ctrl+C*/

    system("clear");
    printf(DISABLE_CURSOR);

    init_menu();
    init_terminal();
    load_database();

    bool_t running = true;
    menu_t *t = NULL;
    while (running)
    {
        if (t == NULL || t->sub_menu == NULL)
        {
            t = select_menu(Main_Menu, "Main Menu");
        }
        else
        {
            t = select_menu(t->sub_menu, t->menu_text);
        }

        if (t == NULL)
        {
            running = false;
            break;
        }

        if (t->menu_fun != NULL)
        {
            t->menu_fun();
            t = NULL;
            fflush(stdout);
        }
    }

    cleanup_and_exit();

    return 0;
}
