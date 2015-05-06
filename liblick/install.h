#pragma once

#include "lickdir.h"
#include "llist.h"
#include "menu.h"

typedef struct {
    char *id;
    char *name;
} installed_t;

node_t *list_installed(lickdir_t *lick);
void free_installed(installed_t *i);
void free_list_installed(node_t *n);

int install(char *id, char *name, char *iso,
        lickdir_t *lick, char *install_dir, menu_t *menu);

int uninstall(lickdir_t *lick, char *id, menu_t *menu);
