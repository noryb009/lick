#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "boot-loader/me.h"

program_status_t *new_program_status() {
    program_status_t *p = malloc(sizeof(program_status_t));
    p->lick = NULL;
    p->loader = NULL;
    p->menu = NULL;
    p->info = NULL;
    p->volume = VOLUME_NORMAL;
    return p;
}

void free_program_status(program_status_t *p) {
    if(p->lick)
        free_lickdir(p->lick);
    if(p->loader)
        free_loader(p->loader);
    if(p->menu)
        free_menu(p->menu);
    if(p->info)
        free_sys_info(p->info);
    free(p);
}

void free_program_args(program_args_t *a) {
    free_list(a->install, NULL);
    free_list(a->uninstall, NULL);
    free(a);
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
        fprintf(stderr, "Error: an unknown error occurred.\n");
    else
        fprintf(stderr, "Error: %s\n", p->lick->err);
}

// 1 = yes, 0 = no
int ask_bool(int default_val, char *try_again) {
    char *buf = NULL;

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

char *ask_drive(node_t *drives) {
    while(1) {
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
            continue;
        } else {
            node_t *n = drives;
            for(int j = 1; j < choice; ++j) {
                n = n->next;
            }
            drive_t *drv = n->val;
            char *ret = strdup2(drv->path);
            free_drive_list(drives);
            return ret;
        }
    }
}

char *ask_iso() {
    printf("ISO file: ");
    char *c = read_line(stdin);
    if(strcmp(c, "") == 0 || strcmp(c, "\n") == 0) {
        free(c);
        return NULL;
    }
    return c;
}

int install_iso(program_status_t *p, char *iso) {
    if(iso == NULL)
        iso = ask_iso();
    if(iso == NULL)
        return 0;

    if(!p->loader)
        p->loader = get_loader(p->info);
    if(!p->menu)
        p->menu = get_menu(p->loader);

    char *drive;
    char *id;
    char *name;
    char *auto_name;

    if(p->volume > VOLUME_NO_QUESTIONS) {
        printf("Install to drive:\n");
        drive = ask_drive(all_drives());

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
        auto_name = gen_name(id);

        printf("Enter Name [%s]:\n", auto_name);
        name = read_line(stdin);
        if(name == NULL || name[0] == '\0') {
            if(name)
                free(name);
            name = auto_name;
        } else
            free(auto_name);
    } else {
        drive = strdup2(p->lick->drive);
        id = gen_id(iso, p->lick, drive);
        name = gen_name(id);
    }

    char *install_to = concat_strs(3, drive, "/", id);
    free(drive);

    // install
    int ret = 0;
    do {
        if(!check_loader(p->loader))
            if(!install_loader(p->loader, p->info, p->lick))
                break;

        if(!install(id, name, iso, install_to, p->lick, p->menu))
            break;

        ret = 1;
    } while(0);

    free(id);
    free(name);
    free(iso);
    free(install_to);
    if(!ret)
        handle_error(p);
    return ret;
}

int ask_install_loader(program_status_t *p) {
    if(p->volume <= VOLUME_NO_QUESTIONS)
        return 1;
    printf("Do you wish to install the boot loader? [Y/n]\n");
    return ask_bool(1,
            "Invalid input. Do you wish to install the boot loader? [Y/n]");
}

int ask_uninstall_loader(program_status_t *p) {
    if(p->volume <= VOLUME_NO_QUESTIONS)
        return 1;
    printf("Do you wish to uninstall the boot loader? [Y/n]\n");
    return ask_bool(1,
            "Invalid input. Do you wish to uninstall the boot loader? [Y/n]");
}

