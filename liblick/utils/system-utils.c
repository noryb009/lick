#include <assert.h>
#include <stdlib.h>
#include "fs-utils.h"
#include "string-utils.h"
#include "system-utils.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sddl.h>
#include <shellapi.h>
#include <shlobj.h>

#define PIPE_BUF_SIZE 256
void read_from_pipe(HANDLE pipe, char **out) {
    size_t size = PIPE_BUF_SIZE * 4;
    size_t used = 0;
    *out = malloc(size + 1);
    (*out)[used] = '\0';

    DWORD c_read;
    char buf[PIPE_BUF_SIZE];
    BOOL success = FALSE;
    for(;;) {
        success = ReadFile(pipe, buf, PIPE_BUF_SIZE, &c_read, NULL);
        if(!success || c_read == 0)
            break;
        if(used + c_read > size) {
            size *= 2;
            *out = realloc(*out, size + 1);
        }
        strncpy((*out) + used, buf, c_read);
        used += c_read;
        (*out)[used] = '\0';
    }
}

int create_pipes(STARTUPINFO *s, HANDLE *p_output_read, HANDLE *p_input_write) {
    HANDLE p_input_read;
    HANDLE p_output_write;
    HANDLE p_error_write;
    HANDLE p_output_read_tmp, p_input_write_tmp;

    SECURITY_ATTRIBUTES sec;
    memset(&sec, 0, sizeof(sec));
    sec.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec.bInheritHandle = TRUE;
    sec.lpSecurityDescriptor = NULL;

    if(!CreatePipe(&p_output_read_tmp, &p_output_write, &sec, 0))
        return 0;
    if(!CreatePipe(&p_input_read, &p_input_write_tmp, &sec, 0)) {
        CloseHandle(p_output_read_tmp);
        CloseHandle(p_output_write);
        return 0;
    }
    if(!DuplicateHandle(GetCurrentProcess(), p_output_write,
                GetCurrentProcess(), &p_error_write, 0, TRUE,
                DUPLICATE_SAME_ACCESS)
            || !DuplicateHandle(GetCurrentProcess(), p_output_read_tmp,
                GetCurrentProcess(), p_output_read, 0, FALSE,
                DUPLICATE_SAME_ACCESS)
            || !DuplicateHandle(GetCurrentProcess(), p_input_write_tmp,
                GetCurrentProcess(), p_input_write, 0, FALSE,
                DUPLICATE_SAME_ACCESS)) {
        CloseHandle(p_output_read_tmp);
        CloseHandle(p_input_write_tmp);
        CloseHandle(p_input_read);
        CloseHandle(p_output_write);
        return 0;
    }
    CloseHandle(p_output_read_tmp);
    CloseHandle(p_input_write_tmp);

    s->hStdInput = p_input_read;
    s->hStdOutput = p_output_write;
    s->hStdError = p_error_write;
    s->dwFlags |= STARTF_USESTDHANDLES;
    return 1;
}

int unprivileged_token(HANDLE *token, PSID *sid) {
    typedef BOOL (WINAPI *open_p_tok)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
    typedef BOOL (WINAPI *dup_token)(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES lpTokenAttributes, SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken);
    typedef BOOL (WINAPI *conv_sid)(LPCTSTR StringSid, PSID *Sid);

    HMODULE a = LoadLibrary("Advapi32.dll");
    if(!a)
        return -1;
    open_p_tok fn_open = GetProcAddress(a, "OpenProcessToken");
    dup_token fn_dup = GetProcAddress(a, "DuplicateTokenEx");
    conv_sid fn_conv = GetProcAddress(a, "ConvertStringSidToSidA");
    if(!fn_open || !fn_dup || !fn_conv) {
        FreeLibrary(a);
        return -1;
    }

    char *med_integrity = "S-1-16-8192";
    HANDLE t;
    TOKEN_MANDATORY_LABEL TIL = {0};

    if(!fn_open(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &t)
            || !fn_dup(t, 0, NULL, SecurityImpersonation, TokenPrimary, token)
            || !fn_conv(med_integrity, sid)) {
        FreeLibrary(a);
        return 0;
    }
    FreeLibrary(a);

    TIL.Label.Attributes = SE_GROUP_INTEGRITY;
    TIL.Label.Sid = *sid;
    SetTokenInformation(*token, TokenIntegrityLevel, &TIL, sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(med_integrity));
    return 1;
}

