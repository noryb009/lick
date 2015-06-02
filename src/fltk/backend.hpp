#pragma once

#include "../../liblick/lick.h"
#include "drop-area.hpp"
#include "gui.h"

class lick_fltk;

class Backend {
    public:
        Backend();
        ~Backend();
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

        lick_fltk *w;
        lickdir_t *lick;
        loader_t *loader;
        menu_t *menu;
        sys_info_t *info;

        char *iso;
        node_t *entries;
        Fl_Color id_bg;
};
