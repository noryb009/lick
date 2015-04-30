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

char *file_to_str(FILE *f) {
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';

    return buf;
}

char *advance_to_newline(char *s) {
    while(s[0] != '\0' && s[0] != '\n')
        s++;
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
