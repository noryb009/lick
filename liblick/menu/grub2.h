/**
 * @file
 * @brief Manage grub2 menus
 */

#pragma once

#include "../menu.h"

/**
 * @brief get the grub2 menu plugin
 * @return a menu_t. Must be freed using free
 */
menu_t *get_grub2();
