#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sddl.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

#include "scandir.h"
#include "utils.h"

int is_slash(char c) {
    return (c == '/' || c == '\\');
}

int make_dir(const char *d) {
    char d_name[strlen(d) + 1];
    unix_path(strcpy(d_name, d));
#ifdef _WIN32
        if(mkdir(d_name) == 0)
#else
        if(mkdir(d_name, S_IRWXU) == 0)
#endif
            return 1;
        else if(errno == EEXIST)
            return 1;
        return 0;
}

int make_dir_parents(const char *d) {
    size_t len = strlen(d);
    char *buf = strdup2(d);

    // strip trailing '/' and '\'
    while(len > 0 && is_slash(buf[len-1])) {
        --len;
        buf[len] = '\0';
    }

    for(char *p = buf + 1; *p != '\0'; ++p) {
        if(is_slash(*p)) {
            *p = '\0';
            make_dir(buf);
            *p = '/';
        }
    }

    int ret = make_dir(buf);
    free(buf);
    return ret;
}

int copy_file(const char *dst, const char *src) {
    char src_name[strlen(src) + 1];
    FILE *s = fopen(unix_path(strcpy(src_name, src)), "rb");
    if(!s)
        return 0;
    char dst_name[strlen(dst) + 1];
    FILE *d = fopen(unix_path(strcpy(dst_name, dst)), "wb");
    if(!d) {
        fclose(s);
        return 0;
    }

#define BUFFER_SIZE 512
    char buf[BUFFER_SIZE];
    while(1) {
        int c = fread(buf, 1, BUFFER_SIZE, s);
        if(c <= 0)
            break;
        if(fwrite(buf, 1, c, d) != c) {
            fclose(d);
            fclose(s);
            unlink_file(dst);
            return 0;
        }
    }
    fclose(d);
    fclose(s);
    return 1;
}

int unlink_dir(const char *d) {
    char d_name[strlen(d) + 1];
    return !rmdir(unix_path(strcpy(d_name, d)));
}

int unlink_dir_parents(const char *d) {
    size_t len = strlen(d);
    char *buf = unix_path(strdup2(d));
    int ret = 0;

    for(;;) {
        // strip trailing '/' and '\'
        while(len > 0 && is_slash(buf[len-1])) {
            --len;
            buf[len] = '\0';
        }

        if(len == 0 || !unlink_dir(buf)) {
            free(buf);
            return ret;
        }

        ret = 1;

        // strip trailing folder name
        while(len > 0 && !is_slash(buf[len-1])) {
            --len;
            buf[len] = '\0';
        }
    }
}

int unlink_file(const char *f) {
    char f_name[strlen(f) + 1];
    return !unlink(unix_path(strcpy(f_name, f)));
}

int unlink_recursive(const char *d) {
    struct dirent **e;
    int len = scandir2(d, &e, NULL, NULL);
    if(len < 0)
        return 0;
    for(int i = 0; i < len; ++i) {
        char *path = unix_path(concat_strs(3, d, "/", e[i]->d_name));
        if(file_type(path) == FILE_TYPE_DIR)
            unlink_recursive(path);
        else
            unlink_file(path);
        free(path);
        free(e[i]);
    }
    free(e);
    unlink_dir(d);
    return 1;
}

