#pragma once

#include "ipc.hpp"
#include "../../liblick/lick.h"

typedef enum {
    // commands, frontend -> backend
    IPC_EXIT,
    IPC_INSTALL,
    IPC_UNINSTALL,
    IPC_CHECK_LOADER,
    IPC_LOADER,
    // responses, backend -> frontend
    IPC_READY,
    IPC_STATUS,
    IPC_ERROR,
} IPC_COMMANDS;

class ipc_lick : public ipc_command {
    public:
        virtual IPC_COMMANDS type() = 0;
};

class ipc_exit : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_EXIT;}
        void exchange(ipc *p) {}
};

class ipc_install : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_INSTALL;}
        ipc_install() {}
        ipc_install(const char *id, const char *name, const char *iso,
                const char *install_dir, lickdir_t *lick);
        ~ipc_install();
        void exchange(ipc *p);

        char *id;
        char *name;
        char *iso;
        char *install_dir;
        lickdir_t *lick;
};

class ipc_uninstall : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_UNINSTALL;}
        ipc_uninstall() {}
        ipc_uninstall(const char *id, lickdir_t *lick);
        ~ipc_uninstall();
        void exchange(ipc *p);

        char *id;
        lickdir_t *lick;
};

class ipc_check_loader : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_CHECK_LOADER;}
        void exchange(ipc *p) {}
};

class ipc_loader : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_LOADER;}
        ipc_loader() {}
        ipc_loader(int install, lickdir_t *lick);
        ~ipc_loader();
        void exchange(ipc *p);

        int install;
        lickdir_t *lick;
};

class ipc_ready : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_READY;}
        void exchange(ipc *p) {}
};

class ipc_status : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_STATUS;}
        ipc_status() {}
        ipc_status(int ret, const char *err);
        ~ipc_status();
        void exchange(ipc *p);

        int ret;
        char *err;
};

class ipc_error : public ipc_lick {
    public:
        IPC_COMMANDS type() {return IPC_ERROR;}
        ipc_error() {}
        ipc_error(const char *err);
        ~ipc_error();
        void exchange(ipc *p);

        char *err;
};

ipc_lick *recv_command(ipc *p);
void send_command(ipc *p, IPC_COMMANDS c);
void send_command(ipc *p, ipc_lick *c);
void send_status(ipc *p, int ret, const char *err);
void send_error(ipc *p, const char *err);
