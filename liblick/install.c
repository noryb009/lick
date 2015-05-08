#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot-loader.h"
#include "install.h"
#include "scandir.h"
#include "uniso.h"
#include "utils.h"

int is_conf_ent(const struct dirent *e) {
    return is_conf_file(e->d_name);
}

installed_t *get_installed(lickdir_t *lick, char *filename) {
    char *path = concat_strs(3, lick->entry, "/", filename);
    FILE *f = fopen(path, "r");
    free(path);
    if(!f)
        return NULL;
    char *name;
    while(1) {
        char *ln = read_line(f);
        if(ln == NULL) {
            fclose(f);
            return NULL;
        }

        char *keyword_start;
        char *item_start;
        conf_option(ln, &keyword_start, &item_start);

        if(strcmp(keyword_start, "name") == 0 && item_start != NULL) {
            fclose(f);
            name = strdup(item_start);
            free(ln);
            break;
        }
        free(ln);
    }

    int id_len = strlen(filename) - 5; // length of file name, - .conf
    char *id = malloc(id_len + 1);
    strncpy(id, filename, id_len);
    id[id_len] = '\0';

    installed_t *i = malloc(sizeof(installed_t));
    i->id = id;
    i->name = name;
    return i;
}

node_t *list_installed(lickdir_t *lick) {
    node_t *lst = NULL;
    installed_t *n2;

    struct dirent **e;
    int n = scandir2(lick->entry, &e, is_conf_ent, alphasort2);
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

int install(char *id, char *name, char *iso, char *install_dir,
        lickdir_t *lick, menu_t *menu) {
    char *info_path = concat_strs(4, lick->entry, "/", id, ".conf");
    char *menu_path = concat_strs(4, lick->menu, "/50-", id, ".conf");

    if(file_exists(info_path) || file_exists(menu_path)) {
        free(info_path);
        free(menu_path);
        if(lick->err == NULL)
            lick->err = strdup("ID conflict.");
        return 0;
    }

    uniso_status_t *status = uniso(iso, install_dir);
    if(status->finished == 0) {
        if(lick->err == NULL)
            lick->err = strdup(status->error);
        uniso_status_free(status);
        free(info_path);
        free(menu_path);
        return 0;
    }

    // write menu entries
    write_menu_frag(menu_path, name, status, install_dir);
    menu->regenerate(lick);

    FILE *info_f = fopen(info_path, "w");
    if(!info_f) {
        // TODO: clean up extracted files
        if(lick->err == NULL)
            lick->err = strdup("Could not write to info file.");
        uniso_status_free(status);
        free(info_path);
        free(menu_path);
        return 0;
    }
    fprintf(info_f, "name %s\n", name);
    fprintf(info_f, "-----\n");
    for(node_t *n = status->files; n != NULL; n = n->next) {
        char *s = concat_strs(3, install_dir, "/", n->val);
        fprintf(info_f, "%s\n", s);
        free(s);
    }
    fprintf(info_f, "%s\n", install_dir);
    fclose(info_f);

    uniso_status_free(status);
    free(info_path);
    free(menu_path);
    return 1;
}

int uninstall_delete_files(char *info, char *menu) {
    FILE *f = fopen(info, "r");
    if(!f)
        return 0;

    char *ln;

    // headers
    while(1) {
        ln = read_line(f);
        if(ln == NULL || strcmp(ln, "-----"))
            break;
        free(ln);
    }
    if(ln != NULL)
        free(ln);

    // files
    while(1) {
        ln = read_line(f);
        if(ln == NULL)
            break;
        else if(strcmp(ln, "") != 0) {
            int l = strlen(ln);
            if(is_file(ln) == 0)
                unlinkDir(ln);
            else
                unlinkFile(ln);
        }
        free(ln);
    }

    fclose(f);
    unlinkFile(info);
    unlinkFile(menu);
    return 1;
}

int uninstall(char *id, lickdir_t *lick, menu_t *menu) {
    char *info = concat_strs(4, lick->entry, "/", id, ".conf");
    char *menu_path = concat_strs(4, lick->menu, "/50-", id, ".conf");
    int ret = uninstall_delete_files(info, menu_path);
    free(info);
    free(menu_path);
    if(menu && !menu->regenerate(lick))
        ret = 0;
    return ret;
}
