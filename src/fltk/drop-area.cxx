#include "drop-area.hpp"

void drop_area::callback_deferred(void *v) {
    drop_area *w = (drop_area *)v;
    w->do_callback();
}

drop_area::drop_area(int X, int Y, int W, int H, const char *L)
    : Fl_Box(X,Y,W,H,L), evt(FL_NO_EVENT), evt_txt(0), evt_len(0) {
    labeltype(FL_NO_LABEL);
    box(FL_NO_BOX);
    clear_visible_focus();
}

drop_area::~drop_area() {
    delete [] evt_txt;
}

int drop_area::event() {
    return evt;
}

const char *drop_area::event_text() {
    return evt_txt;
}

int drop_area::handle(int e) {
    switch(e) {
        // drag and drop
        case FL_DND_ENTER:
        case FL_DND_RELEASE:
        case FL_DND_LEAVE:
        case FL_DND_DRAG:
        // click
        case FL_PUSH:
            evt = e;
            return 1;

        // file dropped
        case FL_PASTE:
            evt = e;
            evt_len = Fl::event_length();
            delete [] evt_txt;

            evt_txt = new char[evt_len];
            strcpy(evt_txt, Fl::event_text());

            if(callback() && ((when() & FL_WHEN_RELEASE) || (when() & FL_WHEN_CHANGED)))
                Fl::add_timeout(0.0, drop_area::callback_deferred, (void *)this);
            return 1;

        // click release
        case FL_RELEASE:
            evt = e;
            evt_len = Fl::event_length();
            delete [] evt_txt;

            evt_txt = new char[evt_len];
            strcpy(evt_txt, Fl::event_text());

            if(callback() && ((when() & FL_WHEN_RELEASE) || (when() & FL_WHEN_CHANGED)))
                Fl::add_timeout(0.0, drop_area::callback_deferred, (void *)this);
            return 1;
    }

    return Fl_Box::handle(e);
}
