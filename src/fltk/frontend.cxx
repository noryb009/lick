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
        free_lick_dir(lick);

    if(iso)
        free(iso);
    if(entries)
        free_list_installed(entries);
    clear();
}

void Frontend::handle_error(const char *err) {
    if(lick->err)
        fl_alert("%s\n%s", err, lick->err);
    else
        fl_alert("%s", err);
}

Frontend::Frontend() {
    send = NULL;
    recv = NULL;
    has_started = false;

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
        fl_alert("Please choose a valid ISO file.");
        return;
    }

    if(!strstr(file, ".iso"))
        if(fl_choice("%s doesn't look like an ISO file. Are you sure you wish to continue?", "Yes", "No", NULL, file) == 1)
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
        case FL_RELEASE:
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
        case FL_PASTE:
            const char *text = drop->event_text();
            // get number of files
            int i = 1;
            for(const char *c = text; c[0] != '\0'; ++c) {
                if(c[0] == '\n' && c[0] + 1 != '\0')
                    ++i;
            }
            if(i > 1 || strlen(text) == 0) {
                fl_alert("Only 1 ISO may be used.");
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

            break;
    }
}

void Frontend::on_install() {
    if(iso == NULL) {
        fl_alert("Please select an ISO.");
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
    commands_queue.push(new ipc_install(w->text_id->value(), w->text_name->value(), iso, install_to, lick));
    free(install_to);
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

    node_t *e = entries;
    for(int i = 1; i < val && e != NULL; ++i)
        e = e->next;
    if(e == NULL) {
        refresh_window();
        return;
    }
    installed_t *ent = (installed_t *)(e->val);

    if(!commands_queue.empty()) {
        fl_alert("Please wait until the current operation finishes.");
    }

    commands_queue.push(new ipc_uninstall(ent->id, lick));
    commands_queue.push(new ipc_loader(0, lick));
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

    drive_t *win_drive = get_windows_drive();
    node_t *drvs = all_drives();
    menu->add(until_first_slash(win_drive->path), 0, 0, 0, 0);
    menu->value(0);
    for(node_t *d = drvs; d != NULL; d = d->next) {
        drive_t *drv = (drive_t *)d->val;
        if(drv->path[0] != win_drive->path[0])
            menu->add(until_first_slash(drv->path), 0, 0, 0, 0);
    }
    free_drive_list(drvs);
    free_drive(win_drive);
    check_id();

    // uninstall list
    if(entries)
        free_list_installed(entries);
    entries = list_installed(lick);
    w->lst_uninstall->clear();
    for(node_t *e = entries; e != NULL; e = e->next) {
        installed_t *install = (installed_t *)e->val;
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

bool is_loader(ipc_lick *c) {
    return c->type() == IPC_LOADER;
}
bool is_loader(ipc_lick *c, bool install) {
    if(!is_loader(c))
        return false;
    ipc_loader *l = (ipc_loader *)c;
    return (install && l->install) || (!install && !l->install);
}

void Frontend::handle_status(ipc_lick *c, ipc_status *s) {
    switch(c->type()) {
    case IPC_INSTALL:
        if(!s->ret) {
            handle_error("Error installing ISO!");
            clear_commands_queue();
            return;
        }
        refresh_window();
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
            if(fl_choice("Uninstalled successfully!\nDo you wish to uninstall the boot loader?", "Yes", "No", NULL) == 0) {
                remove_next = 0;
            }
        } else {
            fl_message("Uninstalled successfully!");
        }

        if(remove_next) {
            delete commands_queue.front();
            commands_queue.pop();
        }

        }break;
    case IPC_CHECK_LOADER:
        if(!commands_queue.empty() && s->ret
                && is_loader(commands_queue.front(), true)) {
            delete commands_queue.front();
            commands_queue.pop();
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
    default:
        return;
    }
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
        id_bg = w->text_id->color();
        refresh_window();
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
        delete command;
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
            send_command(send, commands_queue.front());
            waiting_for_backend = true;
        }
    }
    return 0;
}

int Frontend::main(int argc, char **argv) {
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
