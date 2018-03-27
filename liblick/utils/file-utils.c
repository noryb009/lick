#include "utils/file-utils.h"

#include <stdlib.h>
#include <string.h>

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

char *file_to_str(FILE *f) {
    size_t len = 0;
    rewind(f);
    while(getc(f) != EOF)
        len++;
    rewind(f);

    char *buf = malloc(len + 1);
    if(fread(buf, 1, len, f) < len) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';

    return buf;
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
