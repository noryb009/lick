#include <stdlib.h>

#include "drives.h"
#include "utils.h"

drive_t *new_drive(char *name, enum DRIVE_TYPES type) {
    drive_t *d = malloc(sizeof(drive_t));
    d->path = strdup(name);
    d->type = type;
    return d;
}

void free_drive(drive_t *drive) {
    free(drive->path);
    free(drive);
}

void free_drive_lst(node_t *lst) {
    node_t *next;
    while(lst != NULL) {
        next = lst->next;
        free_drive(lst->val);
        lst = next;
    }
}

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

node_t *all_drives() {
    char paths[MAX_PATH];
    DWORD n = GetLogicalStrings(MAX_PATH, paths);
    node_t *drives = NULL;

    for(DWORD p = 0; p < n; p += strlen(&paths[p]) + 1) {
        DRIVE_TYPES type;
        switch(GetDriveType(&paths[p])) {
            case 3:
                type = DRV_HDD;
                break;
            case 2:
                type = DRV_REMOVABLE;
                break;
            case 4:
                type = DRV_REMOTE;
                break;
            default:
                type = DRV_UNKNOWN:
        }

        drives = new_node(new_drive(&paths[p], type), drives);
    }

    return drives;
}
#else
node_t *all_drives() {
    return new_node(new_drive("/", DRV_HDD), NULL);
}
#endif
