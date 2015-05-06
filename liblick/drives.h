#pragma once

#include "llist.h"

enum DRIVE_TYPES {
    DRV_HDD,
    DRV_REMOVABLE,
    DRV_UNKNOWN,
};

typedef struct {
    char *path;
    enum DRIVE_TYPES type;
} drive_t;

node_t *all_drives();
void free_drive(drive_t *drive);
void free_drive_list(node_t *lst);
int is_lick_drive(drive_t *drive);
drive_t *get_windows_drive();
node_t *get_lick_drives();