#ifdef _WIN32
#define PIPE_BUF_SIZE 256
void read_from_pipe(HANDLE pipe, char **out) {
    size_t size = PIPE_BUF_SIZE * 4;
    size_t used = 0;
    *out = malloc(size + 1);
    (*out)[0] = '\0';

    DWORD c_read;
    char buf[PIPE_BUF_SIZE];
    BOOL success = FALSE;
    for(;;) {
        success = ReadFile(pipe, buf, PIPE_BUF_SIZE, &c_read, NULL);
        if(!success || c_read == 0)
            break;
        while(used + c_read > size) {
            size *= 2;
            *out = realloc(*out, size + 1);
        }
        strncat(*out, buf, c_read);
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

char *strdup2(const char *s) {
    if(!s)
        return NULL;
    char *n = malloc(strlen(s) + 1);
    if(n) {
        strcpy(n, s);
    }
    return n;
}

char *strstrr(const char *haystack, const char *needle) {
    const char *last = haystack - 1;
    while(1) {
        const char *new_last = strstr(last + 1, needle);
        if(new_last)
            last = new_last;
        else
            break;
    }
    if(last == haystack - 1)
        return NULL;
    else
        return (char *)last;
}

char *concat_strs(int n, ...) {
    va_list args;
    int str_len = 1;
    int lens[n];

    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        lens[i] = strlen(va_arg(args, char*));
        str_len += lens[i];
    }
    va_end(args);

    char *s = malloc(str_len);
    s[0] = '\0';

    str_len = 0;
    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        strcpy(s + str_len, va_arg(args, char*));
        str_len += lens[i];
    }
    va_end(args);

    return s;
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
    getFolder fn = (getFolder)GetProcAddress(s, "SHGetFolderPath");

    if(fn) {
        FreeLibrary(s);

        char *path = get_windows_drive_path();
        if(!path)
            return NULL;
        char *config = concat_strs(2, path, "ProgramData");
        free(path);
        return config;
    }

    assert(sizeof(char) == sizeof(TCHAR));
    char *str = malloc(MAX_PATH);
    str[0] = '\0';
    HRESULT ret = fn(NULL, CSIDL_COMMON_APPDATA, NULL, 0, str);
    FreeLibrary(s);

    if(ret == S_OK)
        return str;

    free(str);
    return NULL;
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

char *normalize_path(char *str, char slash) {
    int last_was_slash = 0;
    for(int i = 0, j = 0;; ++i) {
        if(str[i] == '\0') {
            str[j] = '\0';
            break;
        } else if(is_slash(str[i])) {
            if(!last_was_slash) {
                last_was_slash = 1;
                str[j++] = slash;
            }
        } else {
            last_was_slash = 0;
            str[j++] = str[i];
        }
    }
    return str;
}

char *win_path(char *str) {
    return normalize_path(str, '\\');
}

char *unix_path(char *str) {
    return normalize_path(str, '/');
}

file_type_e file_type(const char *path) {
    struct stat s;
    if(stat(path, &s) != 0)
        return -1;
    if(S_ISDIR(s.st_mode))
        return FILE_TYPE_DIR;
    return FILE_TYPE_FILE;
}

int path_exists(const char *path) {
    char path2[strlen(path) + 1];
    struct stat s;
    return stat(unix_path(strcpy(path2, path)), &s) == 0;
}

#define LINE_SIZE_START 1024

char *read_line(FILE *f) {
    char *s = malloc(sizeof(char) * LINE_SIZE_START);
    int size = LINE_SIZE_START;

    for(int i = 0;; ++i) {
        if(i == size) {
            size *= 2;
            s = realloc(s, size);
        }

        int c = getc(f);
        if(c == EOF && i == 0) {
            free(s);
            return NULL;
        } else if(c == '\n' || c == EOF) {
            s[i] = '\0';
            return s;
        }

        s[i] = c;
    }
}

int is_conf_path(const char *name) {
    // if ends with .conf
    char *conf = strstr(name, ".conf");
    while(conf != NULL && strcmp(conf, ".conf") != 0) {
        conf = strstr(conf + 1, ".conf");
    }

    if(conf == NULL)
        return 0;
    return 1;
}

int is_space(char c) {
    return (c == ' ' || c == '\t');
}

void conf_option(char *ln, char **keyword_start, char **item_start) {
    *keyword_start = NULL;
    *item_start = NULL;

    int len = strlen(ln);
    int keyword_done = 0;
    for(int i = 0; i < len; i++) {
        int space = is_space(ln[i]);
        if(ln[i] == '#') // comment
            break;
        else if(*keyword_start == NULL && space) { // padding
        } else if(*keyword_start == NULL) // keyword
            *keyword_start = ln + i;
        else if(*item_start == NULL && space) { // space between
            ln[i] = '\0';
            keyword_done = 1;
        } else if(*item_start == NULL && keyword_done == 1) { // item
            *item_start = ln + i;
        }
    }

    if(*item_start != NULL) // has item
        // trim end padding
        for(int i = strlen(*item_start) - 1; i >= 0; i--) {
            if(is_space((*item_start)[i]))
                (*item_start)[i] = '\0';
            else
                break;
        }
}
