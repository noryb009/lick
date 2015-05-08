#include <stdlib.h>

#include "llist.h"

node_t *new_node(void *val, node_t *next) {
    node_t *n = malloc(sizeof(node_t));
    n->val = val;
    n->next = next;
    return n;
}

int list_length(node_t *lst) {
    int len = 0;
    while(lst != NULL) {
        len++;
        lst = lst->next;
    }
    return len;
}

node_t *list_reverse(node_t *lst) {
    node_t *next;
    node_t *prev = NULL;
    while(lst != NULL) {
        next = lst->next;
        lst->next = prev;
        prev = lst;
        lst = next;
    }

    return prev;
}

void double_filter_list(int (*check)(void *arg),
        node_t *in, node_t **out_true, node_t **out_false) {
    *out_true = NULL;
    *out_false = NULL;

    while(in != NULL) {
        node_t *next = in->next;
        if(check(in->val)) {
            in->next = (*out_true);
            (*out_true) = in;
        } else {
            in->next = (*out_false);
            (*out_false) = in;
        }
        in = next;
    }
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
