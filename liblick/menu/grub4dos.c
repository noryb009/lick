#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edit-flat-menu.h"
#include "grub4dos.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu.h"
#include "../utils.h"

int install_grub4dos(lickdir_t *lick) {
    char *menu = unix_path(concat_strs(2, lick->drive, "/lickmenu.lst"));
    FILE *f = fopen(menu, "w");
    free(menu);

    if(!f) {
        if(lick->err == NULL)
            lick->err = strdup2("Could not write to menu folder");
        return 0;
    }

    fprintf(f, "## start header\n");
    fprintf(f, "static timeout=5\n");
    fprintf(f, "static default=0\n");
    fprintf(f, "## end header\n");
    //fprintf(f, "static gfxmenu=/grub4dos-gui.gz\n");

    fclose(f);

    //char *res_gfxmenu = concat_strs(2, lick->res, "/grub4dos-gui.gz");
    //char *gfxmenu = concat_strs(2, lick->drive, "/grub4dos-gui.gz");
    //copy_file(gfxmenu, res_gfxmenu);
    //free(res_gfxmenu);
    //free(gfxmenu);
    return 1;
}

int uninstall_grub4dos(lickdir_t *lick) {
    (void)lick;
    char *menu_lst = concat_strs(2, lick->drive, "/lickmenu.lst");
    unlink_file(menu_lst);
    free(menu_lst);

    //char *gfxmenu = concat_strs(2, lick->drive, "/grub4dos-gui.gz");
    //unlink_file(gfxmenu);
    //free(gfxmenu);
    return 1;
}

char *gen_grub4dos(distro_info_t *info) {
    return concat_strs(11,
            "title ", (info->name?info->name:""),
            "\nfind --set-root --ignore-floppies ", info->kernel,
            "\nkernel ", info->kernel, (info->options?" ":""),
            (info->options?info->options:""),
            (info->initrd?"\ninitrd ":""), (info->initrd?info->initrd:""),
            "\nboot\n");
}

int append_grub4dos(const char *id, const char *section, lickdir_t *lick) {
    char *menu = unix_path(concat_strs(2, lick->drive, "/lickmenu.lst"));
    int ret = flat_append_section(menu, id, section, lick);
    free(menu);
    return ret;
}

int remove_grub4dos(const char *id, lickdir_t *lick) {
    char *menu = unix_path(concat_strs(2, lick->drive, "/lickmenu.lst"));
    int ret = flat_remove_section(menu, id, lick);
    free(menu);
    return ret;
}

menu_t *get_grub4dos() {
    return new_menu(install_grub4dos, uninstall_grub4dos, gen_grub4dos,
            append_grub4dos, remove_grub4dos);
}
