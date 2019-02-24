// Note: This file is repeatedly included by `llist-type.c`.

LIST_NAME *LIST_NEW_FN(LIST_TYPE *val, LIST_NAME *next) {
    static_assert(sizeof(node_t) == sizeof(LIST_NAME), "List is not the same size as list type.");
    return (LIST_NAME *)new_node((void *)val, (node_t *)next);
}

size_t LIST_LENGTH_FN(LIST_NAME *lst) {
    return list_length((node_t *)lst);
}

LIST_NAME *LIST_REVERSE_FN(LIST_NAME *lst) {
    return (LIST_NAME *)list_reverse((node_t *)lst);
}

LIST_NAME *LIST_SORT_FN(LIST_NAME *lst,
                               int (*compare)(const LIST_TYPE **a, const LIST_TYPE **b)) {
    return (LIST_NAME *)list_sort((node_t *)lst, (compare_list_item_f)compare);
}


void FREE_LIST_FN(void *);

void LIST_FREE_FN(LIST_NAME *lst) {
    free_list((node_t *)lst, (free_list_item_f)FREE_LIST_FN);
}

#undef LIST_NAME
#undef LIST_TYPE
#undef FREE_LIST_FN
