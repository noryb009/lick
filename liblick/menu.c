#include <stdlib.h>

#include "menu.h"
#include "utils.h"

int dummy_fix(lickdir_t *lick) {
    if(!lick->err)
        lick->err = strdup2("This function is not needed on your system.");
    return 0;
}

int dummy_fix_check(lickdir_t *lick) {
    (void)lick;
    return 0;
}

menu_t *new_menu(menu_install_f i, menu_uninstall_f u,
        menu_fix_f f, menu_check_fix_f c,
        menu_gen_section_f g, menu_append_section_f a, menu_remove_section_f r) {
    menu_t *m = malloc(sizeof(menu_t));
    m->install = i;
    m->uninstall = u;
    if(f)
        m->fix_loader = f;
    else
        m->fix_loader = dummy_fix;
    if(c)
        m->check_fix_loader = c;
    else
        m->check_fix_loader = dummy_fix_check;
    m->gen_section = g;
    m->append_section = a;
    m->remove_section = r;
    return m;
}

void free_menu(menu_t *m) {
    free(m);
}

node_t *entries_to_sections(menu_t *menu, node_t *ents) {
    node_t *secs = NULL;
    for(node_t *n = ents; n != NULL; n = n->next) {
        distro_info_t *info = (distro_info_t *)n->val;

        if(info->full_text) {
            secs = new_node(info->full_text, secs);
            info->full_text = NULL;
        } else
            secs = new_node(menu->gen_section(info), secs);
    }
    return secs;
}

char *concat_sections(node_t *secs) {
    if(secs == NULL)
        return strdup2("");
    size_t len = list_length(secs);
    if(len > 1)
        len += (len - 1);
    char *strs[len];

    size_t i = len - 1;
    for(node_t *n = secs; n != NULL; n = n->next, --i) {
        strs[i] = (char *)n->val;
        if(i > 0) {
            --i;
            strs[i] = "\n";
        }
    }

    return concat_strs_arr(len, strs);
}

int install_menu(node_t *files, const char *dst, distro_t *distro,
        const char *id, const char *name, lickdir_t *lick, menu_t *menu) {
    // get type
    node_t *entries = distro->info(files, dst, name);
    if(entries == 0)
        return 1;
    node_t *sections = entries_to_sections(menu, entries);
    char *section_text = concat_sections(sections);

    int ret = menu->append_section(id, section_text, lick);

    free_list(sections, free);
    free_distro_info_list(entries);

    return ret;
}

int uninstall_menu(const char *id, lickdir_t *lick, menu_t *menu) {
    return menu->remove_section(id, lick);
}
