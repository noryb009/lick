#pragma once

#include "llist.h"

enum DRIVE_TYPES {
    DRV_HDD,
    DRV_REMOVABLE,
    DRV_UNKNOWN,
};

struct drive_s {
    char *path;
    enum DRIVE_TYPES type;
};
typedef struct drive_s drive_t;

node_t *all_drives();
void free_drive_lst(node_t *lst);
