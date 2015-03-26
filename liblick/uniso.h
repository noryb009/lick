#pragma once

#include "llist.h"

struct uniso_status_s {
    int finished;
    node_t *files;
    char *error;
    char *initrd;
    char *kernel;
};
typedef struct uniso_status_s uniso_status_t;

uniso_status_t *uniso(const char *src, const char *dst);
void uniso_status_free(uniso_status_t *s);
