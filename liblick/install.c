#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
typedef __u_short u_short; // TODO: fix
#include <fts.h>

#include "install.h"
#include "menu.h"
#include "uniso.h"
#include "utils.h"

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

installed_t *get_installed(char *path, char *filename) {
    // if ends with .conf
    char *conf = strstr(filename, ".conf");
    if(conf == NULL || strlen(conf) != 5)
        return NULL;

    FILE *f = fopen(path, "r");
    if(!f)
        return NULL;
    int done = 0;
    char *name = read_line(f, &done);
    fclose(f);
    if(done) {
        return NULL;
    }

    int l = strlen(filename);
    char *id = malloc(l - 5 + 1);
    strncpy(filename, id, l - 5);
    id[l - 5 + 1] = '\0';

    installed_t *i = malloc(sizeof(installed_t));
    i->id = id;
    i->name = name;
    return i;
}

node_t *list_installed(char *entry_dir) {
    FTS *fts;
    FTSENT *p, *cp;
    int fts_opts = FTS_NOCHDIR | FTS_COMFOLLOW
        | FTS_LOGICAL | FTS_NOCHDIR;

    fts = fts_open(&entry_dir, fts_opts, NULL);
    if(fts == NULL)
        return NULL;

    p = fts_children(fts, 0);
    if(p == NULL)
        return NULL;

    node_t *n = NULL;
    installed_t *n2;

    while((p = fts_read(fts)) != NULL)
        switch(p->fts_info) {
            case FTS_F:
                n2 = get_installed(p->fts_path, p->fts_name);
                if(n2)
                    n = new_node(n2, n);
                break;
        }
    fts_close(fts);
    return n;
}

void free_installed(installed_t *i) {
    free(i->id);
    free(i->name);
    free(i);
}

void free_list_installed(node_t *n) {
    while(n != NULL) {
        node_t *next = n->next;
        free_installed(n->val);
        free(n);
        n = next;
    }
}

int install(char *id, char *name, char *iso,
        char *entry_dir, char *menu_dir, char *install_dir) {
    char *info = concat_strs(4, entry_dir, "/", id, ".conf");
    char *menu = concat_strs(4, menu_dir, "/", id, ".conf");

    if(file_exists(info) || file_exists(menu)) {
        free(info);
        free(menu);
        return 0;
    }

    uniso_status_t *status = uniso(iso, install_dir);
    if(status->finished == 0) {
        uniso_status_free(status);
        free(info);
        free(menu);
        return 0;
    }

    // write menu entries
    write_menu_frag(menu, name, status);
    regenerate_menu();

    FILE *info_f = fopen(info, "w");
    // TODO: headers?
    fprintf(info_f, "%s\n", name);
    for(node_t *n = status->files; n != NULL; n = n->next) {
        fprintf(info_f, "%s\n", n->val);
    }
    fclose(info_f);

    uniso_status_free(status);
    free(info);
    free(menu);
    return 1;
}

int uninstall_delete_files(char *info) {
    FILE *f = fopen(info, "r");
    if(!f)
        return 0;

    int done = 0;

    // TODO: headers?
    char *ln = read_line(f, &done); // name
    while(1) {
        ln = read_line(f, &done);
        if(done)
            break;
        else if(strcmp(ln, "") != 0) {
            int l = strlen(ln);
            if(ln[l-1] == '/' || ln[l-1] == '\\')
                unlinkDir(ln);
            else
                unlinkFile(ln);
        }
        free(ln);
    }

    fclose(f);
    unlinkFile(info);
    return 1;
}

int uninstall(char *entry_dir, char *id) {
    char *info = concat_strs(4, entry_dir, "/", id, ".conf");
    int ret = uninstall_delete_files(info);
    free(info);
    return ret;
}
