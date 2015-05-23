#include <libgen.h>
#include <stdlib.h>

#include "lickdir.h"
#include "utils.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// TODO: clean up unused functions
lickdir_t *new_lick_dir_no_dup(char *lick, char *entry, char *menu,
        char *res) {
    lickdir_t *l = malloc(sizeof(lickdir_t));
    l->lick = lick;
    l->drive = strdup("?:/");
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
    return new_lick_dir_no_dup(strdup(d), concat_strs(2, d, "/entries"),
            concat_strs(2, d, "/menu"), concat_strs(2, d, "/res"));
}

lickdir_t *get_lickdir() {
#ifdef _WIN32
    int size = 128;
    int first = 1;
    int ret_size;

    TCHAR *buf = malloc(sizeof(TCHAR) * size);

    while(first || ret_size == size) {
        if(first)
            first = 0;
        else {
            size *= 2;
            buf = realloc(buf, sizeof(TCHAR) * size);
        }
        ret_size = GetModuleFileName(NULL, buf, size);
    }
    char *name = TCHAR_to_char(buf, ret_size, sizeof(TCHAR));
    lickdir_t *lick = expand_lick_dir(dirname(name));
    free(name);
    free(buf);
    return lick;
#else
    return expand_lick_dir("/lick");
#endif
}
