#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>

#include "common.h"
#include "grade.h"
#include "heap.h"
#include "student.h"
#include "terminal-control.h"

static Grade_t *update_grade(Student_t *student, uint8_t english, uint8_t math, uint8_t history);

static Grade_t *update_grade(Student_t *student, uint8_t english, uint8_t math, uint8_t history)
{
    if (student == NULL)
    {
        return NULL;
    }
    Grade_t *grade = student->grade;
    if (grade == NULL)
    {
        grade = (Grade_t *)calloc(1, sizeof(Grade_t));
        if (grade == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            press_any_key();
            return NULL;
        }
    }

    grade->english = english;
    grade->math = math;
    grade->history = history;

    student->grade = grade;
    grade->student = student;

    return grade;
}

void delete_grade(Grade_t *grade)
{
    if (grade->student != NULL)
    {
        grade->student->grade = NULL;
    }
    free(grade);
}

void grade_from_user()
{
    Student_t *stud = NULL;
    uint32_t english = 0, math = 0, history = 0;
    uint32_t id = 0;

    id = get_int("Student ID", INT_STUDENT_LENGTH + 1, NULL);
    if (id == UINT32_MAX)
    {
        popup("Message", "Student ID not provided.", "OK");
        return;
    }

    stud = search_student(id);
    if (stud == NULL)
    {
        popup("Error", "Student with this ID does not exists.", "OK");
        return;
    }

    english = get_int("English Marks", INT_GRADE_LENGTH + 1, NULL);
    if (english > MAX_GRADE)
    {
        popup("Error", "English Marks is Invalid.", "OK");
        return;
    }

    math = get_int("Math Marks", INT_GRADE_LENGTH + 1, NULL);
    if (math > MAX_GRADE)
    {
        popup("Error", "Math Marks is Invalid.", "OK");
        return;
    }

    history = get_int("History Marks", INT_GRADE_LENGTH + 1, NULL);
    if (history > MAX_GRADE)
    {
        popup("Error", "History Marks is Invalid.", "OK");
        return;
    }

    update_grade(stud, (uint8_t)english, (uint8_t)math, (uint8_t)history);

    return;
}

void delete_grade_from_user()
{
    Student_t *stud = NULL;
    uint32_t id = 0;

    id = get_int("Deleting: Search Student ID", INT_STUDENT_LENGTH + 1, NULL);
    if (id == UINT32_MAX)
    {
        popup("Message", "Student ID not provided.", "OK");
        return;
    }

    stud = search_student(id);
    if (stud == NULL)
    {
        popup("Error", "Student with this ID does not exists.", "OK");
        return;
    }

    delete_grade(stud->grade);

    return;
}

void update_grade_from_user()
{
    Student_t *stud = NULL;
    uint32_t english = 0, math = 0, history = 0;
    uint32_t buffer_length = 0;
    char *buffer = NULL;

    /* create a buffer large enough to hold the string representation of UINT8_MAX */
    buffer_length = snprintf(NULL, 0, "%" PRIu8, UINT8_MAX) + 1;
    buffer = (char *)malloc(buffer_length * sizeof(char));
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed while allocating memory for placeholder text.\n");
        press_any_key();
        return;
    }

    uint32_t id = get_int("Updating: Search Student ID", INT_STUDENT_LENGTH + 1, NULL);
    if (id == UINT32_MAX)
    {
        free(buffer);
        popup("Message", "Student ID not provided.", "OK");
        return;
    }

    stud = search_student(id);
    if (stud == NULL)
    {
        free(buffer);
        popup("Error", "Student with this ID does not exists.", "OK");
        return;
    }

    if (stud->grade != NULL)
    {
        snprintf(buffer, buffer_length, "%" PRIu8, stud->grade->english);
    }
    else
    {
        buffer[0] = '\0';
    }
    english = get_int("English Marks", INT_GRADE_LENGTH + 1, buffer);
    if (english > MAX_GRADE)
    {
        free(buffer);
        popup("Error", "English Marks is Invalid.", "OK");
        return;
    }

    if (stud->grade != NULL)
    {
        snprintf(buffer, buffer_length, "%" PRIu8, stud->grade->math);
    }
    else
    {
        buffer[0] = '\0';
    }
    math = get_int("Math Marks", INT_GRADE_LENGTH + 1, buffer);
    if (math > MAX_GRADE)
    {
        free(buffer);
        popup("Error", "Math Marks is Invalid.", "OK");
        return;
    }

    if (stud->grade != NULL)
    {
        snprintf(buffer, buffer_length, "%" PRIu8, stud->grade->history);
    }
    else
    {
        buffer[0] = '\0';
    }
    history = get_int("History Marks", INT_GRADE_LENGTH + 1, buffer);
    if (history > MAX_GRADE)
    {
        free(buffer);
        popup("Error", "History Marks is Invalid.", "OK");
        return;
    }

    update_grade(stud, (uint8_t)english, (uint8_t)math, (uint8_t)history);

    free(buffer);
    return;
}

