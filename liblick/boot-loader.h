/**
 * @file
 * @brief Add LICK to the current boot loader (or set up a new boot loader)
 */

#pragma once

#include "lickdir.h"
#include "menu.h"
#include "windows-info.h"

/**
 * @brief functions relevant to a boot loader plugin. Do not use directly, use
 *   the wrappers instead
 */
typedef struct {
    /// determines if the plugin supports the system
    int (*supported)(win_info_t *info);
    /// determines if the plugin is currently installed
    int (*check)(win_info_t *info);
    /// installs LICK, using the plugin
    int (*install)(win_info_t *info, lickdir_t *lick);
    /// uninstall LICK, using the plugin
    int (*uninstall)(win_info_t *info, lickdir_t *lick);
    /// get the relevant boot loader menu plugin
    menu_t (*get_menu)();
} loader_t;

/**
 * @brief get the boot loader plugin that should be used with the system
 * @param info a win_info_t, filled with info from the current system
 * @return the relevant plugin
 */
loader_t *get_loader(win_info_t *info);
/**
 * @brief determine if the boot loader is already installed
 * @param l a supported plugin
 * @param info a win_info_t, filled with info from the current system
 * @return the relevant plugin
 */
int check_loader(loader_t *l, win_info_t *info);
/**
 * @brief installs the loader
 * @param l a supported plugin
 * @param info a win_info_t, filled with info from the current system
 * @param lick the LICK directory to boot to
 * @return 1 on success, 0 on error, -1 if already installed
 */
int install_loader(loader_t *l, win_info_t *info,
        lickdir_t *lick);
/**
 * @brief uninstalls the loader
 * @param l a supported plugin
 * @param info a win_info_t, filled with info from the current system
 * @param lick the LICK directory
 * @return 1 on success, 0 on error, -1 if not installed
 */
int uninstall_loader(loader_t *l, win_info_t *info,
        lickdir_t *lick);
/**
 * @brief get the menu corresponding to the plugin
 * @param l a supported plugin
 * @return the corresponding menu plugin
 */
menu_t *get_menu(loader_t *l);
