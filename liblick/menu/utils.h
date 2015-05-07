/**
 * @file
 * @brief Functions used in multiple menus
 */

#pragma once

#include <stdio.h>

#include "../llist.h"

/**
 * @brief An entry in a LICK menu file
 */
typedef struct {
    /// human-friendly title of the menu item
    char *title;
    /// path to the initrd file
    char *initrd;
    /// path to the kernel
    char *kernel;
    /// options to be passed to the kernel
    char *options;
    /// copy text directly to the menu
    char *static_text;
} entry_t;

/**
 * @brief return a list of configuration files in a directory
 * @param dir the directory to search in
 * @return
 *   a list of configuration files in the directory, in alphabetical order
 */
node_t *get_conf_files(char *dir);
/**
 * @brief read a file to fill an entry
 * @param f the file to read from
 * @return an entry_t, filled with one entry; or NULL if there are no more entries
 */
entry_t *get_entry(FILE *f);
/**
 * @brief free the memory used by an entry
 * @param e the entry to free
 */
void free_entry(entry_t *e);
/**
 * @brief convert a path into a menu-friendly path
 * @param path the path to convert
 * @return a string that suitable be used in a menu. Free using free
 */
char *to_menu_path(char *path);
