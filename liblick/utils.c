#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "scandir.h"
#include "utils.h"

int is_slash(char c) {
    return (c == '/' || c == '\\');
}

int make_dir(const char *d) {
#ifdef _WIN32
        if(mkdir(d) == 0)
#else
        if(mkdir(d, S_IRWXU) == 0)
#endif
            return 1;
        else if(errno == EEXIST)
            return 1;
        return 0;
}

int make_dir_parents(const char *d) {
    size_t len = strlen(d);
    char *buf = strdup(d);

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
    FILE *s = fopen(src, "rb");
    if(!s)
        return 0;
    FILE *d = fopen(dst, "wb");
    if(!d) {
        fclose(s);
        return 0;
    }

    while(1) {
        int c = fgetc(s);
        if(c == EOF)
            break;
        if(fputc(c, d) == EOF) {
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
    return !rmdir(d);
}
int unlink_file(const char *f) {
    return !unlink(f);
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

int run_system(const char *c) {
    //printf("Running command: %s\n", c);
    // TODO: silence
    return (system(c) == 0);
}

char *strdup(const char *s) {
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
    int len = 1;

    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        len += strlen(va_arg(args, char*));
    }
    va_end(args);

    char *s = malloc(len);
    s[0] = '\0';

    va_start(args, n);
    for(int i = 0; i < n; ++i) {
        strcat(s, va_arg(args, char*));
    }
    va_end(args);

    return s;
}

char *TCHAR_to_char(void *s, int len, int size) {
#ifdef _WIN32
    char *str = s;
    char *to = malloc(len + 1);
    for(int i = 0; i < len; ++i)
        to[i] = str[i*size];
    to[len] = '\0';
    return to;
#else
    return strdup((char *)s);
#endif
}

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
    struct stat s;
    return (stat(path, &s) == 0);
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
