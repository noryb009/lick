#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "user-interface.h"

int ask_int() {
    int ret;
    char c;
    if(scanf("%d%c", &ret, &c) == 0 || c != '\n') {
        while (getchar() != '\n') {}
        return INVALID_INT;
    }

    return ret;
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

char *ask_iso() {
    printf("ISO file or CD drive: ");
    char *c = read_line(stdin);
    if(strcmp(c, "") == 0 || strcmp(c, "\n") == 0) {
        free(c);
        return NULL;
    }
    return c;
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
    volume_e old_vol = p->volume;
    if(ask_bool(1, "Invalid input. Press enter or n, then enter.\n"))
        p->volume = VOLUME_NO_QUESTIONS;
    int ret = install_iso(p, strdup2(iso));
    p->volume = old_vol;
    return ret;
}

int main_menu(program_status_t *p) {
    if(p->volume <= VOLUME_NO_QUESTIONS)
        return 0;

    while(1) {
        printf("\n\nMain menu:\n");
        printf("1) Install ISO or from CD\n");
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

int entry_submenu(program_status_t *p) {
    while(1) {
        printf("\n\nEntry menu:\n");
        printf("Entries:\n");
        installed_node_t *entries = list_installed(p->lick);
        for(installed_node_t *n = entries; n != NULL; n = n->next)
            printf("- %s (%s)\n", n->val->name, n->val->id);

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

int ask_uninstall(program_status_t *p) {
    installed_node_t *entries = list_installed(p->lick);
    int c;
    while(1) {
        int len = 0;
        for(installed_node_t *n = entries; n != NULL; n = n->next, ++len)
            printf("%d) %s\n", len + 1, n->val->name);

        if(len == 0) {
            printf("No entries!\n");
            return 0;
        }
        printf("0) Back\n");

        c = ask_int();

        if(c == 0) {
            free_installed_node_t(entries);
            return 0;
        }
        else if(c > 0 && c <= len)
            break;
    }

    installed_node_t *n = entries;
    for(int i = 1; i < c; i++)
        n = n->next;

    installed_t *install = n->val;
    char *q = concat_strs(5, "Are you sure you wish to uninstall ",
            install->name, " (", install->id, ")? [Y/n]\n");
    printf("%s", q);
    char *invalid = concat_strs(2, "Invalid input. ", q);
    int check = ask_bool(1, invalid);

    free(invalid);
    free(q);
    if(!check)
        return 0;

    int ret = uninstall_id(p, install->id);
    free_installed_node_t(entries);
    return ret;
}

char *ask_drive(drive_node_t *drives) {
    while(1) {
        int i = 1;
        for(drive_node_t *n = drives; n != NULL; ++i, n = n->next) {
            printf("%d) %s\n", i, n->val->path);
        }
        if(i == 1)
            return NULL;
        int choice = ask_int();
        if(choice < 1 || choice >= i) {
            printf("Select a valid drive.\n\n");
            continue;
        } else {
            drive_node_t *n = drives;
            for(int j = 1; j < choice; ++j) {
                n = n->next;
            }
            drive_t *drv = n->val;
            char *ret = strdup2(drv->path);
            free_drive_node_t(drives);
            return ret;
        }
    }
}
