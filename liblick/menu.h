#pragma once

#include "uniso.h"

struct menu_s {
    int (*install)(char *menu_dir, char *res_dir);
    int (*regenerate)(char *menu_dir);
    int (*uninstall)(char *menu_dir, char *res_dir);
};

typedef struct menu_s menu_t;

int write_menu_frag(char *dst, char *name, uniso_status_t *info,
        char *subdir);
