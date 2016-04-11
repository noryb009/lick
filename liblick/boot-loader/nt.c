#include <stdlib.h>
#include <string.h>

#include "nt.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub4dos.h"
#include "../utils.h"

#define BOOT_ITEM "=\"" START_LOADER_DESC "\""
#define BOOT_FILE "boot.ini"

char *boot_drive_nt() {
    return boot_drive(BOOT_FILE);
}

char *boot_ini_path_with_drive(const char *boot_drive) {
    return unix_path(concat_strs(3, boot_drive, "/", BOOT_FILE));
}

char *boot_ini_path() {
    char *boot_drive = boot_drive_nt();
    if(!boot_drive)
        return NULL;
    char *path = boot_ini_path_with_drive(boot_drive);
    free(boot_drive);
    return path;
}

int supported_loader_nt(sys_info_t *info) {
    return info->family == F_WINDOWS_NT;
}

int check_loader_nt() {
    // load boot.ini into a string
    char *boot_ini = boot_ini_path();
    if(!boot_ini)
        return 0;
    FILE *f = fopen(boot_ini, "r");
    free(boot_ini);
    if(!f)
        return 0;

    char *boot = file_to_str(f);
    fclose(f);

    char *pupldr = strstr(boot, "pupldr");
    free(boot);
    return pupldr != NULL;
}

char *install_to_boot_ini(char *boot, lickdir_t *lick) {
    char *start, *end;
    if(!find_section(boot, "[operating systems]", &start, &end)) {
        if(!lick->err)
            lick->err = strdup2("Invalid boot.ini");
        return NULL;
    }

    char *after = after_last_entry(start, end, "=");
    char *pupldr = win_path(concat_strs(2, lick->drive, "/pupldr"));

    // print start of file, newline,
    //   C:\pupldr="Start Puppy Linux", rest of file
    char *ret = concat_strs(6,
            boot,
            "\n", pupldr, BOOT_ITEM, "\n",
            after);
    free(pupldr);

    return check_timeout(ret, "timeout", "=");
}

char *uninstall_from_boot_ini(char *boot, lickdir_t *lick) {
    (void)lick;
    // find ="Start Puppy Linux"
    char *bootitem = strstr(boot, "pupldr=");
    if(!bootitem)
        bootitem = strstr(boot, "pupldr");
    if(!bootitem)
        // looks like it's already uninstalled
        return boot;

    // find start of next line
    char *bootitem_end = advance_to_newline(bootitem);
    if(bootitem_end[0] != '\0')
        ++bootitem_end;

    // back up to start of current line
    while(boot < bootitem && bootitem[-1] != '\n')
        --bootitem;

    // remove boot item
    bootitem[0] = '\0';

    return concat_strs(2, boot, bootitem_end);
}

// load boot.ini
// make sure good timeout
// [operating systems] ~> /path/to/grldr="Puppy Linux"
int install_loader_nt(sys_info_t *info, lickdir_t *lick) {
    (void)info;

    // add to boot.ini
    char *boot_drive = boot_drive_nt();
    if(!boot_drive) {
        if(!lick->err)
            lick->err = concat_strs(2, "Could not load boot loader file: ", BOOT_FILE);
        return 0;
    }
    char *boot_ini = boot_ini_path_with_drive(boot_drive);

    int ret = apply_fn_to_file(boot_ini, install_to_boot_ini, 1, lick);
    free(boot_ini);
    if(!ret) {
        free(boot_drive);
        return 0;
    }

    char *pupldr = concat_strs(2, boot_drive, "/pupldr");
    char *res_pupldr = concat_strs(2, lick->res, "/pupldr");
    copy_file(pupldr, res_pupldr);

    free(res_pupldr);
    free(pupldr);
    free(boot_drive);
    return 1;
}

int uninstall_loader_nt(sys_info_t *info, lickdir_t *lick) {
    (void)info;

    // remove from boot.ini
    char *boot_drive = boot_drive_nt();
    if(!boot_drive) {
        if(!lick->err)
            lick->err = concat_strs(2, "Could not load boot loader file: ", BOOT_FILE);
        return 0;
    }
    char *boot_ini = boot_ini_path_with_drive(boot_drive);

    int ret = apply_fn_to_file(boot_ini, uninstall_from_boot_ini, 0, lick);
    free(boot_ini);
    if(!ret) {
        free(boot_drive);
        return 0;
    }

    char *pupldr = concat_strs(2, boot_drive, "/pupldr");
    unlink_file(pupldr);

    free(pupldr);
    free(boot_drive);
    return 1;
}

loader_t get_nt() {
    loader_t loader;
    loader.supported = supported_loader_nt;
    loader.check = check_loader_nt;
    loader.install = install_loader_nt;
    loader.uninstall = uninstall_loader_nt;
    loader.get_menu = get_grub4dos;
    return loader;
}
