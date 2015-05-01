#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "install.h"
#include "install-loader.h"
#include "scandir.h"
#include "uniso.h"
#include "utils.h"

int is_conf(const struct dirent *e) {
    return is_conf_file(e->d_name);
}

installed_t *get_installed(lickdir_t *lick, char *filename) {
    char *path = concat_strs(3, lick->entry, "/", filename);
    FILE *f = fopen(path, "r");
    free(path);
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

node_t *list_installed(lickdir_t *lick) {
    node_t *lst = NULL;
    installed_t *n2;

    struct dirent **e;
    int n = scandir2(lick->entry, &e, is_conf, alphasort2);
    if(n < 0)
        return NULL;

    for(int i = 0; i < n; ++i) {
        if(is_file(e[i]->d_name)) {
            n2 = get_installed(lick, e[i]->d_name);
            if(n2)
                lst = new_node(n2, lst);
        }
        free(e[i]);
    }

    free(e);
    return lst;
}

void free_installed(installed_t *i) {
    free(i->id);
    free(i->name);
    free(i);
}

void free_list_installed(node_t *n) {
    free_list(n, (void (*)(void *))free_installed);
}

int install(char *id, char *name, char *iso,
        lickdir_t *lick, char *install_dir, menu_t *menu) {
    char *info_path = concat_strs(4, lick->entry, "/", id, ".conf");
    char *menu_path = concat_strs(4, lick->menu, "/", id, ".conf");

    if(file_exists(info_path) || file_exists(menu_path)) {
        free(info_path);
        free(menu_path);
        return 0;
    }

    uniso_status_t *status = uniso(iso, install_dir);
    if(status->finished == 0) {
        uniso_status_free(status);
        free(info_path);
        free(menu_path);
        return 0;
    }

    // write menu entries
    write_menu_frag(menu_path, name, status, install_dir);
    menu->regenerate(lick);

    FILE *info_f = fopen(info_path, "w");
    fprintf(info_f, "name %s\n", name);
    fprintf(info_f, "-----\n");
    for(node_t *n = status->files; n != NULL; n = n->next) {
        fprintf(info_f, "%s\n", n->val);
    }
    fclose(info_f);

    uniso_status_free(status);
    free(info_path);
    free(menu_path);
    return 1;
}

int uninstall_delete_files(char *info) {
    FILE *f = fopen(info, "r");
    if(!f)
        return 0;

    int done = 0;

    char *ln;

    // headers
    while(1) {
        ln = read_line(f, &done);
        if(done || strcmp(ln, "-----"))
            break;
    }

    // files
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

int uninstall(lickdir_t *lick, char *id) {
    char *info = concat_strs(4, lick->entry, "/", id, ".conf");
    int ret = uninstall_delete_files(info);
    free(info);
    return ret;
}
