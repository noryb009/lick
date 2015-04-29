#pragma once

#include <stdio.h>

#include "../llist.h"

struct entry_s {
    char *title;
    char *initrd;
    char *kernel;
    char *options;
};

typedef struct entry_s entry_t;

node_t *get_files(char *menu_dir);
entry_t *get_entry(FILE *f);
void free_entry(entry_t *e);
