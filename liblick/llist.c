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
