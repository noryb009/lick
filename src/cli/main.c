#include <boot-loader/me.h>
#include <lick.h>
#include <stdlib.h>
#include <string.h>

#include "arguments.h"
#include "commands.h"
#include "program-status.h"
#include "user-interface.h"

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

    if(a->check_fix_loader) {
        p->loader = get_loader(p->info);
        if(!p->loader) {
            free_program_status(p);
            free_program_args(a);
            return 1;
        }
        p->menu = get_menu(p->loader);
        if(!p->menu) {
            free_program_status(p);
            free_program_args(a);
            return 1;
        }
        int ret = check_fix_loader(p->loader, p->menu, p->lick);
        if(p->volume > VOLUME_SILENCE) {
            printf("Boot loader fix is ");
            if(ret != 1)
                printf("not ");
            printf("installed\n");
        }
        free_program_status(p);
        free_program_args(a);
        switch(ret) {
        case 1:
            return 2; // Installed.
        default:
            return 3; // Not installed.
        }
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

    if(a->fix_loader == 1) {
        int ret = fix_loader(p->loader, p->menu, p->lick);
        switch(ret) {
        case -1:
            if(p->volume > VOLUME_SILENCE)
                printf("Boot loader not installed\n");
            return 1;
        case 0:
            if(p->volume > VOLUME_SILENCE)
                printf("Error installing loader fix\n");
            return 1;
        case 1:
            if(p->volume > VOLUME_SILENCE)
                printf("Installed loader fix\n");
            break;
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
