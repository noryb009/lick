#pragma once

enum WINDOWS_FAMILY {
    F_UNKNOWN,
    F_WINDOWS_9X,
    F_WINDOWS_NT,
    F_WINDOWS_VISTA,
};

enum WINDOWS_VERSION {
    V_UNKNOWN,
    V_WINDOWS_95,
    V_WINDOWS_98,
    V_WINDOWS_ME,
    V_WINDOWS_2000,
    V_WINDOWS_XP,
    V_WINDOWS_VISTA,
    V_WINDOWS_7,
    V_WINDOWS_8,
    V_WINDOWS_8_1,
    V_WINDOWS_10,
};

enum WINDOWS_ARCHITECTURE {
    A_UNKNOWN,
    A_WINDOWS_X86,
    A_WINDOWS_X86_64,
};

enum IS_ADMIN {
    ADMIN_UNKNOWN,
    ADMIN_YES,
    ADMIN_NO,
};

enum IS_BIOS {
    BIOS_UNKNOWN,
    BIOS_BIOS,
    BIOS_UEFI,
};

char *version_name(enum WINDOWS_VERSION v);
char *family_name(enum WINDOWS_FAMILY f);
char *arch_name(enum WINDOWS_ARCHITECTURE a);
char *bios_name(enum IS_BIOS b);

typedef struct {
    enum WINDOWS_FAMILY family;
    char *family_name;
    enum WINDOWS_VERSION version;
    char *version_name;
    enum WINDOWS_ARCHITECTURE arch;
    char *arch_name;
    enum IS_BIOS is_bios;
    char *bios_name;
    enum IS_ADMIN is_admin;
} win_info_t;

win_info_t get_windows_version_info();
