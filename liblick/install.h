/**
 * @file
 * @brief Install and uninstall distributions
 */

#pragma once

#include "lickdir.h"
#include "llist.h"
#include "menu.h"
#include "uniso.h"

/**
 * @brief information about an installed distribution
 */
typedef struct {
    /// the distribution id
    char *id;
    /// the distribution human-friendly name
    char *name;
} installed_t;

/**
 * @brief returns a list of paths to conf files in a directory
 * @param path the directory to look in
 * @return a list of strings of absolute paths to conf files
 */
node_t *get_conf_files(const char *path);
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
 * @param distro the Linux distribution the ISO is
 * @param iso the ISO file to extract from
 * @param lick the LICK directory
 * @param install_dir the directory to install to
 * @param menu the menu plugin
 * @return 1 on success, 0 on error
 */
int install(const char *id, const char *name, distro_t *distro,
        const char *iso, const char *install_dir, lickdir_t *lick,
        menu_t *menu);

/**
 * @brief install a distribution
 *
 * see install for more information on parameters
 *
 * @param cb a function to call for extraction progress
 * @param cb_data extra data to call the callback with
 * @return 1 on success, 0 on error
 */
int install_cb(const char *id, const char *name, distro_t *distro,
        const char *iso, const char *install_dir, lickdir_t *lick,
        menu_t *menu, uniso_progress_cb cb, void *cb_data);

/**
 * @brief uninstall a distribution
 * @param id the id
 * @param lick the LICK directory
 * @param menu the menu plugin
 * @return 1 on success, 0 on error
 */
int uninstall(const char *id, lickdir_t *lick, menu_t *menu);
