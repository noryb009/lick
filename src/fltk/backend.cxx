#include <cstdlib>

#include "backend.hpp"

Backend::Backend() {
    send = NULL;
    recv = NULL;
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

void progress_cb(uniso_progress_t cur, uniso_progress_t total, void *backend) {
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

        ipc_install *in;
        ipc_uninstall *un;
        ipc_loader *l;

        switch(c->type()) {
        case IPC_EXIT:
            return 0;
        case IPC_INSTALL:
            in = (ipc_install *)c;
            send_status(send, install(in->id, in->name, in->iso, in->install_dir, in->lick, menu), in->lick->err);
            break;
        case IPC_UNINSTALL:
            un = (ipc_uninstall *)c;
            send_status(send, uninstall(un->id, un->lick, menu), un->lick->err);
            break;
        case IPC_CHECK_LOADER:
            send_status(send, check_loader(loader, info), NULL);
            break;
        case IPC_LOADER:
            l = (ipc_loader *)c;
            if(l->install)
                send_status(send, install_loader(loader, info, l->lick), l->lick->err);
            else
                send_status(send, uninstall_loader(loader, info, l->lick), l->lick->err);
            break;
        case IPC_READY:
        case IPC_STATUS:
        case IPC_ERROR:
            //printf("Received command sent the wrong way.\n");
            break;
        }
        delete c;
    }
}

int Backend::main(int argc, char *argv[]) {
    char *p = get_program_path();
    char *c = concat_strs(2, p, " --frontend");
#ifdef _WIN32
    HANDLE input, output;
    if(!run_unprivileged(c, &input, &output))
        return 1;
    free(p);
    free(c);
    send = new ipc(DIRECTION_SEND, (pipe_t)input);
    recv = new ipc(DIRECTION_RECV, (pipe_t)output);
#else
    return 0;
#endif

    // get ready
    info = get_system_info();
    if(info->is_admin != ADMIN_YES) {
        send_error(send, "LICK must be run as an admin.");
        return 1;
    }

    loader = get_loader(info);
    if(loader)
        menu = get_menu(loader);
    if(!loader || !menu) {
        switch(info->version) {
        case V_WINDOWS_ME:
            send_error(send, "LICK is not supported on Windows ME. There's nothing we can do. Sorry. :(");
            break;
        case V_UNKNOWN:
            send_error(send, "You are using an unknown version of Windows. Make sure you are using the latest version of LICK, and if so, open an issue at github.com/noryb009/lick");
            break;
        default:
            char *str = concat_strs(8, "Something went wrong. Please open a new issue here: github.com/noryb009/lick\nInclude the following informaion:\nFamily: ",
                    info->family_name, "\nVersion: ", info->version_name, "\nArch: ", info->arch_name, "\nBios: ", info->bios_name);
            send_error(send, str);
            free(str);
            break;
        }
        return 1;
    }

    send_command(send, IPC_READY);
    return event_loop();
}
