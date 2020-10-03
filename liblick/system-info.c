#include "system-info.h"

#include <stdio.h>
#include <stdlib.h>

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

    int M = verInfo.dwMajorVersion;
    int m = verInfo.dwMinorVersion;
    int p = verInfo.dwPlatformId;

    if(M == 4 && m == 0 && p != 2)
        v->version = V_WINDOWS_95;
    else if(M == 4 && m == 10)
        v->version = V_WINDOWS_98;
    else if(M == 4 && m == 90)
        v->version = V_WINDOWS_ME;
    else if(M == 4 && p == 2)
        v->version = V_WINDOWS_NT;
    else if(M == 5 && m == 0)
        v->version = V_WINDOWS_2000;
    else if(M == 5 && (m == 1 || m == 2))
        v->version = V_WINDOWS_XP;
    else if(M == 6 && m == 0)
        v->version = V_WINDOWS_VISTA;
    else if(M == 6 && m == 1)
        v->version = V_WINDOWS_7;
    else if(M == 6 && m == 2)
        v->version = V_WINDOWS_8;
    else if(M == 6 && m == 3)
        v->version = V_WINDOWS_8_1;
    else if(M == 10 && m == 0)
        v->version = V_WINDOWS_10;
    else
        v->version = V_UNKNOWN;
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

typedef DWORD (WINAPI *GetLastErrorT)(void);

// Based on:
// https://github.com/microsoft/Windows-classic-samples/blob/1d363ff4bd17d8e20415b92e2ee989d615cc0d91/Samples/ManagementInfrastructure/cpp/Process/Provider/WindowsProcess.c#L49
void enable_se_env_name() {
    typedef BOOL (WINAPI *LookupPrivilegeValueT)(
        LPCSTR lpSystemName, LPCSTR lpName, PLUID lpLuid);
    typedef BOOL (WINAPI *OpenProcessTokenT)(
        HANDLE processHandle, DWORD desiredAccess, PHANDLE tokenHandle);
    typedef HANDLE (WINAPI *GetCurrentProcessT)();
    typedef BOOL (WINAPI *OpenProcessTokenT)(
        HANDLE processHandle, DWORD desiredAccess, PHANDLE tokenHandle);
    typedef BOOL (WINAPI *GetTokenInformationT)(
        HANDLE TokenHandle,
        TOKEN_INFORMATION_CLASS TokenInformationClass,
        LPVOID TokenInformation,
        DWORD TokenInformationLength,
        PDWORD ReturnLength);
    typedef BOOL (WINAPI *AdjustTokenPrivilegesT)(
        HANDLE TokenHandle,
        BOOL DisableAllPrivileges,
        PTOKEN_PRIVILEGES NewState,
        DWORD BufferLength,
        PTOKEN_PRIVILEGES PreviousState,
        PDWORD ReturnLength);

    HMODULE kernel32 = LoadLibrary("Kernel32.dll");
    if (!kernel32) {
        return;
    }
    HMODULE advapi32 = LoadLibrary("Advapi32.dll");
    if (!advapi32) {
        CloseHandle(kernel32);
        return;
    }

    LookupPrivilegeValueT lookup_privilege_value =
        (LookupPrivilegeValueT)GetProcAddress(advapi32, "LookupPrivilegeValueA");
    OpenProcessTokenT open_process_token =
        (OpenProcessTokenT)GetProcAddress(advapi32, "OpenProcessToken");
    GetCurrentProcessT get_current_process =
        (GetCurrentProcessT)GetProcAddress(kernel32, "GetCurrentProcess");
    GetTokenInformationT get_token_information =
        (GetTokenInformationT)GetProcAddress(advapi32, "GetTokenInformation");
    AdjustTokenPrivilegesT adjust_token_privileges =
        (AdjustTokenPrivilegesT)GetProcAddress(advapi32, "AdjustTokenPrivileges");

    bool have_token = false;
    HANDLE token;
    unsigned char *token_bytes = NULL;

    do {
        if (!lookup_privilege_value
            || !open_process_token
            || !get_current_process
            || !get_token_information
            || !adjust_token_privileges) {
            break;
        }

        LUID reqPrivilege;
        if (!lookup_privilege_value(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &reqPrivilege)) {
            break;
        }

        if (!open_process_token(get_current_process(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &token)) {                    
            break;
        }
        have_token = true;

        DWORD len;
        if (get_token_information(token, TokenPrivileges, NULL, 0, &len)) {
            break;
        }

        token_bytes = malloc(len);

        if (!get_token_information(token, TokenPrivileges, token_bytes, len, &len)) {
            break;
        }

        TOKEN_PRIVILEGES *privs = (TOKEN_PRIVILEGES *)token_bytes;
        bool found = false;
        for (size_t i = 0; i < privs->PrivilegeCount; ++i) {
            if (privs->Privileges[i].Luid.LowPart == reqPrivilege.LowPart
                  && privs->Privileges[i].Luid.HighPart == reqPrivilege.HighPart) {
                found = true;
                privs->Privileges[i].Attributes |= SE_PRIVILEGE_ENABLED;
                adjust_token_privileges(token, false, privs, len, NULL, NULL);
                break;
            }
        }
    } while (false);

    if (token_bytes) {
        free(token_bytes);
    }
    if (have_token) {
        CloseHandle(token);
    }
    CloseHandle(kernel32);
    CloseHandle(advapi32);
}

void bios(sys_info_t *v) {
    HMODULE k = LoadLibrary("Kernel32.dll");

    typedef DWORD (WINAPI *GetVar)(LPCTSTR lpName, LPCTSTR lpGuid,
                                   PVOID pBuffer, DWORD nSize);
    GetVar fn = (GetVar)GetProcAddress(k, "GetFirmwareEnvironmentVariable");
    if(!fn) {fn = (GetVar)GetProcAddress(k, "GetFirmwareEnvironmentVariableA");}
    if(!fn) {fn = (GetVar)GetProcAddress(k, "GetFirmwareEnvironmentVariableW");}
    if(!fn) {v->is_bios = BIOS_BIOS; FreeLibrary(k); return;}

    GetLastErrorT errfn = (GetLastErrorT)GetProcAddress(k, "GetLastError");

    fn("", "{00000000-0000-0000-0000-000000000000}", NULL, 0);
    if(errfn != NULL && errfn() == ERROR_PRIVILEGE_NOT_HELD) {
        enable_se_env_name();
        fn("", "{00000000-0000-0000-0000-000000000000}", NULL, 0);
    }

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
    case V_WINDOWS_NT:
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
    case V_WINDOWS_NT:
        return "Windows NT 4.0";
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
