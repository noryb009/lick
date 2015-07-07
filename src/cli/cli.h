#pragma once

#include <lick.h>

enum VOLUME {
    VOLUME_SILENCE = 0,
    VOLUME_NO_QUESTIONS = 10,
    VOLUME_NORMAL = 20,
};

typedef struct {
    enum VOLUME volume;
    lickdir_t *lick;
    loader_t *loader;
    menu_t *menu;
    sys_info_t *info;
} program_status_t;