int uninstall_id(program_status_t *p, char *id) {
    // uninstall
    if(!uninstall(id, p->lick, p->menu)) {
        handle_error(p);
        return 0;
    }

    // check if last entry
    node_t *entries = list_installed(p->lick);
    if(entries == NULL)
        if(check_loader(p->loader))
            if(ask_uninstall_loader(p))
                if(!uninstall_loader(p->loader, 0, p->info, p->lick)) {
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

    while(1) {
        printf("\n\nMain menu:\n");
        printf("1) Install ISO\n");
        printf("2) Entry submenu (view/delete)\n");
        printf("3) Install/uninstall boot loader\n");
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
            if(check_loader(p->loader)) {
                if(ask_uninstall_loader(p))
                    if(!uninstall_loader(p->loader, 0, p->info, p->lick))
                        printf("Error uninstalling loader!\n");
            } else {
                if(ask_install_loader(p))
                    if(!install_loader(p->loader, p->info, p->lick))
                        printf("Error installing loader!\n");
            }
            break;
        case 4:
            free_program_status(p);
            exit(0);
            break;
        default:
            printf("Invalid number\n");
        }
    }
}

int auto_install(program_status_t *p, char *iso) {
    if(p->volume <= VOLUME_NO_QUESTIONS)
        return install_iso(p, strdup2(iso));

    char *id = gen_id(iso, p->lick, p->lick->drive);
    char *name = gen_name(id);
    char *path = unix_path(concat_strs(3, p->lick->drive, "/", id));

    printf("Auto install:\n");
    printf("\tInstall to: %s\n", path);
    printf("\tID: %s\n", id);
    printf("\tName: %s\n", name);

    free(id);
    free(name);
    free(path);

    printf("For auto-install, press enter. Otherwise, press n, then enter.\n");
    enum VOLUME old_vol = p->volume;
    if(ask_bool(1, "Invalid input. Press enter or n, then enter.\n"))
        p->volume = VOLUME_NO_QUESTIONS;
    int ret = install_iso(p, strdup2(iso));
    p->volume = old_vol;
    return ret;
}

void print_help() {
    printf("Usage: lick-cli [options]...\n");
    printf("\n");
    printf("Volume:\n");
    printf("      --verbose          Be verbose (default)\n");
    printf("  -m, --no-menu          Ask questions, but do not show a menu\n");
    printf("      --no-questions     Do not ask questions; assume yes\n");
    printf("  -s, --silent           No output; assume yes\n");
    printf("\n");
    printf("Install/uninstall:\n");
    printf("  -c, --check-loader     Check if boot loader is installed\n");
    printf("      --install-loader   Install boot loader\n");
    printf("      --uninstall-loader   Uninstall boot loader\n");
    printf("  -i, --install          Install the given ISO\n");
    printf("  -u, --uninstall        Uninstall the given ID\n");
    printf("      --uninstall-all    Uninstall all IDs and the bootloader\n");
    printf("\n");
    printf("      --check-program    Check if the binary can run\n");
    printf("      --ignore-errors    If errors occur, continue\n");
    printf("      --no-try-uac       Do not ask to elevate process\n");
    printf("      --no-me-check      Ignore possible ME incompatibilities\n");
    printf("  -h, --help             Show this help\n");
}

