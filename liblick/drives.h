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
typedef struct {
    /// the path to the root of a drive
    char *path;
    /// the type of drive
    drive_type_e type;
} drive_t;

/**
 * @brief free a drive structure
 * @param drive the drive structure to free
 */
void free_drive(drive_t *drive);

#define LIST_NAME drive_node_t
#define LIST_TYPE drive_t
#define FREE_LIST_FN free_drive
#include "llist-type.h"

/**
 * @brief determines the type of a drive
 * @param path a drive in the form "A:/"
 * @return the type of drive
 */
drive_type_e drive_type(char *path);
/**
 * @brief return a list of all drives in the system
 * @return a linked list of all drives
 */
drive_node_t *all_drives();
/**
 * @brief return an unused drive path
 * @return an unused drive path. Must be freed using #free
 */
char *unused_drive();
/**
 * @brief determines if a drive has LICK installed on it
 *
 * LICK can be installed on multiple drives
 *
 * @param drive the drive to check
 * @return 1 if the drive has LICK installed, otherwise 0
 */
int is_lick_drive(drive_t *drive);
