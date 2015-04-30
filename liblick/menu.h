#pragma once

#include "uniso.h"

struct menu_s {
    // TODO: install
    int (*regenerate)(char *menu_dir);
    // TODO: uninstall?
};

typedef struct menu_s menu_t;

int write_menu_frag(char *dst, char *name, uniso_status_t *info,
        char *subdir);

int regenerate_menu(char *menu_dir);
