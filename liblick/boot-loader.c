#include <stdlib.h>

#include "boot-loader.h"
#include "boot-loader/9x.h"
#include "boot-loader/nt.h"
#include "boot-loader/uefi.h"
#include "boot-loader/vista.h"

#define NUM_LOADERS 4

loader_t *get_loaders() {
    loader_t *loaders = malloc(sizeof(loader_t) * NUM_LOADERS);
    loaders[0] = get_9x();
    loaders[1] = get_nt();
    loaders[2] = get_vista();
    loaders[3] = get_uefi();
    return loaders;
}

loader_t *get_loader(sys_info_t *info) {
    loader_t *loaders = get_loaders();

    loader_t *ret = NULL;
    for(int i = 0; i < NUM_LOADERS; ++i) {
        if(loaders[i].supported(info)) {
            if(loaders[i].check(info)) {
                // if installed, return this
                if(ret == NULL)
                    ret = malloc(sizeof(loader_t));
                *ret = loaders[i];
                break;
            } else if(ret == NULL) {
                // not installed, but supported
                ret = malloc(sizeof(loader_t));
                *ret = loaders[i];
            }
        }
    }
    free(loaders);
    return ret;
}

void free_loader(loader_t *loader) {
    free(loader);
}

// consistency
int check_loader(loader_t *l, sys_info_t *info) {
    return l->check(info);
}

int install_loader(loader_t *l, sys_info_t *info,
        lickdir_t *lick) {
    if(!l->check(info)) {
        if(!l->install(info, lick))
            return 0;
        menu_t *m = get_menu(l);
        m->install(lick);
        m->regenerate(lick);
        free(m);
        return 1;
    }
    return -1; // already installed
}

int uninstall_loader(loader_t *l, sys_info_t *info,
        lickdir_t *lick) {
    if(l->check(info)) {
        if(!l->uninstall(info, lick))
            return 0;
        menu_t *m = get_menu(l);
        m->uninstall(lick);
        free(m);
        return 1;
    }
    return -1; // already uninstalled
}

menu_t *get_menu(loader_t *l) {
    if(l == NULL)
        return NULL;
    else
        return l->get_menu();
}

void free_menu(menu_t *menu) {
    free(menu);
}
