#ifndef __HEAP_H__
#define __HEAP_H__

typedef struct Student Student_t;

void sorted_student_init();
Student_t *sorted_student_next();
void sorted_student_free();

#endif /* __HEAP_H__ */