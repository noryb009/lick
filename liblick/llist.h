/**
 * @file
 * @brief A linked list library
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef struct node_t node_t;
typedef void (*free_list_item_f)(void *);
typedef int (*compare_list_item_f)(const void *a, const void *b);

/**
 * @brief A linked list node
 */
struct node_t {
    /// the current element value
    void *val;
    /// the next node in the linked list, or NULL if there are no more elements
    node_t *next;
};

/**
 * @brief create a new node, and add it to the start of a list
 * @param val the value of the node
 * @param next the list to add it to
 * @return the new linked list
 */
node_t *new_node(void *val, node_t *next);
/**
 * @brief find the length of a linked list
 * @param lst the list to count
 * @return the length of the list
 */
size_t list_length(node_t *lst);
/**
 * @brief reverse a list
 * @param lst
 *   the list to reverse. This pointer passed is not valid after the function
 * @return the reversed list
 */
node_t *list_reverse(node_t *lst);
/**
 * @brief sort a list
 * @param lst
 *   the list to sort. This pointer passed is not valid after the function
 * @param compare the comparison function
 * @return the sorted list
 */
node_t *list_sort(node_t *lst, compare_list_item_f compare);
/**
 * @brief free a list, freeing all elements along the way
 * @param n the list to free
 * @param free_fn
 *   a function to free all the elements of the list. For example, if the
 *   linked list contains strings allocated using malloc(), free_fn would
 *   be free(). Can be NULL to not free the elements.
 */
void free_list(node_t *n, free_list_item_f free_fn);

// A list of strings.
#define LIST_NAME string_node_t
#define LIST_TYPE char
#define FREE_LIST_FN free
#include "llist-type.h"

#ifdef __cplusplus
}
#endif
