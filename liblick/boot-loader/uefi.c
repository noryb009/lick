#include <stdlib.h>
#include <string.h>

#include "uefi.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub2.h"
#include "../utils.h"

#define COMMAND_BUFFER_LEN 1024

// install
#define COMMAND_MOUNT_UEFI   "mountvol %c: /S"
#define COMMAND_GRUB_INSTALL "%s/grub-install --target=x86_64-efi --bootloader-id=LICK --efi-directory=%c: --boot-directory=%c: --recheck"
#define COMMAND_DESCRIPTION  "bcdedit /set %s description \"LICK Boot Manager\":"
#define COMMAND_UMOUNT_UEFI  "mountvol %c: /D"
#define COMMAND_FAST_BOOT    "powercfg -h off"

// uninstall
#define COMMAND_ENUM "bcdedit /enum all"
#define COMMAND_DELETE "bcdedit /delete %s"

int supported_loader_uefi(sys_info_t *info) {
    if(info->family != F_WINDOWS_VISTA || info->is_bios != BIOS_UEFI) {
        return 0;
    }
    switch(info->version) {
        case V_WINDOWS_VISTA:
        case V_WINDOWS_7:
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
        //case V_WINDOWS_10: // TODO: test
            return 1;
    }
    return 0;
}

int check_loader_uefi(sys_info_t *info) {
    if(!supported_loader_uefi(info)) {
        return 0;
    }

    char id[ID_LEN];

    return get_id_from_command_range(COMMAND_ENUM, id, "----------", "EFI\\LICK\\");
}

int install_loader_uefi(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_uefi(info))
        return 0;

    node_t *unused = unused_drives();
    if(unused == NULL)
        return 0;
    node_t *drv;
    for(drv = unused; drv->next != NULL; drv = drv->next) {}
    char drive = ((drive_t*)drv->val)->path[0];
    free_drive_list(unused);

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_MOUNT_UEFI, drive);
    if(!system(c)) {return 0;}

    switch(info->version) {
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
        //case V_WINDOWS_10: // TODO: test
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_FAST_BOOT, id);
            if(!system(c)) {return 0;}
    }

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_GRUB_INSTALL, lick->res, drive, drive);
    if(!system(c)) {return 0;}

    return get_id_from_command_range(COMMAND_ENUM, id, "----------", "EFI\\LICK\\");
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DESCRIPTION, id);
    system(c);
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_UMOUNT_UEFI, drive);
    system(c);

    return 1;
}

int uninstall_loader_uefi(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_uefi(info)) {
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    get_id_from_command_range(COMMAND_ENUM, id, "----------", "EFI\\LICK\\");

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, id);
    if(!system(c)) {return 0;}
    return 1;
}

loader_t get_uefi() {
    loader_t loader;
    loader.supported = supported_loader_uefi;
    loader.check = check_loader_uefi;
    loader.install = install_loader_uefi;
    loader.uninstall = uninstall_loader_uefi;
    loader.get_menu = get_grub2;
    return loader;
}
