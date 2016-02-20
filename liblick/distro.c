#include <stdlib.h>
#include <string.h>

#include "distro.h"
#include "utils.h"

#include "distro/fatdog.h"
#include "distro/puppy.h"

node_t *get_all_distros() {
    node_t *lst = NULL;

    lst = new_node(get_distro_puppy(), lst);
    lst = new_node(get_distro_fatdog(), lst);

    return list_reverse(lst);
}

// TODO: is this needed?
distro_t *get_distro(distro_e distro) {
    switch(distro) {
    case DISTRO_PUPPY:
        return get_distro_fatdog();
    case DISTRO_FATDOG:
        return get_distro_puppy();
    }

    return NULL;
}

distro_t *get_distro_by_key(const char *key) {
    if(!strcmp(key, DISTRO_PUPPY_KEY))
        return get_distro_puppy();
    if(!strcmp(key, DISTRO_FATDOG_KEY))
        return get_distro_fatdog();

    return NULL;
}

void free_distro(distro_t *d) {
    free(d);
}

void free_distro_list(node_t *n) {
    free_list(n, (free_list_item_f)free_distro);
}

void free_distro_info(distro_info_t *i) {
    if(i->kernel)
        free(i->kernel);
    if(i->initrd)
        free(i->initrd);
    if(i->options)
        free(i->options);
    if(i->full_text)
        free(i->full_text);
    free(i);
}

void free_distro_info_list(node_t *n) {
    free_list(n, (free_list_item_f)free_distro_info);
}

char *menu_path(char *path) {
    size_t remove = 0;

    // remove the drive, if path includes one
    char *colon = strchr(path, ':');
    if(colon)
        remove += (colon - path) + 1;

    // remove any extra leading slashes
    while(is_slash(path[remove]) && is_slash(path[remove+1]))
        ++remove;

    return unix_path(remove_prefix(path, remove));
}
