#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "backend.hpp"

Backend::Backend() {
    send = NULL;
    recv = NULL;

    loader = NULL;
    menu = NULL;
    info = NULL;
}

Backend::~Backend() {
    if(send)
        delete send;
    if(recv)
        delete recv;
}

void Backend::progress_cb(uniso_progress_t cur, uniso_progress_t total) {
    send_progress(send, cur, total);
}

void inner_progress_cb(uniso_progress_t cur, uniso_progress_t total, void *backend) {
    ((Backend *)backend)->progress_cb(cur, total);
}

int Backend::event_loop() {
    for(;;) {
        ipc_lick *c = recv_command(recv);
        if(send->had_error() || recv->had_error() || c == NULL) {
            //printf("Error, exiting...\n");
            send_error(send, "Unknown error.");
            delete send;
            delete recv;
            return 1;
        }

        switch(c->type()) {
        case IPC_EXIT:
            return 0;
        case IPC_INSTALL: {
            ipc_install *in = (ipc_install *)c;
            int ret = install_cb(in->id, in->name, in->iso,
                        in->install_dir, in->lick, menu, inner_progress_cb,
                        this);
            send_status(send, ret, in->lick->err);
            }break;
        case IPC_UNINSTALL: {
            ipc_uninstall *un = (ipc_uninstall *)c;
            int ret = uninstall(un->id, un->lick, menu);
            send_status(send, ret, un->lick->err);
            }break;
        case IPC_CHECK_LOADER:
            send_status(send, check_loader(loader, info), NULL);
            break;
        case IPC_LOADER: {
            ipc_loader *l = (ipc_loader *)c;
            int ret;
            if(l->install) {
                ret = install_loader(loader, info, l->lick);
            } else
                ret = uninstall_loader(loader, info, l->lick);
            send_status(send, ret, l->lick->err);
            }break;
        case IPC_READY:
        case IPC_STATUS:
        case IPC_PROGRESS:
        case IPC_ERROR:
            //printf("Received command sent the wrong way.\n");
            break;
        }
        delete c;
    }
}

int Backend::main(int argc, char *argv[]) {
    char *p = get_program_path();

    info = get_system_info();

    // Win98 gets stuck showing the window
    if(info->family == F_WINDOWS_9X) {
        free_sys_info(info);
        free(p);

        MessageBox(NULL,
                "The LICK GUI does not work on your system. Try running the command line interface.",
                "LICK", MB_OK);
        return run_system("lick-cli.exe");
    }

    if(info->is_admin != ADMIN_YES) {
        int try_uac = 1;
        for(int i = 0; i < argc; ++i) {
            if(strcmp(argv[i], "--no-try-uac") == 0) {
                try_uac = 0;
                break;
            }
        }
        if(try_uac) {
            int process_ret;
            char *args = concat_strs(2, get_command_line(), " --no-try-uac");
            int ret = run_privileged(p, args, &process_ret);
            free(args);
            if(ret) {
                free(p);
                return process_ret;
            }
        }
        // if already tried, or if the call fails, then continue, make the
        //   frontend, and display an error
    }

    char *c = concat_strs(2, p, " --frontend");
#ifdef _WIN32
    HANDLE input, output;
    if(!run_unprivileged(c, &input, &output)) {
        free(p);
        free(c);
        return 1;
    }
    free(p);
    free(c);
    send = new ipc(DIRECTION_SEND, (pipe_t)input);
    recv = new ipc(DIRECTION_RECV, (pipe_t)output);
#else
    return 0;
#endif

    // get ready
    if(info->is_admin != ADMIN_YES) {
        send_error(send, "LICK must be run as an admin.");
        return 1;
    }

    loader = get_loader(info);
    if(loader)
        menu = get_menu(loader);
    if(!loader || !menu) {
        if(info->version == V_UNKNOWN)
            send_error(send, "You are using an unknown version of Windows. Make sure you are using the latest version of LICK, and if so, open an issue at github.com/noryb009/lick");
        else {
            char *str = concat_strs(8, "Something went wrong. Please open a new issue here: github.com/noryb009/lick\nInclude the following informaion:\nFamily: ",
                    info->family_name, "\nVersion: ", info->version_name, "\nArch: ", info->arch_name, "\nBios: ", info->bios_name);
            send_error(send, str);
            free(str);
        }
        return 1;
    }

    send_command(send, IPC_READY);
    return event_loop();
}
