#include <lick.h>

#include "commands.h"
#include "user-interface.h"

void handle_error(program_status_t *p) {
    if(p->lick->err == NULL)
        fprintf(stderr, "Error: an unknown error occurred.\n");
    else
        fprintf(stderr, "Error: %s\n", p->lick->err);
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
    distro_t *distro = get_distro_by_key("puppy");
    int ret = 0;
    do {
        if(!check_loader(p->loader))
            if(!install_loader(p->loader, p->info, p->lick))
                break;

        if(!install(id, name, distro, iso, install_to, p->lick, p->menu))
            break;

        ret = 1;
    } while(0);

    free(distro);
    free(id);
    free(name);
    free(iso);
    free(install_to);
    if(!ret)
        handle_error(p);
    return ret;
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
