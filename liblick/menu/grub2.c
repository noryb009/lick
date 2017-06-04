#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edit-flat-menu.h"
#include "grub2.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../utils.h"

#define GRUB2_TITLE "menuentry \'%s\' {\n"
#define GRUB2_SEARCH INDENT "search --set=root --file %s\n"
#define GRUB2_KERNEL INDENT "linux %s %s\n"
#define GRUB2_INITRD INDENT "initrd %s\n"
#define GRUB2_END "}\n"

#define COMMAND_MOUNT_UEFI   "mountvol %c: /S"
#define COMMAND_UMOUNT_UEFI  "mountvol %c: /D"

typedef enum {
    GRUB2_FIX_INSTALL,
    GRUB2_FIX_UNINSTALL,
    GRUB2_FIX_CHECK,
} grub2_fix_function;

typedef enum {
    GRUB2_FIX_INSTALLED,
    GRUB2_FIX_UNINSTALLED,
    GRUB2_FIX_PARTLY_INSTALLED,
} grub2_fix_status;

char mount_uefi_partition() {
    char *unused = unused_drive();
    if(unused == NULL)
        return '\0';
    char drive = unused[0];
    free(unused);

    char c[strlen(COMMAND_MOUNT_UEFI) + 1];
    sprintf(c, COMMAND_MOUNT_UEFI, drive);
    if(!run_system(c))
        return '\0';

    return drive;
}

int unmount_uefi_partition(char drive) {
    char c[strlen(COMMAND_UMOUNT_UEFI) + 1];
    sprintf(c, COMMAND_UMOUNT_UEFI, drive);
    return run_system(c);
}

int install_grub2(lickdir_t *lick) {
    char *grub_cfg_lick = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));
    if(!path_exists(grub_cfg_lick)) {
        char *grub_cfg_header = unix_path(concat_strs(2, lick->res, "/lickgrub.cfg"));
        if(!copy_file(grub_cfg_lick, grub_cfg_header)) {
            free(grub_cfg_lick);
            free(grub_cfg_header);
            if(!lick->err)
                lick->err = strdup2("Error writing to grub menu.");
            return 0;
        }
        free(grub_cfg_header);
    }
    free(grub_cfg_lick);
    return 1;
}

