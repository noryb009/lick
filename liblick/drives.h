/**
 * @file
 * @brief Find drives in the system
 */

#pragma once

#include "llist.h"

/**
 * the types of drives
 */
typedef enum {
    /// other
    DRV_UNKNOWN,
    /// an unused drive letter
    DRV_UNUSED,
    /// an internal hard drive
    DRV_HDD,
    /// an external hard drive, USB, etc.
    DRV_REMOVABLE,
    /// a network drive
    DRV_REMOTE,
    /// a CD drive
    DRV_CDROM,
} drive_type_e;

/**
 * @brief info about a drive
 */
// TODO: compression
typedef struct {
    /// the path to the root of a drive
    char *path;
    /// the type of drive
    drive_type_e type;
} drive_t;

/**
 * @brief determines the type of a drive
 * @param path a drive in the form "A:/"
 * @return the type of drive
 */
drive_type_e drive_type(char *path);
/**
 * @brief return a list of all drives in the system
 * @return a linked list of all drives. Must be freed using #free_drive_list
 */
node_t *all_drives();
/**
 * @brief return a list of all drive paths not currently used
 * @return a linked list of all unused drives. Must be freed using #free_drive_list
 */
node_t *unused_drives();
/**
 * @brief free a drive structure
 * @param drive the drive structure to free
 */
void free_drive(drive_t *drive);
/**
 * @brief free a list of drive structures
 * @param lst the list of drive structures to free
 */
void free_drive_list(node_t *lst);
/**
 * @brief determines if a drive has LICK installed on it
 *
 * LICK can be installed on multiple drives
 *
 * @param drive the drive to check
 * @return 1 if the drive has LICK installed, otherwise 0
 */
int is_lick_drive(drive_t *drive);
/**
 * @brief get the path of the Windows folder
 * @return
 *   the path to the Windows folder. Must be freed using free
 */
char *get_windows_path();
/**
 * @brief get the drive Windows is installed on
 * @return
 *   the drive structure corresponding to the drive Windows is installed on.
 *   must be freed using #free_drive
 */
drive_t *get_windows_drive();
