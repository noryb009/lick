/**
 * @file
 * @brief a plugin system to manage boot loader menus
 */

#pragma once

#include "lickdir.h"
#include "uniso.h"

/**
 * @brief Hooks to call boot loader menu plugins
 */
typedef struct {
    /// install hook
    int (*install)(lickdir_t *lick);
    /// regenerate menu
    int (*regenerate)(lickdir_t *lick);
    /// uninstall hook
    int (*uninstall)(lickdir_t *lick);
} menu_t;

/**
 * @brief create a menu fragment file
 * @param dst the path to create the file at
 * @param name the human-friendly name of the distribution
 * @param info the extraction status information
 * @param subdir the installation directory
 * @return 1 on success, 0 on error
 */
int write_menu_frag(char *dst, char *name, uniso_status_t *info,
        char *subdir);
