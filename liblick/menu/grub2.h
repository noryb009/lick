/**
 * @file
 * @brief Manage grub2 menus
 */

#pragma once

#include "../menu.h"

// Either PreLoader or shim can be used.
#define USE_PRELOADER

#ifdef USE_PRELOADER
#  define GRUB2_EFI "loader.efi"
#  define SHIM_EFI "PreLoader.efi"
#  define MOKMANAGER_EFI "HashTool.efi"
#else
#  define GRUB2_EFI "grubx64.efi"
#  define SHIM_EFI "shim.efi"
#  define MOKMANAGER_EFI "MokManager.efi"
#endif

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
