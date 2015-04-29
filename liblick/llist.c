#include <stdlib.h>

#include "llist.h"

node_t *new_node(void *val, node_t *next) {
    node_t *n = malloc(sizeof(node_t));
    n->val = val;
    n->next = next;
    return n;
}

size_t list_length(node_t *lst) {
    size_t len = 0;
    while(lst != NULL) {
        len++;
        lst = lst->next;
    }
    return len;
}

void free_list(node_t *n, void (*free_fn)(void *n)) {
    node_t *next;
    while(n != NULL) {
        next = n->next;
        if(free_fn != NULL)
            free_fn(n->val);
        free(n);
        n = next;
    }
}
