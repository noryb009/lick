#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "install.h"
#include "menu.h"
#include "scandir.h"
#include "uniso.h"
#include "utils.h"

installed_t *get_installed(char *entry_dir, char *filename) {
    // if ends with .conf
    char *conf;
    do {
        conf = strstr(conf + 1, ".conf");
    } while(conf != NULL && strlen(conf) > 5);
    if(conf == NULL)
        return NULL;

    char *path = concat_strs(3, entry_dir, "/", filename);
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

node_t *list_installed(char *entry_dir) {
    node_t *lst = NULL;
    installed_t *n2;

    struct dirent **e;
    int n = scandir2(entry_dir, &e, 0, alphasort2);
    if(n < 0)
        return NULL;

    for(int i = 0; i < n; ++i) {
        printf("%s\n", e[i]->d_name); // TODO: remove
        //TODO: make sure file, not directory
        //TODO: use filter
        n2 = get_installed(entry_dir, e[i]->d_name);
        if(n2)
            lst = new_node(n2, lst);
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
    regenerate_menu(menu);

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
