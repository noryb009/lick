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

// TODO: C:/
char *dirname(const char *p) {
    if(strcmp(p, "/") == 0) {
        return strdup("/");
    }
    int l = strlen(p);
    while(l > 0 && is_slash(p[l-1])) {l--;} // trailing /
    while(l > 0 && !is_slash(p[l-1])) {l--;} // name
    while(l > 1 && is_slash(p[l-1])) {l--;} // trailing /

    if(l == 0) {
        return strdup(".");
    }
    char *n = malloc(l + 1);
    strncpy(n, p, l);
    n[l] = '\0';
    return n;
}

// TODO: C:/
char *basename(const char *p) {
    int l = strlen(p);
    while(l > 0 && is_slash(p[l-1])) {l--;}

    if(l == 0) {
        return strdup("/");
    }

    int s = l;
    while(s > 0 && !is_slash(s)) {s--;}

    char *n = malloc(l - s + 1);
    strncpy(n, p + s, l - s);
    n[l - s] = '\0';
    return n;
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
