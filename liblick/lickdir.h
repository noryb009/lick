/**
 * @file
 * @brief Information about a LICK directory
 *
 * LICK directories should be found on the root of a drive. Inside,
 * there are a few subdirectories - to allow flexibility with the names of
 * these folders, they are stored in a lickdir_t structure.
 */

#pragma once

// TODO: error message? info?
/**
 * @brief Paths to the LICK directory and its components
 */
typedef struct {
    /// path to the lick directory
    char *lick;
    /// path to the directory to store information about installed distributions
    char *entry;
    /// path to the directory to store menu files
    char *menu;
    /// path to the directory to store LICK's resource files
    char *res;
} lickdir_t;

/**
 * @brief
 *   given the path to a LICK directory, return a lickdir_t structure with
 *   default values for subdirectories
 * @param d path to the LICK directory
 * @return a lickdir_t with default subdirectory names
 */
lickdir_t *expand_lick_dir(char *d);
// TODO: new_lick_dir
// TODO: free_lick_dir
