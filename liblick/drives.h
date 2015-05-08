/**
 * @file
 * @brief Find drives in the system
 */

#pragma once

#include "llist.h"

/**
 * the types of drives
 */
enum DRIVE_TYPES {
    /// other
    DRV_UNKNOWN,
    /// an internal hard drive
    DRV_HDD,
    /// an external hard drive, CD, etc.
    DRV_REMOVABLE,
};

/**
 * @brief info about a drive
 */
typedef struct {
    /// the path to the root of a drive
    char *path;
    /// the type of drive
    enum DRIVE_TYPES type;
} drive_t;

/**
 * @brief return a list of all drives in the system
 * @return a linked list of all drives. Must be freed using #free_drive_list
 */
node_t *all_drives();
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
 * @brief get the drive Windows is installed on
 * @return
 *   the drive structure corresponding to the drive Windows is installed on.
 *   must be freed using #free_drive
 */
drive_t *get_windows_drive();
/**
 * @brief get a list of all drives with LICK installed
 * @return
 *   a list of drive structures which have LICK installed.
 *   Must be freed using #free_drive_list
 */
node_t *get_lick_drives();
/**
 * @brief return the drive which is likely the main LICK drive
 *
 * the drive that is most likely what the user would select, if given a choice
 *   of LICK drives. First checks the Windows drive, then goes alphabetically.
 *
 * @return
 *   a drive which is most likely the LICK drive. Must be freed using
 *   #free_drive
 */
drive_t *get_likely_lick_drive();
