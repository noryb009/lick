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

void **list_to_array(node_t *lst, int *len) {
    *len = list_length(lst);
    void **arr = malloc(sizeof(void *) * *len);
    int i = 0;
    for(node_t *n = lst; n != NULL; n = n->next) {
        arr[i] = n->val;
        i++;
    }
    return arr;
}

node_t *array_to_list(void **arr, int len) {
    node_t *lst = NULL;
    for(int i = 0; i < len; ++i) {
        lst = new_node(arr[i], lst);
    }
    return lst;
}

node_t *list_sort(node_t *lst, int (*compare)(const void *a, const void *b)) {
    int len;
    void **arr = list_to_array(lst, &len);
    free_list(lst, do_nothing);

    qsort(arr, len, sizeof(void *), compare);

    lst = list_reverse(array_to_list(arr, len));
    free(arr);
    return lst;
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

void do_nothing(void *n) {}

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
