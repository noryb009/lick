#include <cstdlib>

#include "ipc-lick.hpp"

void free_nonnull(void *p) {
    if(p)
        free(p);
}

lickdir_t *copy_lickdir_t(lickdir_t *src) {
    lickdir_t *dst = (lickdir_t *)malloc(sizeof(lickdir_t));
    dst->drive = strdup2(src->drive);
    dst->entry = strdup2(src->entry);
    dst->res = strdup2(src->res);
    dst->err = strdup2(src->err);
    return dst;
}

void exchange_lick(ipc *p, lickdir_t *&lick) {
    if(lick == NULL)
        lick = (lickdir_t *)malloc(sizeof(lickdir_t));
    p
        ->exchange_str(lick->drive)
        ->exchange_str(lick->entry)
        ->exchange_str(lick->res)
        ->exchange_str(lick->err)
    ;
}

// IPC_INSTALL

ipc_install::ipc_install(const char *id, const char *name, const char *iso,
        const char *install_dir, lickdir_t *lick) {
    this->id = strdup2(id);
    this->name = strdup2(name);
    this->iso = strdup2(iso);
    this->install_dir = strdup2(install_dir);
    this->lick = copy_lickdir_t(lick);
}

ipc_install::~ipc_install() {
    free_nonnull(id);
    free_nonnull(name);
    free_nonnull(iso);
    free_nonnull(install_dir);
    free_lickdir(lick);
}

void ipc_install::exchange(ipc *p) {
    p
        ->exchange_str(id)
        ->exchange_str(name)
        ->exchange_str(iso)
        ->exchange_str(install_dir)
    ;
    exchange_lick(p, lick);
}

// IPC_UNINSTALL

ipc_uninstall::ipc_uninstall(const char *id, lickdir_t *lick) {
    this->id = strdup2(id);
    this->lick = copy_lickdir_t(lick);
}
ipc_uninstall::~ipc_uninstall() {
    free_nonnull(id);
    free_lickdir(lick);
}
void ipc_uninstall::exchange(ipc *p) {
    p
        ->exchange_str(id)
    ;
    exchange_lick(p, lick);
}

// IPC_LOADER

ipc_loader::ipc_loader(int install, lickdir_t *lick) {
    this->install = install;
    this->lick = copy_lickdir_t(lick);
}
ipc_loader::~ipc_loader() {
    free_lickdir(lick);
}
void ipc_loader::exchange(ipc *p) {
    p
        ->exchange(install)
    ;
    exchange_lick(p, lick);
}

// IPC_STATUS

ipc_status::ipc_status(int ret, const char *err) {
    this->ret = ret;
    this->err = strdup2(err);
}
ipc_status::~ipc_status() {
    free_nonnull(err);
}
void send_status_inner(ipc *p, int &ret, char *&err) {
    p
        ->exchange(ret)
        ->exchange_str(err)
    ;
}
void ipc_status::exchange(ipc *p) {
    send_status_inner(p, ret, err);
}

// IPC_PROGRESS

ipc_progress::ipc_progress(uniso_progress_t cur, uniso_progress_t total) {
    this->cur = cur;
    this->total = total;
}
void send_progress_inner(ipc *p, uniso_progress_t &cur, uniso_progress_t &total) {
    p
        ->exchange(cur)
        ->exchange(total)
    ;
}
void ipc_progress::exchange(ipc *p) {
    send_progress_inner(p, cur, total);
}

// IPC_ERROR

ipc_error::ipc_error(const char *err) {
    this->err = strdup2(err);
}
ipc_error::~ipc_error() {
    free_nonnull(err);
}
void send_error_inner(ipc *p, char *&err) {
    p
        ->exchange_str(err)
    ;
}
void ipc_error::exchange(ipc *p) {
    send_error_inner(p, err);
}

ipc_lick *recv_command(ipc *p) {
    IPC_COMMANDS type;
    p->exchange(type);
    if(p->had_error())
        return NULL;

    ipc_lick *c;
    switch(type) {
    case IPC_EXIT:
        c = new ipc_exit();
        break;
    case IPC_READY:
        c = new ipc_ready();
        break;
    case IPC_INSTALL:
        c = new ipc_install();
        break;
    case IPC_UNINSTALL:
        c = new ipc_uninstall();
        break;
    case IPC_CHECK_LOADER:
        c = new ipc_check_loader();
        break;
    case IPC_LOADER:
        c = new ipc_loader();
        break;
    case IPC_STATUS:
        c = new ipc_status();
        break;
    case IPC_PROGRESS:
        c = new ipc_progress();
        break;
    case IPC_ERROR:
        c = new ipc_error();
        break;
    default:
        return NULL;
    }
    c->exchange(p);
    return c;
}

void send_command(ipc *p, IPC_COMMANDS c) {
    p->assert_send();
    p->exchange(c);
}

void send_command(ipc *p, ipc_lick *c) {
    p->assert_send();
    IPC_COMMANDS type = c->type();
    send_command(p, type);
    c->exchange(p);
}

void send_status(ipc *p, int ret, const char *err) {
    p->assert_send();
    send_command(p, IPC_STATUS);
    send_status_inner(p, ret, (char *&)err);
}

void send_progress(ipc *p, uniso_progress_t cur, uniso_progress_t total) {
    p->assert_send();
    send_command(p, IPC_PROGRESS);
    send_progress_inner(p, cur, total);
}

void send_error(ipc *p, const char *err) {
    p->assert_send();
    send_command(p, IPC_ERROR);
    send_error_inner(p, (char *&)err);
}
