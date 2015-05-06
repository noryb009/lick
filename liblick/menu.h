#pragma once

#include "lickdir.h"
#include "uniso.h"

typedef struct {
    int (*install)(lickdir_t *lick);
    int (*regenerate)(lickdir_t *lick);
    int (*uninstall)(lickdir_t *lick);
} menu_t;

int write_menu_frag(char *dst, char *name, uniso_status_t *info,
        char *subdir);
