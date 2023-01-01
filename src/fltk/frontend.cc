#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "backend.hpp"
#include "drop-area.hpp"
#include "frontend.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void Frontend::clear() {
    w = NULL;
    lick = NULL;
    info = NULL;

    iso = NULL;
    entries = NULL;
}

void Frontend::reset() {
    if(w)
        delete w;
    if(info)
        free_sys_info(info);
    if(lick)
        free_lickdir(lick);

    if(iso)
        free(iso);
    if(entries)
        free_installed_node_t(entries);
    clear();
}

void Frontend::handle_error(const char *err) {
    if(lick->err) {
        if(err)
            fl_alert("%s\n%s", err, lick->err);
        else
            fl_alert("%s", lick->err);
        free(lick->err);
        lick->err = NULL;
    } else if(err)
        fl_alert("%s", err);
}

Frontend::Frontend() {
    send = NULL;
    recv = NULL;
    has_started = false;

    bar = NULL;
    waiting_for_backend = 0;
    id_bg = 0;

    clear();
}

Frontend::~Frontend() {
    if(send) {
        send_command(send, IPC_EXIT);
        delete send;
    }
    if(recv)
        delete recv;

    reset();
}

void Frontend::handle_file(const char *file) {
    if(!path_exists(file)) {
        fl_alert("Please choose a valid ISO file or CD drive.");
        return;
    }

    if(file_type(file) == FILE_TYPE_DIR) {
        if(fl_choice("%s is a directory. Do you want to install the Linux files found at this location?", NULL, "Yes", "No", file) == 2)
            return;
    } else if(!strstr(file, ".iso"))
        if(fl_choice("%s doesn't look like an ISO file. Are you sure you wish to continue?", NULL, "Yes", "No", file) == 2)
            return;

    char *id = gen_id(file, lick, w->choice_install_drive->text());
    if(id) {
        w->text_id->value(id);
        free(id);
    } else
        w->text_id->value("");
    check_id();

    char *name = gen_name(file);
    w->text_name->value(name);
    free(name);

    iso = strdup2(file);
}

void Frontend::on_drop() {
    drop_area *drop = w->drop_area_iso;
    Fl_Native_File_Chooser *chooser;
    switch(drop->event()) {
    case DROP_AREA_CLICK:
        chooser = new Fl_Native_File_Chooser();
        switch(chooser->show()) {
        case -1:
            fl_alert("Error:\n%s", chooser->errmsg());
            break;
        case 1:
            break;
        default:
            handle_file(chooser->filename());
            break;
        }
        break;
    case DROP_AREA_DROP: {
        const char *text = drop->event_text();
        // get number of files
        int i = 1;
        for(const char *c = text; c[0] != '\0'; ++c) {
            if(c[0] == '\n' && c[0] + 1 != '\0')
                ++i;
        }
        if(i > 1 || strlen(text) == 0) {
            fl_alert("Only 1 ISO file or drive may be used.");
            return;
        }

        char *file = new char [strlen(text) + 1];
        char *file_to_delete = file;
        strcpy(file, text);
        fl_decode_uri(file);

        for(char *c = file; c[0] != '\0'; ++c)
            if(c[0] == '\n' || c[0] == '\r') {
                c[0] = '\0';
                break;
            }
        if(strncmp(file, "file://", 7) == 0)
            file += 7;
        handle_file(file);
        delete [] file_to_delete;

    }break;
    case DROP_AREA_DROP_API:
        handle_file(drop->event_text());
        break;
    case DROP_AREA_NO_EVENT:
        break;
    }
}

void Frontend::on_install() {
    if(iso == NULL) {
        fl_alert("Please select an ISO or CD drive.");
        return;
    }
    if(!check_id()) {
        fl_alert("Please enter a valid ID.");
        return;
    }
    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes.");
        return;
    }

    char *install_to = concat_strs(3, w->choice_install_drive->text(), "/", w->text_id->value());
    commands_queue.push(new ipc_check_loader());
    commands_queue.push(new ipc_loader(1, lick));
    commands_queue.push(new ipc_install(w->text_id->value(),
                w->text_name->value(), "puppy", iso, install_to, lick));
    free(install_to);
    progress_set_size();
}

