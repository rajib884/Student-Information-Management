#ifndef __DEPT_H__
#define __DEPT_H__

#include <stdint.h>

#include "linked-list.h"

typedef struct Student Student_t;

typedef struct Dept
{
    ListNode_t node;
    uint32_t id;
    char *name;
    Student_t *students;
} Dept_t;

extern Dept_t *Dept_Head;

void print_dept();
void dept_from_user();
void cleanup_dept();
void delete_dept_from_user();
void update_dept_from_user();
int32_t match_dept(ListNode_t *node, uint32_t id);
void save_depts(const char *filename);
void load_depts(const char *filename);

#endif /* __DEPT_H__ */