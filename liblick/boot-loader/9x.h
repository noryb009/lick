/**
 * @file
 * @brief Install LICK to Windows 95 and 98
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "../boot-loader.h"

/**
 * @brief for testing
 */
char *install_to_config_sys(char *config, lickdir_t *lick);
/**
 * @brief for testing
 */
char *uninstall_from_config_sys(char *config, lickdir_t *lick);
/**
 * @brief get the 9x boot loader plugin
 */
loader_t get_9x();

#ifdef __cplusplus
}
#endif
