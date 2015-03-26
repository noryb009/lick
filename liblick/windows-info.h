#pragma once

enum WINDOWS_FAMILY {
    F_WINDOWS_9X,
    F_WINDOWS_NT,
    F_WINDOWS_VISTA,
    F_UNKNOWN,
};

enum WINDOWS_VERSION {
    V_WINDOWS_95,
    V_WINDOWS_98,
    V_WINDOWS_ME,
    V_WINDOWS_2000,
    V_WINDOWS_XP,
    V_WINDOWS_VISTA,
    V_WINDOWS_7,
    V_WINDOWS_8,
    V_WINDOWS_8_1,
    V_UNKNOWN,
};

enum WINDOWS_ARCHITECTURE {
    A_WINDOWS_X86,
    A_WINDOWS_X86_64,
    A_UNKNOWN,
};

enum IS_ADMIN {
    ADMIN_YES,
    ADMIN_NO,
    ADMIN_UNKNOWN,
};

enum IS_BIOS {
    BIOS_BIOS,
    BIOS_UEFI,
    BIOS_UNKNOWN,
};

struct win_info_s {
    enum WINDOWS_FAMILY family;
    enum WINDOWS_VERSION version;
    enum WINDOWS_ARCHITECTURE arch;
    enum IS_ADMIN is_admin;
    enum IS_BIOS is_bios;
};
typedef struct win_info_s win_info_t;

win_info_t *get_windows_version_info();
