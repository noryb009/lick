/**
 * @file
 * @brief A linked list cast to have values of a certain type
 */

// Note: this file does not have pragma once.

#ifndef LIST_NAME
#   error "LIST_NAME required when including llist_type.h."
#endif
#ifndef LIST_TYPE
#   error "LIST_TYPE required when including llist_type.h."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include "llist.h"

// Macros to combine names.
#define CREATE_NAME2(a, b) a ## _ ## b
#define CREATE_NAME(a, b) CREATE_NAME2(a, b)
#define CREATE_NAME_PREFIX(fn) CREATE_NAME(LIST_NAME, fn)
#define CREATE_NAME_SUFFIX(fn) CREATE_NAME(fn, LIST_NAME)

// Create function names.
#define LIST_NEW_FN CREATE_NAME_SUFFIX(new)
#define LIST_LENGTH_FN CREATE_NAME_PREFIX(length)
#define LIST_REVERSE_FN CREATE_NAME_PREFIX(reverse)
#define LIST_SORT_FN CREATE_NAME_PREFIX(sort)
#define LIST_FREE_FN CREATE_NAME_SUFFIX(free)

typedef struct LIST_NAME LIST_NAME;

// The same struct as llist, but with types.
// While these could technically be different, we assume the compiler doesn't
// swap the order of the pointers, or do anything else that would make them
// non-binary compatible.
struct LIST_NAME {
    /// the current element value
    LIST_TYPE *val;
    /// the next node in the linked list, or NULL if there are no more elements
    LIST_NAME *next;
};

LIST_NAME *LIST_NEW_FN(LIST_TYPE *val, LIST_NAME *next);

size_t LIST_LENGTH_FN(LIST_NAME *lst);

LIST_NAME *LIST_REVERSE_FN(LIST_NAME *lst);

LIST_NAME *LIST_SORT_FN(LIST_NAME *lst,
    int (*compare)(const LIST_TYPE **a, const LIST_TYPE **b));

void LIST_FREE_FN(LIST_NAME *lst);

// Clean up the function names.
#ifndef NO_UNDEF
#undef LIST_NEW_FN
#undef LIST_LENGTH_FN
#undef LIST_REVERSE_FN
#undef LIST_SORT_FN
#undef LIST_FREE_FN
#endif

// Remove the parameters.
#undef LIST_NAME
#undef LIST_TYPE

#ifdef __cplusplus
}
#endif
