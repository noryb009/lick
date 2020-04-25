#include "boot-loader.h"

#include <stdlib.h>

#include "boot-loader/9x.h"
#include "boot-loader/me.h"
#include "boot-loader/nt.h"
#include "boot-loader/uefi.h"
#include "boot-loader/vista.h"
#include "trace.h"

#define NUM_LOADERS 5

loader_t *get_loaders() {
    loader_t *loaders = malloc(sizeof(loader_t) * NUM_LOADERS);
    loaders[0] = get_9x();
    loaders[1] = get_me();
    loaders[2] = get_nt();
    loaders[3] = get_vista();
    loaders[4] = get_uefi();
    return loaders;
}

loader_t *get_loader(sys_info_t *info) {
    loader_t *loaders = get_loaders();

    loader_t *ret = NULL;
    for(int i = 0; i < NUM_LOADERS; ++i) {
        if(loaders[i].supported(info)) {
            if(loaders[i].check()) {
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
int check_loader(loader_t *l) {
    return l->check();
}

int install_loader(loader_t *l, sys_info_t *info,
        lickdir_t *lick) {
    LICK_TRACE(lick, "Installing boot loader");

    if(!l->check()) {
        LICK_TRACE(lick, "Boot loader not installed");
        if(!l->install(info, lick)) {
            LICK_TRACE(lick, "Boot loader failed installation");
            return 0;
        }
        LICK_TRACE(lick, "Getting menu");
        menu_t *m = get_menu(l);
        LICK_TRACE(lick, "Installing to menu");
        m->install(lick);
        free(m);
        return 1;
    }
    LICK_TRACE(lick, "Boot loader already installed");
    return -1; // already installed
}

int uninstall_loader(loader_t *l, int reinstall, sys_info_t *info,
        lickdir_t *lick) {
    if(l->check()) {
        if(!l->uninstall(info, lick))
            return 0;
        if(!reinstall) {
            menu_t *m = get_menu(l);
            m->uninstall(lick);
            free(m);
        }
        return 1;
    }
    return -1; // already uninstalled
}

int check_fix_loader(loader_t *l, menu_t *m, lickdir_t *lick) {
    if(l->check())
        return m->check_fix_loader(lick);
    return -1; // not installed
}

int fix_loader(loader_t *l, menu_t *m, lickdir_t *lick) {
    if(l->check())
        return m->fix_loader(lick);
    return -1; // not installed
}

menu_t *get_menu(loader_t *l) {
    if(l == NULL)
        return NULL;
    else
        return l->get_menu();
}
