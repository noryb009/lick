#pragma once

#include "lickdir.h"
#include "uniso.h"

struct menu_s {
    int (*install)(lickdir_t *lick);
    int (*regenerate)(lickdir_t *lick);
    int (*uninstall)(lickdir_t *lick);
};

typedef struct menu_s menu_t;

int write_menu_frag(char *dst, char *name, uniso_status_t *info,
        char *subdir);
