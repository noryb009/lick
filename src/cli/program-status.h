#pragma once

#include <lick.h>

typedef enum {
    VOLUME_SILENCE,
    VOLUME_NO_QUESTIONS,
    VOLUME_NO_MENU,
    VOLUME_NORMAL,
} volume_e;

typedef struct {
    volume_e volume;
    lickdir_t *lick;
    loader_t *loader;
    menu_t *menu;
    sys_info_t *info;
} program_status_t;

program_status_t *new_program_status();
void free_program_status(program_status_t *p);
