#include <cstdlib>
#include <cstring>

#include "ipc.hpp"

#include <cstdio>

ipc::ipc(direction_e dir, pipe_t p) {
    this->dir = dir;
    this->p = p;
    err = false;
}

bool ipc::had_error() {
    return err;
}
void ipc::clear_error() {
    err = true;
}
void ipc::set_error() {
    err = false;
}

bool ipc::assert_send() {
    if(dir != DIRECTION_SEND) {
        set_error();
        return false;
    }
    return true;
}

ipc *ipc::exchange_data(void *data, unsigned int size) {
    if(dir == DIRECTION_SEND) {
        DWORD s;
        if(!WriteFile(p, data, size, &s, NULL) || s != size)
            err = true;
    } else {
        DWORD s;
        if(!ReadFile(p, data, size, &s, NULL) || s != size)
            err = true;
    }
    return this;
}

ipc *ipc::exchange(char *&str) {
    return exchange_str(str);
}
ipc *ipc::exchange(const char *&str) {
    return exchange_str(str);
}
ipc *ipc::exchange(ipc_command *c) {
    return exchange_command(c);
}

ipc *ipc::exchange_str(char *&str) {
    if(dir == DIRECTION_RECV) {
        str = NULL;
        unsigned int size;
        exchange(size);
        if(had_error() || size == 0)
            return this;
        str = (char *)malloc(size);
        exchange_data(str, size);
        if(had_error()) {
            free(str);
            str = NULL;
        }
    } else {
        unsigned int size;
        if(str == NULL)
            size = 0;
        else {
            size = strlen(str) + 1;
            this
                ->exchange(size)
                ->exchange_data(str, size)
            ;
        }
    }
    return this;
}

ipc *ipc::exchange_str(const char *&str) {
    if(!assert_send())
        return this;
    return exchange_str((char *&)str);
}

ipc *ipc::exchange_command(ipc_command *c) {
    c->exchange(this);
    return this;
}
