#include <stdlib.h>
#include <string.h>

#include "9x.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub4dos.h"
#include "../utils.h"

#define MENU_ITEM "menuitem=LICK, Start Puppy Linux\n"
#define LICK_SECTION_1 "[LICK]\ndevice="
#define LICK_SECTION_2 "\ninstall="
#define LICK_SECTION_3 "\nshell="

char *config_sys_path() {
    drive_t *drive = get_windows_drive();
    if(!drive)
        return NULL;
    char *loc = unix_path(concat_strs(2, drive->path, "/config.sys"));
    free_drive(drive);
    return loc;
}

int supported_loader_9x(sys_info_t *info) {
    return info->family == F_WINDOWS_9X && info->version != V_WINDOWS_ME;
}

int check_loader_9x(sys_info_t *info) {
    if(!supported_loader_9x(info)) {
        return 0;
    }

    // load config.sys into a string
    char *config_sys = config_sys_path();
    if(!config_sys)
        return 0;
    FILE *f = fopen(config_sys, "r");
    if(!f) {
        free(config_sys);
        return 0;
    }
    char *config = file_to_str(f);
    fclose(f);

    char *lick = strstr(config, "[LICK]");
    free(config);
    free(config_sys);
    return lick != NULL;
}

char *install_to_config_sys(char *config, lickdir_t *lick) {
    // find [menu] section
    char *start = strstr(config, "[menu]");
    if(start == NULL) {
        // config.sys doesn't have sections
        char *grub_exe = win_path(concat_strs(2, lick->drive, "/pupl.exe"));
        char *ret = concat_strs(11,
                "[menu]\nmenuitem=WINDOWS,Start Windows\n",
                MENU_ITEM,
                "menudefault=WINDOWS,10\nmenucolor=7,0\n\n",
                LICK_SECTION_1, grub_exe,
                LICK_SECTION_2, grub_exe,
                LICK_SECTION_3, grub_exe,
                "\n\n[WINDOWS]\n", config);
        free(grub_exe);
        return ret;
    }

    // find end of menu (start of next section, or EOF)
    char *end = strchr(start + 1, '[');
    if(end == NULL) {
        end = start;
        while(end[0] != '\0')
            end++;
    }

    // location of menuitem
    char *menuitem = NULL;

    // attempt to find a "nice" place to put menuitem=...
    // aka. after the other menuitems
    char *next_menu = strstr(start, "menuitem=");
    while(next_menu != NULL && next_menu < end) {
        menuitem = next_menu;
        next_menu = strstr(menuitem + 1, "menuitem=");
    }

    if(menuitem != NULL)
        // at last menuitem, move to end of line where the next will go
        menuitem = advance_to_newline(menuitem);
    else
        // otherwise, put right after [menu]
        menuitem = advance_to_newline(start);

    // TODO: timeout?

    char *after = menuitem + 1;
    if(menuitem[0] == '\0')
        after = menuitem;

    char *grub_exe = win_path(concat_strs(2, lick->drive, "/pupl.exe"));

    menuitem[0] = '\0';
    char *ret = concat_strs(12,
            config,
            "\n", MENU_ITEM, after, "\n\n",
            LICK_SECTION_1, grub_exe,
            LICK_SECTION_2, grub_exe,
            LICK_SECTION_3, grub_exe,
            "\n");

    free(grub_exe);
    return ret;
}

char *uninstall_from_config_sys(char *config, lickdir_t *lick) {
    // find the LICK menu item
    char *menuitem = strstr(config, MENU_ITEM);
    if(menuitem == NULL) {
        if(!lick->err)
            lick->err = strdup2("Could not find the LICK menuitem in config.sys");
        return NULL;
    }

    // find the start of the next line
    char *menuitem_end = advance_to_newline(menuitem);
    if(menuitem_end[0] != '\0')
        menuitem_end++;

    // find the start of the LICK section
    char *lick_section = strstr(config, "[LICK]");
    if(lick_section == NULL) {
        if(!lick->err)
            lick->err = strdup2("Could not find the LICK section in config.sys");
        return NULL;
    }

    // find end of LICK section
    char *lick_section_end = strchr(lick_section + 1, '[');
    if(lick_section_end == NULL)
        lick_section_end = strchr(lick_section, '\0');

    //sanity check
    if((menuitem < lick_section && lick_section < menuitem_end)
            || (lick_section < menuitem && menuitem < lick_section_end)) {
        if(!lick->err)
            lick->err = strdup2("Error uninstalling from config.sys");
        return NULL;
    }

    // cutting out these parts, so set beginning to NULL
    menuitem[0] = '\0';
    lick_section[0] = '\0';

    // swap pointers if lick is before menu to reuse code
    if(menuitem > lick_section) {
        char *tmp = lick_section_end;
        lick_section_end = menuitem_end;
        menuitem_end = tmp;
    }

    return concat_strs(3,
            config,
            menuitem_end,
            lick_section_end);
}

// load config.sys
// set [menu] menuitem="PUPPYLINUX, Start Puppy"
// create [PUPPYLINUX] ~> add shell="/path/to/grub.exe"
// save, with attributes
int install_loader_9x(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_9x(info)) {
        return 0;
    }

    // add to config.sys
    char *config_sys = config_sys_path();
    int ret = apply_fn_to_file(config_sys, install_to_config_sys, 1, lick);
    free(config_sys);
    if(!ret)
        return 0;

    char *grub_exe = concat_strs(2, lick->drive, "/pupl.exe");
    char *res_grub_exe = concat_strs(2, lick->res, "/pupl.exe");
    copy_file(grub_exe, res_grub_exe);
    free(grub_exe);
    free(res_grub_exe);
    return 1;
}

int uninstall_loader_9x(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_9x(info)) {
        return 0;
    }

    // remove from config.sys
    char *config_sys = config_sys_path();
    int ret = apply_fn_to_file(config_sys, uninstall_from_config_sys, 0, lick);
    free(config_sys);
    if(!ret)
        return 0;

    char *grub_exe = concat_strs(2, lick->drive, "/pupl.exe");
    unlink_file(grub_exe);
    free(grub_exe);
    return 1;
}

loader_t get_9x() {
    loader_t loader;
    loader.supported = supported_loader_9x;
    loader.check = check_loader_9x;
    loader.install = install_loader_9x;
    loader.uninstall = uninstall_loader_9x;
    loader.get_menu = get_grub4dos;
    return loader;
}
