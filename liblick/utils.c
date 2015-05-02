#include "utils.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int makeDir(const char *d) {
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

int makeDirR(const char *d) {
    size_t len = strlen(d);
    char *buf = strdup(d);

    // strip trailing '/' and '\'
    while(len > 0 && (buf[len-1] == '/' || buf[len-1] == '\\')) {
        --len;
        buf[len] = '\0';
    }

    for(char *p = buf + 1; *p != '\0'; ++p) {
        if(*p == '/' || *p == '\\') {
            *p = '\0';
            makeDir(buf);
            *p = '/';
        }
    }

    int ret = makeDir(buf);
    free(buf);
    return ret;
}

int unlinkDir(const char *d) {
    return !rmdir(d);
}
int unlinkFile(const char *f) {
    return !unlink(f);
}

char *strdup(const char *s) {
    char *n = malloc(strlen(s) + 1);
    if(n) {
        strcpy(n, s);
    }
    return n;
}

int is_slash(char c) {
    return (c == '/' || c == '\\');
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

// 1 = yes, 0 = no, -1 = error
int is_file(char *path) {
    struct stat s;
    if(stat(path, &s) != 0)
        return -1;
    if(S_ISDIR(s.st_mode))
        return 0;
    return 1;
}

int file_exists(char *path) {
    FILE *f = fopen(path, "r");
    if(!f)
        return 0;
    fclose(f);
    return 1;
}

#define LINE_SIZE_START 1024

char *read_line(FILE *f, int *done) {
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
            *done = 1;
            return NULL;
        } else if(c == '\n' || c == EOF) {
            s[i] = '\0';
            return s;
        }

        s[i] = c;
    }
}

int is_conf_file(const char *name) {
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
        } else if(*item_start == NULL && keyword_done == 1) // item
            *item_start = ln + i;
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
