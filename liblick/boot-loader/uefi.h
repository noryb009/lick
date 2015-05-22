/**
 * @file
 * @brief Install LICK from Windows Vista to 10, on UEFI systems
 */

#pragma once

#include "../boot-loader.h"

/**
 * @brief get the UEFI boot loader plugin
 */
loader_t get_uefi();
