#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "../utils.h"

entry_t *new_entry() {
    entry_t *e = malloc(sizeof(entry_t));
    e->title = NULL;
    e->kernel = NULL;
    e->initrd = NULL;
    e->options = NULL;
    e->static_text = NULL;
    return e;
}

entry_t *get_entry(FILE *f) {
    entry_t *e = NULL;
    fpos_t pos;

    while(1) {
        fgetpos(f, &pos);
        char *ln = read_line(f);
        if(ln == NULL)
            break;

        char *keyword_start;
        char *item_start;
        conf_option(ln, &keyword_start, &item_start);

        if(keyword_start == NULL) { // nothing found
            free(ln);
            continue;
        }

        enum type_e {
            TITLE,
            KERNEL,
            INITRD,
            OPTIONS,
            STATIC,
        } type;

        if(strcmp(keyword_start, "title") == 0)
            type = TITLE;
        else if(strcmp(keyword_start, "kernel") == 0)
            type = KERNEL;
        else if(strcmp(keyword_start, "initrd") == 0)
            type = INITRD;
        else if(strcmp(keyword_start, "options") == 0)
            type = OPTIONS;
        else if(strcmp(keyword_start, "static") == 0)
            type = STATIC;
        else {
            free(ln);
            continue;
        }

        if(e == NULL)
            e = new_entry();

        char **target;
        switch(type) {
            case TITLE:
                target = &(e->title);
                break;
            case KERNEL:
                target = &(e->kernel);
                break;
            case INITRD:
                target = &(e->initrd);
                break;
            case OPTIONS:
                target = &(e->options);
                break;
            case STATIC:
                target = &(e->static_text);
                break;
        }

        // if item already found, current entry complete; unread line
        if(*target != NULL) {
            free(ln);
            if(!fsetpos(f, &pos)) {
                // unlikely to happen
                free(e);
                return NULL;
            }
            return e;
        }

        *target = strdup(item_start);
        free(ln);

        if((e->title && e->kernel && e->initrd && e->options) || e->static_text)
            return e;
    }

    return e;
}

void free_entry(entry_t *e) {
    if(e == NULL)
        return;
    if(e->title)
        free(e->title);
    if(e->kernel)
        free(e->kernel);
    if(e->initrd)
        free(e->initrd);
    if(e->options)
        free(e->options);
    if(e->static_text)
        free(e->static_text);
    free(e);
}

char *to_menu_path(char *path) {
    char *start = path;
    char *colon = strchr(path, ':');
    if(colon != NULL)
        start = colon + 1;

    char *ret = strdup(start);
    for(char *i = ret; i[0] != '\0'; ++i)
        if(i[0] == '\\')
            i[0] = '/';

    return ret;
}
