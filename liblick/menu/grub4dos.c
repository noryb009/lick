#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grub4dos.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../utils.h"

#define GRUB4DOS_TITLE "title %s\n"
#define GRUB4DOS_FIND INDENT "find --set-root --ignore-floppies %s\n"
#define GRUB4DOS_KERNEL INDENT "kernel %s %s\n"
#define GRUB4DOS_INITRD INDENT "initrd %s\n"
#define GRUB4DOS_BOOT INDENT "boot\n"

void grub4dos_write_entry(FILE *f, entry_t *e) {
    if(e->static_text != NULL) {
        fprintf(f, "%s\n", e->static_text);
        return;
    }
    if(e->title == NULL || e->kernel == NULL || e->options == NULL)
        return;

    fprintf(f, "\n");
    fprintf(f, GRUB4DOS_TITLE, e->title);
    fprintf(f, GRUB4DOS_FIND, e->kernel);
    fprintf(f, GRUB4DOS_KERNEL, e->kernel, e->options);
    if(e->initrd != NULL)
        fprintf(f, GRUB4DOS_INITRD, e->initrd);
    fprintf(f, GRUB4DOS_BOOT);
}

int regenerate_grub4dos(lickdir_t *lick) {
    drive_t *win_drive = get_windows_drive();
    char *menu_lst = unix_path(concat_strs(2, win_drive->path, "/lickmenu.lst"));

    FILE *menu = fopen(menu_lst, "w");
    if(!menu) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not write to lickmenu.lst");
        free(menu_lst);
        free_drive(win_drive);
        return 0;
    }
    write_menu(lick, menu, grub4dos_write_entry);

    free(menu_lst);
    free_drive(win_drive);
    fclose(menu);
    return 1;
}

int install_grub4dos(lickdir_t *lick) {
    char *header = unix_path(concat_strs(2, lick->menu, "/00-header.conf"));
    FILE *f = fopen(header, "w");
    free(header);

    if(!f) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not write to menu folder");
        return 0;
    }

    fprintf(f, "static timeout=5\n");
    fprintf(f, "static default=0\n");
    fprintf(f, "static gfxmenu=/grub4dos-gui.gz\n");

    fclose(f);

    char *res_gfxmenu = concat_strs(2, lick->res, "/grub4dos-gui.gz");
    char *gfxmenu = concat_strs(2, lick->drive, "/grub4dos-gui.gz");
    copy_file(gfxmenu, res_gfxmenu);
    free(res_gfxmenu);
    free(gfxmenu);
    return 1;
}

int uninstall_grub4dos(lickdir_t *lick) {
    char *header = concat_strs(2, lick->menu, "/00-header.conf");
    unlink_file(header);
    free(header);

    drive_t *win_drive = get_windows_drive();
    char *menu_lst = concat_strs(2, win_drive->path, "/lickmenu.lst");
    unlink_file(menu_lst);
    free(menu_lst);
    free_drive(win_drive);

    char *gfxmenu = concat_strs(2, lick->drive, "/grub4dos-gui.gz");
    unlink_file(gfxmenu);
    free(gfxmenu);
    return 1;
}

menu_t *get_grub4dos() {
    menu_t *menu = malloc(sizeof(menu_t));
    menu->regenerate = regenerate_grub4dos;
    menu->install = install_grub4dos;
    menu->uninstall = uninstall_grub4dos;
    return menu;
}
