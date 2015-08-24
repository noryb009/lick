/**
 * @file
 * @brief Extract relevant files from an ISO file
 */

#pragma once

#include "distro.h"
#include "llist.h"

/**
 * @brief Info about the extraction of an ISO
 */
typedef struct {
    /// 1 if the extraction finished, otherwise 0
    int finished;
    /// a list of relative paths to files extracted
    node_t *files;
    /// an error message, if applicable
    char *error;
} uniso_status_t;

typedef size_t uniso_progress_t;
typedef void (*uniso_progress_cb)(uniso_progress_t cur,
        uniso_progress_t total, void *cb_data);

/**
 * @brief extract an ISO file
 * @param src the path of the ISO file to extract
 * @param dst the path to extract the ISO file to
 * @param cb an optional callback to update the status
 * @param cb_data data to call the callback with
 * @return information about the extraction
 */
uniso_status_t *uniso(const char *src, const char *dst, distro_filter_f filter,
        uniso_progress_cb cb, void *cb_data);
/**
 * @brief free the memory used by an uniso_status_t
 * @param s the status to free
 */
void free_uniso_status(uniso_status_t *s);
