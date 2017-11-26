#include <stdlib.h>
#include <string.h>

#include "fatdog.h"
#include "puppy.h"
#include "utils.h"
#include "../llist.h"
#include "../utils.h"

distro_info_node_t *distro_fatdog(string_node_t *files, const char *dst, const char *name, lickdir_t *lick) {
    distro_info_node_t *lst = distro_puppy(files, dst, name, lick);

    // need to change pfix=ram to savefile=none in second item
    if(!lst || !lst->next)
        return lst;

    distro_info_t *i = lst->val;

    char *new_options = strrep(i->options, "pfix=ram", "savefile=none");
    free(i->options);
    i->options = new_options;

    return lst;
}

distro_t *get_distro_fatdog() {
    return new_distro(
        DISTRO_FATDOG,
        DISTRO_FATDOG_KEY,
        "FatDog",
        distro_fatdog,
        distro_puppy_filter
    );
}
