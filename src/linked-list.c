#include <stdio.h>
#include <stdlib.h>

#include "linked-list.h"

void delete_node(ListNode_t **head, ListNode_t *node, void (*free_data)(ListNode_t *))
{
    if (node == NULL || head == NULL || *head == NULL)
    {
        return;
    }

    if (*head == node)
    {
        *head = node->next;

        if (*head != NULL)
        {
            (*head)->prev = NULL;
        }
    }
    else
    {
        if (node->prev != NULL)
        {
            node->prev->next = node->next;
            node->prev = NULL;
        }

        if (node->next != NULL)
        {
            node->next->prev = node->prev;
            node->next = NULL;
        }
    }

    if (free_data != NULL)
    {
        free_data(node);
    }

    return;
}

ListNode_t *search_sorted(uint32_t search_item, int32_t (*match_func)(ListNode_t *, uint32_t),
                          ListNode_t *head)
{
    int32_t result = 0;
    if (match_func == NULL)
    {
        return NULL;
    }
    while (head != NULL)
    {
        result = match_func(head, search_item);
        if (result > 0)
        {
            break;
        }
        else if (result == 0)
        {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

void insert_sorted(ListNode_t **head, ListNode_t *new_node,
                   int32_t (*cmp_func)(ListNode_t *, ListNode_t *))
{
    ListNode_t *current = NULL;
    ListNode_t *last = NULL;

    if (head == NULL || new_node == NULL || cmp_func == NULL)
    {
        return;
    }

    if (*head == NULL)
    {
        /* Insert into empty list */
        *head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        return;
    }

    /* Find the correct insertion point */
    current = *head;
    while (current != NULL && cmp_func(current, new_node) < 0)
    {
        last = current;
        current = current->next;
    }

    if (current == *head)
    {
        /* Insert at the beginning */
        new_node->next = *head;
        new_node->prev = NULL;
        if (*head != NULL)
        {
            (*head)->prev = new_node;
        }
        *head = new_node;

        return;
    }

    if (current == NULL)
    {
        last->next = new_node;
        new_node->prev = last;
        new_node->next = NULL;

        return;
    }

    /* Insert in the middle */
    new_node->next = current;
    new_node->prev = current->prev;
    if (current->prev != NULL)
    {
        current->prev->next = new_node;
    }
    else
    {
        *head = new_node;
    }
    current->prev = new_node;

    return;
}

void merge_sorted(ListNode_t **head1, ListNode_t *head2,
                  int32_t (*cmp_func)(ListNode_t *, ListNode_t *))
{
    ListNode_t *dummy = NULL;
    ListNode_t *tail = NULL;
    ListNode_t *list1 = NULL;
    ListNode_t *list2 = NULL;

    if (head1 == NULL || head2 == NULL || cmp_func == NULL)
    {
        return;
    }

    dummy = (ListNode_t *)malloc(sizeof(ListNode_t));
    if (dummy == NULL)
    {
        return;
    }
    dummy->next = NULL;
    dummy->prev = NULL;

    tail = dummy;
    list1 = *head1;
    list2 = head2;

    /* Merge the two lists */
    while (list1 != NULL && list2 != NULL)
    {
        if (cmp_func(list1, list2) <= 0)
        {
            tail->next = list1;
            list1->prev = tail;
            list1 = list1->next;
        }
        else
        {
            tail->next = list2;
            list2->prev = tail;
            list2 = list2->next;
        }
        tail = tail->next;
    }

    /* Attach any remaining nodes */
    if (list1 != NULL)
    {
        tail->next = list1;
        list1->prev = tail;
    }

    if (list2 != NULL)
    {
        tail->next = list2;
        list2->prev = tail;
    }

    /* Update the head1 to point to the merged list */
    *head1 = dummy->next;
    if (*head1 != NULL)
    {
        (*head1)->prev = NULL;
    }

    free(dummy);

    return;
}
