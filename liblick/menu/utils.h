/**
 * @file
 * @brief Functions used in multiple menus
 */

#pragma once

#include <stdio.h>

#include "../llist.h"
#include "../lickdir.h"

#define INDENT "    "

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
char *to_menu_path(const char *path);
/**
 * @brief write entries to a file
 * @param lick the lick directory
 * @param menu the file to write to
 * @param write a function that writes a given entry
 */
void write_menu(lickdir_t *lick, FILE *menu,
        void (*write)(FILE *, entry_t *));
