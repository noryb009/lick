/**
 * @file
 * @brief Install LICK from Windows NT to XP
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "../boot-loader.h"

/**
 * @brief for testing
 */
char *install_to_boot_ini(char *boot, lickdir_t *lick);
/**
 * @brief for testing
 */
char *uninstall_from_boot_ini(char *boot, lickdir_t *lick);
/**
 * @brief get the NT boot loader plugin
 */
loader_t get_nt();

#ifdef __cplusplus
}
#endif
