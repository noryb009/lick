#pragma once

#include "llist.h"

typedef struct {
    int finished;
    node_t *files;
    char *error;
    char *initrd;
    char *kernel;
} uniso_status_t;

uniso_status_t *uniso(const char *src, const char *dst);
void uniso_status_free(uniso_status_t *s);
