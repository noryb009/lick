/**
 * @file
 * @brief Manage grub4dos menus
 */

#pragma once

#include "../menu.h"

/**
 * @brief get the grub4dos menu plugin
 * @return a menu_t. Must be freed using free
 */
menu_t *get_grub4dos();
