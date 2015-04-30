#include <stdlib.h>
#include <string.h>

#include "vista.h"
#include "../drives.h"
#include "../utils.h"


#define ID_LEN 37
#define COMMAND_BUFFER_LEN 256

// install
#define COMMAND_CREATE "bcdedit /create /d \"Puppy Linux\" /application bootsector"
#define COMMAND_DEVICE "bcdedit /set {%s} device \"partition=%s\""
#define COMMAND_PATH "bcdedit /set {%s} path %s%s"
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

int supported_loader_vista(win_info_t *info) {
    if(info->family != F_WINDOWS_VISTA || info->is_bios != BIOS_BIOS) {
        return 0;
    }
    switch(info->version) {
        case V_WINDOWS_VISTA:
        case V_WINDOWS_7:
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
        //case V_WINDOWS_10: // TODO: test
            return 1;
    }
    return 0;
}

int check_loader_vista(win_info_t *info) {
    if(!supported_loader_vista(info)) {
        return 0;
    }

    char id[ID_LEN];

    return get_id_from_command_range(COMMAND_ENUM, id, "----------", "pupldr.mbr");
}

int install_loader_vista(win_info_t *info, char *lick_res_dir) {
    if(!supported_loader_vista(info)) {
        return 0;
    }

    if(strlen(lick_res_dir) < 2 || lick_res_dir[1] != ':')
        return 0;

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    char *drive = "C:";
    drive[0] = lick_res_dir[0];
    char lick_res_dir_path[strlen(lick_res_dir)+1];
    strcpy(lick_res_dir_path, lick_res_dir + 2);

    get_id_from_command(COMMAND_CREATE, id);

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DEVICE, id, drive);
    if(!system(c)) {return 0;}
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_PATH, id,
            lick_res_dir_path, "\\pupldr.mbr");
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

int uninstall_loader_vista(win_info_t *info, char *lick_res_dir) {
    if(!supported_loader_vista(info)) {
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];

    get_id_from_command_range(COMMAND_ENUM, id, "----------", "pupldr.mbr");

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, id);
    if(!system(c)) {return 0;}
}

loader_t get_vista() {
    loader_t loader;
    loader.supported = supported_loader_vista;
    loader.check = check_loader_vista;
    loader.install = install_loader_vista;
    loader.uninstall = uninstall_loader_vista;
    return loader;
}
