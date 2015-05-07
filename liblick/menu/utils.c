#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "../scandir.h"
#include "../utils.h"

int is_conf(const struct dirent *e) {
    return is_conf_file(e->d_name);
}

node_t *get_conf_files(char *dir) {
    node_t *lst = NULL;

    struct dirent **e;
    int n = scandir2(dir, &e, is_conf, alphasort2);
    if(n < 0)
        return NULL;

    for(int i = 0; i < n; ++i) {
        if(is_file(e[i]->d_name))
            lst = new_node(concat_strs(3, dir, "\\", e[i]->d_name), lst);
        free(e[i]);
    }

    free(e);
    return lst;
}

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
