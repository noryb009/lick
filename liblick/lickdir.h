/**
 * @file
 * @brief Information about a LICK directory
 *
 * LICK directories should be found on the root of a drive. Inside,
 * there are a few subdirectories - to allow flexibility with the names of
 * these folders, they are stored in a lickdir_t structure.
 */

#pragma once

/**
 * @brief Paths to the LICK directory and its components
 */
typedef struct {
    /// path to the lick directory
    char *lick;
    /// path to the drive the lick directory is on
    char *drive;
    /// path to the directory to store information about installed distributions
    char *entry;
    /// path to the directory to store menu files
    char *menu;
    /// path to the directory to store LICK's resource files
    char *res;
    /// error message
    char *err;
} lickdir_t;

/**
 * @brief create a new lickdir_t, with the given attributes
 *
 * Each parameter must be allocated on the heap. The parameters are no longer
 * valid after the call, and must not be freed
 *
 * @param lick the root lick directory
 * @param entry the entry subdirectory
 * @param menu the subdirectory for menu fragments
 * @param res the resource subdirectory
 * @return a new lickdir_t. Must be freed with #free_lickdir
 */
lickdir_t *new_lickdir(char *lick, char *entry, char *menu,
        char *res);
/**
 * @brief free the memory a lickdir_t is using
 * @param l the lickdir_t to free
 */
void free_lickdir(lickdir_t *l);
/**
 * @brief
 *   given the path to a LICK directory, return a lickdir_t structure with
 *   default values for subdirectories
 * @param d path to the LICK directory
 * @return a lickdir_t with default subdirectory names
 */
lickdir_t *expand_lickdir(char *d);
/**
 * @brief returns the LICK directory, if the program running is in
 *   the root of the directory
 *
 * Looks at the path to the binary currently being run, and determines if a
 * res directory exists. If so, returns a lickdir_t corresponding to the
 * directory. If not, returns NULL. This is to prevent later errors while
 * installing the loader, and to prevent users from running an interface
 * alone, for example directly from a zip file
 *
 * @return a lickdir_t filled with needed info. If the current directory is
 *   an invalid LICK directory, returns NULL.
 */
lickdir_t *get_lickdir();
