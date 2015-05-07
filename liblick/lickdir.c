#include <stdlib.h>

#include "lickdir.h"
#include "utils.h"

lickdir_t *new_lick_dir_no_dup(char *lick, char *entry, char *menu,
        char *res) {
    lickdir_t *l = malloc(sizeof(lickdir_t));
    l->lick = lick;
    l->drive = strdup("?:\\");
    l->drive[0] = lick[0];
    l->entry = entry;
    l->menu = menu;
    l->res = res;
    l->err = NULL;
    return l;
}

lickdir_t *new_lick_dir(char *lick, char *entry, char *menu,
        char *res) {
    return new_lick_dir_no_dup(strdup(lick), strdup(entry),
            strdup(menu), strdup(res));
}

void free_lick_dir(lickdir_t *l) {
    free(l->lick);
    free(l->drive);
    free(l->entry);
    free(l->menu);
    free(l->res);
    if(l->err != NULL)
        free(l->err);
}

lickdir_t *expand_lick_dir(char *d) {
    return new_lick_dir_no_dup(strdup(d), concat_strs(2, d, "\\entries"),
            concat_strs(2, d, "\\menu"), concat_strs(2, d, "\\res"));
}
