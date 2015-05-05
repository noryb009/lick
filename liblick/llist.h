#pragma once

struct node_s {
    void *val;
    struct node_s *next;
};
typedef struct node_s node_t;

node_t *new_node(void *val, node_t *next);
size_t list_length(node_t *lst);
void double_filter_list(int (*check)(void *arg), node_t *in,
        node_t **out_true, node_t **out_false);
void free_list(node_t *n, void (*free_fn)(void *n));
