#include <stdlib.h>

#include "program-status.h"

program_status_t *new_program_status() {
    program_status_t *p = malloc(sizeof(program_status_t));
    p->lick = NULL;
    p->loader = NULL;
    p->menu = NULL;
    p->info = NULL;
    p->volume = VOLUME_NORMAL;
    return p;
}

void free_program_status(program_status_t *p) {
    if(p->lick)
        free_lickdir(p->lick);
    if(p->loader)
        free_loader(p->loader);
    if(p->menu)
        free_menu(p->menu);
    if(p->info)
        free_sys_info(p->info);
    free(p);
}
