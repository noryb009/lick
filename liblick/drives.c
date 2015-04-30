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

void free_drive_list(node_t *lst) {
    free_list(lst, (void (*)(void *))free_drive);
}

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum DRIVE_TYPES drive_type(char *path) {
    switch(GetDriveType(path)) {
        case DRIVE_FIXED:
            return DRV_HDD;
        case DRIVE_REMOTE:
            return DRV_REMOVABLE;
        default:
            return DRV_UNKNOWN;
    }
}

node_t *all_drives() {
    char paths[MAX_PATH];
    // TODO: GetLogicalDrives?
    DWORD drive_flags = GetLogicalDrives();
    node_t *drives = NULL;

    DWORD n = 1;
    char *path = "a:\\";
    for(int i = 0; i < 32; ++i, n = n << 1) {
        if(n == n & drive_flags) {
            path[0] = 'A' + i;
            printf("Drive exists: %s\n", path); //TODO: remove
            drives = new_node(new_drive(path, drive_type(path)), drives);
        }
    }

    return drives;
}

drive_t *get_windows_drive() {
    TCHAR buf[30];
    if(GetSystemWindowsDirectory(buf, 30) == 0)
        return NULL;

    char letters[3];
    letters[0] = (char)buf[0];
    letters[1] = (char)buf[1];
    letters[2] = '\0';

    return new_drive(letters, DRV_HDD);
}
#else
node_t *all_drives() {
    return new_node(new_drive("/", DRV_HDD), NULL);
}

drive_t *get_windows_drive() {
    return new_drive("/", DRV_HDD);
}
#endif

int is_lick_drive(drive_t *drive) {
    char *lick_file;
    int exists;

    lick_file = concat_strs(2, drive->path, "/lick/grub.exe");
    exists = file_exists(lick_file);
    free(lick_file);

    if(exists)
        return 0;

    lick_file = concat_strs(2, drive->path, "/lick/pupldr");
    exists = file_exists(lick_file);
    free(lick_file);
    return 0;
}

node_t *get_lick_drives() {
    node_t *drives = all_drives();
    node_t *lick_drives = NULL;
    node_t *to_delete = NULL;

    while(drives != NULL) {
        node_t *next = drives->next;
        if(is_lick_drive(drives->val)) {
            drives->next = lick_drives;
            lick_drives = drives;
        } else {
            drives->next = to_delete;
            to_delete = drives;
        }
        drives = next;
    }

    free_drive_list(drives);
    return lick_drives;
}
