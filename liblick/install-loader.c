#include <stdlib.h>

#include "install-loader.h"
#include "install-loader/9x.h"
#include "install-loader/nt.h"
#include "install-loader/vista.h"

#define NUM_LOADERS 3

loader_t *get_loaders() {
    loader_t *loaders = malloc(sizeof(loader_t) * NUM_LOADERS);
    loaders[0] = get_9x();
    loaders[1] = get_nt();
    loaders[2] = get_vista();
    return loaders;
}

loader_t *get_loader(win_info_t *info) {
    loader_t *loaders = get_loaders();

    loader_t *ret = NULL;
    for(int i = 0; i < NUM_LOADERS; ++i) {
        if(loaders[i].supported(info)) {
            ret = malloc(sizeof(loader_t));
            *ret = loaders[i];
            break;
        }
    }
    free(loaders);
    return ret;
}

// consistency
int check_loader(loader_t *l, win_info_t *info) {
    return l->check(info);
}

int install_loader(loader_t *l, win_info_t *info,
        char *lick_menu, char *lick_res) {
    if(!l->check(info) && l->install(info, lick_res)) {
        menu_t *m = get_menu(l);
        m->install(lick_menu, lick_res);
        free(m);
        return 0;
    }
    return -1; // already installed
}

int uninstall_loader(loader_t *l, win_info_t *info,
        char *lick_menu, char *lick_res) {
    if(!l->check(info) && l->uninstall(info, lick_res)) {
        menu_t *m = get_menu(l);
        m->uninstall(lick_menu, lick_res);
        free(m);
        return 0;
    }
    return -1; // already uninstalled
}

menu_t *get_menu(loader_t *l) {
    menu_t *m = malloc(sizeof(menu_t));
    *m = l->get_menu();
    return m;
}
