#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "cli.h"

program_status_t *new_program_status() {
    program_status_t *p = malloc(sizeof(program_status_t));
    p->lick_drive = NULL;
    p->lick = NULL;
    p->loader = NULL;
    p->menu = NULL;
    p->info = NULL;
    return p;
}

int is_iso_file(char *f) {
    char *iso = strstr(f, ".iso");
    while(iso != NULL) {
        if(strcmp(iso, ".iso") == 0)
            return 1;
        iso = strstr(iso + 1, ".iso");
    }
    return 0;
}

void handle_error(program_status_t *p) {
    if(p->lick->err == NULL)
        fprintf(stderr, "Error: an unknown error occurred.");
    else
        fprintf(stderr, "Error: %s", p->lick->err);
}

// 1 = yes, 0 = no
int ask_bool(int default_val, char *try_again) {
    char *buf = NULL;
    size_t n;

    while(1) {
        buf = read_line(stdin);
        if(strlen(buf) > 0) {
            switch(buf[0]) {
                case 'y':
                case 'Y':
                case 't':
                case 'T':
                    free(buf);
                    return 1;
                case 'n':
                case 'N':
                case 'f':
                case 'F':
                    free(buf);
                    return 0;
            }
        } else if(default_val == 0 || default_val == 1) {
            free(buf);
            return default_val;
        }
        free(buf);
        printf("%s", try_again);
    }
}

#define INVALID_INT -1
int ask_int() {
    int ret;
    char c;
    if(scanf("%d%c", &ret, &c) == 0 || c != '\n') {
        while (getchar() != '\n') {}
        return INVALID_INT;
    }

    return ret;
}

void ask_lick_dir(program_status_t *p) {
    if(p->volume > VOLUME_NO_QUESTIONS && 0) {
        // ask user
        // TODO
    } else {
        // default drive
        p->lick_drive = get_likely_lick_drive();
        if(p->lick_drive == NULL) {
            p->lick_drive = get_windows_drive();
            printf("TODO: create LICK directory");
            exit(1); // TODO: create LICK directories
        }
        char *lick_path = concat_strs(2, p->lick_drive->path, "/lick");
        p->lick = expand_lick_dir(lick_path);
        free(lick_path);
    }
}

char *ask_iso(program_status_t *p) {
    printf("ISO file: ");
    char *c = read_line(stdin);
    if(strcmp(c, "") == 0 && strcmp(c, "\n") == 0) {
        free(c);
        return NULL;
    }
    return c;
}

char *ask_drive() {
    while(1) {
        node_t *drives = all_drives();
        int i = 1;
        for(node_t *n = drives; n != NULL; ++i, n = n->next) {
            drive_t *drv = (drive_t *)n->val;
            printf("%d) %s\n", i, drv->path);
        }
        if(i == 1)
            return NULL;
        int choice = ask_int();
        if(choice < 1 || choice >= i) {
            printf("Select a valid drive.\n\n");
            free_drive_list(drives);
            continue;
        } else {
            node_t *n = drives;
            for(int j = 1; j < choice; ++j) {
                n = n->next;
            }
            drive_t *drv = n->val;
            char *ret = strdup(drv->path);
            free_drive_list(drives);
            return ret;
        }
    }
}

int install_iso(program_status_t *p, char *iso) {
    if(iso == NULL)
        iso = ask_iso(p);
    if(iso == NULL)
        return 0;

    if(!p->lick_drive)
        ask_lick_dir(p);
    if(!p->loader)
        p->loader = get_loader(p->info);
    if(!p->menu)
        p->menu = get_menu(p->loader);

    char *drive;
    char *id;
    char *name;
    char *auto_name = gen_name(iso);

    if(p->volume > VOLUME_NO_QUESTIONS) {
        printf("Install to drive:\n");
        drive = ask_drive();

        // ID
        char *auto_id = gen_id(iso, p->lick, drive);
        while(1) {
            printf("Enter ID [%s]:\n", auto_id);
            id = read_line(stdin);
            if(id == NULL || id[0] == '\0') {
                if(id)
                    free(id);
                id = auto_id;
                break;
            } else if(!is_valid_id(id, p->lick, drive)) {
                free(id);
                printf("Invalid ID. IDs can only contain A-Z, a-z, 0-9, '.', '-' and '_'\n");
            } else {
                free(auto_id);
                break;
            }
        }

        printf("Enter Name [%s]:\n", auto_name);
        name = read_line(stdin);
        if(name == NULL || name[0] == '\0') {
            if(name)
                free(name);
            name = auto_name;
        } else
            free(auto_name);
    } else {
        drive_t *drv = get_likely_lick_drive();
        drive = strdup(drv->path);
        free_drive(drv);

        id = gen_id(iso, p->lick, drive);
        name = auto_name;
    }

    char *install_to = concat_strs(3, drive, "/", id);
    free(drive);

    // install
    if(!check_loader(p->loader, p->info))
        if(!install_loader(p->loader, p->info, p->lick))
            goto free_and_handle_error;

    if(!install(id, name, iso, install_to, p->lick, p->menu))
        goto free_and_handle_error;

    free(id);
    free(name);
    free(install_to);
    return 1;
free_and_handle_error:
    free(id);
    free(name);
    free(install_to);
    handle_error(p);
    return 0;
}

