#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "dept.h"
#include "grade.h"
#include "heap.h"
#include "student.h"
#include "terminal-control.h"

Student_t *Student_Head = NULL;

static Student_t *create_student(uint32_t id, const char *name, char gender, Dept_t *dept);
static int32_t match_student(ListNode_t *node, uint32_t id);
static void free_student(ListNode_t *node);

static Student_t *create_student(uint32_t id, const char *name, char gender, Dept_t *dept)
{
    Student_t *new_student = (Student_t *)calloc(1, sizeof(Student_t));
    if (new_student == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        press_any_key();
        return NULL;
    }

    new_student->id = id;
    new_student->gender = gender;
    new_student->dept = dept;
    new_student->name = string_alloc(name, STUDENT_NAME_SIZE);
    if (new_student->name == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(new_student);
        press_any_key();
        return NULL;
    }

    return new_student;
}

static int32_t match_student(ListNode_t *node, uint32_t id)
{
    if (node == NULL)
    {
        return 1;
    }
    return compare_uint32(((Student_t *)node)->id, id);
}

int32_t cmp_student(ListNode_t *node1, ListNode_t *node2)
{
    if (node1 == NULL)
    {
        return 1;
    }
    if (node2 == NULL)
    {
        return -1;
    }
    return compare_uint32(((Student_t *)node1)->id, ((Student_t *)node2)->id);
}

static void free_student(ListNode_t *node)
{
    Student_t *student = (Student_t *)node;
    if (student->name != NULL)
    {
        free(student->name);
    }
    if (student->grade != NULL)
    {
        free(student->grade); // todo: fix
    }
    free(student);
}

void cleanup_student()
{
    while (Student_Head != NULL)
    {
        delete_node((ListNode_t **)&Student_Head, (ListNode_t *)Student_Head, free_student);
    }
}

Student_t *search_student(uint32_t id)
{
    int32_t result = -1;
    Student_t *stud = NULL;

    sorted_student_init();
    stud = sorted_student_next();
    while (stud != NULL)
    {
        result = match_student((ListNode_t *)stud, id);
        if (result >= 0)
        {
            break;
        }
        stud = sorted_student_next();
    }
    sorted_student_free();

    if (result == 0)
    {
        return stud;
    }
    return NULL;
}

void student_from_user()
{
    Dept_t *dept = NULL;
    Student_t *stud = NULL;
    char *name = NULL;
    char gender = '\0';
    uint32_t dept_id = 0;

    uint32_t id = get_int("Student ID", INT_STUDENT_LENGTH + 1, NULL);
    if (id == UINT32_MAX)
    {
        popup("Message", "Student ID not provided.", "OK");
        return;
    }

    stud = search_student(id);
    if (stud != NULL)
    {
        popup("Error", "Student with this ID already exists.", "OK");
        return;
    }

    name = get_str("Enter Student Name", STUDENT_NAME_SIZE, &isprint, NULL);
    if (name == NULL || name[0] == '\0')
    {
        free(name);
        popup("Error", "No student name provided.", "OK");

        return;
    }

    gender = (select_option((char *[]){"Select Student Gender", "Male", "Female"}, 3, 1) == 0)
                 ? 'm'
                 : 'f';

    dept_id = get_int("Department ID", INT_DEPT_LENGTH, NULL);
    if (dept_id != UINT32_MAX)
    {
        dept = (Dept_t *)search_sorted(dept_id, &match_dept, (ListNode_t *)Dept_Head);
    }

    if (dept == NULL)
    {
        insert_sorted((ListNode_t **)&Student_Head,
                      (ListNode_t *)create_student(id, name, gender, NULL), &cmp_student);
        popup("No department with given department id", "Student created with no department.",
              "OK");
    }
    else
    {
        insert_sorted((ListNode_t **)&dept->students,
                      (ListNode_t *)create_student(id, name, gender, dept), &cmp_student);
    }
    free(name);

    return;
}

void delete_student_from_user()
{
    uint32_t id = 0;
    Student_t *student = NULL;

    id = get_int("Deleting: Search Student ID", INT_DEPT_LENGTH, NULL);
    if (id == UINT32_MAX)
    {
        popup("Message", "Student ID not provided.", "OK");
        return;
    }

    student = search_student(id);
    if (student == NULL)
    {
        popup("Error", "No Student found with this ID.", "OK");
        return;
    }

    if (student->dept == NULL)
    {
        delete_node((ListNode_t **)&Student_Head, (ListNode_t *)student, &free_student);
    }
    else
    {
        delete_node((ListNode_t **)&student->dept->students, (ListNode_t *)student, &free_student);
    }

    return;
}

