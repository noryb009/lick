#include <stdlib.h>
#include <string.h>

#include "nt.h"
#include "utils.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub4dos.h"
#include "../utils.h"

#define BOOT_ITEM "=\"Start Puppy Linux\""

char *boot_ini_path() {
    drive_t *drive = get_windows_drive();
    char *loc = concat_strs(2, drive->path, "\\boot.ini");
    free_drive(drive);
    return loc;
}

int supported_loader_nt(sys_info_t *info) {
    return info->family == F_WINDOWS_NT;
}

int check_loader_nt(sys_info_t *info) {
    if(!supported_loader_nt(info)) {
        return 0;
    }

    // load boot.ini into a string
    char *boot_ini = boot_ini_path();
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

// load boot.ini
// make sure good timeout
// [operating systems] ~> /path/to/grldr="Puppy Linux"
int install_loader_nt(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_nt(info)) {
        return 0;
    }

    // load boot.ini into a string
    char *boot_ini = boot_ini_path();
    FILE *f = fopen(boot_ini, "r");
    if(!f) {
        free(boot_ini);
        return 0;
    }
    char *boot = file_to_str(f);
    fclose(f);

    // TODO: check timeout

    char *start = strstr(boot, "[operating systems]");
    if(start == NULL) {
        free(boot);
        free(boot_ini);
        return 0;
    }
    char *end = strchr(start + 1, '[');
    if(end == NULL) {
        end = start;
        while(end[0] != '\0')
            end++;
    }

    char *bootitem = NULL;

    // attempt to find a "nice" place to put the entry
    // aka. after the other items
    char *next_boot;

    next_boot = strchr(start, '=');
    while(next_boot != NULL && next_boot < end) {
        bootitem = next_boot;
        next_boot = strchr(bootitem + 1, '=');
    }

    if(bootitem != NULL)
        bootitem = advance_to_newline(bootitem);

    // otherwise, right after [operating systems]
    if(bootitem == NULL)
        bootitem = advance_to_newline(start);

    // TODO: backup boot.ini?

    attrib_t *attrib = attrib_open(boot_ini);
    f = fopen(boot_ini, "w");
    if(!f) {
        attrib_save(boot_ini, attrib);
        free(boot_ini);
        free(boot);
        return 0;
    }

    char *before = "\n";
    char *after = bootitem + 1;
    if(bootitem[0] == '\0') {
        before = "\r\n";
        after = bootitem;
    }

    char *pupldr = concat_strs(2, lick->drive, "\\pupldr");

    bootitem[0] = '\0';
    // print start of file, newline,
    //   C:\pupldr="Start Puppy Linux", rest of file
    fprintf(f, "%s%s%s%s\r\n%s", boot, before, pupldr, BOOT_ITEM, after);
    fclose(f);
    attrib_save(boot_ini, attrib);

    free(pupldr);
    free(boot);
    free(boot_ini);
    return 1;
}

int uninstall_loader_nt(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_nt(info)) {
        return 0;
    }

    // load boot.ini into a string
    char *boot_ini = boot_ini_path();
    FILE *f = fopen(boot_ini, "r");
    if(!f) {
        free(boot_ini);
        return 0;
    }
    char *boot = file_to_str(f);
    fclose(f);

    // find ="Start Puppy Linux"
    char *bootitem = strstr(boot, BOOT_ITEM);
    if(bootitem == NULL) {
        free(boot);
        free(boot_ini);
        return 0;
    }

    // find start of next line
    char *bootitem_end = advance_to_newline(bootitem);
    if(bootitem_end[0] != '\0')
        bootitem_end++;

    // back up to start of current line
    while(boot < bootitem && bootitem[-1] != '\n')
        bootitem--;

    bootitem[0] = '\0';

    // TODO: backup boot.ini?

    attrib_t *attrib = attrib_open(boot_ini);
    f = fopen(boot_ini, "w");
    if(!f) {
        attrib_save(boot_ini, attrib);
        free(boot);
        free(boot_ini);
        return 0;
    }

    fprintf(f, "%s%s", boot, bootitem_end);
    fclose(f);
    attrib_save(boot_ini, attrib);

    free(boot);
    free(boot_ini);
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
