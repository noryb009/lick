#include <stdlib.h>

#include "9x.h"
#include "me.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub4dos.h"
#include "../utils.h"

#define ME2DOS_BACKUP "/Me2Dos/backup/"

int supported_loader_me(sys_info_t *info) {
    return info->version == V_WINDOWS_ME;
}

int check_loader_me_patch() {
    drive_t *drv = get_windows_drive();
    char *check = unix_path(concat_strs(2, drv->path, "/Me2Dos"));
    int ret = 0;

    if(path_exists(check))
        ret = 1;

    free(check);
    free_drive(drv);
    return ret;
}

loader_t get_me() {
    loader_t loader = get_9x();
    loader.supported = supported_loader_me;
    return loader;
}