int run_unprivileged(const char *c, void *input, void *output) {
    STARTUPINFO s;
    PROCESS_INFORMATION p;
    HANDLE *in = (HANDLE*)input;
    HANDLE *out = (HANDLE*)output;
    PSID sid;
    HANDLE token;
    int token_supported = 1;

    int tok_ret = unprivileged_token(&token, &sid);
    if(tok_ret == 0)
        return 0;
    else if(tok_ret == -1) { // not supported
        token_supported = 0;
    }

    memset(&s, 0, sizeof(s));
    s.cb = sizeof(s);

    if(!create_pipes(&s, out, in)) {
        if(token_supported) {
            LocalFree(sid);
            CloseHandle(token);
        }
        return 0;
    }

    char *command = strdup2(c);
    DWORD ret;
    if(token_supported) {
        typedef BOOL (WINAPI *create_as_user)(HANDLE hToken, LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFO lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
        HANDLE a = LoadLibrary("Advapi32.dll");
        if(!a)
            ret = 0;
        else {
            create_as_user fn = GetProcAddress(a, "CreateProcessAsUserA");
            if(!fn)
                ret = 0;
            else
                ret = fn(token, NULL, command, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 0, 0, &s, &p);
        }
    } else
        ret = CreateProcess(NULL, command, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 0, 0, &s, &p);
    free(command);
    CloseHandle(s.hStdInput);
    CloseHandle(s.hStdOutput);
    CloseHandle(s.hStdError);
    if(token_supported) {
        LocalFree(sid);
        CloseHandle(token);
    }
    if(!ret)
        return 0;
    CloseHandle(p.hProcess);
    CloseHandle(p.hThread);
    return 1;
}

int run_privileged(const char *c, const char *p, int *ret) {
    SHELLEXECUTEINFO info = {0};
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.hwnd = NULL;
    info.lpVerb = "runas";
    info.lpFile = c;
    info.lpParameters = p;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWDEFAULT;
    info.hInstApp = NULL;
    if(!ShellExecuteEx(&info))
        return 0;
    WaitForSingleObject(info.hProcess, INFINITE);
    DWORD process_ret;
    DWORD success = GetExitCodeProcess(info.hProcess, &process_ret);
    CloseHandle(info.hProcess);
    if(ret)
        *ret = process_ret;
    return success;
}

int run_system_output(const char *c, char **out) {
    //printf("Running command: %s\n", c);
    STARTUPINFO s;
    PROCESS_INFORMATION p;
    HANDLE p_output, p_input;

    memset(&s, 0, sizeof(s));
    s.cb = sizeof(s);

    if(out != NULL) {
        if(!create_pipes(&s, &p_output, &p_input))
            return 0;
    }

    char *command = strdup2(c);
    if(!CreateProcess(NULL, command, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 0, 0, &s, &p)) {
        free(command);
        if(out != NULL) {
            CloseHandle(s.hStdInput);
            CloseHandle(s.hStdOutput);
            CloseHandle(s.hStdError);
        }
        return 0;
    }

    if(out != NULL) {
        CloseHandle(s.hStdInput);
        CloseHandle(s.hStdOutput);
        CloseHandle(s.hStdError);

        read_from_pipe(p_output, out);
        CloseHandle(p_output);
        CloseHandle(p_input);
    }
    WaitForSingleObject(p.hProcess, INFINITE);
    DWORD process_ret;
    DWORD ret = GetExitCodeProcess(p.hProcess, &process_ret);
    CloseHandle(p.hProcess);
    CloseHandle(p.hThread);
    free(command);

    if(!ret)
        return 0;
    else
        return process_ret == 0;
}
#else
int run_system_output(const char *c, char **out) {
    if(out != NULL)
        *out = NULL;
    return (system(c) == 0);
}
int run_unprivileged(const char *c, void *input, void *output) {
    return run_system_output(c, NULL);
}
#endif

int run_system(const char *c) {
    return run_system_output(c, NULL);
}

#ifdef _WIN32
char *get_windows_path() {
    typedef UINT (WINAPI *get_dir)(LPTSTR lpBuffer, UINT uSize);
    HMODULE k = LoadLibrary("Kernel32.dll");
    if(!k)
        return NULL;
    get_dir fn = (get_dir)GetProcAddress(k, "GetSystemWindowsDirectoryA");
    if(!fn) fn = (get_dir)GetProcAddress(k, "GetWindowsDirectoryA");
    if(!fn) {
        FreeLibrary(k);
        return NULL;
    }
    assert(sizeof(char) == sizeof(TCHAR));
    char buf[256];
    int len = fn(buf, 255);
    if(len == 0) {
        FreeLibrary(k);
        return NULL;
    }
    if(len < 255) {
        FreeLibrary(k);
        return strdup2(buf);
    }

    // not big enough
    char buf2[len + 1];
    len = fn(buf2, len + 1);
    FreeLibrary(k);
    if(len == 0)
        return NULL;
   return strdup2(buf2);
}

char *get_windows_drive_path() {
    char *path = get_windows_path();
    if(!path)
        return NULL;

    char *drv = malloc(3 + 1);
    strcpy(drv, "?:/");
    drv[0] = path[0];

    free(path);
    return drv;
}

char *get_program_path() {
    int size = 128;
    int first = 1;
    int ret_size;

    assert(sizeof(char) == sizeof(TCHAR));
    char *buf = malloc(size);

    while(first || ret_size == size) {
        if(first)
            first = 0;
        else {
            size *= 2;
            buf = realloc(buf, size);
        }
        ret_size = GetModuleFileName(NULL, buf, size);
    }
    return buf;
}

char *get_config_path() {
    HMODULE s = LoadLibrary("Shell32.dll");

    typedef HRESULT (WINAPI *getFolder)(HWND hwndOwner, int nFolder,
            HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);
    getFolder fn = (getFolder)GetProcAddress(s, "SHGetFolderPathA");

    if(!fn) {
        FreeLibrary(s);

        char *path = get_windows_drive_path();
        if(!path)
            return NULL;
        char *config = concat_strs(2, path, "ProgramData/lick");
        free(path);
        return config;
    }

    assert(sizeof(char) == sizeof(TCHAR));
    char *str = malloc(MAX_PATH);
    str[0] = '\0';
    HRESULT ret = fn(NULL, CSIDL_COMMON_APPDATA, NULL, 0, str);
    FreeLibrary(s);

    char *ret_str = NULL;
    if(ret == S_OK)
        ret_str = unix_path(concat_strs(2, str, "/lick"));

    free(str);
    return ret_str;
}

const char *get_command_line() {
    return GetCommandLine();
}
#else
char *get_windows_path() {
    return strdup2("/");
}
char *get_windows_drive_path() {
    return strdup2("/");
}
char *get_program_path() {
    return strdup2("./lick-fltk");
}
char *get_config_path() {
    return strdup2("/lick");
}
const char *get_command_line() {
    return "";
}
#endif
