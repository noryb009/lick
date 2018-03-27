#include "drives.h"

#include <stdlib.h>

#include "utils.h"

drive_t *new_drive(char *name, drive_type_e type) {
    drive_t *d = malloc(sizeof(drive_t));
    d->path = strdup2(name);
    d->type = type;
    return d;
}

void free_drive(drive_t *drive) {
    free(drive->path);
    free(drive);
}

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

drive_type_e drive_type(char *path) {
    switch(GetDriveType(path)) {
    case DRIVE_FIXED:
        return DRV_HDD;
    case DRIVE_REMOVABLE:
        return DRV_REMOVABLE;
    case DRIVE_REMOTE:
        return DRV_REMOTE;
    case DRIVE_CDROM:
        return DRV_CDROM;
    default:
        return DRV_UNKNOWN;
    }
}

drive_node_t *all_drives() {
    DWORD drive_flags = GetLogicalDrives();
    drive_node_t *drives = NULL;

    DWORD n = 1 << 25;
    char path[4];
    strcpy(path, "a:/");

    // go backwards, pushing to front of list, so returns in order
    for(int i = 25; i >= 0; --i, n = n >> 1) {
        if(n == (n & drive_flags)) {
            path[0] = 'A' + i;
            drive_type_e type = drive_type(path);
            if(type != DRV_UNKNOWN)
                drives = new_drive_node_t(new_drive(path, type), drives);
        }
    }

    return drives;
}

char *unused_drive() {
    DWORD drive_flags = GetLogicalDrives();
    DWORD n = 1 << 25;

    // go backwards, pushing to front of list, so returns in order
    for(int i = 25; i >= 0; --i, n = n >> 1) {
        if(0 == (n & drive_flags)) {
            char *path = strdup2("?:/");
            path[0] = 'A' + i;
            return path;
        }
    }

    return NULL;
}
#else
drive_type_e drive_type(char *path) {
    return DRV_HDD;
}

drive_node_t *all_drives() {
    return new_drive_node(new_drive("/", DRV_HDD), NULL);
}

char *unused_drive() {
    return strdup2("/");
}
#endif
