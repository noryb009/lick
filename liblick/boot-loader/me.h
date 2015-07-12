/**
 * @file
 * @brief Install LICK to Windows ME
 */

#pragma once

#include "../boot-loader.h"

/**
 * @brief check if the Windows ME boot loader patch is installed
 * @return 1 if it is installed, otherwise 0
 */
int check_loader_me_patch();
/**
 * @brief get the ME boot loader plugin
 */
loader_t get_me();
