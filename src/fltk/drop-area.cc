#include "drop-area.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <shellapi.h>
#endif
#include <FL/fl_ask.H>
#include <FL/x.H>
#include <stdint.h>

const uint32_t wm_dropfiles = 0x233;

void drop_area::callback_deferred(void *v) {
    drop_area *w = (drop_area *)v;
    w->do_callback();
}

drop_area::drop_area(int X, int Y, int W, int H, const char *L)
    : Fl_Box(X,Y,W,H,L), evt(DROP_AREA_NO_EVENT), evt_txt(0), evt_len(0) {
    labeltype(FL_NO_LABEL);
    box(FL_NO_BOX);
    clear_visible_focus();
}

drop_area::~drop_area() {
    delete [] evt_txt;
}

drop_area_events drop_area::event() {
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
            return 1;

        // file dropped
        case FL_PASTE:
            // TODO: remove this code?
            evt = DROP_AREA_DROP;
            evt_len = Fl::event_length() + 1;
            delete [] evt_txt;

            evt_txt = new char[evt_len];
            strcpy(evt_txt, Fl::event_text());

            if(callback() && ((when() & FL_WHEN_RELEASE) || (when() & FL_WHEN_CHANGED)))
                Fl::add_timeout(0.0, drop_area::callback_deferred, (void *)this);
            return 1;

        // click release
        case FL_RELEASE:
            evt = DROP_AREA_CLICK;
            evt_len = Fl::event_length() + 1;
            delete [] evt_txt;

            evt_txt = new char[evt_len];
            strcpy(evt_txt, Fl::event_text());

            if(callback() && ((when() & FL_WHEN_RELEASE) || (when() & FL_WHEN_CHANGED)))
                Fl::add_timeout(0.0, drop_area::callback_deferred, (void *)this);
            return 1;
    }

    return Fl_Box::handle(e);
}

static int drop_handler(void *event, void *data) {
    drop_area *da = static_cast<drop_area*>(data);
    return da->handle_drag_event(event);
}

int drop_area::handle_drag_event(void *event) {
#ifdef _WIN32
    MSG *msg = static_cast<MSG*>(event);
    if(msg->message != wm_dropfiles)
        return 0;

    HDROP hdrop = reinterpret_cast<HDROP>(msg->wParam);

    // Get number of files, and make sure it is one.
    uint32_t files = DragQueryFileA(hdrop, 0xFFFFFFFF, nullptr, 0);
    if(files != 1)
        fl_alert("Only drag and drop one file at a time.");
    else {
        // Get length of the first (and only) filename.
        evt_len = DragQueryFileA(hdrop, 0, nullptr, 0) + 1;

        // Copy into the filename.
        delete [] evt_txt;
        evt_txt = new char[evt_len];
        if(!DragQueryFileA(hdrop, 0, evt_txt, evt_len))
            fl_alert("An error occurred while processing the file you dropped. Try clicking the button instead.");
        else {
            // Process the filename later.
            evt = DROP_AREA_DROP_API; // TODO: make this better.
            Fl::add_timeout(0.0, drop_area::callback_deferred, (void *)this);
        }
    }
    DragFinish(hdrop);
    return 1;
#else
    (void)event;
    return 1;
#endif
}

void drop_area::enable_drag_drop(const Fl_Window *window) {
#ifdef _WIN32
    // Remove the fltk drag and drop handler.
    RevokeDragDrop(fl_xid(window));
    // Add our own handler.
    DragAcceptFiles(fl_xid(window), 1);
    Fl::add_system_handler(drop_handler, this);

    // Newer systems don't allow drag and drop between different privileges processes.
    // To fix this, the messages need to be removed from the filter.
    typedef BOOL (WINAPI *change_window_message_filter)(uint32_t msg, uint16_t flags);
    HMODULE lib = LoadLibrary("User32.dll");
    if(!lib)
        return;
    change_window_message_filter message_filter =
        (change_window_message_filter)GetProcAddress(lib, "ChangeWindowMessageFilter");
    if (message_filter) {
        // See issue #15 for why these three must be enabled.
        // In short, drag and drop doesn't work without all 3.
        const uint32_t wm_copydata = 0x4A;
        const uint32_t wm_copyglobaldata = 0x49;

        message_filter(wm_dropfiles, 1);
        message_filter(wm_copydata, 1);
        message_filter(wm_copyglobaldata, 1);
    }
    FreeLibrary(lib);
#else
    (void)window;
    (void)drop_handler;
#endif
}
