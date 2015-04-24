#include <stdlib.h>
#include <string.h>

#include "nt.h"
#include "../drives.h"
#include "../utils.h"

#define BOOT_ITEM "=\"Start Puppy Linux\""

// TODO: dynamic drive
FILE *get_boot_ini(char *mode) {
    char *drive = get_windows_drive();
    char *loc = concat_strs(2, drive, "boot.ini");
    FILE *f = fopen(loc, mode);
    free(loc);

    return f;
}

int supported_loader_nt(win_info_t *info) {
    return info->family == F_WINDOWS_NT;
}

int check_loader_nt(win_info_t *info) {
    if(!supported_loader_nt(info)) {
        return 0;
    }

    // load boot.ini into a string
    FILE *f = get_boot_ini("r");
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
int install_loader_nt(win_info_t *info) {
    if(!supported_loader_nt(info)) {
        return 0;
    }

    // load boot.ini into a string
    FILE *f = get_boot_ini("r");
    if(!f)
        return 0;
    char *boot = file_to_str(f);
    fclose(f);

    // TODO: check timeout

    char *start = strstr(boot, "[operating systems]");
    if(start == NULL) {
        free(boot);
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
    // TODO: attributes?

    f = get_boot_ini("w");
    if(!f) {
        free(boot);
        return 0;
    }

    char *before = "\n";
    char *after = bootitem + 1;
    if(bootitem[0] == '\0') {
        before = "\r\n";
        after = bootitem;
    }

    char *pupldr = concat_strs(2, get_windows_drive(), "lick\\pupldr");

    bootitem[0] = '\0';
    // print start of file, newline,
    //   C:\lick\pupldr="Start Puppy Linux", rest of file
    fprintf(f, "%s%s%s%s\r\n%s", boot, before, pupldr, BOOT_ITEM, after);
    fclose(f);

    free(pupldr);
    free(boot);
    return 1;
}

int uninstall_loader_nt(win_info_t *info) {
    if(!supported_loader_nt(info)) {
        return 0;
    }

    // load boot.ini into a string
    FILE *f = get_boot_ini("r");
    if(!f)
        return 0;
    char *boot = file_to_str(f);
    fclose(f);

    // find ="Start Puppy Linux"
    char *bootitem = strstr(boot, BOOT_ITEM);
    if(bootitem == NULL) {
        free(boot);
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

    // TODO: backup config.sys?
    // TODO: attributes?

    f = get_boot_ini("w");
    if(!f) {
        free(boot);
        return 0;
    }

    fprintf(f, "%s%s", boot, bootitem_end);
    fclose(f);
    free(boot);
    return 1;
}

loader_t get_nt() {
    loader_t loader;
    loader.supported = supported_loader_nt;
    loader.check = check_loader_nt;
    loader.install = install_loader_nt;
    loader.uninstall = uninstall_loader_nt;
    return loader;
}
