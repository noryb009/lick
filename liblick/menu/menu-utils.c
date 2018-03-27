#include <stdio.h>

#include "menu/edit-flat-menu.h"
#include "utils/file-utils.h"

int has_valuable_info(const char *menu) {
    FILE *f = fopen(menu, "r");
    if(!f)
        return 1;
    char *menu_contents = file_to_str(f);
    fclose(f);

    section_node_t *secs = get_sections(menu_contents);
    for(section_node_t *cur = secs; cur; cur = cur->next) {
        section_t *sec = cur->val;
        if(sec->type != S_HEADER && sec->type != S_FOOTER) {
            free_section_node_t(secs);
            free(menu_contents);
            return 1;
        }
    }

    free_section_node_t(secs);
    free(menu_contents);
    return 0;
}
