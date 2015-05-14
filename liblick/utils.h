/**
 * @file
 * @brief An assortment of functions used in multiple sections.
 *
 * Utility functions which are used in multiple sections of the program.
 */

#pragma once

#include <stdio.h>

/**
 * The type of a file
 */
typedef enum {
    /// other
    FILE_TYPE_OTHER,
    /// a file
    FILE_TYPE_FILE,
    /// a directory
    FILE_TYPE_DIR,
} file_type_e;

/**
 * @brief create a directory
 * @param d the directory to create
 * @return true on success
 */
int make_dir(const char *d);
/**
 * @brief creating a directory and its parents as necessary
 * @param d the directory to create
 * @return true on success
 */
int make_dir_parents(const char *d);
/**
 * @brief copy a file
 * @param dst the absolute path of a new file
 * @param src the path of a file to make a copy of
 * @return true on success
 */
int copy_file(const char *dst, const char *src);
/**
 * @brief delete a directory
 * @param d the directory to delete
 * @return true on success
 */
int unlink_dir(const char *d);
/**
 * @brief delete a file
 * @param f the file to delete
 * @return true on success
 */
int unlink_file(const char *f);
/**
 * @brief create a copy of a string, on the heap
 * @param s the string to copy
 * @return a copy of s. Free using free()
 */
char *strdup(const char *s);
/**
 * @brief concatenate strings together
 * @param n the number of strings
 * @param ... the strings to concatenate, in order
 * @return the resulting string. Free using free()
 */
char *concat_strs(int n, ...);
/**
 * @brief determine the type of a file
 * @param path to the file or directory to check
 * @return a #file_type_e corresponding to the type of file
 */
file_type_e file_type(const char *path);
/**
 * @brief determine if a file or directory exists
 * @param path the file to check
 * @return 1 if the file exists, otherwise 0
 */
int path_exists(const char *path);
/**
 * @brief read a line of any size from a file
 * @param f the file to read from
 * @return the line. Free with free().
 */
char *read_line(FILE *f);
/**
 * @brief determine if a file is a configuration file, from the file name
 * @param name the file name
 * @return 1 if a configuration file, otherwise 0
 */
int is_conf_path(const char *name);
/**
 * @brief extract info from a line of a configuration file
 *
 * Given a line of a configuration file, such as "key value", separates
 * the key from the value, and returns strings of each. Note ln is no longer
 * a valid string. If ln is on the heap, it must be freed in an appropriate
 * manner, which frees the strings pointed to by keyword_start and item_start. ln must
 * be freed if it is on the heap, regardless of the output of this function.
 *
 * @param[in] ln a line of a configuration file
 * @param[out] keyword_start
 *   where to place the keyword string, or null if no items found
 * @param[out] item_start
 *   where to place the item string, or null if no item found
 */
void conf_option(char *ln, char **keyword_start, char **item_start);
