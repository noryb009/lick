#include <libgen.h>
#include <stdlib.h>

#include "lickdir.h"
#include "utils.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

lickdir_t *new_lickdir(char *lick, char *entry, char *menu, char *res) {
    lickdir_t *l = malloc(sizeof(lickdir_t));
    l->lick = lick;
    l->drive = strdup2("?:/");
    l->drive[0] = lick[0];
    l->entry = entry;
    l->menu = menu;
    l->res = res;
    l->err = NULL;

    // cmake doesn't include empty directories in zip files
    make_dir_parents(l->entry);
    make_dir_parents(l->menu);
    return l;
}

void free_lickdir(lickdir_t *l) {
    free(l->lick);
    free(l->drive);
    free(l->entry);
    free(l->menu);
    free(l->res);
    if(l->err != NULL)
        free(l->err);
    free(l);
}

lickdir_t *expand_lickdir(char *d) {
    char *res = concat_strs(2, d, "/res");
    if(!path_exists(res)) {
        free(res);
        return NULL;
    }
    return new_lickdir(strdup2(d), concat_strs(2, d, "/entries"),
            concat_strs(2, d, "/menu"), res);
}

lickdir_t *get_lickdir() {
#ifdef _WIN32
    char *name = get_program_path();
    lickdir_t *lick = expand_lickdir(dirname(name));
    free(name);
    return lick;
#else
    return expand_lickdir("/lick");
#endif
}
