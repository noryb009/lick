#pragma once

#include "../../liblick/lick.h"
#include "ipc-lick.hpp"

class Backend {
    public:
        int main(int argc, char *argv[]);
        Backend();
        ~Backend();

    private:
        int event_loop();

        loader_t *loader;
        menu_t *menu;
        sys_info_t *info;

        ipc *recv;
        ipc *send;
};
