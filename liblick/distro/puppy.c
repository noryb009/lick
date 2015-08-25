#include <stdlib.h>
#include <string.h>

#include "puppy.h"
#include "utils.h"
#include "../llist.h"
#include "../utils.h"

node_t *distro_puppy(node_t *files, const char *dst, const char *name) {
    distro_info_t *i = new_empty_distro_info();
    i->kernel = NULL;
    i->initrd = NULL;
    i->options = NULL;

    for(node_t *f = files; f != NULL; f = f->next) {
        char *n = (char *)f->val;

        if(i->kernel == NULL && (strstr(n, "vmlinu") || strstr(n, "VMLINU"))) {
            i->kernel = menu_path(concat_strs(3, dst, "/", n));
        }
        if(i->initrd == NULL && (strstr(n, "initr") || strstr(n, "INITR"))) {
            i->initrd = menu_path(concat_strs(3, dst, "/", n));
        }
    }

    char *dst_menu = menu_path(strdup2(dst));
    node_t *ret = NULL;
    if(i->kernel) {
        distro_info_t *i2 = copy_distro_info(i);
        i->options = concat_strs(2, "pfix=fsck psubdir=", dst_menu);
        i2->options = concat_strs(2, "pfix=ram psubdir=", dst_menu);
        i2->name = concat_strs(2, name, " (no save file)");
        ret = new_node(i2, ret);
    }
    i->name = strdup2(name);
    ret = new_node(i, ret);

    free(dst_menu);
    return ret;
}

int distro_puppy_filter(const char *f) {
    if(!strcasecmp(f, "boot.cat")
            || !strcasecmp(f, "boot.msg")
            || !strcasecmp(f, "help.msg")
            || !strcasecmp(f, "help2.msg")
            || !strcasecmp(f, "isolinux.bin")
            || !strcasecmp(f, "isolinux.cfg")
            || !strcasecmp(f, "logo.16")
            || !strcasecmp(f, "README.HTM")
            || strstr(f, "/")) {
        return 0;
    }

    return 1;
}

distro_t *get_distro_puppy() {
    return new_distro(distro_puppy, distro_puppy_filter);
}
