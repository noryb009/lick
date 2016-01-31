#include <stdlib.h>
#include <string.h>

#include "fatdog.h"
#include "puppy.h"
#include "utils.h"
#include "../llist.h"
#include "../utils.h"

node_t *distro_fatdog(node_t *files, const char *dst, const char *name) {
    node_t *lst = distro_puppy(files, dst, name);

    // need to change pfix=ram to savefile=none in second item
    if(!lst || !lst->next)
        return lst;

    distro_info_t *i = (distro_info_t*)lst->next;

    char *new_options = strrep(i->options, "pfix=ram", "savefile=none");
    free(i->options);
    i->options = new_options;

    return lst;
}

distro_t *get_distro_fatdog() {
    return new_distro(distro_fatdog, distro_puppy_filter);
}
