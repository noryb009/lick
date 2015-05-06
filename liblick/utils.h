/**
 * @file
 * @brief An assortment of functions used in multiple sections.
 *
 * Utility functions which are used in multiple sections of the program.
 */

#pragma once

#include <stdio.h>

/**
 * @brief create a directory
 * @param d the directory to create
 * @return true on success
 */
int makeDir(const char *d);
/**
 * @brief creating a directory and its parents as necessary
 * @param d the directory to create
 * @return true on success
 */
int makeDirR(const char *d);
/**
 * @brief delete a directory
 * @param d the directory to delete
 * @return true on success
 */
int unlinkDir(const char *d);
/**
 * @brief delete a file
 * @param f the file to delete
 * @return true on success
 */
int unlinkFile(const char *f);
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
 * @param path the file to check
 * @return
 * 1 if it is a file, 0 if it is not a file, and -1 if the file does not exist
 */
int is_file(char *path);
/**
 * @brief determine if a file exists
 * @param path the file to check
 * @return 1 if the file exists, otherwise 0
 */
int file_exists(char *path);
/**
 * @brief read a line of any size from a file
 * @param f the file to read from
 * @param done the integer pointed to is set to 1 if finished reading the file
 * @return the line. Free with free().
 */
char *read_line(FILE *f, int *done);
/**
 * @brief determine if a file is a configuration file, from the file name
 * @param name the file name
 * @return 1 if a configuration file, otherwise 0
 */
int is_conf_file(const char *name);
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