void update_student_from_user()
{
    uint32_t student_id = 0;
    Student_t *student = NULL;
    char *new_name = NULL;
    char new_gender = '\0';
    Dept_t *new_dept = NULL;
    uint32_t new_dept_id = 0;
    ListNode_t **new_head = NULL;
    ListNode_t **old_head = NULL;
    char *buffer = NULL;
    uint32_t buffer_length = 0;

    /* create a buffer large enough to hold the string representation of UINT32_MAX */
    buffer_length = snprintf(NULL, 0, "%" PRIu32, UINT32_MAX) + 1;
    buffer = (char *)malloc(buffer_length * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed while allocating memory for placeholder text.\n");
        press_any_key();
        return;
    }

    /* get user inputs */
    student_id = get_int("Editing: Search student ID", INT_DEPT_LENGTH, NULL);
    if (student_id == UINT32_MAX)
    {
        free(buffer);
        popup("Message", "Student ID not provided.", "OK");
        return;
    }

    student = search_student(student_id);
    if (student == NULL)
    {
        free(buffer);
        popup("Error", "No student found with this ID.", "OK");
        return;
    }

    new_name = get_str("Enter Updated student name", DEPT_NAME_SIZE, &isprint, student->name);
    if (new_name == NULL || new_name[0] == '\0')
    {
        free(buffer);
        free(new_name);
        popup("Error", "Student's new name not provided.", "OK");
        return;
    }

    new_gender = (select_option((char *[]){"Select Student Gender", "Male", "Female"}, 3, 1) == 0)
                     ? 'm'
                     : 'f';

    if (student->dept != NULL)
    {
        snprintf(buffer, buffer_length, "%" PRIu32, student->dept->id);
    }
    else
    {
        buffer[0] = '\0';
    }
    new_dept_id = get_int("Department ID", INT_DEPT_LENGTH, buffer);
    if (new_dept_id != UINT32_MAX)
    {
        new_dept = (Dept_t *)search_sorted(new_dept_id, &match_dept, (ListNode_t *)Dept_Head);
    }

    if (student->dept == NULL)
    {
        old_head = (ListNode_t **)&Student_Head;
    }
    else
    {
        old_head = (ListNode_t **)&student->dept->students;
    }

    if (new_dept == NULL)
    {
        new_head = (ListNode_t **)&Student_Head;
        popup("No department with this id", "Departent changed to None.", "OK");
    }
    else
    {
        new_head = (ListNode_t **)&new_dept->students;
    }

    /* update the student with new info */
    free(buffer);
    free(student->name);
    student->name = new_name;
    student->gender = new_gender;
    delete_node(old_head, (ListNode_t *)student, NULL);
    insert_sorted(new_head, (ListNode_t *)student, &cmp_student);
    student->dept = new_dept;

    return;
}

static void print_student_row(Student_t *student)
{
    printf(PIPE2 " BDCOM%03" PRIu32 " " PIPE2 " %*.*s " PIPE2 " %6s ", student->id,
           STUDENT_NAME_SIZE, STUDENT_NAME_SIZE, student->name,
           (student->gender == 'm') ? "Male" : "Female");
    if (student->dept == NULL)
    {
        printf(PIPE2 " %*.*s ", DEPT_NAME_SIZE, DEPT_NAME_SIZE, "None");
    }
    else
    {
        printf(PIPE2 " %*.*s ", DEPT_NAME_SIZE, DEPT_NAME_SIZE, student->dept->name);
    }
    if (student->grade == NULL)
    {
        printf(PIPE2 "    None " PIPE2 "    None " PIPE2 "    None " PIPE2 "\n");
    }
    else
    {
        printf(PIPE2 " %7" PRIu8 " ", student->grade->english);
        printf(PIPE2 " %7" PRIu8 " ", student->grade->math);
        printf(PIPE2 " %7" PRIu8 " " PIPE2 "\n", student->grade->history);
    }
    fflush(stdout);
    return;
}

