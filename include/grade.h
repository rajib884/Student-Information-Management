#ifndef __GRADE_H__
#define __GRADE_H__

#include "linked-list.h"

typedef struct Student Student_t;

typedef struct Grade
{
    Student_t *student;
    uint8_t english;
    uint8_t math;
    uint8_t history;
} Grade_t;

void grade_from_user();
void delete_grade_from_user();
void update_grade_from_user();
void delete_grade(Grade_t *grade);
void print_grades();
void save_grades(const char *filename);
void load_grades(const char *filename);

#endif /* __GRADE_H__ */