#pragma once

#include "lickdir.h"
#include "menu.h"
#include "windows-info.h"

struct loader_s {
    int (*supported)(win_info_t *info);
    int (*check)(win_info_t *info);
    int (*install)(win_info_t *info, lickdir_t *lick);
    int (*uninstall)(win_info_t *info, lickdir_t *lick);
    menu_t (*get_menu)();
};

typedef struct loader_s loader_t;

loader_t *get_loader(win_info_t *info);
int check_loader(loader_t *l, win_info_t *info);
int install_loader(loader_t *l, win_info_t *info,
        lickdir_t *lick);
int uninstall_loader(loader_t *l, win_info_t *info,
        lickdir_t *lick);
menu_t *get_menu(loader_t *l);
