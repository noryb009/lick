/**
 * @file
 * @brief An assortment of functions used to manipulate files and directories.
 */

#pragma once

#include <stdio.h>

/**
 * @brief read a line of any size from a file
 * @param f the file to read from
 * @return the line. Free with free().
 */
char *read_line(FILE *f);
/**
 * @brief read a file into a string
 * @param f the file to read
 * @return the contents of the file, in a string
 */
char *file_to_str(FILE *f);
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
/**
 * @brief determine if a file is a configuration file, from the file name
 * @param name the file name
 * @return 1 if a configuration file, otherwise 0
 */
int is_conf_path(const char *name);
