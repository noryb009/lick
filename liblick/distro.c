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
    char *colon = strchr(path, ':');
    if(colon == NULL)
        return unix_path(path);

    for(char *from = colon + 1, *to = path;; ++from, ++to) {
        *to = *from;
        if(*from == '\0')
            break;
    }

    return unix_path(path);
}
