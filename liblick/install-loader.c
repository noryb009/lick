#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "install-loader.h"
#include "utils.h"

// TODO: move into plugins

#define MENU_ITEM "menuitem=LICK, Start Puppy Linux\r\n"
#define LICK_SECTION "\r\n[LICK]\r\nshell="

// TODO: dynamic drive
FILE *get_config_sys(char *mode) {
    char *drive = get_windows_drive();
    char *loc = concat_strs(2, drive, "config.sys");
    FILE *f = fopen(loc, mode);
    free(loc);

    return f;
}

char *advance_to_newline(char *s) {
    while(s[0] != '\0' && s[0] != '\n')
        s++;
    return s;
}

// load config.sys
// set [menu] menuitem="PUPPYLINUX, Start Puppy"
// create [PUPPYLINUX] ~> add shell="/path/to/grub.exe"
// save, with attributes
int install_loader_9x(win_info_t *info) {
    if(info->version == V_WINDOWS_ME) {
        return 0;
    }

    // load config.sys into a string
    FILE *f = get_config_sys("r");
    if(!f)
        return 0;
    char *config = file_to_str(f);
    fclose(f);

    // find [menu] section
    char *start = strstr(config, "[menu]");
    if(start == NULL) {
        free(config);
        return 0;
    }
    char *end = strchr(start + 1, '[');
    if(end == NULL) {
        end = start;
        while(end[0] != '\0')
            end++;
    }

    char *menuitem = NULL;

    // attempt to find a "nice" place to put menuitem=...
    // aka. after the other menuitems
    char *next_menu;

    next_menu = strstr(start, "menuitem=");
    while(next_menu != NULL && next_menu < end) {
        menuitem = next_menu;
        next_menu = strstr(menuitem + 1, "menuitem=");
    }

    if(menuitem != NULL)
        menuitem = advance_to_newline(menuitem);

    // otherwise, right after [menu]
    if(menuitem == NULL)
        menuitem = advance_to_newline(start);

    // TODO: backup config.sys?
    // TODO: attributes?
    // TODO: timeout?

    f = get_config_sys("w");
    if(!f) {
        free(config);
        return 0;
    }

    char *before = "\n";
    char *after = menuitem + 1;
    if(menuitem[0] == '\0') {
        before = "\r\n";
        after = menuitem;
    }

    char *grub_exe = concat_strs(2, get_windows_drive(), "lick\\grub.exe");

    menuitem[0] = '\0';
    fprintf(f, "%s%s%s%s", config, before, MENU_ITEM, after);
    // add [LICK] section to end of file
    fprintf(f, "\r\n%s%s\r\n", LICK_SECTION, grub_exe);
    fclose(f);

    free(grub_exe);
    free(config);
    return 1;
}

int uninstall_loader_9x(win_info_t *info) {
    if(info->version == V_WINDOWS_ME) {
        return 0;
    }

    // load config.sys into a string
    FILE *f = get_config_sys("r");
    if(!f)
        return 0;
    char *config = file_to_str(f);
    fclose(f);

    char *menuitem = strstr(config, MENU_ITEM);
    if(menuitem == NULL) {
        free(config);
        return 0;
    }

    char *menuitem_end = advance_to_newline(menuitem);
    if(menuitem_end[0] != '\0')
        menuitem_end++;

    char *lick_section = strstr(config, LICK_SECTION);
    char *lick_section_end =
        advance_to_newline(lick_section + strlen(LICK_SECTION));
    if(lick_section_end[0] != '\0')
        lick_section_end++;
    if(lick_section == NULL) {
        free(config);
        return 0;
    }

    while(lick_section[-4] == '\r'
            && lick_section[-3] == '\n'
            && lick_section[-2] == '\r'
            && lick_section[-1] == '\n')
        lick_section -= 2;

    //sanity check
    if((menuitem < lick_section && lick_section < menuitem_end)
            || (lick_section < menuitem && menuitem < lick_section_end)) {
        free(config);
        return 0;
    }

    menuitem[0] = '\0';
    lick_section[0] = '\0';

    // TODO: backup config.sys?
    // TODO: attributes?

    f = get_config_sys("w");
    if(!f) {
        free(config);
        return 0;
    }

    fprintf(f, "%s", config);
    if(menuitem < lick_section)
        fprintf(f, "%s%s", menuitem_end, lick_section_end);
    else
        fprintf(f, "%s%s", lick_section_end, menuitem_end);
    fclose(f);
    free(config);
    return 1;
}

