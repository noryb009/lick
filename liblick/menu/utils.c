#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
typedef __u_short u_short; // TODO: fix
#include <fts.h>

#include "utils.h"
#include "../utils.h"

int is_file(char *path, char *filename) {
    // if ends with .conf
    char *conf;
    do {
        conf = strstr(conf + 1, ".conf");
    } while(conf != NULL && strlen(conf) > 5);

    if(conf == NULL)
        return 0;
    return 1;
}

int alphabetical(const FTSENT **a, const FTSENT **b) {
    return strcmp((*a)->fts_name, (*b)->fts_name);
}

node_t *get_files(char *menu_dir) {
    FTS *fts;
    FTSENT *p, *cp;
    int fts_opts = FTS_NOCHDIR | FTS_COMFOLLOW
        | FTS_LOGICAL | FTS_NOCHDIR;

    fts = fts_open(&menu_dir, fts_opts, &alphabetical);
    if(fts == NULL)
        return NULL;

    p = fts_children(fts, 0);
    if(p == NULL)
        return NULL;

    node_t *n = NULL;

    while((p = fts_read(fts)) != NULL)
        switch(p->fts_info) {
            case FTS_F:
                printf("DEBUG: %s\n", p->fts_path);
                if(is_file(p->fts_path, p->fts_name))
                    n = new_node(strdup(p->fts_path), n);
                break;
        }
    fts_close(fts);
    return n;
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
        char *item_start = NULL;
        for(int i = 0; i < len; i++) {
            int space = is_space(ln[i]);
            if(ln[i] == '#') // comment
                break;
            else if(keyword_start == NULL && space) { // padding
            } else if(keyword_start == NULL) // keyword
                keyword_start = ln + i;
            else if(item_start == NULL && space) // space between
                ln[i] = '\0';
            else if(item_start == NULL) // item
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
        } type;

        if(strcmp(keyword_start, "title") == 0)
            type = TITLE;
        else if(strcmp(keyword_start, "title") == 0)
            type = KERNEL;
        else if(strcmp(keyword_start, "title") == 0)
            type = INITRD;
        else if(strcmp(keyword_start, "title") == 0)
            type = OPTIONS;
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
        }

        if(*target != NULL)
            // TODO: error/warning
            free(*target);
        *target = strdup(item_start);

        if(e->title && e->kernel && e->initrd && e->options)
            return e;
    }

    if(e)
        free_entry(e);
    return NULL;
}

void free_entry(entry_t *e) {
    if(e->title)
        free(e->title);
    if(e->kernel)
        free(e->kernel);
    if(e->initrd)
        free(e->initrd);
    if(e->options)
        free(e->options);
}