int ask_uninstall_loader(program_status_t *p) {
    if(p->volume <= VOLUME_NO_QUESTIONS)
        return 1;
    printf("Do you wish to uninstall the boot loader? [Y/n]\n");
    return ask_bool(1,
            "Invalid input. Do you wish to uninstall the boot loader? [Y/n]");
}

int uninstall_id(program_status_t *p, char *id) {
    if(!p->lick_drive)
        ask_lick_dir(p);
    if(!p->loader)
        p->loader = get_loader(p->info);
    if(!p->menu)
        p->menu = get_menu(p->loader);

    // uninstall
    if(!uninstall(id, p->lick, p->menu)) {
        handle_error(p);
        return 0;
    }

    // check if last entry
    node_t *entries = list_installed(p->lick);
    if(entries == NULL)
        if(check_loader(p->loader, p->info))
            if(ask_uninstall_loader(p))
                if(!uninstall_loader(p->loader, p->info, p->lick)) {
                    handle_error(p);
                    // don't return error - still uninstalled entry
                }
    free_list_installed(entries);

    return 1;
}

int ask_uninstall(program_status_t *p) {
    node_t *entries = list_installed(p->lick);
    int c;
    while(1) {
        int len = 0;
        for(node_t *n = entries; n != NULL; n = n->next, ++len)
            printf("%d) %s\n", len + 1, ((installed_t *)n->val)->name);

        if(len == 0) {
            printf("No entries!\n");
            return 0;
        }
        printf("0) Back\n");

        c = ask_int();

        if(c == 0) {
            free_list_installed(entries);
            return 0;
        }
        else if(c > 0 && c <= len)
            break;
    }

    node_t *n = entries;
    for(int i = 1; i < c; i++)
        n = n->next;

    installed_t *install = n->val;
    char *q = concat_strs(5, "Are you sure you wish to uninstall ",
            install->name, " (", install->id, ")? [Y/n]\n");
    printf(q);
    char *invalid = concat_strs(2, "Invalid input. ", q);
    int check = ask_bool(1, invalid);

    free(invalid);
    free(q);
    if(!check)
        return 0;

    int ret = uninstall_id(p, install->id);
    free_list_installed(entries);
    return ret;
}

int entry_submenu(program_status_t *p) {
    while(1) {
        printf("\n\nEntry menu:\n");
        printf("Entries:\n");
        node_t *entries = list_installed(p->lick);
        for(node_t *n = entries; n != NULL; n = n->next)
            printf("- %s (%s)\n", ((installed_t *)n->val)->name, ((installed_t *)n->val)->id);

        printf("1) Uninstall\n");
        printf("2) Return to main menu\n");

        printf("Choice: ");

        switch(ask_int()) {
            case 1:
                ask_uninstall(p);
                break;
            case 2:
                return 0;
            default:
                printf("Invalid number\n");
        }
    }
}

int main_menu(program_status_t *p) {
    if(p->volume <= VOLUME_NO_QUESTIONS)
        return 0;

    if(!p->lick_drive)
        ask_lick_dir(p);

    while(1) {
        printf("\n\nMain menu:\n");
        printf("1) Install ISO\n");
        printf("2) Entry submenu (view/delete)\n");
        printf("3) Regenerate boot loader menu\n");
        printf("4) Quit\n");

        printf("Choice: ");

        switch(ask_int()) {
            case 1:
                install_iso(p, NULL);
                break;
            case 2:
                entry_submenu(p);
                break;
            case 3:
                if(p->loader == NULL)
                    p->loader = get_loader(p->info);
                if(p->menu == NULL)
                    p->menu = get_menu(p->loader);
                if(p->menu->regenerate(p->lick))
                    printf("Done.\n");
                else
                    handle_error(p);
                break;
            case 4:
                exit(0);
                break;
            default:
                printf("Invalid number\n");
        }
    }
}

int main(int argc, char *argv[]) {
    program_status_t *p = new_program_status();

    p->info = get_system_info();
    p->volume = VOLUME_NORMAL;

    if(!p->loader)
        p->loader = get_loader(p->info);
    if(!p->menu)
        p->menu = get_menu(p->loader);

    // TODO: real command line processing
    if(argc > 1) {
        for(int i = 1; i < argc; ++i) {
            if(is_iso_file(argv[i])) {
                enum VOLUME volume = p->volume;
                printf("For auto-install, press enter. Otherwise, press n, then enter.\n");
                if(ask_bool(1, "Invalid input. Press enter or n, then enter.\n")) {
                    p->volume = VOLUME_NO_QUESTIONS;
                }
                install_iso(p, argv[i]);
                p->volume = volume;
                break;
            }
        }
    }

    return main_menu(p);
}