void Frontend::on_uninstall() {
    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes.");
        return;
    }
    Fl_Browser *lst = w->lst_uninstall;
    int val = lst->value();
    if(val <= 0)
        return;
    if(fl_choice("Are you sure you wish to uninstall %s?", "No", "Yes", NULL, lst->text(val)) != 1)
        return;

    installed_node_t *e = entries;
    for(int i = 1; i < val && e != NULL; ++i)
        e = e->next;
    if(e == NULL) {
        refresh_window();
        return;
    }

    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes.");
    }

    commands_queue.push(new ipc_uninstall(e->val->id, lick));
    commands_queue.push(new ipc_loader(0, lick));
    progress_set_size();
}

void Frontend::on_loader_inst() {
    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes.");
        return;
    }

    commands_queue.push(new ipc_check_loader());
    commands_queue.push(new ipc_loader(2, lick));
    progress_set_size();
}

void Frontend::on_about() {
    fl_message("LICK Version %s\nBy Luke Lorimer (noryb009)", LIBLICK_VERSION);
}

void Frontend::on_nothing_happens() {
    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes.");
        return;
    }

    int ans = fl_choice(
            "If you are on UEFI, and have rebooted,\n"
            "but Windows immediately boots, there is another\n"
            "step that can be taken. This is risky, so it is\n"
            "recommended that you have a live CD and make a backup\n"
            "of your EFI drive before you begin.\n"
            "Do you want to start?",
            "Yes",
            "No",
            NULL
    );
    if(ans != 0)
        return;

    ans = fl_choice(
            "Do you have a live CD and have backed up your EFI drive?",
            "Yes",
            "No",
            NULL
    );
    if(ans != 0)
        return;

    commands_queue.push(new ipc_fix(lick));
    progress_set_size();
}

void Frontend::on_quit() {
    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes before exiting.");
        return;
    }

    w->window->hide();
}


int Frontend::check_id(int ok) {
    if(ok)
        w->text_id->color(id_bg);
    else
        w->text_id->color(FL_RED);
    w->text_id->redraw();
    return ok;
}

int Frontend::check_id() {
    if(strcmp(w->text_id->value(), "") == 0) {
        check_id(1);
        return 0;
    } else if(is_valid_id(w->text_id->value(),
                lick, w->choice_install_drive->text()))
        return check_id(1);
    else
        return check_id(0);
}

char *until_first_slash(char *path) {
    if(!path)
        return NULL;
    char *slash = strchr(path, '/');
    if(slash)
        slash[0] = '\0';
    return path;
}

void Frontend::refresh_window() {
    // ISO
    if(iso)
        free(iso);
    iso = NULL;

    // text boxes
    w->text_id->value("");
    w->text_name->value("");

    // drive list
    Fl_Choice *menu = w->choice_install_drive;
    menu->clear();

    char *win_drive = get_windows_drive_path();
    drive_node_t *drvs = all_drives();
    if(win_drive)
        menu->add(until_first_slash(win_drive), 0, 0, 0, 0);
    for(drive_node_t *d = drvs; d != NULL; d = d->next) {
        drive_t *drv = d->val;
        if(win_drive && drv->path[0] != win_drive[0])
            menu->add(until_first_slash(drv->path), 0, 0, 0, 0);
    }
    free_drive_node_t(drvs);
    if(win_drive)
        free(win_drive);
    menu->value(0);

    check_id();

    // uninstall list
    if(entries)
        free_installed_node_t(entries);
    entries = list_installed(lick);
    w->lst_uninstall->clear();
    for(installed_node_t *e = entries; e != NULL; e = e->next) {
        installed_t *install = e->val;
        char *text = concat_strs(4, install->name, " (", install->id, ")");
        w->lst_uninstall->add(text, entries);
        free(text);
    }
}

