/**
 * @file
 * @brief A linked list library
 */

#pragma once

typedef struct node_t node_t;

struct node_t {
    void *val;
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
 * @brief filter a list into two separate lists using a predicate
 *
 * Given a predicate, filters the list into two separate lists: one of elements
 * which satisfy the given predicate, and one of elements which do not.
 *
 * @param check
 *   the predicate to determine which list a given node should be added to
 * @param in[in] the list to filter. The given pointer is no longer valid
 * @param out_true[out]
 *   the list of elements in the list which satisfy the predicate,
 *   in reverse order
 * @param out_true[out]
 *   the list of elements in the list which do not satisfy the predicate,
 *   in reverse order
 */
void double_filter_list(int (*check)(void *arg),
        node_t *in, node_t **out_true, node_t **out_false);
/**
 * @brief free a list, freeing all elements along the way
 * @param n the list to free
 * @param free_fn
 *   a function to free all the elements of the list. For example, if the
 *   linked list contains strings allocated using malloc(), free_fn would
 *   be free()
 */
void free_list(node_t *n, void (*free_fn)(void *n));
