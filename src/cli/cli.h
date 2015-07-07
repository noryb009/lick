#pragma once

#include <lick.h>

enum VOLUME {
    VOLUME_SILENCE,
    VOLUME_NO_QUESTIONS,
    VOLUME_NO_MENU,
    VOLUME_NORMAL,
};

typedef struct {
    enum VOLUME volume;
    lickdir_t *lick;
    loader_t *loader;
    menu_t *menu;
    sys_info_t *info;
} program_status_t;

typedef struct {
    int check_program;
    int try_uac;
    int ignore_errors;
    int check_loader;
    int install_loader;
    node_t *install;
    node_t *uninstall;
    int uninstall_all;
} program_args_t;
