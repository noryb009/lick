#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HANDLE pipe_t;
#else
typedef int pipe_t;
#endif

typedef enum {
    DIRECTION_SEND,
    DIRECTION_RECV,
} direction_e;

class ipc;

class ipc_command {
    public:
        virtual ~ipc_command() {};
        virtual void exchange(ipc *p) = 0;
};

class ipc {
    public:
        ipc(direction_e dir, pipe_t p);
        bool had_error();
        void clear_error();
        void set_error();
        bool data_waiting();

        ipc *exchange_data(void *data, unsigned int size);

        template<typename T>
        ipc *exchange(T &data) {
            return exchange_data(&data, sizeof(data));
        }
        template<typename T>
        ipc *exchange(const T &data) {
            if(!assert_send())
                return this;
            T copy = data;
            return this->exchange(copy);
        }

        ipc *exchange(char *&str);
        ipc *exchange(const char *&str);

        ipc *exchange_str(char *&str);
        ipc *exchange_str(const char *&str);
        ipc *exchange_command(ipc_command *c);

    protected:
        bool assert_send();
        bool assert_recv();
        bool err;
        direction_e dir;
        pipe_t p;
};
