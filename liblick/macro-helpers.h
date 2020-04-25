/**
 * @file
 * @brief Macros that help make coding easier. This should not be included in
 *    public headers.
 */

#pragma once

#include <stdlib.h>

#include "trace.h"
#include "utils/string-utils.h"

/// Return the 32nd argument.
#define _LICK_NUM_ARGS(extra,                                                  \
                        a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9, a10,       \
                        a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,      \
                        a21, a22, a23, a24, a25, a26, a27, a28, a29, a30,      \
                        a31, a32, ...)                                         \
                        a32

/// Pass the given arguments, plus a bunch of numbers to `_LICK_NUM_ARGS`. If
/// there is (eg.) 0 arguments, then we will pass 0 arguments + 32 arguments.
/// The 32nd argument is 0, so that will be returned, which is what we want.
/// If there is 1 argument, then all numbers will be bumped by 1, so we will
/// instead return 1, which is what we want.
/// Note: This only works for up to 32 arguments.
#define LICK_NUM_ARGS(...)                                                     \
    _LICK_NUM_ARGS(                                                            \
        "extra",                                                               \
        ## __VA_ARGS__,                                                        \
        31, 30,                                                                \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20,                                \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10,                                \
         9,  8,  7,  6,  5,  4,  3,  2,  1,  0                                 \
    )

#define LICK_CONCAT_STRS(...)                                                  \
    concat_strs(LICK_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

#define LICK_TRACE(LICK, LINE)                                                 \
    do {                                                                       \
        if (LICK->trace != NULL) {                                             \
            trace_str(LICK->trace, LINE);                                      \
        }                                                                      \
    } while (0)

#define LICK_TRACE2(LICK, ...)                                                 \
    do {                                                                       \
        if (LICK->trace != NULL) {                                             \
            char *TRACE_LINE = LICK_CONCAT_STRS(__VA_ARGS__);                  \
            trace_str(LICK->trace, TRACE_LINE);                                \
            free(TRACE_LINE);                                                  \
        }                                                                      \
    } while (0)

#define LICK_ERROR(LICK, ...)                                                  \
    do {                                                                       \
        if (!LICK->err) {                                                      \
            lick->err = LICK_CONCAT_STRS(__VA_ARGS__);                         \
            if (lick->trace != NULL) {                                         \
                trace_str(LICK->trace, lick->err);                             \
            }                                                                  \
        } else if (lick->trace != NULL) {                                      \
            char *TRACE_LINE = LICK_CONCAT_STRS("Extra error: ", __VA_ARGS__); \
            trace_str(LICK->trace, TRACE_LINE);                                \
            free(TRACE_LINE);                                                  \
        }                                                                      \
    } while (0)
