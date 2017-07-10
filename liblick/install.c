#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot-loader.h"
#include "distro.h"
#include "install.h"
#include "scandir.h"
#include "utils.h"

int is_conf_file(const char *e) {
    return (file_type(e) == FILE_TYPE_FILE
            && is_conf_path(e));
}

string_node_t *get_conf_files(const char *path) {
    string_node_t *lst = NULL;

    struct dirent **e;
    int n = scandir_full_path(path, &e, is_conf_file, antialphasort2);
    if(n < 0)
        return NULL;

    for(int i = 0; i < n; ++i) {
        lst = new_string_node_t(concat_strs(3, path, "/", e[i]->d_name), lst);
        free(e[i]);
    }

    free(e);
    return lst;
}

installed_t *get_installed(lickdir_t *lick, const char *filename) {
    (void)lick;
    FILE *f = fopen(filename, "r");
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
            name = strdup2(item_start);
            free(ln);
            break;
        }
        free(ln);
    }

    // get last slash
    const char *last_slash = filename - 1;
    while(1) {
        const char *new_last_slash = strpbrk(last_slash + 1, "/\\");
        if(new_last_slash == NULL)
            break;
        last_slash = new_last_slash;
    }
    if(last_slash == filename - 1)
        last_slash = NULL;

    const char *base_name;
    if(last_slash == NULL)
        base_name = filename;
    else
        base_name = last_slash + 1;

    int id_len = strlen(base_name) - 5; // length of file name, - .conf
    char *id = malloc(id_len + 1);
    strncpy(id, base_name, id_len);
    id[id_len] = '\0';

    installed_t *i = malloc(sizeof(installed_t));
    i->id = id;
    i->name = name;
    return i;
}

int compare_install_name(const installed_t **a, const installed_t **b) {
    return strcmp((*a)->name, (*b)->name);
}

installed_node_t *list_installed(lickdir_t *lick) {
    string_node_t *lst = get_conf_files(lick->entry);
    installed_t *install;
    installed_node_t *installed = NULL;

    for(string_node_t *n = lst; n != NULL; n = n->next) {
        char install_name[strlen(n->val) + 1];
        unix_path(strcpy(install_name, n->val));
        install = get_installed(lick, install_name);
        if(install)
            installed = new_installed_node_t(install, installed);
    }

    free_string_node_t(lst);
    return installed_node_t_sort(installed, compare_install_name);
}

void free_installed(installed_t *i) {
    free(i->id);
    free(i->name);
    free(i);
}

int install_cb(const char *id, const char *name, distro_t *distro,
        const char *iso, const char *install_dir, lickdir_t *lick,
        menu_t *menu, uniso_progress_cb cb, void *cb_data) {
    int ret = 0;
    char *info_path = NULL;
    bool entry_dir_created = false;
    FILE *info_f = NULL;
    uniso_status_t *status = NULL;

    for(;;) {
        info_path = unix_path(concat_strs(4, lick->entry, "/", id, ".conf"));
        if(path_exists(info_path)) {
            if(lick->err == NULL)
                lick->err = strdup2("ID conflict.");
            break;
        }

        char iso_name[strlen(iso) + 1];
        if(!path_exists(unix_path(strcpy(iso_name, iso)))) {
            if(lick->err == NULL)
                lick->err = strdup2("Could not find ISO file.");
            break;
        }

        make_dir_parents(lick->entry);
        entry_dir_created = true;

        info_f = fopen(info_path, "w");
        if(!info_f) {
            if(lick->err == NULL)
                lick->err = strdup2("Could not write to info file.");
            break;
        }

        status = uniso(iso_name, install_dir, distro->filter, cb, cb_data);
        if(status->finished == 0) {
            if(lick->err == NULL)
                lick->err = strdup2(status->error);
            break;
        }

        // write menu entries
        install_menu(status->files, install_dir, distro, id, name, lick, menu);

        fprintf(info_f, "name %s\n", name);
        fprintf(info_f, "distribution %s\n", distro->key);
        fprintf(info_f, "-----\n");
        for(string_node_t *n = status->files; n != NULL; n = n->next) {
            char *s = concat_strs(3, install_dir, "/", n->val);
            fprintf(info_f, "%s\n", unix_path(s));
            free(s);
        }
        char install_dir_unix[strlen(install_dir) + 1];
        strcpy(install_dir_unix, install_dir);
        fprintf(info_f, "%s\n", unix_path(install_dir_unix));

        // success
        ret = 1;
        break;
    }

    if(status) {
        if(ret != 1) {
            // delete files
            for(string_node_t *n = status->files; n != NULL; n = n->next) {
                char *s = concat_strs(3, install_dir, "/", n->val);
                unlink_file(unix_path(s));
                free(s);
            }

            // unlink directory
            char install_dir_copy[strlen(install_dir) + 1];
            unlink_dir(unix_path(strcpy(install_dir_copy, install_dir)));
        }

        free_uniso_status(status);
    }
    if(info_f) {
        fclose(info_f);
        if(ret != 1)
            unlink_file(info_path);
    }
    if(entry_dir_created && ret != 1)
        unlink_dir_parents(lick->entry);
    if(info_path)
        free(info_path);

    return ret;
}

int install(const char *id, const char *name, distro_t *distro,
        const char *iso, const char *install_dir, lickdir_t *lick,
        menu_t *menu) {
    return install_cb(id, name, distro, iso, install_dir, lick, menu,
            NULL, NULL);
}

int uninstall_delete_files(const char *info) {
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
            if(file_type(ln) == FILE_TYPE_DIR)
                unlink_dir(ln);
            else
                unlink_file(ln);
        }
        free(ln);
    }

    fclose(f);
    unlink_file(info);
    return 1;
}

int uninstall(const char *id, lickdir_t *lick, menu_t *menu) {
    char *info = unix_path(concat_strs(4, lick->entry, "/", id, ".conf"));
    int ret = uninstall_delete_files(info);
    free(info);

    uninstall_menu(id, lick, menu);

    unlink_dir_parents(lick->entry);
    return ret;
}
