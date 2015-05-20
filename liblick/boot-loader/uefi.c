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
#define COMMAND_GRUB_INSTALL "%s/grub-install --target=x86_64-efi --bootloader-id=LICK --efi-directory=%c: --boot-directory=%c: --recheck"
#define COMMAND_DESCRIPTION  "bcdedit /set %s description \"LICK Boot Manager\":"
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

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    switch(info->version) {
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
        //case V_WINDOWS_10: // TODO: test
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_FAST_BOOT, id);
            if(!system(c)) {return 0;}
    }

    char drive = mount_uefi_partition();
    if(drive == '\0')
        return 0;

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_GRUB_INSTALL, lick->res, drive, drive);
    if(!system(c)) {
        unmount_uefi_partition(drive);
        return 0;
    }

    get_id_from_command_range(COMMAND_ENUM, id, "----------", "EFI\\LICK\\");
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DESCRIPTION, id);
    system(c);
    unmount_uefi_partition(drive);

    // add PreLoader.efi and HashTool.efi
    char *efi_grub = strdup("?:/EFI/LICK/grubx64.efi");
    char *efi_loader = strdup("?:/EFI/LICK/loader.efi");
    char *efi_hashtool = strdup("?:/EFI/LICK/HashTool.efi");
    char *res_preloader = concat_strs(2, lick->res, "/PreLoader.efi");
    char *res_hashtool = concat_strs(2, lick->res, "/HashTool.efi");
    efi_loader[0] = drive;
    efi_grub[0] = drive;
    efi_hashtool[0] = drive;

    // copy "grub" -> "loader", "hashtool" -> "hashtool", then
    //   "preloader" -> "grub". This can safely fail at any time.
    if(!copy_file(efi_loader, efi_grub))
        goto cleanup_preloader;
    if(!copy_file(efi_hashtool, res_hashtool))
        goto cleanup_preloader;
    if(!copy_file(efi_grub, res_preloader))
        goto cleanup_preloader;

cleanup_preloader:
    free(efi_loader);
    free(efi_grub);
    free(efi_hashtool);
    free(res_preloader);
    free(res_hashtool);
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
