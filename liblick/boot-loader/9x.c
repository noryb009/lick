#include <stdlib.h>
#include <string.h>

#include "9x.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub4dos.h"
#include "../utils.h"

#define MENU_ITEM_KEY "menuitem=LICK,"
#define MENU_ITEM MENU_ITEM_KEY START_LOADER_DESC "\n"
#define LICK_SECTION_1 "[LICK]\ndevice="
#define LICK_SECTION_2 "\ninstall="
#define LICK_SECTION_3 "\nshell="
#define BOOT_FILE "config.sys"

char *boot_drive_9x() {
    return boot_drive(BOOT_FILE);
}

char *config_sys_path_with_drive(const char *boot_drive) {
    return unix_path(concat_strs(3, boot_drive, "/", BOOT_FILE));
}

char *config_sys_path() {
    char *boot_drive = boot_drive_9x();
    if(!boot_drive)
        return NULL;
    char *path = config_sys_path_with_drive(boot_drive);
    free(boot_drive);
    return path;
}

int supported_loader_9x(sys_info_t *info) {
    return info->family == F_WINDOWS_9X && info->version != V_WINDOWS_ME;
}

int check_loader_9x() {
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
    char *start, *end;
    if(!find_section(config, "[menu]", &start, &end)) {
        // config.sys doesn't have sections
        char *grub_exe = win_path(concat_strs(2, lick->drive, "/pupl.exe"));
        char *ret = concat_strs(11,
                "[menu]\nmenuitem=WINDOWS,Start Windows\n",
                MENU_ITEM,
                "menudefault=WINDOWS,5\nmenucolor=7,0\n\n",
                LICK_SECTION_1, grub_exe,
                LICK_SECTION_2, grub_exe,
                LICK_SECTION_3, grub_exe,
                "\n\n[WINDOWS]\n", config);
        free(grub_exe);
        return ret;
    }

    char *after = after_last_entry(start, end, "menuitem=");
    char *grub_exe = win_path(concat_strs(2, lick->drive, "/pupl.exe"));

    char *ret = concat_strs(12,
            config,
            "\n", MENU_ITEM, after, "\n\n",
            LICK_SECTION_1, grub_exe,
            LICK_SECTION_2, grub_exe,
            LICK_SECTION_3, grub_exe,
            "\n");

    free(grub_exe);

    return check_timeout(ret, "menudefault", ",");
}

char *uninstall_from_config_sys(char *config, lickdir_t *lick) {
    char *item, *item_end, *menu_sec, *menu_sec_end;
    (void)lick;
    // find the LICK menu item, in the menu section
    if(find_section(config, "[menu]", &menu_sec, &menu_sec_end)
            && (item = strstr(menu_sec, MENU_ITEM_KEY)) != NULL) {
        // find the start of the next line
        item_end = advance_to_newline(item);
        if(item_end[0] != '\0')
            item_end++;
    } else
        item = item_end = strchr(config, '\0');

    // find the LICK section
    char *sec, *sec_end;
    find_section(config, "[LICK]", &sec, &sec_end);
    if(sec == NULL)
        // item is closer to the end of config
        sec = sec_end = strchr(item, '\0');

    // make sure sections aren't overlapping
    if(item[0] != '\0' && sec[0] != '\0'
            && ((item < sec && sec < item_end)
                || (sec < item && item < sec_end))) {
        // this shouldn't happen
        // TODO: figure out what to do here
        return NULL;
    }

    // removing these parts, so set beginning to NULL
    item[0] = '\0';
    sec[0] = '\0';

    // swap pointers if lick is before menu to reuse code
    if(item_end > sec_end) {
        char *tmp = sec_end;
        sec_end = item_end;
        item_end = tmp;
    }

    return concat_strs(3,
            config,
            item_end,
            sec_end);
}

// load config.sys
// set [menu] menuitem="PUPPYLINUX, Start Puppy"
// create [PUPPYLINUX] ~> add shell="/path/to/grub.exe"
// save, with attributes
int install_loader_9x(sys_info_t *info, lickdir_t *lick) {
    (void)info;

    // add to config.sys
    char *boot_drive = boot_drive_9x();
    if(!boot_drive) {
        if(!lick->err)
            lick->err = concat_strs(2, "Could not load boot loader file: ", BOOT_FILE);
        return 0;
    }
    char *config_sys = config_sys_path_with_drive(boot_drive);

    int ret = apply_fn_to_file(config_sys, install_to_config_sys, 1, lick);
    free(config_sys);
    if(!ret) {
        free(boot_drive);
        return 0;
    }

    char *grub_exe = concat_strs(2, boot_drive, "/pupl.exe");
    char *res_grub_exe = concat_strs(2, lick->res, "/pupl.exe");
    copy_file(grub_exe, res_grub_exe);
    free(grub_exe);
    free(res_grub_exe);
    free(boot_drive);
    return 1;
}

int uninstall_loader_9x(sys_info_t *info, lickdir_t *lick) {
    (void)info;

    // remove from config.sys
    char *boot_drive = boot_drive_9x();
    if(!boot_drive) {
        if(!lick->err)
            lick->err = concat_strs(2, "Could not load boot loader file: ", BOOT_FILE);
        return 0;
    }
    char *config_sys = config_sys_path_with_drive(boot_drive);
    int ret = apply_fn_to_file(config_sys, uninstall_from_config_sys, 0, lick);
    free(config_sys);
    if(!ret) {
        free(boot_drive);
        return 0;
    }

    char *grub_exe = concat_strs(2, boot_drive, "/pupl.exe");
    unlink_file(grub_exe);
    free(grub_exe);
    free(boot_drive);
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
