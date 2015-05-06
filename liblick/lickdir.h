#pragma once

// TODO: error message? info?
typedef struct {
    char *lick;
    char *entry;
    char *menu;
    char *res;
} lickdir_t;

lickdir_t *expand_lick_dir(char *d);
