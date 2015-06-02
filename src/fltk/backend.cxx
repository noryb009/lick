#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "backend.hpp"
#include "drop-area.hpp"

void Backend::clear() {
    w = NULL;
    lick = NULL;
    loader = NULL;
    menu = NULL;
    info = NULL;

    iso = NULL;
    entries = NULL;
}

void Backend::reset() {
    if(w)
        delete w;
    if(info)
        free_sys_info(info);
    if(menu)
        free_menu(menu);
    if(loader)
        free_loader(loader);
    if(lick)
        free_lick_dir(lick);

    if(iso)
        free(iso);
    if(entries)
        free_list_installed(entries);
    clear();
}

void Backend::handle_error(const char *err) {
    if(lick->err)
        fl_alert("%s\n%s", err, lick->err);
    else
        fl_alert("%s", err);
}

Backend::Backend() {
    clear();
}

Backend::~Backend() {
    reset();
}

void Backend::handle_file(const char *file) {
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

    iso = strdup(file);
}

void Backend::on_drop() {
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

void Backend::on_install() {
    if(iso == NULL) {
        fl_alert("Please select an ISO.");
        return;
    }
    if(!check_id()) {
        fl_alert("Please enter a valid ID.");
        return;
    }

    if(!check_loader(loader, info))
        if(!install_loader(loader, info, lick)) {
            handle_error("Error installing loader!");
            return;
        }

    char *install_to = concat_strs(3, w->choice_install_drive->text(), "/", w->text_id->value());
    if(!install(w->text_id->value(), w->text_name->value(), iso, install_to, lick, menu)) {
        handle_error("Error installing ISO!");
        free(install_to);
        return;
    }
    free(install_to);
    fl_message("Installed successfully!");
}

void Backend::on_uninstall() {
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
    if(!uninstall(ent->id, lick, menu)) {
        handle_error("There was an error uninstalling the installation.");
        return;
    }

    refresh_window();
    if(entries == NULL) {
        if(fl_choice("Do you wish to uninstall the boot loader?", "Yes", "No", NULL) == 0) {
            if(!uninstall_loader(loader, info, lick)) {
                handle_error("There was an error uninstalling the loader.");
                return;
            }
        }
    }
}

int Backend::check_id(int ok) {
    if(ok)
        w->text_id->color(id_bg);
    else
        w->text_id->color(FL_RED);
    w->text_id->redraw();
    return ok;
}

int Backend::check_id() {
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

void Backend::refresh_window() {
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

int Backend::main(int argc, char **argv) {
    reset();

    info = get_system_info();
    if(info->is_admin != ADMIN_YES) {
        fl_alert("LICK must be run as an admin.");
        return 1;
    }

    lick = get_lickdir();
    if(lick == NULL) {
        fl_alert("LICK is not in a valid location. Please make sure you have extracted or installed LICK completely.");
        return 1;
    }

    loader = get_loader(info);
    if(loader != NULL)
        menu = get_menu(loader);
    if(loader == NULL || menu == NULL) {
        switch(info->version) {
            case V_WINDOWS_ME:
                fl_alert("LICK is not supported on Windows ME. There's nothing we can do. Sorry. :(");
                break;
            case V_UNKNOWN:
                fl_alert("You are using an unknown version of Windows. Make sure you are using the latest version of LICK, and if so, open an issue at github.com/noryb009/lick");
                break;
            default:
                fl_alert("Something went wrong. Please open a new issue here: github.com/noryb009/lick\nInclude the following informaion:\nFamily: %s\nVersion: %s\nArch: %s\nBios: %s",
                        info->family_name, info->version_name, info->arch_name, info->bios_name);
                break;
        }
        return 1;
    }

    w = new lick_fltk();
    Fl_Double_Window *window = w->make_window(this);
    id_bg = w->text_id->color();
    window->show();
    refresh_window();
    return Fl::run();
}
