#pragma once

#include <stdio.h>

#include "../llist.h"

typedef struct {
    char *title;
    char *initrd;
    char *kernel;
    char *options;
    char *static_text;
} entry_t;

node_t *get_conf_files(char *dir);
entry_t *get_entry(FILE *f);
void free_entry(entry_t *e);
