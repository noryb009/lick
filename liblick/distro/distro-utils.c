#include "distro/distro-utils.h"

#include <stdlib.h>
#include <string.h>

#include "../distro.h"
#include "../utils.h"

distro_info_t *new_distro_info(char *k, char *i, char *op, char *ft) {
    distro_info_t *info = malloc(sizeof(distro_info_t));
    info->kernel = strdup2(k);
    info->initrd = strdup2(i);
    info->options = strdup2(op);
    info->full_text = strdup2(ft);
    return info;
}

distro_info_t *new_empty_distro_info() {
    return new_distro_info(NULL, NULL, NULL, NULL);
}

distro_info_t *copy_distro_info(distro_info_t *s) {
    distro_info_t *i = malloc(sizeof(distro_info_t));
    i->kernel = strdup2(s->kernel);
    i->initrd = strdup2(s->initrd);
    i->options = strdup2(s->options);
    i->full_text = strdup2(s->full_text);
    return i;
}


distro_t *new_distro(distro_e id, const char *key, const char *name,
        distro_info_f info, distro_filter_f filter) {
    distro_t *d = malloc(sizeof(distro_t));
    d->id = id;
    d->key = key;
    d->name = name;
    d->info = info;
    d->filter = filter;
    return d;
}
