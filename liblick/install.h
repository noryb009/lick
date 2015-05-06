/**
 * @file
 * @brief Install and uninstall distributions
 */

#pragma once

#include "lickdir.h"
#include "llist.h"
#include "menu.h"

/**
 * information about an installed distribution
 */
typedef struct {
    /// the distribution id
    char *id;
    /// the distribution human-friendly name
    char *name;
} installed_t;

/**
 * @brief returns a list of installed distributions
 * @param lick the LICK directory
 * @return
 *   a list of installed_t structures. Must be freed using free_list_installed
 */
node_t *list_installed(lickdir_t *lick);
/**
 * @brief free the memory a installed_t is using
 * @param i the installed_t to free
 */
void free_installed(installed_t *i);
/**
 * @brief free the memory a list of installed_t is using
 * @param n the list of installed_t to free
 */
void free_list_installed(node_t *n);

/**
 * @brief install a distribution
 * @param id
 *   the id, without spaces or quotes. Note the user may have multiple
 *   installations of the same distribution
 * @param name
 *   the human-friendly name. Note the user may have multiple installations of
 *   the same distribution
 * @param iso the ISO file to extract from
 * @param lick the LICK directory
 * @param install_dir the directory to install to
 * @param menu the menu plugin
 * @return 1 on success, 0 on error
 */
int install(char *id, char *name, char *iso,
        lickdir_t *lick, char *install_dir, menu_t *menu);

/**
 * @brief install a distribution
 * @param lick the LICK directory
 * @param id the id
 * @param menu the menu plugin
 * @return 1 on success, 0 on error
 */
int uninstall(lickdir_t *lick, char *id, menu_t *menu);
