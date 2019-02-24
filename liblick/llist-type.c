#include "llist.h"

#include <assert.h>

#undef LIST_NAME
#undef LIST_TYPE
#undef FREE_LIST

#define DEFINE_LIST_DEFN(LIST_NAME, LIST_TYPE, FREE_LIST_FN) \
    LIST_NAME *LIST_NEW_FN(LIST_NAME)(LIST_TYPE *val, LIST_NAME *next) { \
        static_assert(sizeof(node_t) == sizeof(LIST_NAME), "List is not the same size as list type."); \
        return (LIST_NAME *)new_node((void *)val, (node_t *)next); \
    } \
    \
    size_t LIST_LENGTH_FN(LIST_NAME)(LIST_NAME *lst) { \
        return list_length((node_t *)lst); \
    } \
    \
    LIST_NAME *LIST_REVERSE_FN(LIST_NAME)(LIST_NAME *lst) { \
        return (LIST_NAME *)list_reverse((node_t *)lst); \
    } \
    \
    LIST_NAME *LIST_SORT_FN(LIST_NAME)(LIST_NAME *lst, \
                                   int (*compare)(const LIST_TYPE **a, const LIST_TYPE **b)) { \
        return (LIST_NAME *)list_sort((node_t *)lst, (compare_list_item_f)compare); \
    } \
    \
    void FREE_LIST_FN(void *); \
    \
    void LIST_FREE_FN(LIST_NAME)(LIST_NAME *lst) { \
        free_list((node_t *)lst, (free_list_item_f)FREE_LIST_FN); \
    }

DEFINE_LIST_DEFN(string_node_t, char, free)
DEFINE_LIST_DEFN(drive_node_t, struct drive_t, free_drive)
DEFINE_LIST_DEFN(installed_node_t, struct installed_t, free_installed)
DEFINE_LIST_DEFN(distro_info_node_t, struct distro_info_t, free_distro_info)
DEFINE_LIST_DEFN(distro_node_t, struct distro_t, free_distro)
DEFINE_LIST_DEFN(section_node_t, struct section_t, free_section)
