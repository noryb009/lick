#include <stdio.h>
#include <stdlib.h>

#include "system-info.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void version(sys_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");
    typedef int (WINAPI *getVersion)(LPOSVERSIONINFO lpVersionInfo);
    getVersion fn = GetProcAddress(k, "GetVersionEx");
    if(!fn) {fn = GetProcAddress(k, "GetVersionExA");}
    if(!fn) {fn = GetProcAddress(k, "GetVersionExW");}
    if(!fn) {v->version = V_UNKNOWN;FreeLibrary(k);return;}

    OSVERSIONINFO verInfo;
    memset(&verInfo, 0, sizeof(OSVERSIONINFO));
    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    fn(&verInfo);

    int M = verInfo.dwMajorVersion, m = verInfo.dwMinorVersion;

    if(M == 4 && m == 0) {
        v->version = V_WINDOWS_95;
    } else if(M == 4 && m == 10) {
        v->version = V_WINDOWS_98;
    } else if(M == 4 && m == 90) {
        v->version = V_WINDOWS_ME;
    } else if(M == 5 && m == 0) {
        v->version = V_WINDOWS_2000;
    } else if(M == 5 && (m == 1 || m == 2)) {
        v->version = V_WINDOWS_XP;
    } else if(M == 6 && m == 0) {
        v->version = V_WINDOWS_VISTA;
    } else if(M == 6 && m == 1) {
        v->version = V_WINDOWS_7;
    } else if(M == 6 && m == 2) {
        v->version = V_WINDOWS_8;
    } else if(M == 6 && m == 3) {
        v->version = V_WINDOWS_8_1;
    } else if(M == 10 && m == 0) {
        v->version = V_WINDOWS_10;
    } else {
        v->version = V_UNKNOWN;
    }
    FreeLibrary(k);
}

void arch(sys_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");

    typedef void (WINAPI *SysInfo)(LPSYSTEM_INFO lpSystemInfo);
    SysInfo fn = (SysInfo)GetProcAddress(k, "GetNativeSystemInfo");
    if(!fn) {v->arch = A_WINDOWS_X86; FreeLibrary(k); return;}

    SYSTEM_INFO sys_info;
    fn(&sys_info);
    switch(sys_info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            v->arch = A_WINDOWS_X86;
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            v->arch = A_WINDOWS_X86_64;
            break;
        default:
            v->arch = A_UNKNOWN;
    }
    FreeLibrary(k);
}

void admin(sys_info_t *v) {
    HMODULE s = LoadLibrary("Shell32.dll");

    typedef BOOL (WINAPI *IsAdmin)(void);
    IsAdmin fn = GetProcAddress(s, "IsUserAnAdmin");

    if(!fn) {v->is_admin = ADMIN_YES; FreeLibrary(s); return;}

    BOOL result = fn();
    if(result == TRUE) {
        v->is_admin = ADMIN_YES;
    } else {
        v->is_admin = ADMIN_NO;
    }
    FreeLibrary(s);
}

void bios(sys_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");

    typedef DWORD (WINAPI *GetVar)(LPCTSTR lpName, LPCTSTR lpGuid,
                                   PVOID pBuffer, DWORD nSize);
    GetVar fn = (GetVar)GetProcAddress(k, "GetFirmwareEnvironmentVariable");
    if(!fn) {fn = (GetVar)GetProcAddress(k, "GetFirmwareEnvironmentVariableA");}
    if(!fn) {fn = (GetVar)GetProcAddress(k, "GetFirmwareEnvironmentVariableW");}
    if(!fn) {v->is_bios = BIOS_BIOS; FreeLibrary(k); return;}

    typedef DWORD (WINAPI *GetError)(void);
    GetError errfn = (GetError)GetProcAddress(k, "GetLastError");

    fn("", "{00000000-0000-0000-0000-000000000000}", NULL, 0);
    if(!errfn || errfn() == ERROR_INVALID_FUNCTION) {
        v->is_bios = BIOS_BIOS;
    } else {
        v->is_bios = BIOS_UEFI;
    }
    FreeLibrary(k);
}
#endif

void family(sys_info_t *v) {
    switch(v->version) {
        case V_WINDOWS_95:
        case V_WINDOWS_98:
        case V_WINDOWS_ME:
            v->family = F_WINDOWS_9X;
            break;
        case V_WINDOWS_2000:
        case V_WINDOWS_XP:
            v->family = F_WINDOWS_NT;
            break;
        case V_WINDOWS_VISTA:
        case V_WINDOWS_7:
        case V_WINDOWS_8:
        case V_WINDOWS_8_1:
        case V_WINDOWS_10:
            v->family = F_WINDOWS_VISTA;
            break;
        default:
            v->family = F_UNKNOWN;
    }
}
char *version_name(version_e v) {
    switch(v) {
        case V_WINDOWS_95:
            return "Windows 95";
        case V_WINDOWS_98:
            return "Windows 98";
        case V_WINDOWS_ME:
            return "Windows ME";
        case V_WINDOWS_2000:
            return "Windows 2000";
        case V_WINDOWS_XP:
            return "Windows XP";
        case V_WINDOWS_VISTA:
            return "Windows Vista";
        case V_WINDOWS_7:
            return "Windows 7";
        case V_WINDOWS_8:
            return "Windows 8";
        case V_WINDOWS_8_1:
            return "Windows 8.1";
        case V_WINDOWS_10:
            return "Windows 10";
        default:
            return "Unknown";
    }
}

char *family_name(family_e f) {
    switch(f) {
        case F_WINDOWS_9X:
            return "Windows 9X";
        case F_WINDOWS_NT:
            return "Windows NT Family (Pre-Vista)";
        case F_WINDOWS_VISTA:
            return "Windows NT Family (Post-XP)";
        default:
            return "Unknown";
    }
}

char *arch_name(architecture_e a) {
    switch(a) {
        case A_WINDOWS_X86:
            return "32-bit";
        case A_WINDOWS_X86_64:
            return "64-bit";
        default:
            return "Unknown";
    }
}

char *bios_name(bios_type_e b) {
    switch(b) {
        case BIOS_BIOS:
            return "BIOS";
        case BIOS_UEFI:
            return "UEFI";
        default:
            return "Unknown";
    }
}

sys_info_t *get_system_info() {
    sys_info_t *v = malloc(sizeof(sys_info_t));
#ifdef _WIN32
    version(v);
    family(v);
    arch(v);
    admin(v);
    bios(v);
#else
    v->version = V_WINDOWS_95;
    family(v);
    v->arch = A_WINDOWS_X86;
    v->is_admin = ADMIN_YES;
    v->is_bios = BIOS_BIOS;
#endif
    v->version_name = version_name(v->version);
    v->family_name = family_name(v->family);
    v->arch_name = arch_name(v->arch);
    v->bios_name = bios_name(v->is_bios);
    return v;
}

void free_sys_info(sys_info_t *info) {
    free(info);
}