program_args_t *handle_args(program_status_t *p, int argc, char **argv) {
    program_args_t *a = malloc(sizeof(program_args_t));
    a->check_program = 0;
    a->try_uac = 1;
    a->ignore_errors = 0;
    a->me_check = 1;
    a->check_loader = 0;
    a->install_loader = -1;
    a->install = NULL;
    a->uninstall = NULL;
    a->uninstall_all = 0;
    a->reinstall = 0;

    struct option ops[] = {
        // meta
        {"help", no_argument, 0, 'h'},
        {"check-program", no_argument, &a->check_program, 1},
        {"no-try-uac", no_argument, &a->try_uac, 0},
        {"ignore-errors", no_argument, &a->ignore_errors, 1},
        {"no-me-check", no_argument, &a->me_check, 0},
        // volume
        {"verbose", no_argument, 0, 'V'},
        {"no-menu", no_argument, 0, 'm'},
        {"no-questions", no_argument, 0, 'Q'},
        {"silent", no_argument, 0, 's'},
        // install/uninstall
        {"check-loader", no_argument, 0, 'c'},
        {"install-loader", no_argument, &a->install_loader, 1},
        {"uninstall-loader", no_argument, &a->install_loader, 0},
        {"install", required_argument, 0, 'i'},
        {"uninstall", required_argument, 0, 'u'},
        {"uninstall-all", no_argument, &a->uninstall_all, 1},
        {"reinstall", no_argument, &a->reinstall, 1},
        {0, 0, 0, 0}
    };
    int c;
    while((c = getopt_long(argc, argv, "chi:msu:", ops, NULL)) != -1) {
        switch(c) {
        case 0: // set flag
            break;
        case 'v':
            p->volume = VOLUME_NORMAL;
            break;
        case 'm':
            p->volume = VOLUME_NO_MENU;
            break;
        case 'Q':
            p->volume = VOLUME_NO_QUESTIONS;
            break;
        case 's':
            p->volume = VOLUME_SILENCE;
            break;
        case 'i':
            if(strcmp(optarg, "--") == 0) {
                for(int i = optind; i < argc; ++i)
                    a->install = new_node(argv[i], a->install);
                optind = argc;
            } else
                a->install = new_node(optarg, a->install);
            break;
        case 'u':
            if(strcmp(optarg, "--") == 0) {
                for(int i = optind; i < argc; ++i)
                    a->uninstall = new_node(argv[i], a->uninstall);
                optind = argc;
            } else
                a->uninstall = new_node(optarg, a->uninstall);
            break;
        case 'c':
            a->check_loader = 1;
            break;
        case 'h':
        case '?':
        default:
            print_help();
            free_program_status(p);
            free_program_args(a);
            if(c == 'h')
                exit(0);
            else
                exit(1);
        }
    }
    a->install = list_reverse(a->install);
    a->uninstall = list_reverse(a->uninstall);
    return a;
}

