#include <stdlib.h>

#include "drives.h"
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

void free_drive_list(node_t *lst) {
    free_list(lst, (void (*)(void *))free_drive);
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

node_t *all_drives() {
    DWORD drive_flags = GetLogicalDrives();
    node_t *drives = NULL;

    DWORD n = 1 << 25;
    char path[4];
    strcpy(path, "a:/");

    // go backwards, pushing to front of list, so returns in order
    for(int i = 25; i >= 0; --i, n = n >> 1) {
        if(n == (n & drive_flags)) {
            path[0] = 'A' + i;
            drive_type_e type = drive_type(path);
            if(type != DRV_UNKNOWN)
                drives = new_node(new_drive(path, type), drives);
        }
    }

    return drives;
}

node_t *unused_drives() {
    DWORD drive_flags = GetLogicalDrives();
    node_t *drives = NULL;

    DWORD n = 1 << 25;
    char path[4];
    strcpy(path, "a:/");

    // go backwards, pushing to front of list, so returns in order
    for(int i = 25; i >= 0; --i, n = n >> 1) {
        if(0 == (n & drive_flags)) {
            path[0] = 'A' + i;
            drives = new_node(new_drive(path, DRV_UNUSED), drives);
        }
    }

    return drives;
}

char *get_windows_path() {
    typedef UINT (WINAPI *get_dir)(LPTSTR lpBuffer, UINT uSize);
    HMODULE k = LoadLibrary("Kernel32.dll");
    if(!k)
        return NULL;
    get_dir fn = (get_dir)GetProcAddress(k, "GetSystemWindowsDirectoryA");
    if(!fn) fn = (get_dir)GetProcAddress(k, "GetWindowsDirectoryA");
    if(!fn) {
        FreeLibrary(k);
        return NULL;
    }
    TCHAR buf[256];
    int len = fn(buf, 255);
    if(len == 0) {
        FreeLibrary(k);
        return NULL;
    }
    if(len <= 255) {
        FreeLibrary(k);
        return TCHAR_to_char(buf, len, sizeof(TCHAR));
    }

    // not big enough
    TCHAR buf2[len + 1];
    len = fn(buf2, len + 1);
    FreeLibrary(k);
    if(len == 0)
        return NULL;
    return TCHAR_to_char(buf2, len, sizeof(TCHAR));
}

drive_t *get_windows_drive() {
    char *path = get_windows_path();
    if(!path)
        return NULL;

    char letters[4];
    letters[0] = path[0];
    letters[1] = path[1];
    letters[2] = '/';
    letters[3] = '\0';

    free(path);
    return new_drive(letters, DRV_HDD);
}
#else
drive_type_e drive_type(char *path) {
    return DRV_HDD;
}

node_t *all_drives() {
    return new_node(new_drive("/", DRV_HDD), NULL);
}

node_t *unused_drives() {
    return new_node(new_drive("/", DRV_UNUSED), NULL);
}

char *get_windows_path() {
    return strdup2("/");
}
drive_t *get_windows_drive() {
    return new_drive("/", DRV_HDD);
}
#endif