void Frontend::clear_commands_queue() {
    while(!commands_queue.empty()) {
        delete commands_queue.front();
        commands_queue.pop();
    }
}

const char *command_name(ipc_lick *c) {
    switch(c->type()) {
    case IPC_INSTALL:
        return "Installing";
    case IPC_UNINSTALL:
        return "Uninstalling";
    case IPC_CHECK_LOADER:
        return "Checking boot loader status";
    case IPC_LOADER:
        if(((ipc_loader *)c)->install)
            return "Installing bootloader";
        else
            return "Uninstalling bootloader";
    case IPC_FIX:
        return "Fixing bootloader.";
    case IPC_READY:
    case IPC_STATUS:
    case IPC_PROGRESS:
    case IPC_ERROR:
    case IPC_EXIT:
    default:
        return "Unknown";
    }
}

void Frontend::progress_reset() {
    w->progress_window->hide();
}
void Frontend::progress_reset_if_done() {
    if(commands_queue.empty())
        progress_reset();
}
void Frontend::progress_set_size() {
    bar->maximum(commands_queue.size());
    bar->value(0);
    bar->label("");
    w->progress_window->show();
}
void Frontend::progress_set() {
    int size = commands_queue.size();
    int max_size = (int)(0.5+bar->maximum());
    bar->value(max_size - size);
    if(size == 0)
        bar->label("");
    else {
        char *lbl = new char[128];
        snprintf(lbl, 127, "Step %d / %d: %s",
                max_size-size+1, max_size, command_name(commands_queue.front()));
        bar->copy_label(lbl);
        delete [] lbl;
    }
}
void Frontend::progress_set_sub(uniso_progress_t cur, uniso_progress_t total) {
    int size = commands_queue.size();
    int max_size = (int)(0.5+bar->maximum());

    char *lbl = new char[128];
    snprintf(lbl, 127, "Step %d / %d: %s, %zu / %zu files extracted",
            max_size-size+1, max_size, command_name(commands_queue.front()), cur, total);
    bar->copy_label(lbl);
    delete [] lbl;
}

bool is_loader(ipc_lick *c, bool install) {
    if(c->type() != IPC_LOADER)
        return false;
    ipc_loader *l = (ipc_loader *)c;
    return (install && l->install) || (!install && !l->install);
}

void Frontend::handle_status(ipc_lick *c, ipc_status *s) {
    if(s->err) {
        if(lick->err)
            free(lick->err);
        lick->err = s->err;
        s->err = NULL;
    }

    switch(c->type()) {
    case IPC_INSTALL:
        if(!s->ret) {
            handle_error("Error installing ISO!");
            clear_commands_queue();
            return;
        }
        refresh_window();
        progress_reset_if_done();
        fl_message("Installed successfully!");
        break;
    case IPC_UNINSTALL: {
        if(!s->ret) {
            handle_error("There was an error uninstalling the installation.");
            clear_commands_queue();
            return;
        }

        refresh_window();

        int next_uninstall_loader = 0;
        if(!commands_queue.empty() && is_loader(commands_queue.front(), false))
            next_uninstall_loader = 1;
        int remove_next = next_uninstall_loader;

        if(next_uninstall_loader && entries == NULL) {
            if(fl_choice("Uninstalled successfully!\nDo you wish to uninstall the boot loader? If you are not sure, answer 'YES'.", "No", "Yes", NULL) == 1)
                remove_next = 0;
        } else {
            if(remove_next && commands_queue.size() == 1)
                progress_reset();
            fl_message("Uninstalled successfully!");
        }

        if(remove_next) {
            delete commands_queue.front();
            commands_queue.pop();
        }
        }break;
    case IPC_CHECK_LOADER:
        if(!commands_queue.empty()
                && commands_queue.front()->type() == IPC_LOADER) {
            ipc_loader *l = (ipc_loader *)commands_queue.front();
            if(l->install == 2) { // toggle
                if(s->ret)
                    l->install = 0;
                else
                    l->install = 1;
                if((l->install && fl_choice("Are you sure you want to install the boot loader?", "No", "Yes", NULL) == 0)
                            || (!l->install && fl_choice("Are you sure you want to uninstall the boot loader?", "No", "Yes", NULL) == 0)) {
                    delete commands_queue.front();
                    commands_queue.pop();
                }
            } else if(s->ret) {
                delete commands_queue.front();
                commands_queue.pop();
            }
        }
        break;
    case IPC_LOADER:
        if(!s->ret) {
            ipc_loader *l = (ipc_loader *)c;
            if(l->install)
                handle_error("Error installing loader!");
            else
                handle_error("Error uninstalling loader!");
            clear_commands_queue();
            return;
        }
        break;
    case IPC_FIX:
        if(s->ret == -1) {
            handle_error("Install the bootloader first.");
            clear_commands_queue();
            return;
        } else if(!s->ret) {
            handle_error("Error fixing the bootloader!");
            clear_commands_queue();
            return;
        }
        fl_message("Fix complete!");
        break;
    default:
        return;
    }
    // had error, but didn't message
    if(lick->err)
        handle_error(NULL);
}

