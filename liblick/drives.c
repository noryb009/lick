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

drive_t *get_windows_drive() {
    TCHAR buf[30];
    if(GetSystemWindowsDirectory(buf, 30) == 0)
        return NULL;

    char letters[3];
    letters[0] = (char)buf[0];
    letters[1] = (char)buf[1];
    letters[2] = '\0';

    return drive =new_drive(letters, DRV_HDD);
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

    while(drives != NULL && !is_lick_drive(drives->val)) {
        node_t *next = drives->next;
        free_drive(drives->val);
        drives = next;
    }

    if(drives == NULL)
        return NULL;

    node_t *head = drives;

    while(drives->next != NULL) {
        node_t *next = drives->next;
        if(is_lick_drive(next->val))
            drives = next;
        else {
            drives->next = next->next;
            free(next->val);
        }
    }

    return head;
}