int fix_grub2_inner(lickdir_t *lick, grub2_fix_function function, char original_drive) {
    char drive = original_drive;
    if(!drive) {
        drive = mount_uefi_partition();
        if(!drive) {
            lick->err = strdup2("Could not mount UEFI partition.");
            return 0;
        }
    }

    char *lick_grub = strdup2("?:/efi/lick/grubx64.efi");
    char *lick_shim = strdup2("?:/efi/lick/shim.efi");
    char *lick_mokmanager = strdup2("?:/efi/lick/MokManager.efi");
    char *boot_grub = strdup2("?:/efi/boot/grubx64.efi");
    char *boot_shim = strdup2("?:/efi/boot/shim.efi");
    char *boot_mokmanager = strdup2("?:/efi/boot/MokManager.efi");
    char *boot_file = strdup("?:/efi/boot/bootx64.efi");
    char *boot_file_backup = strdup2("?:/efi/boot/bootx64-orig.efi");
    char *ms_loader = strdup2("?:/efi/microsoft/boot/bootmgfw.efi");
    char *ms_loader_backup = strdup2("?:/efi/microsoft/boot/bootmgfw-backup.efi");
    char *grub_cfg = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));
    char *grub_menu = NULL;

    lick_grub[0] = drive;
    lick_shim[0] = drive;
    lick_mokmanager[0] = drive;
    boot_grub[0] = drive;
    boot_shim[0] = drive;
    boot_mokmanager[0] = drive;
    ms_loader[0] = drive;
    ms_loader_backup[0] = drive;
    boot_file[0] = drive;
    boot_file_backup[0] = drive;

    const char *fatal_warning =
        "WARNING! The fix has only been half applied.\n"
        "The system may not boot when rebooted.\n"
        "Find out why renaming files on the UEFI partition is not working,\n"
        "then move `/EFI/Microsoft` to `/EFI/Microsoft-backup`.";

    grub2_fix_status status = GRUB2_FIX_UNINSTALLED;

    if(path_exists(boot_grub) || path_exists(boot_mokmanager)
            || path_exists(ms_loader_backup)
            || path_exists(boot_file_backup)) {
        if(path_exists(boot_grub) && path_exists(boot_mokmanager)
                && path_exists(ms_loader_backup)
                && path_exists(boot_file_backup))
            status = GRUB2_FIX_INSTALLED;
        else
            status = GRUB2_FIX_PARTLY_INSTALLED;
    }

    int ret = 0;

    if(function == GRUB2_FIX_CHECK) {
        if(status == GRUB2_FIX_INSTALLED)
            ret = 1;
    } else if(status == GRUB2_FIX_PARTLY_INSTALLED)
        lick->err = strdup2("Partial boot fix applied.");
    else if(function == GRUB2_FIX_INSTALL && status == GRUB2_FIX_INSTALLED)
        lick->err = strdup2("Boot fix already applied!");
    else if(function == GRUB2_FIX_UNINSTALL && status == GRUB2_FIX_UNINSTALLED)
        ret = 1;
    else if(function == GRUB2_FIX_INSTALL) {
        /* Steps:
         * 1) Copy `/EFI/LICK/{grubx64,shim,MokManager}.efi` to `/EFI/Boot/`
         * 2) Rename `/EFI/Boot/bootx64.efi` to `bootx64-orig.efi`
         * 3) Rename `/EFI/Boot/shim.efi` to `bootx64.efi`
         * 4) Rename `/EFI/Microsoft/Boot/bootmgfw.efi` to `bootmgfw-backup.efi`
         */
        do {
            int fail = 1;
            do {
                if(!copy_file(boot_grub, lick_grub))
                    break;
                if(!copy_file(boot_shim, lick_shim))
                    break;
                if(!copy_file(boot_mokmanager, lick_mokmanager))
                    break;
                if(!copy_file(boot_file_backup, boot_file))
                    break;

                attrib_t boot_attrs = attrib_open(boot_file);
                if(!replace_file(boot_file, boot_shim)) {
                    attrib_save(boot_file, boot_attrs);
                    lick->err = strdup2("Could not overwrite boot file.");
                    break;
                }
                if(!rename_file(ms_loader_backup, ms_loader)) {
                    // Try to restore the backup.
                    if(!replace_file(boot_file, boot_file_backup)) {
                        // At this point we are stuck with the fix being
                        // half applied. Realistically, this should never occur.
                        attrib_save(boot_file, boot_attrs);
                        lick->err = strdup2(fatal_warning);
                        fail = 0;
                        break;
                    }
                    lick->err = strdup2("Could not rename directory.");
                    break;
                }

                attrib_save(boot_file_backup, boot_attrs);
                fail = 0;
            } while(0);

            if(fail) {
                unlink_file(boot_grub);
                unlink_file(boot_shim);
                unlink_file(boot_mokmanager);
                unlink_file(boot_file_backup);
                if(!lick->err)
                    lick->err = strdup2("Could not copy files on EFI partition.");
                break;
            }

            // Edit grub menu.
            FILE *f = fopen(grub_cfg, "r");
            if(!f) {
                if(!lick->err)
                    lick->err = strdup2("Successfully installed, but could not modify configuration file.");
                break;
            }
            grub_menu = file_to_str(f);
            fclose(f);
            char *grub_menu_lower = lower_str(strdup2(grub_menu));

            // Search for `/efi/microsoft` (case insensitive)
            // and replace with `/efi/microsoft-backup`
            // First, find the number of `/efi/microsoft`
            size_t cnt = 0;
            const char *str = grub_menu_lower;
            const char *needle = "/efi/microsoft/boot/bootmgfw.efi";
            const char *replacement = "/efi/microsoft/boot/bootmgfw-backup.efi";
            for(;;) {
                str = strstr(str, needle);
                if(!str)
                    break;
                ++cnt;
                ++str; // Increment string to find the next occurrance.
            }

            if(cnt > 0) {
                /* Here, there are 3 strings:
                 * - The original menu, grub_menu.
                 *   This is pointed to by start and str_normal.
                 * - The lowercase menu, grub_menu_lower.
                 *   This is pointed to by str.
                 * - The new menu, new_grub_menu.
                 *   This is pointed to by dst.
                 */
                size_t newsize = strlen(grub_menu)
                    + cnt * (strlen(replacement) - strlen(needle))
                    + 1;
                char *new_grub_menu = malloc(newsize);
                char *dst = new_grub_menu;

                const char *start = grub_menu;
                str = grub_menu_lower;
                for(size_t i = 0; i < cnt; ++i) {
                    str = strstr(str, needle);
                    const char *str_normal = str - grub_menu_lower + grub_menu;
                    // Print everything from start to str.
                    size_t len = str_normal - start;
                    memcpy(dst, start, len);
                    dst += len;
                    strcpy(dst, replacement);
                    str += strlen(needle);
                    dst += strlen(replacement);

                    start = str_normal + strlen(needle);
                }
                strcpy(dst, start);
                grub_menu[newsize - 1] = '\0';

                f = fopen(grub_cfg, "w");
                if(!f) {
                    if(!lick->err)
                        lick->err = strdup2("Successfully installed, but could not modify configuration file.");
                    free(new_grub_menu);
                    free(grub_menu_lower);
                    break;
                }
                fprintf(f, "%s", new_grub_menu);
                fclose(f);
                free(new_grub_menu);
                free(grub_menu_lower);
            }
            ret = 1;
        } while(0);
    } else {
        /* Steps:
         * 1) Rename `/EFI/Microsoft/Boot/bootmgfw-backup.efi` to `bootmgfw.efi`
         * 2) Rename `/EFI/Boot/bootx64-orig.efi` to `bootx64.efi`
         * 3) Delete `/EFI/Boot/{grubx64,MokManager}.efi`
         */
        do {
            if(!rename_file(ms_loader, ms_loader_backup)) {
                lick->err = strdup2("Could not rename directory.");
                break;
            }
            attrib_t boot_attrs = attrib_open(boot_file_backup);
            if(!replace_file(boot_file, boot_file_backup)) {
                attrib_save(boot_file_backup, boot_attrs);
                if(!rename_file(ms_loader_backup, ms_loader)) {
                    lick->err = strdup2(fatal_warning);
                    break;
                }
                lick->err = strdup2("Could not replace boot file.");
                break;
            }
            attrib_save(boot_file, boot_attrs);

            unlink_file(boot_grub);
            unlink_file(boot_mokmanager);
            ret = 1;
        } while(0);
    }

    if(!original_drive)
        unmount_uefi_partition(drive);
    free(lick_grub);
    free(lick_shim);
    free(lick_mokmanager);
    free(boot_grub);
    free(boot_shim);
    free(boot_mokmanager);
    free(ms_loader);
    free(ms_loader_backup);
    free(boot_file);
    free(boot_file_backup);
    free(grub_cfg);
    free(grub_menu);
    return ret;
}