void print_student()
{
    Student_t *student = NULL;
    system("clear");

#ifdef USE_UNICODE
    printf("┌──────────┬──────────────────────┬────────┬──────────────────────┬─────────┬─────────┬"
           "─────────┐\n");
    printf("│    ID    │    Student Name      │ Gender │       Dept Name      │ English │   Math  "
           "│ History │\n");
    printf("├──────────┼──────────────────────┼────────┼──────────────────────┼─────────┼─────────┼"
           "─────────┤\n");
#else
    printf("+----------+----------------------+--------+----------------------+---------+---------+"
           "---------+\n");
    printf("|    ID    |    Student Name      | Gender |       Dept Name      | English |   Math  "
           "| History |\n");
    printf("+----------+----------------------+--------+----------------------+---------+---------+"
           "---------+\n");
#endif

    sorted_student_init();
    student = sorted_student_next();
    while (student != NULL)
    {
        print_student_row(student);
        student = sorted_student_next();
    }
    sorted_student_free();

#ifdef USE_UNICODE
    printf("└──────────┴──────────────────────┴────────┴──────────────────────┴─────────┴─────────┴"
           "─────────┘\n");
#else
    printf("+----------+----------------------+--------+----------------------+---------+---------+"
           "---------+\n");
#endif
    press_any_key();

    return;
}

void save_students(const char *filename)
{
    FILE *file = NULL;
    uint8_t name_length = 0;
    uint32_t dept_id = 0;
    Student_t *current = NULL;

    file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file for writing: %s\n", filename);
        press_any_key();
        return;
    }

    sorted_student_init();
    current = sorted_student_next();
    while (current != NULL)
    {
        name_length = strnlen(current->name, STUDENT_NAME_SIZE - 1) + 1;
        dept_id = (current->dept != NULL) ? current->dept->id : UINT32_MAX;

        if (fwrite(&current->id, sizeof(current->id), 1, file) != 1 ||
            fwrite(&name_length, sizeof(name_length), 1, file) != 1 ||
            fwrite(current->name, sizeof(char), name_length, file) != name_length ||
            fwrite(&current->gender, sizeof(current->gender), 1, file) != 1 ||
            fwrite(&dept_id, sizeof(dept_id), 1, file) != 1)
        {
            fprintf(stderr, "Error writing to file: %s\n", filename);
            press_any_key();
            break;
        }

        current = sorted_student_next();
    }
    sorted_student_free();

    fclose(file);
    return;
}

void load_students(const char *filename)
{
    FILE *file = NULL;
    long file_length = 0;
    Dept_t *dept = NULL;
    uint32_t dept_id = 0;
    uint8_t name_length = 0;
    Student_t *new_student = NULL;

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
        new_student = (Student_t *)calloc(1, sizeof(Student_t));
        if (new_student == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            press_any_key();
            break;
        }

        if (fread(&new_student->id, sizeof(new_student->id), 1, file) != 1 ||
            fread(&name_length, sizeof(name_length), 1, file) != 1)
        {
            free(new_student);
            fprintf(stderr, "Error reading student data from file: %s\n", filename);
            press_any_key();
            break;
        }
        new_student->name = (char *)malloc(name_length);
        if (new_student->name == NULL)
        {
            free(new_student);
            fprintf(stderr, "Memory allocation failed\n");
            press_any_key();
            break;
        }

        if (fread(new_student->name, sizeof(char), name_length, file) != name_length ||
            fread(&new_student->gender, sizeof(new_student->gender), 1, file) != 1 ||
            fread(&dept_id, sizeof(dept_id), 1, file) != 1)
        {
            free(new_student->name);
            free(new_student);
            fprintf(stderr, "Error reading student data from file: %s\n", filename);
            press_any_key();
            break;
        }
        new_student->name[name_length - 1] = '\0';

        if (dept_id == UINT32_MAX)
        {
            dept = NULL;
        }
        else
        {
            dept = (Dept_t *)search_sorted(dept_id, &match_dept, (ListNode_t *)Dept_Head);
        }

        new_student->dept = dept;

        if (dept == NULL)
        {
            insert_sorted((ListNode_t **)&Student_Head, (ListNode_t *)new_student, &cmp_student);
        }
        else
        {
            insert_sorted((ListNode_t **)&dept->students, (ListNode_t *)new_student, &cmp_student);
        }
    }

    fclose(file);

    return;
}
