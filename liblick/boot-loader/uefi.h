/**
 * @file
 * @brief Install LICK from Windows Vista to 10, on EFI systems
 */

#pragma once

#include "../boot-loader.h"

/**
 * @brief get the EFI boot loader plugin
 */
loader_t get_efi();