int uninstall_grub2(lickdir_t *lick) {
    char drive = mount_uefi_partition();
    if(drive == '\0') {
        if(!lick->err)
            lick->err = strdup2("Could not mount EFI partition.");
        return 0;
    }

    if(!fix_grub2_inner(lick, GRUB2_FIX_UNINSTALL, drive)) {
        unmount_uefi_partition(drive);
        if(!lick->err)
            lick->err = strdup2("Could not revert boot fix.");
        return 0;
    }

    char *grub_cfg = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));
    if(!has_valuable_info(grub_cfg)) {
        unlink_file(grub_cfg);
    }
    free(grub_cfg);

    char *lick_cert = strdup2("?:/lick.cer");
    lick_cert[0] = drive;
    unlink_file(lick_cert);
    free(lick_cert);

    char lick_dir[] = "?:/EFI/LICK";
    lick_dir[0] = drive;
    unlink_recursive(lick_dir);
    unmount_uefi_partition(drive);

    return 1;
}

char *gen_grub2(distro_info_t *info) {
    return concat_strs(12,
            "menuentry '", (info->name?info->name:""), "' {\n",
            "search --set=root --file ", info->kernel,
            "\nlinux ", info->kernel, (info->options?" ":""),
            (info->options?info->options:""),
            (info->initrd?"\ninitrd ":""), (info->initrd?info->initrd:""),
            "\n}\n");
}

int append_grub2(const char *id, const char *section, lickdir_t *lick) {
    char *menu_path = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));
    int ret = flat_append_section(menu_path, id, section, lick);
    free(menu_path);
    return ret;
}

int remove_grub2(const char *id, lickdir_t *lick) {
    char *menu_path = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));
    int ret = flat_remove_section(menu_path, id, lick);
    free(menu_path);
    return ret;
}

int fix_grub2(lickdir_t *lick) {
    return fix_grub2_inner(lick, GRUB2_FIX_INSTALL, '\0');
}

int check_fix_grub2(lickdir_t *lick) {
    return fix_grub2_inner(lick, GRUB2_FIX_CHECK, '\0');
}

menu_t *get_grub2() {
    return new_menu(install_grub2, uninstall_grub2,
            fix_grub2, check_fix_grub2,
            gen_grub2, append_grub2, remove_grub2);
}