#define BOOT_ITEM "=\"Start Puppy Linux\""

// TODO: dynamic drive
FILE *get_boot_ini(char *mode) {
    char *drive = get_windows_drive();
    char *loc = concat_strs(2, drive, "boot.ini");
    FILE *f = fopen(loc, mode);
    free(loc);

    return f;
}

// load boot.ini
// make sure good timeout
// [operating systems] ~> /path/to/grldr="Puppy Linux"
int install_loader_nt(win_info_t *info) {
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
    printf("next boot = %p\n", next_boot);
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

#define ID_LEN 37
#define COMMAND_BUFFER_LEN 256

// install
#define COMMAND_CREATE "bcdedit /create /d \"Puppy Linux\" /application bootsector"
#define COMMAND_DEVICE "bcdedit /set {%s} device \"partition=%s\""
#define COMMAND_PATH "bcdedit /set {%s} path \\pupldr.mbr"
#define COMMAND_ADD_LAST "bcdedit /displayorder {%s} /addlast"
#define COMMAND_TIME_OUT "bcdedit /timeout 5"
#define COMMAND_BOOT_MENU "bcdedit /set {default} bootmenupolicy legacy"
#define COMMAND_FAST_BOOT "powercfg -h off"

// uninstall
#define COMMAND_ENUM "bcdedit /enum all"
#define COMMAND_DELETE "bcdedit /delete %s"

int get_id_from_command_range(const char *c, char *out, char *start, char *end) {
    out[0] = '\0';
    int ret = 0;

    // TODO: portable popen
    FILE *pipe = popen(c, "r");
    if(!pipe) {return 0;}

    char buf[512] = "";
    while(!feof(pipe)) {
        if(fgets(buf, 512, pipe) != NULL) {
            if(start != NULL && strstr(buf, start) != NULL) {
                ret = 0;
            }
            char *id = strstr(buf, "{");
            if(id != NULL) {
                char *id_end = strstr(buf, "}");
                if(id_end != NULL) {
                    id_end[0] = '\0';
                    if(id_end-id-1 >= ID_LEN) {
                        ret = 0;
                    } else {
                        strncpy(out, id+1, ID_LEN);
                        ret = 1;
                    }
                }
            } else if(end != NULL && strstr(buf, end) != NULL) {
                break;
            }
        }
    }

    pclose(pipe);
    return ret;
}

int get_id_from_command(const char *c, char *out) {
    return get_id_from_command_range(c, out, NULL, NULL);
}

int install_loader_vista(win_info_t *info) {
    if(info->is_bios != BIOS_BIOS) {
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];
    char *drive = get_windows_drive();

    get_id_from_command(COMMAND_CREATE, id);

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DEVICE, id, drive);
    if(!system(c)) {return 0;}
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_PATH, id);
    if(!system(c)) {return 0;}
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ADD_LAST, id);
    if(!system(c)) {return 0;}
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_TIME_OUT, id);
    if(!system(c)) {return 0;}

    switch(info->version) {
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_BOOT_MENU, id);
            if(!system(c)) {return 0;}
            snprintf(c, COMMAND_BUFFER_LEN, COMMAND_FAST_BOOT, id);
            if(!system(c)) {return 0;}
    }
    return 1;
}

int uninstall_loader_vista(win_info_t *info) {
    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    get_id_from_command_range(COMMAND_ENUM, id, "----------", "pupldr.mbr");

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, id);
    if(!system(c)) {return 0;}
}

int install_loader(win_info_t *info) {
    switch(info->family) {
        case F_WINDOWS_9X:
            return install_loader_9x(info);
        case F_WINDOWS_NT:
            return install_loader_nt(info);
        case F_WINDOWS_VISTA:
            return install_loader_vista(info);
        default:
            return 0;
    }
}

int uninstall_loader(win_info_t *info) {
    switch(info->family) {
        case F_WINDOWS_9X:
            return uninstall_loader_9x(info);
        case F_WINDOWS_NT:
            return uninstall_loader_nt(info);
        case F_WINDOWS_VISTA:
            return uninstall_loader_vista(info);
        default:
            return 0;
    }
}
