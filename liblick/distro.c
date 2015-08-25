#include <stdlib.h>
#include <string.h>

#include "distro.h"
#include "utils.h"

void free_distro(distro_t *d) {
    free(d);
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
    free_list(n, (void(*)(void *))free_distro_info);
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
