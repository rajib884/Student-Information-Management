#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "dept.h"
#include "linked-list.h"
#include "student.h"
#include "terminal-control.h"

Dept_t *Dept_Head = NULL;
static uint32_t Dept_ID = 1;

static Dept_t *create_dept(const char *name);
static int32_t cmp_dept(ListNode_t *node1, ListNode_t *node2);
static void free_dept(ListNode_t *node);
static void count_male_female(Student_t *head, uint32_t *male, uint32_t *female);

static Dept_t *create_dept(const char *name)
{
    if (name == NULL)
    {
        fprintf(stderr, "Department name can not be a pointer to NULL.");
        press_any_key();

        return NULL;
    }

    Dept_t *new_dept = (Dept_t *)calloc(1, sizeof(Dept_t));
    if (new_dept == NULL)
    {
        fprintf(stderr, "Memory allocation failed\nNot enough memory to create new department.");
        press_any_key();

        return NULL;
    }

    new_dept->id = Dept_ID++;
    new_dept->students = NULL;
    new_dept->name = string_alloc(name, DEPT_NAME_SIZE);
    if (new_dept->name == NULL)
    {
        fprintf(stderr, "Memory allocation failed\nNot enough memory to allocate department name.");
        free(new_dept);
        press_any_key();

        return NULL;
    }

    return new_dept;
}

static int32_t cmp_dept(ListNode_t *node1, ListNode_t *node2)
{
    if (node1 == NULL || node2 == NULL)
    {
        return 1;
    }
    return compare_uint32(((Dept_t *)node1)->id, ((Dept_t *)node2)->id);
}

int32_t match_dept(ListNode_t *node, uint32_t id)
{
    if (node == NULL)
    {
        return 1;
    }
    return compare_uint32(((Dept_t *)node)->id, id);
}

static void free_dept(ListNode_t *node)
{
    Dept_t *dept = (Dept_t *)node;
    Student_t *student = NULL;
    if (dept->name != NULL)
    {
        free(dept->name);
    }
    if (dept->students != NULL)
    {
        student = dept->students;
        while (student != NULL)
        {
            student->dept = NULL;
            student = (Student_t *)student->node.next;
        }
        merge_sorted((ListNode_t **)&Student_Head, (ListNode_t *)dept->students, &cmp_student);
    }
    free(dept);
}

static void count_male_female(Student_t *head, uint32_t *male, uint32_t *female)
{
    *male = 0;
    *female = 0;

    while (head != NULL)
    {
        if (head->gender == 'm')
        {
            (*male)++;
        }
        else if (head->gender == 'f')
        {
            (*female)++;
        }
        head = (Student_t *)head->node.next;
    }

    return;
}

void cleanup_dept()
{
    while (Dept_Head != NULL)
    {
        delete_node((ListNode_t **)&Dept_Head, (ListNode_t *)Dept_Head, &free_dept);
    }
}

void dept_from_user()
{
    char *str = NULL;

    str = get_str("Enter Department Name", DEPT_NAME_SIZE, &isprint, NULL);
    if (str == NULL || str[0] == '\0')
    {
        free(str);
        popup("Error", "No department name provided.", "OK");
        return;
    }

    insert_sorted((ListNode_t **)&Dept_Head, (ListNode_t *)create_dept(str), &cmp_dept);
    free(str);

    return;
}

void delete_dept_from_user()
{
    uint32_t id = 0;
    Dept_t *dept = NULL;

    id = get_int("Deleting: Search Department ID", INT_DEPT_LENGTH + 1, NULL);
    if (id == UINT32_MAX)
    {
        popup("Message", "Department ID not provided.", "OK");
        return;
    }

    dept = (Dept_t *)search_sorted(id, &match_dept, (ListNode_t *)Dept_Head);
    if (dept == NULL)
    {
        popup("Error", "No Department found with this ID.", "OK");
        return;
    }

    delete_node((ListNode_t **)&Dept_Head, (ListNode_t *)dept, &free_dept);

    return;
}

void update_dept_from_user()
{
    uint32_t id = 0;
    char *str = NULL;
    Dept_t *dept = NULL;

    id = get_int("Editing: Search Department ID", INT_DEPT_LENGTH + 1, NULL);
    if (id == UINT32_MAX)
    {
        popup("Message", "Department ID not provided.", "OK");
        return;
    }

    dept = (Dept_t *)search_sorted(id, &match_dept, (ListNode_t *)Dept_Head);
    if (dept == NULL)
    {
        popup("Error", "No Department found with this ID.", "OK");
        return;
    }

    str = get_str("Enter Updated Department Name", DEPT_NAME_SIZE, &isprint, dept->name);
    if (str == NULL || str[0] == '\0')
    {
        free(str);
        popup("Error", "Department's new name not provided.", "OK");
        return;
    }

    free(dept->name);
    dept->name = str;

    return;
}

