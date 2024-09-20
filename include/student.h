#ifndef __STUDENT_H__
#define __STUDENT_H__

#include <stdint.h>

#include "linked-list.h"

typedef struct Grade Grade_t;
typedef struct Dept Dept_t;

typedef struct Student
{
    ListNode_t node;
    uint32_t id;
    char *name;
    char gender;
    Grade_t *grade;
    Dept_t *dept;
} Student_t;

extern Student_t *Student_Head;

int32_t cmp_student(ListNode_t *node1, ListNode_t *node2);

void cleanup_student();
void student_from_user();
void delete_student_from_user();
void update_student_from_user();
void print_student();
Student_t *search_student(uint32_t id);
void save_students(const char *filename);
void load_students(const char *filename);

#endif /* __STUDENT_H__ */