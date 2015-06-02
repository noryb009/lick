// based on http://www.fltk.org/articles.php?L770
#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>

class drop_area : public Fl_Box {
    public:
        static void callback_deferred(void *v);
        drop_area(int X, int Y, int W, int H, const char *L = 0);
        virtual ~drop_area();
        int event();
        const char *event_text();
        int handle(int e);
    protected:
        int evt;
        char *evt_txt;
        int evt_len;
};
