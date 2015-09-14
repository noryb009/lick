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

    char drive = mount_uefi_partition();
    if(drive == '\0')
        return 0;
    char *grub_cfg = strdup2("?:/EFI/LICK/grub/grub.cfg");
    grub_cfg[0] = drive;

    FILE *menu = fopen(grub_cfg, "w");
    if(!menu) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not write to grub.cfg");
        free(grub_cfg);
        unmount_uefi_partition(drive);
        return 0;
    }
    fprintf(menu, "insmod part_gpt\n");
    fprintf(menu, "insmod part_msdos\n");
    fprintf(menu, "insmod ntfs\n\n");
    fprintf(menu, "search --set=root --file /lickgrub.cfg\n");
    fprintf(menu, "configfile ($root)/lickgrub.cfg\n");
    free(grub_cfg);
    fclose(menu);
    unmount_uefi_partition(drive);
    return 1;
}

int uninstall_grub2(lickdir_t *lick) {
    char *grub_cfg = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));
    unlink_file(grub_cfg);
    free(grub_cfg);

    char drive = mount_uefi_partition();
    if(drive == '\0')
        return 0;
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

menu_t *get_grub2() {
    return new_menu(install_grub2, uninstall_grub2, gen_grub2,
            append_grub2, remove_grub2);
}
