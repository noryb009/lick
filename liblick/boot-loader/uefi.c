#include <stdlib.h>
#include <string.h>

#include "uefi.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub2.h"
#include "../utils.h"

// install
#define COMMAND_COPY      "%s /copy {bootmgr} /d \"" LOADER_DESC "\""
#define COMMAND_PATH      "%s /set {%s} path \\EFI\\LICK\\shim.efi"
#define COMMAND_DISPLAY   "%s /set {fwbootmgr} displayorder {%s} /addfirst"
#define COMMAND_FAST_BOOT "powercfg -h off"

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
    case V_WINDOWS_10:
        return 1;
    case V_UNKNOWN:
    case V_WINDOWS_95:
    case V_WINDOWS_98:
    case V_WINDOWS_ME:
    case V_WINDOWS_NT:
    case V_WINDOWS_2000:
    case V_WINDOWS_XP:
        break;
    }
    return 0;
}

int check_loader_uefi() {
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
    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    switch(info->version) {
    case V_WINDOWS_8:
    case V_WINDOWS_8_1:
    case V_WINDOWS_10:
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_FAST_BOOT);
        if(!run_system(c)) {return 0;}
    case V_UNKNOWN:
    case V_WINDOWS_95:
    case V_WINDOWS_98:
    case V_WINDOWS_ME:
    case V_WINDOWS_NT:
    case V_WINDOWS_2000:
    case V_WINDOWS_XP:
    case V_WINDOWS_VISTA:
    case V_WINDOWS_7:
        break;
    }

    char drive = mount_uefi_partition();
    if(drive == '\0')
        return 0;

    // Add files.
    char *efi_dir = strdup2("?:/EFI/LICK");
    char *efi_cert = strdup2("?:/lick.cer");
    char *efi_grub = strdup2("?:/EFI/LICK/" GRUB2_EFI);
    char *efi_shim = strdup2("?:/EFI/LICK/shim.efi");
    char *efi_mokmanager = strdup2("?:/EFI/LICK/" MOKMANAGER_EFI);
    char *res_cert = concat_strs(2, lick->res, "/lick.cer");
    char *res_grub = concat_strs(2, lick->res, "/grub2x64.efi");
    char *res_shim = concat_strs(2, lick->res, "/" SHIM_EFI);
    char *res_mokmanager = concat_strs(2, lick->res, "/" MOKMANAGER_EFI);
    efi_dir[0] = drive;
    efi_cert[0] = drive;
    efi_grub[0] = drive;
    efi_shim[0] = drive;
    efi_mokmanager[0] = drive;

    char *bcdedit = NULL;
    int fail = 1;
    id[0] = '\0';
    do {
        if(!make_dir_parents(efi_dir))
            break;
        if(!copy_file(efi_cert, res_cert))
            break;
        if(!copy_file(efi_grub, res_grub))
            break;
        if(!copy_file(efi_shim, res_shim))
            break;
        if(!copy_file(efi_mokmanager, res_mokmanager))
            break;

        bcdedit = get_bcdedit();

        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_COPY, bcdedit);
        if(!get_id_from_command(c, id))
            break;
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_PATH, bcdedit, id);
        if(!run_system(c))
            break;
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DISPLAY, bcdedit, id);
        if(!run_system(c))
            break;

        fail = 0;
    } while(0);

    int ret = 1;
    if(fail) {
        ret = 0;

        if(id[0] != '\0')
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, bcdedit, id);

        unlink_file(efi_cert);
        unlink_file(efi_grub);
        unlink_file(efi_shim);
        unlink_file(efi_mokmanager);
        unlink_dir(efi_dir);
    }
    free(bcdedit);
    unmount_uefi_partition(drive);
    // Clean up.
    free(efi_dir);
    free(efi_cert);
    free(efi_grub);
    free(efi_shim);
    free(efi_mokmanager);
    free(res_cert);
    free(res_grub);
    free(res_shim);
    free(res_mokmanager);
    return ret;
}

int uninstall_loader_uefi(sys_info_t *info, lickdir_t *lick) {
    (void)info;

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];
    char *bcdedit = get_bcdedit();
    if(!bcdedit) {
        if(!lick->err)
            lick->err = strdup2("Could not find bcdedit!");
        return 0;
    }

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ENUM, bcdedit);
    get_id_from_command_range(c, id, "----------", "EFI\\LICK\\");

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, bcdedit, id);
    free(bcdedit);
    if(!run_system(c)) {
        if(!lick->err)
            lick->err = strdup2("Could not delete entry from boot loader!");
        return 0;
    }

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
