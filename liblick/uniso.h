/**
 * @file
 * @brief Extract relevant files from an ISO file
 */

#pragma once

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
    /// the relative path to the initrd file
    char *initrd;
    /// the relative path to the kernel
    char *kernel;
} uniso_status_t;

/**
 * @brief extract an ISO file
 * @param src the path of the ISO file to extract
 * @param dst the path to extract the ISO file to
 * @return information about the extraction
 */
uniso_status_t *uniso(const char *src, const char *dst);
/**
 * @brief free the memory used by an uniso_status_t
 * @param s the status to free
 */
void uniso_status_free(uniso_status_t *s);