int main(int argc, char **argv) {
    program_status_t *p = new_program_status();
    program_args_t *a = handle_args(p, argc, argv);
    int ret = 0;

    p->info = get_system_info();
    p->lick = get_lickdir();

    if(a->check_program) {
        int ret = 0;
        p->loader = get_loader(p->info);
        if(!p->lick || !p->loader || p->info->is_admin != ADMIN_YES)
            ret = 1;
        else if(a->me_check && p->info->version == V_WINDOWS_ME
                && !check_loader_me_patch())
            ret = 1;
        free_program_status(p);
        free_program_args(a);
        return ret;
    }

    if(!p->lick) {
        if(p->volume > VOLUME_SILENCE)
            printf("LICK is not in a valid location. Please make sure you have extracted or installed LICK completely.\n");
        free_program_status(p);
        free_program_args(a);
        return 1;
    }

    if(p->info->is_admin != ADMIN_YES) {
        if(a->try_uac && p->volume > VOLUME_NO_QUESTIONS) {
            char *program = get_program_path();
            char *args = concat_strs(2, get_command_line(), " --no-try-uac");
            int process_ret;
            int ret = run_privileged(program, args, &process_ret);
            free(args);
            free(program);
            if(ret) {
                free_program_status(p);
                free_program_args(a);
                return process_ret;
            }
        }
        if(p->volume > VOLUME_SILENCE)
            printf("Must be admin.\n");
        free_program_status(p);
        free_program_args(a);
        return 1;
    }

    if(a->check_loader) {
        p->loader = get_loader(p->info);
        if(!p->loader) {
            free_program_status(p);
            free_program_args(a);
            return 1;
        }
        int ret = check_loader(p->loader);
        if(p->volume > VOLUME_SILENCE) {
            printf("Boot loader is ");
            if(!ret)
                printf("not ");
            printf("installed\n");
        }
        free_program_status(p);
        free_program_args(a);
        return ret;
    }

    p->loader = get_loader(p->info);
    if(p->loader)
        p->menu = get_menu(p->loader);
    if(!p->loader || !p->menu) {
        if(p->info->version == V_UNKNOWN)
            printf("You are using an unknown version of Windows. Make sure you are using the latest version of LICK, and if so, open an issue at github.com/noryb009/lick");
        else
            printf("Something went wrong. Please open a new issue here: github.com/noryb009/lick\nInclude the following informaion:\nFamily: %s\nVersion: %s\nArch: %s\nBios: %s",
                p->info->family_name, p->info->version_name,
                p->info->arch_name, p->info->bios_name);
        free_program_status(p);
        free_program_args(a);
        return 1;
    }

    if(a->me_check && p->info->version == V_WINDOWS_ME
           && !check_loader_me_patch()) {
        if(p->volume <= VOLUME_NO_QUESTIONS) {
            free_program_status(p);
            free_program_args(a);
            return 1;
        }

        char *drv = get_windows_drive_path();
        if(!drv) {
            printf("There was an error finding Windows.\n");
            free_program_status(p);
            free_program_args(a);
            return 1;
        }
        char *me2dos = unix_path(concat_strs(2, drv, "/Me2Dos"));
        free(drv);
        printf("By default, Windows ME does not have everything LICK requires.\n");
        printf("\n");
        printf("To fix this, you can install a program Me2Dos, which you can get from:\n");
        printf("    http://www.rkgage.net/bobby/download/Me2Dos.exe\n");
        printf("Or view the README at:\n");
        printf("    http://www.rkgage.net/bobby/download/readme.txt\n");
        printf("\n");
        printf("If you have enabled booting to DOS another way, create the\n");
        printf("folder %s to disable this message, or press `c' to continue.\n", me2dos);
        printf("\n");
        printf("Press enter to exit.\n");
        free(me2dos);
        int c = getchar();
        if(c != 'c') {
            free_program_status(p);
            free_program_args(a);
            return 1;
        }
    }

    if(a->install_loader == 1) {
        if(check_loader(p->loader)) {
            if(p->volume > VOLUME_SILENCE)
                printf("Loader already installed\n");
        } else if(!install_loader(p->loader, p->info, p->lick)) {
            if(p->volume > VOLUME_SILENCE)
                printf("Error installing loader\n");
            return 1;
        }
    }

    if(a->uninstall_all) {
        node_t *entries = list_installed(p->lick);
        for(node_t *n = entries; n != NULL; n = n->next) {
            installed_t *install = n->val;
            if(!uninstall(install->id, p->lick, p->menu)) {
                if(p->volume > VOLUME_SILENCE)
                    printf("Could not uninstall %s\n", install->id);
                if(!a->ignore_errors) {
                    return 1;
                }
                ret = 1;
            } else {
                if(p->volume > VOLUME_SILENCE)
                    printf("Uninstalled %s\n", install->id);
            }
        }
        free_list_installed(entries);
    } else if(a->uninstall) {
        for(node_t *n = a->uninstall; n != NULL; n = n->next) {
            if(uninstall((char *)n->val, p->lick, p->menu)) {
                if(p->volume > VOLUME_SILENCE)
                    printf("Uninstalled %s\n", (char *)n->val);
            } else {
                if(p->volume > VOLUME_SILENCE)
                    printf("Could not uninstall %s\n", (char *)n->val);
                if(!a->ignore_errors) {
                    return 1;
                }
                ret = 1;
            }
        }
    }

    for(node_t *n = a->install; n != NULL; n = n->next) {
        if(auto_install(p, n->val)) {
            if(p->volume > VOLUME_SILENCE)
                printf("Installed %s\n", (char *)n->val);
        } else {
            if(p->volume > VOLUME_SILENCE) {
                printf("Could not install %s\n", (char *)n->val);
                if(!a->ignore_errors) {
                    return 1;
                }
                ret = 1;
            }
        }
    }

    if((a->install_loader == 0)
            || (a->uninstall_all && a->install == NULL
                && a->install_loader == -1)) {
        if(!check_loader(p->loader)) {
            if(p->volume > VOLUME_SILENCE)
                printf("Loader not installed\n");
        } else if(!uninstall_loader(p->loader, a->reinstall, p->info, p->lick)) {
            if(p->volume > VOLUME_SILENCE)
                printf("Error uninstalling loader\n");
            if(!a->ignore_errors)
                return 1;
            ret = 1;
        } else if(p->volume > VOLUME_SILENCE)
            printf("Loader uninstalled\n");
    }

    free_program_args(a);

    if(p->volume > VOLUME_NO_MENU && ret == 0)
        return main_menu(p);
    else
        return ret;
}
