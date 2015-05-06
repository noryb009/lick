/**
 * @file
 * @brief An implementation of scandir, since MinGW does not include one
 */

#pragma once

#include <dirent.h>

int alphasort2(const struct dirent **a, const struct dirent **b);
/**
 * @brief scan a directory, returning relevant files
 * @param path[in] the directory to search
 * @param e[out] an array of dirents returned
 * @param filter[in] a predicate to determine if a dirent should be returned
 * @param compare[in] a function to determine the order of the return values
 * @return
 *   the number of files returned if successful, or a negative number on error
 */
int scandir2(const char *path, struct dirent ***e,
        int (*filter)(const struct dirent *),
        int (*compare)(const struct dirent **, const struct dirent **));
