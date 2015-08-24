#include <libgen.h>
#include <stdlib.h>

#include "drives.h"
#include "lickdir.h"
#include "utils.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

lickdir_t *new_lickdir(char drive, char *entry, char *res) {
    lickdir_t *l = malloc(sizeof(lickdir_t));
    l->drive = strdup2("?:/");
    l->drive[0] = drive;
    l->entry = unix_path(entry);
    l->res = unix_path(res);
    l->err = NULL;

    return l;
}

void free_lickdir(lickdir_t *l) {
    free(l->drive);
    free(l->entry);
    free(l->res);
    if(l->err != NULL)
        free(l->err);
    free(l);
}

lickdir_t *get_lickdir() {
#ifdef _WIN32
    char *p = get_program_path();
    char *c = get_config_path();
    char *win = get_windows_path();
    if(!p || !c || !win) {
        if(p)
            free(p);
        if(c)
            free(c);
        if(win)
            free(win);
        return NULL;
    }

    lickdir_t *lick = NULL;
    char *res = unix_path(concat_strs(2, dirname(p), "/res"));
    if(path_exists(res))
        lick = new_lickdir(win[0], concat_strs(2, c, "/entry"), res);
    else
        free(res);
    free(p);
    free(c);
    free(win);
    return lick;
#else
    return new_lickdir('c', strdup2("/lick/entry"), strdup2("/lick/res"));
#endif
}
