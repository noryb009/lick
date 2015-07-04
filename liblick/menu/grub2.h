/**
 * @file
 * @brief Manage grub2 menus
 */

#pragma once

#include "../menu.h"

/**
 * @brief mounts the UEFI partition on a free drive
 * @return the letter to the drive where the UEFI partition is mounted, or
 *   '\0' if it could not be mounted
 */
char mount_uefi_partition();
/**
 * @brief unmounts the UEFI partition
 * @param drive the drive letter where the partition is mounted
 * @return 1 if the partition is unmounted
 */
int unmount_uefi_partition(char drive);
/**
 * @brief get the grub2 menu plugin
 * @return a menu_t. Must be freed using free
 */
menu_t *get_grub2();
