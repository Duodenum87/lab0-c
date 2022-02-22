#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (q == NULL)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL)
        return;
    element_t *ele, *tmp;
    list_for_each_entry_safe (ele, tmp, l, list) {
        free(ele->value);
        free(ele);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *newh = malloc(sizeof(element_t));
    if (newh == NULL)
        return false;
    int l = strlen(s);
    newh->value = malloc(sizeof(char) * (l + 1));
    // one more byte for '\0'
    if (newh->value == NULL) {
        free(newh);
        return false;
    }
    strncpy(newh->value, s, l);
    newh->value[l] = '\0';
    // set the string terminator
    list_add(&newh->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *newt = malloc(sizeof(element_t));
    if (newt == NULL)
        return false;
    int l = strlen(s);
    newt->value = malloc(sizeof(char) * (l + 1));
    /* one more byte for '\0' */
    if (newt->value == NULL) {
        free(newt);
        return false;
    }
    strncpy(newt->value, s, l);
    // set the string terminator
    newt->value[l] = '\0';
    list_add_tail(&newt->list, head);
    return true;
}
/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *fptr = list_first_entry(head, element_t, list);
    // ptr redirect
    list_del(head->next);
    // to copy string
    if (sp != NULL) {
        strncpy(sp, fptr->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return fptr;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *fptr = list_last_entry(head, element_t, list);
    // ptr redirect
    list_del(head->prev);
    // to copy string
    if (sp != NULL) {
        strncpy(sp, fptr->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return fptr;
}


/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (head == NULL || list_empty(head))
        return false;
    struct list_head *fast = head->next->next, *slow = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    element_t *ele = list_entry(slow, element_t, list);
    list_del(slow);
    q_release_element(ele);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (head == NULL || list_empty(head))
        return false;
    char *cmp = NULL;
    element_t *ele = NULL, *tmp = NULL;
    list_for_each_entry_safe (ele, tmp, head, list) {
        if (strcmp(cmp, ele->value) == 0 && cmp) {
            list_del(&ele->list);
            free(ele->value);
            free(ele);
        } else {
            cmp = ele->value;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (head == NULL || list_empty(head))
        return;
    struct list_head *i;
    list_for_each (i, head) {
        // to identify odd list count
        if (i->next == head)
            break;
        list_move(i, i->next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return;
    struct list_head *i;
    for (i = head; i->next != head->prev; i = i->next) {
        list_move(head->prev, i);
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
