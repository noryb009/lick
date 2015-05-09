/**
 * @file
 * @brief Determine information about the system
 */

#pragma once

typedef enum {
    F_UNKNOWN,
    F_WINDOWS_9X,
    F_WINDOWS_NT,
    F_WINDOWS_VISTA,
} family_e;

typedef enum {
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
} version_e;

typedef enum {
    A_UNKNOWN,
    A_WINDOWS_X86,
    A_WINDOWS_X86_64,
} architecture_e;

typedef enum {
    ADMIN_UNKNOWN,
    ADMIN_YES,
    ADMIN_NO,
} is_admin_e;

typedef enum {
    BIOS_UNKNOWN,
    BIOS_BIOS,
    BIOS_UEFI,
} bios_type_e;

/**
 * @brief determines the name of the given windows version
 * @param v the windows version
 * @return the name of the version. Do not free this value
 */
char *version_name(version_e v);
/**
 * @brief determines the name of the given windows family
 * @param f the windows family
 * @return the name of the family. Do not free this value
 */
char *family_name(family_e f);
/**
 * @brief determines the name of the given architecture
 * @param a the architecture
 * @return the name of the architecture. Do not free this value
 */
char *arch_name(architecture_e a);
/**
 * @brief determines the name of the given BIOS type
 * @param b the BIOS type
 * @return the name of the BIOS type. Do not free this value
 */
char *bios_name(bios_type_e b);

/**
 * @brief relevant info about the system
 */
typedef struct {
    /// the windows version family
    family_e family;
    /// name of the windows version family
    char *family_name;
    /// the windows version
    version_e version;
    /// name of the windows version
    char *version_name;
    /// the architecture
    architecture_e arch;
    /// name of the architecture
    char *arch_name;
    /// the type of BIOS
    bios_type_e is_bios;
    /// name of the type of BIOS
    char *bios_name;
    /// does the process have administrator privileges
    is_admin_e is_admin;
} sys_info_t;

/**
 * @brief returns info about the system
 * @return info about the system. Must be freed using free
 */
sys_info_t *get_system_info();
