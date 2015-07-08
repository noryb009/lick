#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "vista.h"
#include "../drives.h"
#include "../lickdir.h"
#include "../menu/grub4dos.h"
#include "../utils.h"

// install
#define COMMAND_CREATE "%s /create /d \"Puppy Linux\" /application bootsector"
#define COMMAND_DEVICE "%s /set {%s} device partition=%c:"
#define COMMAND_PATH "%s /set {%s} path \\pupldr.mbr"
#define COMMAND_ADD_LAST "%s /displayorder {%s} /addlast"
#define COMMAND_TIME_OUT "%s /timeout 5"
#define COMMAND_BOOT_MENU "%s /set {default} bootmenupolicy legacy"
#define COMMAND_FAST_BOOT "powercfg -h off"

// uninstall
#define COMMAND_ENUM "%s /enum all"
#define COMMAND_DELETE "%s /delete {%s}"

int supported_loader_vista(sys_info_t *info) {
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
    case V_UNKNOWN:
    case V_WINDOWS_95:
    case V_WINDOWS_98:
    case V_WINDOWS_ME:
    case V_WINDOWS_2000:
    case V_WINDOWS_XP:
        break;
    }
    return 0;
}

int check_loader_vista(sys_info_t *info) {
    if(!supported_loader_vista(info)) {
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];
    char *bcdedit = get_bcdedit();
    if(!bcdedit)
        return 0;

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ENUM, bcdedit);
    free(bcdedit);
    return get_id_from_command_range(c, id, "----------", "pupldr.mbr");
}

int install_loader_vista(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_vista(info)) {
        return 0;
    }

    if(strlen(lick->res) < 2 || lick->res[1] != ':') {
        if(!lick->err)
            lick->err = strdup2("LICK is on an invalid drive");
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];
    char *bcdedit = get_bcdedit();
    if(!bcdedit) {
        if(!lick->err)
            lick->err = strdup2("Could not find bcdedit");
        return 0;
    }

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_CREATE, bcdedit);
    get_id_from_command(c, id);

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DEVICE, bcdedit, id, lick->drive[0]);
    if(!run_system(c)) {
        free(bcdedit);
        if(!lick->err)
            lick->err = strdup2("Error editing bcd device");
        return 0;
    }
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_PATH, bcdedit, id);
    if(!run_system(c)) {
        free(bcdedit);
        if(!lick->err)
            lick->err = strdup2("Error editing bcd path");
        return 0;
    }
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ADD_LAST, bcdedit, id);
    if(!run_system(c)) {
        free(bcdedit);
        if(!lick->err)
            lick->err = strdup2("Error adding to bcd menu");
        return 0;
    }
    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_TIME_OUT, bcdedit, id);
    if(!run_system(c)) {
        free(bcdedit);
        if(!lick->err)
            lick->err = strdup2("Error editing bcd time out");
        return 0;
    }

    switch(info->version) {
    case V_WINDOWS_8:
    case V_WINDOWS_8_1:
    //case V_WINDOWS_10: // TODO: test
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_BOOT_MENU, bcdedit, id);
        if(!run_system(c)) {
            free(bcdedit);
            if(!lick->err)
                lick->err = strdup2("Error changing boot menu to legacy");
            return 0;
        }
        snprintf(c, COMMAND_BUFFER_LEN, COMMAND_FAST_BOOT, id);
        if(!run_system(c)) {
            free(bcdedit);
            if(!lick->err)
                lick->err = strdup2("Error disabling fast boot");
            return 0;
        }
    case V_UNKNOWN:
    case V_WINDOWS_95:
    case V_WINDOWS_98:
    case V_WINDOWS_ME:
    case V_WINDOWS_2000:
    case V_WINDOWS_XP:
    case V_WINDOWS_VISTA:
    case V_WINDOWS_7:
        break;
    }
    free(bcdedit);

    // copy pupldr and pupldr.mbr
    char *pupldr_src = concat_strs(2, lick->res, "/pupldr");
    char *pupldr_dst = concat_strs(2, lick->drive, "/pupldr");
    copy_file(pupldr_dst, pupldr_src);
    free(pupldr_src);
    free(pupldr_dst);

    pupldr_src = concat_strs(2, lick->res, "/pupldr.mbr");
    pupldr_dst = concat_strs(2, lick->drive, "/pupldr.mbr");
    copy_file(pupldr_dst, pupldr_src);
    free(pupldr_src);
    free(pupldr_dst);

    return 1;
}

int uninstall_loader_vista(sys_info_t *info, lickdir_t *lick) {
    if(!supported_loader_vista(info)) {
        return 0;
    }

    char c[COMMAND_BUFFER_LEN];
    char id[ID_LEN];
    char *bcdedit = get_bcdedit();
    if(!bcdedit)
        return 0;

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_ENUM, bcdedit);
    get_id_from_command_range(c, id, "----------", "pupldr.mbr");

    snprintf(c, COMMAND_BUFFER_LEN, COMMAND_DELETE, bcdedit, id);
    free(bcdedit);
    if(!run_system(c)) {return 0;}

    char *pupldr = concat_strs(2, lick->drive, "/pupldr");
    char *pupldr_mbr = concat_strs(2, lick->drive, "/pupldr.mbr");
    unlink_file(pupldr);
    unlink_file(pupldr_mbr);
    free(pupldr);
    free(pupldr_mbr);
    return 1;
}

loader_t get_vista() {
    loader_t loader;
    loader.supported = supported_loader_vista;
    loader.check = check_loader_vista;
    loader.install = install_loader_vista;
    loader.uninstall = uninstall_loader_vista;
    loader.get_menu = get_grub4dos;
    return loader;
}
