/**
 * @file
 * @brief A linked list cast to have values of a certain type
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Macro to combine names.
#define CREATE_NAME(a, b) a ## _ ## b

// Create function names.
#define LIST_NEW_FN(LIST_NAME) CREATE_NAME(new, LIST_NAME)
#define LIST_LENGTH_FN(LIST_NAME) CREATE_NAME(LIST_NAME, length)
#define LIST_REVERSE_FN(LIST_NAME) CREATE_NAME(LIST_NAME, reverse)
#define LIST_SORT_FN(LIST_NAME) CREATE_NAME(LIST_NAME, sort)
#define LIST_FREE_FN(LIST_NAME) CREATE_NAME(free, LIST_NAME)

#define DEFINE_LIST(LIST_NAME, LIST_TYPE, FREE_LIST_FN) \
    typedef struct LIST_NAME LIST_NAME; \
    \
    /* The same struct as llist, but with types. \
     * While these could technically be different, we assume the compiler doesn't \
     * swap the order of the pointers, or do anything else that would make them \
     * non-binary compatible.*/ \
    struct LIST_NAME { \
        /** the current element value */ \
        LIST_TYPE *val; \
        /** the next node in the linked list, or NULL if there are no more elements */ \
        LIST_NAME *next; \
    }; \
    \
    LIST_NAME *LIST_NEW_FN(LIST_NAME)(LIST_TYPE *val, LIST_NAME *next); \
    \
    size_t LIST_LENGTH_FN(LIST_NAME)(LIST_NAME *lst); \
    \
    LIST_NAME *LIST_REVERSE_FN(LIST_NAME)(LIST_NAME *lst); \
    \
    LIST_NAME *LIST_SORT_FN(LIST_NAME)(LIST_NAME *lst, \
        int (*compare)(const LIST_TYPE **a, const LIST_TYPE **b)); \
    \
    void LIST_FREE_FN(LIST_NAME)(LIST_NAME *lst);

#ifdef __cplusplus
}
#endif
