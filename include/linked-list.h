#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <stdint.h>

typedef struct ListNode
{
    struct ListNode *prev;
    struct ListNode *next;
} ListNode_t;

void delete_node(ListNode_t **head, ListNode_t *node, void (*free_data)(ListNode_t *));
ListNode_t *search_sorted(uint32_t search_item, int32_t (*match_func)(ListNode_t *, uint32_t), ListNode_t *head);
void insert_sorted(ListNode_t **head, ListNode_t *new_node, int32_t (*cmp_func)(ListNode_t *, ListNode_t *));
void merge_sorted(ListNode_t **head1, ListNode_t *head2, int32_t (*cmp_func)(ListNode_t *, ListNode_t *));

#endif /* __LINKED_LIST_H__ */