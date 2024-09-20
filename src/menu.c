#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "dept.h"
#include "grade.h"
#include "heap.h"
#include "menu.h"
#include "student.h"
#include "terminal-control.h"

static menu_t *add_menu(char *, void (*)(void), struct menu_t *, struct menu_t *);
static void recursive_free_menu(menu_t *);
void exit_warning();

menu_t *Main_Menu = NULL;

/****************************************************************************
 * Name: init_menu
 * Input: None
 * Return: None
 * Description: Initializes the main menu and its submenus for the application.
 ****************************************************************************/
void init_menu()
{
    if (Main_Menu != NULL)
    {
        return;
    }

    menu_t *sub_menu = NULL;
    Main_Menu = add_menu("Exit", &exit_warning, NULL, NULL);
    Main_Menu = add_menu("Save Data", &save_database, NULL, Main_Menu);

    sub_menu = add_menu("Return", NULL, NULL, NULL);
    sub_menu = add_menu("Display All Grades", &print_grades, NULL, sub_menu);
    sub_menu = add_menu("Update Grade", &update_grade_from_user, NULL, sub_menu);
    sub_menu = add_menu("Delete Grade", &delete_grade_from_user, NULL, sub_menu);
    sub_menu = add_menu("Add a New Grade", &grade_from_user, NULL, sub_menu);
    Main_Menu = add_menu("Grade Management", NULL, sub_menu, Main_Menu);

    sub_menu = add_menu("Return", NULL, NULL, NULL);
    sub_menu = add_menu("Display All Students", &print_student, NULL, sub_menu);
    sub_menu = add_menu("Update Student", &update_student_from_user, NULL, sub_menu);
    sub_menu = add_menu("Delete Student", &delete_student_from_user, NULL, sub_menu);
    sub_menu = add_menu("Add a New Student", &student_from_user, NULL, sub_menu);
    Main_Menu = add_menu("Student Management", NULL, sub_menu, Main_Menu);

    sub_menu = add_menu("Return", NULL, NULL, NULL);
    sub_menu = add_menu("Display All Departments", &print_dept, NULL, sub_menu);
    sub_menu = add_menu("Update Department", &update_dept_from_user, NULL, sub_menu);
    sub_menu = add_menu("Delete Department", &delete_dept_from_user, NULL, sub_menu);
    sub_menu = add_menu("Add a New Department", &dept_from_user, NULL, sub_menu);
    Main_Menu = add_menu("Department Management", NULL, sub_menu, Main_Menu);

    return;
}

static menu_t *add_menu(char *menu_text, void (*menu_fun)(void), struct menu_t *sub_menu,
                        struct menu_t *next)
{
    menu_t *temp = NULL;

    temp = (menu_t *)malloc(sizeof(menu_t));
    if (temp == NULL)
    {
        fprintf(stderr, "Out of memory while allocating memory for menu.\n");
        cleanup_and_exit();

        return NULL;
    }

    temp->menu_text = menu_text;
    temp->menu_fun = menu_fun;
    temp->sub_menu = sub_menu;
    temp->next = next;

    return temp;
}

void release_menu_resources()
{
    recursive_free_menu(Main_Menu);
    Main_Menu = NULL;
}

static void recursive_free_menu(menu_t *menu)
{
    if (menu != NULL)
    {
        recursive_free_menu(menu->sub_menu);
        menu->sub_menu = NULL;
        recursive_free_menu(menu->next);
        menu->next = NULL;
        free(menu);
    }

    return;
}

menu_t *select_menu(menu_t *const menu, char *const menu_header)
{
    int32_t selected = 0;
    size_t option_size = 0;
    menu_t *crnt_menu = NULL;
    char **temp_options = NULL;

    if (menu == NULL)
    {
        return NULL;
    }

    option_size = (menu_header == NULL) ? 0 : 1;
    crnt_menu = menu;
    while (crnt_menu != NULL)
    {
        option_size++;
        crnt_menu = crnt_menu->next;
    }

    if (Menu_Options != NULL)
    {
        fprintf(stderr, "Something went wrong! Menu_Options is not NULL.\n");
        cleanup_and_exit();
        return NULL;
    }

    Menu_Options = (char **)malloc(option_size * sizeof(char *));
    if (Menu_Options == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for menu options.\n");
        press_any_key();
        return NULL;
    }

    temp_options = Menu_Options;
    if (menu_header != NULL)
    {
        *temp_options++ = menu_header;
    }

    crnt_menu = menu;
    while (crnt_menu != NULL)
    {
        *temp_options++ = crnt_menu->menu_text;
        crnt_menu = crnt_menu->next;
    }

    selected = select_option(Menu_Options, option_size, (menu_header == NULL) ? 0 : 1);
    free(Menu_Options);
    Menu_Options = NULL;

    if (selected < 0)
    {
        return NULL;
    }

    crnt_menu = menu;
    while (crnt_menu != NULL && selected-- != 0)
    {
        crnt_menu = crnt_menu->next;
    }

    return crnt_menu;
}

void exit_warning()
{
    int p = select_option((char *[]){"Warning!", "Any unsaved changes will be lost!",
                                     "Are you sure to exit program?", "Go Back", "Exit"},
                          5, 3);
    if (p == 0)
    {
        return;
    }
    else
    {
        cleanup_and_exit();
    }
    return;
}