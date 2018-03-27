#include "boot-loader/me.h"

#include <stdlib.h>

#include "boot-loader/9x.h"
#include "boot-loader/boot-loader-utils.h"
#include "drives.h"
#include "lickdir.h"
#include "menu/grub4dos.h"
#include "utils.h"

int supported_loader_me(sys_info_t *info) {
    return info->version == V_WINDOWS_ME;
}

int check_loader_me_patch() {
    char *drive = get_windows_drive_path();
    if(!drive)
        return 0;
    char *check = unix_path(concat_strs(2, drive, "/Me2Dos"));
    int ret = 0;

    if(path_exists(check))
        ret = 1;

    free(check);
    free(drive);
    return ret;
}

loader_t get_me() {
    loader_t loader = get_9x();
    loader.supported = supported_loader_me;
    return loader;
}
