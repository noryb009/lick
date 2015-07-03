#pragma once

#include <FL/Fl_Progress.H>
#include <queue>

#include "../../liblick/lick.h"
#include "drop-area.hpp"
#include "ipc-lick.hpp"
#include "gui.h"

class lick_fltk;

class Frontend {
    public:
        Frontend();
        ~Frontend();
        void handle_file(const char *file);
        void on_drop();
        void on_install();
        void on_uninstall();
        int check_id(int ok);
        int check_id();
        void refresh_window();
        int main(int argc, char **argv);
    private:
        void clear();
        void reset();
        void handle_error(const char *err);
        int handle_event(ipc_lick *c);
        void handle_status(ipc_lick *c, ipc_status *s);
        int event_loop();
        void clear_commands_queue();

        void progress_reset();
        void progress_set_size();
        void progress_set();
        void progress_set_sub(uniso_progress_t cur, uniso_progress_t total);
        Fl_Progress *bar;

        std::queue<ipc_lick *> commands_queue;
        bool waiting_for_backend;

        ipc *recv;
        ipc *send;

        bool has_started;
        lick_fltk *w;
        lickdir_t *lick;
        sys_info_t *info;

        char *iso;
        node_t *entries;
        Fl_Color id_bg;
};

int main2(int argc, char *argv[]);
