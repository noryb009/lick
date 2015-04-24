#pragma once

#include "uniso.h"

struct menu_s {
    int (*regenerate)();
};

typedef struct menu_s menu_t;

int write_menu_frag(char *dst, char *name, uniso_status_t *info);

int regenerate_menu();