static void print_grade_row(Student_t *student)
{
    Grade_t *grade = NULL;
    if (student != NULL && student->grade != NULL)
    {
        grade = student->grade;
        printf(PIPE2 " %20s " PIPE2 " %7" PRIu8 " " PIPE2 " %7" PRIu8 " " PIPE2 " %7" PRIu8
                     " " PIPE2 "\n",
               student->name, grade->english, grade->math, grade->history);
    }
    return;
}

void print_grades()
{
    Student_t *s = NULL;
    system("clear");
#ifdef USE_UNICODE
    printf("┌──────────────────────┬─────────┬─────────┬─────────┐\n");
    printf("│    Student Name      │ English │   Math  │ History │\n");
    printf("├──────────────────────┼─────────┼─────────┼─────────┤\n");
#else
    printf("+----------------------+---------+---------+---------+\n");
    printf("|    Student Name      | English |   Math  | History |\n");
    printf("+----------------------+---------+---------+---------+\n");
#endif

    sorted_student_init();
    s = sorted_student_next();
    while (s != NULL)
    {
        print_grade_row(s);
        s = sorted_student_next();
    }
    sorted_student_free();

#ifdef USE_UNICODE
    printf("└──────────────────────┴─────────┴─────────┴─────────┘\n");
#else
    printf("+----------------------+---------+---------+---------+\n");
#endif
    press_any_key();

    return;
}

void save_grades(const char *filename)
{
    FILE *file = NULL;
    Student_t *student = NULL;
    Grade_t *grade = NULL;

    file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file for writing: %s\n", filename);
        press_any_key();
        return;
    }

    sorted_student_init();
    student = sorted_student_next();
    while (student != NULL)
    {
        grade = student->grade;
        if (grade != NULL)
        {
            if (fwrite(&student->id, sizeof(student->id), 1, file) != 1 ||
                fwrite(&grade->english, sizeof(grade->english), 1, file) != 1 ||
                fwrite(&grade->math, sizeof(grade->math), 1, file) != 1 ||
                fwrite(&grade->history, sizeof(grade->history), 1, file) != 1)
            {
                fprintf(stderr, "Error writing to file: %s\n", filename);
                press_any_key();
                break;
            }
        }
        student = sorted_student_next();
    }
    sorted_student_free();

    fclose(file);
    return;
}

void load_grades(const char *filename)
{
    FILE *file = NULL;
    long file_length = 0;
    uint32_t student_id = 0;
    Student_t *student = NULL;
    Grade_t *new_grade = NULL;
    size_t skip_size = 0;

    skip_size += sizeof(new_grade->english);
    skip_size += sizeof(new_grade->math);
    skip_size += sizeof(new_grade->history);

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
        if (fread(&student_id, sizeof(student_id), 1, file) != 1)
        {
            fprintf(stderr, "Error reading student ID from file: %s\n", filename);
            press_any_key();
            break;
        }

        student = search_student(student_id);
        if (student != NULL && student->grade == NULL)
        {
            new_grade = (Grade_t *)malloc(sizeof(Grade_t));
            if (new_grade == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                press_any_key();
                break;
            }

            if (fread(&new_grade->english, sizeof(new_grade->english), 1, file) != 1 ||
                fread(&new_grade->math, sizeof(new_grade->math), 1, file) != 1 ||
                fread(&new_grade->history, sizeof(new_grade->history), 1, file) != 1)
            {
                free(new_grade);
                fprintf(stderr, "Error reading grades from file: %s\n", filename);
                press_any_key();
                break;
            }

            student->grade = new_grade;
        }
        else
        {
            if (fseek(file, skip_size, SEEK_CUR) != 0)
            {
                fprintf(stderr, "Error seeking in file: %s\n", filename);
                press_any_key();
                break;
            }
        }
    }

    fclose(file);

    return;
}