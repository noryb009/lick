#pragma once

#include "llist.h"

struct installed_s {
    char *id;
    char *name;
};

typedef struct installed_s installed_t;

node_t *list_installed(char *entry_dir);
void free_installed(installed_t *i);
void free_list_installed(node_t *n);

int install(char *id, char *name, char *iso,
        char *entry_dir, char *menu_dir, char *install_dir);

int uninstall(char *entry_dir, char *id);
