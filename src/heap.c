#include <stdio.h>
#include <stdlib.h>

#include "dept.h"
#include "grade.h"
#include "heap.h"
#include "student.h"
#include "terminal-control.h"

static Student_t **Heap_Array = NULL;
static int Heap_Size = 0;

static void min_heapify(Student_t *heap[], int size, int i,
                        int (*cmp_func)(ListNode_t *, ListNode_t *));
static void build_min_heap(Student_t *heap[], int size,
                           int (*cmp_func)(ListNode_t *, ListNode_t *));

static void min_heapify(Student_t *heap[], int size, int i,
                        int (*cmp_func)(ListNode_t *, ListNode_t *))
{
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < size && cmp_func((ListNode_t *)heap[left], (ListNode_t *)heap[smallest]) < 0)
        smallest = left;

    if (right < size && cmp_func((ListNode_t *)heap[right], (ListNode_t *)heap[smallest]) < 0)
        smallest = right;

    if (smallest != i)
    {
        Student_t *temp = heap[i];
        heap[i] = heap[smallest];
        heap[smallest] = temp;
        min_heapify(heap, size, smallest, cmp_func);
    }
}

static void build_min_heap(Student_t *heap[], int size, int (*cmp_func)(ListNode_t *, ListNode_t *))
{
    for (int i = size / 2 - 1; i >= 0; i--)
    {
        min_heapify(heap, size, i, cmp_func);
    }
}

void sorted_student_init()
{
    sorted_student_free();

    Dept_t *dept = Dept_Head;
    int dept_count = 1;

    while (dept != NULL)
    {
        dept_count++;
        dept = (Dept_t *)dept->node.next;
    }

    if (dept_count == 0)
    {
        Heap_Array = NULL;
        Heap_Size = 0;
        return;
    }

    Heap_Array = (Student_t **)malloc(dept_count * sizeof(Student_t *));
    if (Heap_Array == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        press_any_key();
        return;
    }

    Heap_Size = dept_count;

    Heap_Array[0] = Student_Head;
    dept = Dept_Head;
    for (int i = 1; i < dept_count; i++)
    {
        Heap_Array[i] = dept->students;
        dept = (Dept_t *)dept->node.next;
    }

    build_min_heap(Heap_Array, Heap_Size, cmp_student);
}

Student_t *sorted_student_next()
{
    if (Heap_Size == 0 || Heap_Array == NULL)
        return NULL;

    Student_t *min_student = Heap_Array[0];

    if (min_student != NULL && min_student->node.next)
    {
        Heap_Array[0] = (Student_t *)min_student->node.next;
    }
    else
    {
        Heap_Array[0] = Heap_Array[--Heap_Size];
    }

    min_heapify(Heap_Array, Heap_Size, 0, cmp_student);

    return min_student;
}

void sorted_student_free()
{
    if (Heap_Array != NULL)
    {
        free(Heap_Array);
    }
    Heap_Array = NULL;
    Heap_Size = 0;
}
