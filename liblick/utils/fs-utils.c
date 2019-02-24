#include "utils/fs-utils.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "scandir.h"
#include "utils/string-utils.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

char *normalize_path(char *str, char slash) {
    int last_was_slash = 0;
    for(int i = 0, j = 0;; ++i) {
        if(str[i] == '\0') {
            str[j] = '\0';
            break;
        } else if(is_slash(str[i])) {
            if(!last_was_slash) {
                // Windows can have network paths, which start with '\\'
                //   (two backslashes, not an escaped backslash)
                if(i != 0)
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
    char dst_name[strlen(dst) + 1];
    unix_path(strcpy(src_name, src));
    unix_path(strcpy(dst_name, dst));
#ifdef _WIN32
    if (CopyFile(src_name, dst_name, 0) != 0) {
        return 1;
    } else {
        return 0;
    }
#else
    FILE *s = fopen(src_name, "rb");
    if(!s)
        return 0;
    FILE *d = fopen(dst_name, "wb");
    if(!d) {
        fclose(s);
        return 0;
    }

#define BUFFER_SIZE 512
    char buf[BUFFER_SIZE];
    while(1) {
        size_t c = fread(buf, 1, BUFFER_SIZE, s);
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
#endif
}

int rename_file(const char *dst, const char *src) {
    return !rename(src, dst);
}

int replace_file(const char *dst, const char *src) {
#ifdef _WIN32
    static_assert(sizeof(char) == sizeof(TCHAR), "Unicode is not supported");

    HMODULE a = LoadLibrary("Kernel32.dll");
    if(!a)
        return 0;
    typedef BOOL (WINAPI *rep_file)(LPCTSTR, LPCTSTR, LPCTSTR, DWORD, LPVOID, LPVOID);
    rep_file fn_rep = GetProcAddress(a, "ReplaceFileA");

    int res = fn_rep(dst, src, NULL, 0, NULL, NULL);
    FreeLibrary(a);
    return res;
#else
    return rename_file(dst, src);
#endif
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

#ifdef _WIN32
attrib_t attrib_get(const char *file) {
    return GetFileAttributes(file);
}

attrib_t attrib_open(const char *file) {
    attrib_t attrib = attrib_get(file);
    SetFileAttributes(file, FILE_ATTRIBUTE_NORMAL);
    return attrib;
}
void attrib_save(const char *file, attrib_t attrib) {
    SetFileAttributes(file, attrib);
}
#else
attrib_t attrib_get(const char *file) {
    (void)file;
    return 0;
}
attrib_t attrib_open(const char *file) {
    (void)file;
    return 0;
}
void attrib_save(const char *file, attrib_t attrib) {
    (void)file;
    (void)attrib;
    return;
}
#endif