void print_dept()
{
    Dept_t *dept = Dept_Head;
    uint32_t male = 0, female = 0;

    system("clear");
#ifdef USE_UNICODE
    printf("┌─────────┬──────────────────────┬──────┬────────┐\n");
    printf("│ Dept ID │       Dept Name      │ Male │ Female │\n");
    printf("├─────────┼──────────────────────┼──────┼────────┤\n");
#else
    printf("+---------+----------------------+------+--------+\n");
    printf("| Dept ID |       Dept Name      | Male | Female |\n");
    printf("+---------+----------------------+------+--------+\n");
#endif
    while (dept != NULL)
    {
        count_male_female(dept->students, &male, &female);
        printf(PIPE2 " %7" PRIu32 " " PIPE2 " %*.*s " PIPE2 " %4" PRIu32 " " PIPE2 "   %4" PRIu32
                     " " PIPE2 "\n",
               dept->id, DEPT_NAME_SIZE, DEPT_NAME_SIZE, dept->name, male, female);
        dept = (Dept_t *)dept->node.next;
    }
#ifdef USE_UNICODE
    printf("└─────────┴──────────────────────┴──────┴────────┘\n");
#else
    printf("+---------+----------------------+------+--------+\n");
#endif
    press_any_key();

    return;
}

void save_depts(const char *filename)
{
    FILE *file = NULL;
    uint8_t name_length = 0;
    Dept_t *current = NULL;

    file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file for writing: %s\n", filename);
        press_any_key();
        return;
    }

    current = (Dept_t *)Dept_Head;
    while (current != NULL)
    {
        name_length = strnlen(current->name, DEPT_NAME_SIZE - 1) + 1;

        if (fwrite(&current->id, sizeof(current->id), 1, file) != 1 ||
            fwrite(&name_length, sizeof(name_length), 1, file) != 1 ||
            fwrite(current->name, sizeof(char), name_length, file) != name_length)
        {
            fprintf(stderr, "Error writing to file: %s\n", filename);
            press_any_key();
            break;
        }

        current = (Dept_t *)current->node.next;
    }

    fclose(file);

    return;
}

void load_depts(const char *filename)
{
    FILE *file = NULL;
    long file_length = 0;
    Dept_t *new_dept = NULL;
    uint8_t name_length = 0;

    file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file for reading: %s\n", filename);
        press_any_key();
        return;
    }

    fseek(file, 0, SEEK_END);
    file_length = ftell(file);
    fseek(file, 0, SEEK_SET);

    while (ftell(file) < file_length)
    {
        new_dept = (Dept_t *)calloc(1, sizeof(Dept_t));
        if (new_dept == NULL)
        {
            fprintf(stderr, "Memory allocation failed for new department\n");
            press_any_key();
            break;
        }

        if (fread(&new_dept->id, sizeof(new_dept->id), 1, file) != 1 ||
            fread(&name_length, sizeof(name_length), 1, file) != 1)
        {
            free(new_dept);
            fprintf(stderr, "Error reading department ID or Name from file: %s\n", filename);
            press_any_key();
            break;
        }

        if (search_sorted(new_dept->id, &match_dept, (ListNode_t *)Dept_Head) != NULL)
        {
            fprintf(stderr, "Department ID not unique, discarding.\n");
            free(new_dept);
            press_any_key();
            continue;
        }

        new_dept->name = (char *)malloc(name_length * sizeof(char));
        if (new_dept->name == NULL)
        {
            free(new_dept);
            fprintf(stderr, "Memory allocation failed\n");
            press_any_key();
            break;
        }
        if (fread(new_dept->name, sizeof(char), name_length, file) != name_length)
        {
            free(new_dept->name);
            free(new_dept);
            fprintf(stderr, "Error reading student data from file: %s\n", filename);
            press_any_key();
            break;
        }
        new_dept->name[name_length - 1] = '\0';

        insert_sorted((ListNode_t **)&Dept_Head, (ListNode_t *)new_dept, &cmp_dept);
        if (new_dept->id >= Dept_ID)
        {
            Dept_ID = new_dept->id + 1;
        }
    }

    fclose(file);

    return;
}
