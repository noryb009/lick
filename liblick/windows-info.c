#include <stdio.h>
#include <stdlib.h>

#include "windows-info.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void version(win_info_t *v);
void arch(win_info_t *v);
void admin(win_info_t *v);
void bios(win_info_t *v);

void version(win_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");
    typedef int (WINAPI *getVersion)(LPOSVERSIONINFO lpVersionInfo);
    getVersion fn = GetProcAddress(k, "GetVersionEx");
    if(!fn) {fn = GetProcAddress(l, "GetVersionExA");}
    if(!fn) {fn = GetProcAddress(l, "GetVersionExW");}
    if(!fn) {v->version = V_UNKNOWN;return;}

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
    } else {
        v->version = V_UNKNOWN;
    }

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
            v->family = F_WINDOWS_VISTA;
            break;
        default:
            v->family = F_UNKNOWN;
    }
}

void arch(win_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");

    typedef void (WINAPI *SysInfo)(LPSYSTEM_INFO lpSystemInfo);
    SysInfo fn = GetProcAddress(k, "GetNativeSystemInfo");
    if(!fn) {v->arch = A_WINDOWS_X86; return;}

    SYSTEM_INFO sys_info;
    fn(&sys_info);
    switch(sys_info.wProcessorArchitecture)
        case: PROCESSOR_ARCHITECTURE_INTEL:
            v->arch = A_WINDOWS_X86;
            break;
        case: PROCESSOR_ARCHITECTURE_AMD64:
            v->arch = A_WINDOWS_X86_64;
            break;
        default:
            v->arch = A_UNKNOWN;
    }
}

void admin(win_info_t *v) {
    HMODULE s = LoadLibrary("Shell32.dll");

    typedef BOOL (WINAPI *IsAdmin)(void);
    IsAdmin fn = GetProcAddress(shell32, "IsUserAnAdmin");

    if(!fn) {v->is_admin = ADMIN_YES; return;}

    BOOL result = fn();
    if(result == TRUE) {
        v->is_admin = ADMIN_YES;
    } else {
        v->is_admin = ADMIN_NO;
    }
}

void bios(win_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");

    typedef DWORD (WINAPI *GetVar)(LPCTSTR lpName, LPCTSTR lpGuid, PVOID pBuffer, DWORD nSize);
    GetVar fn = GetProcAddress(k, "GetFirmwareEnvironmentVariable");
    if(!fn) {fn = GetProcAddress(k, "GetFirmwareEnvironmentVariableA");}
    if(!fn) {fn = GetProcAddress(k, "GetFirmwareEnvironmentVariableW");}
    if(!fn) {v->is_bios = BIOS_BIOS;}

    typedef DWORD (WINAPI *GetError)(void);
    GetError errfn = GetProcAddress(k, "GetLastError");

    fn("", "{00000000-0000-0000-0000-000000000000}", NULL, 0);
    if(!getError || getError() == ERROR_INVALID_FUNCTION) {
        v->is_bios = BIOS_BIOS;
    } else {
        v->is_bios = BIOS_UEFI;
    }
}
#endif

win_info_t *get_windows_version_info() {
    win_info_t *v = malloc(sizeof(win_info_t));
#ifdef _WIN32
    version(v);
    arch(v);
    admin(v);
    bios(v);
#else
    v->version = V_WINDOWS_XP;
    v->family = F_WINDOWS_NT;
    v->arch = A_WINDOWS_X86;
    v->is_admin = ADMIN_YES;
    v->is_bios = BIOS_BIOS;
#endif
    return v;
}
