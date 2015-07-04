#include <cstdlib>
#include <cstring>

#include "ipc.hpp"

#define MAX_SIZE 65536

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

bool ipc::assert_recv() {
    if(dir != DIRECTION_RECV) {
        set_error();
        return false;
    }
    return true;
}

bool ipc::data_waiting() {
    if(assert_recv() && !had_error()) {
        DWORD n;
        if(PeekNamedPipe(p, NULL, NULL, NULL, &n, NULL))
            return n > 0;
        else
            set_error();
    }
    return false;
}

ipc *ipc::exchange_data(void *data, size_t size) {
    if(had_error())
        return this;
    if(dir == DIRECTION_SEND) {
        if(size > MAX_SIZE) {
            set_error();
            return this;
        }
        DWORD s;
        if(!WriteFile(p, data, size, &s, NULL) || s != size)
            set_error();
    } else {
        if(size > MAX_SIZE) {
            set_error();
            return this;
        }
        DWORD s;
        if(!ReadFile(p, data, size, &s, NULL) || s != size)
            set_error();
    }
    return this;
}

ipc *ipc::exchange(char *&str) {
    return exchange_str(str);
}
ipc *ipc::exchange(const char *&str) {
    return exchange_str(str);
}

ipc *ipc::exchange_str(char *&str) {
    size_t size;
    if(dir == DIRECTION_RECV) {
        str = NULL;
        exchange(size);
        if(had_error() || size == 0)
            return this;
        if(size > MAX_SIZE) {
            set_error();
            return this;
        }
        str = (char *)malloc(size);
        exchange_data(str, size);
        if(had_error()) {
            free(str);
            str = NULL;
        }
    } else {
        if(str == NULL)
            size = 0;
        else
            size = strlen(str) + 1;
        this
            ->exchange(size)
            ->exchange_data(str, size)
        ;
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
