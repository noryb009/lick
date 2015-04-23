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

int check_loader(win_info_t *info) {
    loader_t *loaders = get_loaders();

    int ret = 0;
    for(int i = 0; i < NUM_LOADERS; ++i) {
        if(loaders[i].supported(info)) {
            ret = loaders[i].check(info);
            break;
        }
    }
    free(loaders);
    return ret;
}

int install_loader(win_info_t *info) {
    loader_t *loaders = get_loaders();

    int ret = 0;
    for(int i = 0; i < NUM_LOADERS; ++i) {
        if(loaders[i].supported(info)) {
            if(!loaders[i].check(info))
                ret = loaders[i].install(info);
            break;
        }
    }
    free(loaders);
    return ret;
}

int uninstall_loader(win_info_t *info) {
    loader_t *loaders = get_loaders();

    int ret = 0;
    for(int i = 0; i < NUM_LOADERS; i++) {
        if(loaders[i].supported(info)) {
            if(loaders[i].check(info))
                ret = loaders[i].uninstall(info);
            break;
        }
    }
    free(loaders);
    return ret;
}
