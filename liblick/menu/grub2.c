#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void grub2_write_entry(FILE *f, entry_t *e) {
    if(e->static_text != NULL) {
        fprintf(f, "%s\n", e->static_text);
        return;
    }
    if(e->title == NULL || e->kernel == NULL || e->options == NULL)
        return;

    fprintf(f, "\n");
    fprintf(f, GRUB2_TITLE, e->title);
    fprintf(f, GRUB2_SEARCH, e->kernel);
    fprintf(f, GRUB2_KERNEL, e->kernel, e->options);
    if(e->initrd != NULL)
        fprintf(f, GRUB2_INITRD, e->initrd);
    fprintf(f, GRUB2_END);
}

int regenerate_grub2(lickdir_t *lick) {
    char *grub_cfg = unix_path(concat_strs(2, lick->drive, "/lickgrub.cfg"));

    FILE *menu = fopen(grub_cfg, "w");
    if(!menu) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not write to lickgrub.cfg");
        free(grub_cfg);
        return 0;
    }
    write_menu(lick, menu, grub2_write_entry);

    free(grub_cfg);
    fclose(menu);
    return 1;
}

int install_grub2(lickdir_t *lick) {
    char *header = unix_path(concat_strs(2, lick->menu, "/00-header.conf"));
    FILE *f = fopen(header, "w");
    free(header);

    if(!f) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not write to menu folder");
        return 0;
    }

    char *grub_cfg_header = unix_path(concat_strs(2, lick->res, "/lickgrub.cfg"));
    FILE *src = fopen(grub_cfg_header, "r");
    free(grub_cfg_header);
    if(!src) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not read from resource directory");
        fclose(f);
        return 0;
    }
    while(1) {
        char *ln = read_line(src);
        if(!ln)
            break;
        fprintf(f, "static %s\n", ln);
        free(ln);
    }

    fclose(src);
    fclose(f);

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
    fprintf(menu, "insmod ntfs\n\n");u
    fprintf(menu, "search --set=root --file /lickgrub.cfg\n");
    fprintf(menu, "configfile ($root)/lickgrub.cfg\n");
    free(grub_cfg);
    fclose(menu);
    unmount_uefi_partition(drive);
    return 1;
}

int uninstall_grub2(lickdir_t *lick) {
    char *header = concat_strs(2, lick->menu, "/00-header.conf");
    unlink_file(header);
    free(header);

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

menu_t *get_grub2() {
    menu_t *menu = malloc(sizeof(menu_t));
    menu->regenerate = regenerate_grub2;
    menu->install = install_grub2;
    menu->uninstall = uninstall_grub2;
    return menu;
}
