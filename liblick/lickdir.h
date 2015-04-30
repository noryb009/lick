#pragma once

// TODO: error message? info?
struct lickdir_s {
    char *lick;
    char *entry;
    char *menu;
    char *res;
};

typedef struct lickdir_s lickdir_t;

lickdir_t *expand_lick_dir(char *d);
