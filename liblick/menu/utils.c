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

int is_space(char c) {
    return (c == ' ' || c == '\t');
}

entry_t *get_entry(FILE *f) {
    entry_t *e = NULL;
    int done = 0;

    while(1) {
        char *v = concat_strs(0);
        char *ln = read_line(f, &done);
        if(done)
            break;

        int len = strlen(ln);
        char *keyword_start = NULL;
        int keyword_done = 0;
        char *item_start = NULL;
        for(int i = 0; i < len; i++) {
            int space = is_space(ln[i]);
            if(ln[i] == '#') // comment
                break;
            else if(keyword_start == NULL && space) { // padding
            } else if(keyword_start == NULL) // keyword
                keyword_start = ln + i;
            else if(item_start == NULL && space) { // space between
                ln[i] = '\0';
                keyword_done = 1;
            } else if(item_start == NULL && keyword_done == 1) // item
                item_start = ln + i;
        }

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

        if(item_start != NULL) // has item
            // trim end padding
            for(int i = strlen(item_start) - 1; i >= 0; i--) {
                if(is_space(item_start[i]))
                    item_start[i] = '\0';
                else
                    break;
            }

        if(e == NULL)
            e = malloc(sizeof(entry_t));

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

        if(*target != NULL)
            // TODO: error/warning
            free(*target);
        *target = strdup(item_start);

        if(e->title && e->kernel && e->initrd && e->options)
            return e;
        if(e->static_text)
            return e;
    }

    if(e)
        free_entry(e);
    return NULL;
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
