#include <stdlib.h>

#include "lickdir.h"
#include "utils.h"

lickdir_t *expand_lick_dir(char *d) {
    lickdir_t *l = malloc(sizeof(lickdir_t));
    l->lick = strdup(d);
    l->entry = concat_strs(2, d, "\\entries");
    l->menu = concat_strs(2, d, "\\menu");
    l->res = concat_strs(2, d, "\\res");
}
