#include <stdlib.h>
#include <string.h>

#include "uefi.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub2.h"
#include "../utils.h"

// install
#define COMMAND_GRUB_INSTALL "%s/grub2/grub-install --target=x86_64-efi --bootloader-id=LICK --efi-directory=%c: --boot-directory=%c:/EFI/LICK --recheck"
#define COMMAND_DESCRIPTION  "%s /set {%s} description \"LICK Boot Manager\":"
#define COMMAND_FAST_BOOT    "powercfg -h off"

// uninstall
#define COMMAND_ENUM "%s /enum all"
#define COMMAND_DELETE "%s /delete {%s}"

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
    case V_UNKNOWN:
    case V_WINDOWS_95:
    case V_WINDOWS_98:
    case V_WINDOWS_ME:
    case V_WINDOWS_2000:
    case V_WINDOWS_XP:
        break;
    }
    return 0;
}

int check_loader_uefi(sys_info_t *info) {
    if(!supported_loader_uefi(info)) {
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];
    char *bcdedit = get_bcdedit();
    if(!bcdedit)
        return 0;

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ENUM, bcdedit);
    free(bcdedit);
    return get_id_from_command_range(c, id, "----------", "EFI\\LICK\\");
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
        if(!run_system(c)) {return 0;}
    case V_UNKNOWN:
    case V_WINDOWS_95:
    case V_WINDOWS_98:
    case V_WINDOWS_ME:
    case V_WINDOWS_2000:
    case V_WINDOWS_XP:
    case V_WINDOWS_VISTA:
    case V_WINDOWS_7:
        break;
    }

    char drive = mount_uefi_partition();
    if(drive == '\0')
        return 0;

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_GRUB_INSTALL, lick->res, drive, drive);
    if(!run_system(c)) {
        unmount_uefi_partition(drive);
        return 0;
    }

    char *bcdedit = get_bcdedit();
    if(bcdedit) {
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ENUM, bcdedit);
        get_id_from_command_range(c, id, "----------", "EFI\\LICK\\");
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DESCRIPTION, bcdedit, id);
        run_system(c);
        free(bcdedit);
    }

    // add PreLoader.efi and HashTool.efi
    char *efi_grub = strdup2("?:/EFI/LICK/grubx64.efi");
    char *efi_loader = strdup2("?:/EFI/LICK/loader.efi");
    char *efi_hashtool = strdup2("?:/EFI/LICK/HashTool.efi");
    char *res_preloader = concat_strs(2, lick->res, "/PreLoader.efi");
    char *res_hashtool = concat_strs(2, lick->res, "/HashTool.efi");
    efi_loader[0] = drive;
    efi_grub[0] = drive;
    efi_hashtool[0] = drive;

    // copy "grub" -> "loader", "hashtool" -> "hashtool", then
    //   "preloader" -> "grub". This can safely fail at any time.
    do {
        if(!copy_file(efi_loader, efi_grub))
            break;
        if(!copy_file(efi_hashtool, res_hashtool))
            break;
        if(!copy_file(efi_grub, res_preloader))
            break;
    } while(0);

    // clean up
    unmount_uefi_partition(drive);
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
    char *bcdedit = get_bcdedit();
    if(!bcdedit)
        return 0;

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ENUM, bcdedit);
    get_id_from_command_range(c, id, "----------", "EFI\\LICK\\");

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, bcdedit, id);
    free(bcdedit);
    if(!run_system(c)) {return 0;}

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
