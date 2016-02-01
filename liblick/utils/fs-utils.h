/**
 * @file
 * @brief An assortment of functions used to manipulate paths and directories.
 */

#pragma once

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
 * @brief normalizes the path str to a Windows path
 * @param path the path to normalize
 * @return a reference to path
 */
char *win_path(char *path);
/**
 * @brief normalizes the path str to a Unix path
 * @param path the path to normalize
 * @return a reference to path
 */
char *unix_path(char *path);
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
 * @brief delete a directory if it is empty, and repeat with each parent directory
 * @param d the directory to delete
 * @return true if at least one directory was deleted
 */
int unlink_dir_parents(const char *d);
/**
 * @brief delete a file
 * @param f the file to delete
 * @return true on success
 */
int unlink_file(const char *f);
/**
 * @brief delete a directory and its contents
 * @param d the directory to delete
 * @return true on success
 */
int unlink_recursive(const char *d);
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
