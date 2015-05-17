#include <stdlib.h>
#include <string.h>

#include "uefi.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub2.h"
#include "../utils.h"

#define COMMAND_BUFFER_LEN 256

// install
#define COMMAND_MOUNT_UEFI   "mountvol %c: /S"
#define COMMAND_COPY         "bcdedit /copy {bootmgr} /d \"LICK Boot Manager\""
#define COMMAND_DEVICE       "bcdedit /set %s device partition=%c:"
#define COMMAND_PATH         "bcdedit /set %s path \\EFI\\LICK\\PreLoader.efi"
#define COMMAND_ADD_LAST     "bcdedit /set {fwbootmgr} displayorder %s /addlast"
#define COMMAND_UMOUNT_UEFI  "mountvol %c: /D"
#define COMMAND_DELETE_VALUE "bcdedit /deletevalue %s %s"
#define COMMAND_TIME_OUT     "bcdedit /timeout 5"
#define COMMAND_BOOT_MENU    "bcdedit /set {default} bootmenupolicy legacy"
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

    get_id_from_command(COMMAND_COPY, id);

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DEVICE, id, drive);
    if(!system(c)) {return 0;}
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_PATH, id);
    if(!system(c)) {return 0;}
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ADD_LAST, id);
    if(!system(c)) {return 0;}

    const char *to_delete[] = {
        "locale", "inherit", "integrityservices", "default", "resumeobject",
        "displayorder", "toolsdisplayorder", "timeout", NULL
    };
    for(int i = 0; to_delete[i] != NULL; i++) {
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE_VALUE, id, to_delete[i]);
        system(c);
    }

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_TIME_OUT, id);
    system(c);
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_UMOUNT_UEFI, drive);
    system(c);

    switch(info->version) {
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_BOOT_MENU, id);
            if(!system(c)) {return 0;}
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_FAST_BOOT, id);
            if(!system(c)) {return 0;}
    }
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
