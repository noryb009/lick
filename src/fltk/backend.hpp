#pragma once

#include <lick.h>

#include "ipc-lick.hpp"

class Backend {
    public:
        int main(int argc, char *argv[]);
        Backend();
        ~Backend();
        void progress_cb(uniso_progress_t cur, uniso_progress_t total);

    private:
        int event_loop();

        loader_t *loader;
        menu_t *menu;
        sys_info_t *info;

        ipc *recv;
        ipc *send;
};
