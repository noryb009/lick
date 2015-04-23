#pragma once

#include "windows-info.h"

struct loader_s {
    int (*supported)(win_info_t *info);
    int (*check)(win_info_t *info);
    int (*install)(win_info_t *info);
    int (*uninstall)(win_info_t *info);
};

typedef struct loader_s loader_t;

int check_loader(win_info_t *info);
int install_loader(win_info_t *info);
int uninstall_loader(win_info_t *info);