int Frontend::handle_event(ipc_lick *c) {
    switch(c->type()) {
    case IPC_READY:
        if(has_started)
            break;
        has_started = true;
        waiting_for_backend = false;

        info = get_system_info();

        lick = get_lickdir();
        if(lick == NULL) {
            fl_alert("LICK is not in a valid location. Please make sure you have extracted or installed LICK completely.");
            send_command(send, IPC_EXIT);
            return 1;
        }

        w = new lick_fltk();
        w->make_window(this)->show();
        w->make_progress_window();
        bar = w->progress_bar;
        id_bg = w->text_id->color();
        refresh_window();

        // Enable drag and drop.
        w->drop_area_iso->enable_drag_drop(w->window);

        break;
    case IPC_STATUS: {
        if(!waiting_for_backend) {
            fl_alert("Received unassociated message.");
            break;
        }
        waiting_for_backend = false;
        if(commands_queue.empty()) {
            fl_alert("Received message while not waiting for one.");
            break;
        }

        ipc_lick *command = commands_queue.front();
        commands_queue.pop();
        handle_status(command, (ipc_status *)c);
        progress_reset_if_done();
        delete command;
        }break;
    case IPC_PROGRESS: {
        ipc_progress *p = (ipc_progress *)c;
        progress_set_sub(p->cur, p->total);
        }break;
    case IPC_ERROR: {
        ipc_error *e = (ipc_error *)c;
        if(e->err) {
            fl_alert("%s", e->err);
        }
        }return 1;
    default:
        // invalid command
        fl_alert("Received invalid command.");
        break;
    }
    return 0;
}

int Frontend::event_loop() {
    while(waiting_for_backend || !has_started || !commands_queue.empty() || w->window->shown()) {
        Fl::wait(0.2);
        if(recv->data_waiting() || !has_started) {
            ipc_lick *c = recv_command(recv);
            if(c == NULL) {
                fl_alert("Unknown error, exiting.");
                return 1;
            }
            int ret = handle_event(c);
            delete c;
            if(ret != 0) {
                return ret;
            }
        }
        if(recv->had_error() || send->had_error()) {
            fl_alert("Unknown error, exiting.");
            return 1;
        }

        if(!waiting_for_backend && !commands_queue.empty()) {
            progress_set();
            send_command(send, commands_queue.front());
            waiting_for_backend = true;
        }
    }
    return 0;
}

int Frontend::main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    waiting_for_backend = true;
    reset();

#ifdef _WIN32
    send = new ipc(DIRECTION_SEND, GetStdHandle(STD_OUTPUT_HANDLE));
    recv = new ipc(DIRECTION_RECV, GetStdHandle(STD_INPUT_HANDLE));
#else
    return 0;
#endif

    return event_loop();
}

int main2(int argc, char *argv[]) {
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "--frontend") == 0) {
            Frontend f;
            return f.main(argc, argv);
        }
    }
    Backend b;
    return b.main(argc, argv);
}
