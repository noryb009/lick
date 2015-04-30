#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "menu/grub4dos.h"

#define MENU_TITLE   "title\t%s%s"
#define MENU_LINUX   "\tlinux\t%s"
#define MENU_INITRD  "\tinitrd\t%s"
#define MENU_OPTIONS "\toptions\t%s"

int print_frag(FILE *f, char *name, char *suffix,
        uniso_status_t *info, char *ops) {
    fprintf(f, MENU_TITLE, name, suffix);
    if(info->kernel)
        fprintf(f, MENU_LINUX, info->kernel);
    if(info->initrd)
        fprintf(f, MENU_INITRD, info->initrd);
    fprintf(f, MENU_OPTIONS, ops);
}

int write_menu_frag(char *dst, char *name, uniso_status_t *info, char *subdir) {
    FILE *f = fopen(dst, "w");
    if(!f)
        return 0;

    int len = 16;
    if(subdir != NULL)
        len += strlen(subdir);
    subdir[len-1] = '\0';

    char options[len];
    if(subdir == NULL)
        snprintf(options, len - 1, "pfix=fsck psubdir=%s", subdir);
    else
        strcpy(options, "pfix=fsck");
    print_frag(f, name, "", info, options);

    fprintf(f, "\n");

    if(subdir == NULL)
        snprintf(options, len - 1, "pfix=ram psubdir=%s", subdir);
    else
        strcpy(options, "pfix=ram");
    print_frag(f, name, " (no save file)", info, "pfix=fsck");

    fclose(f);
    return 1;
}

#define NUM_MENUS 1

menu_t *get_menus() {
    menu_t *menu = malloc(sizeof(menu_t) * NUM_MENUS);
    menu[0] = get_grub4dos();
    return menu;
}

int regenerate_menu(char *menu_dir) {
    menu_t *menu = get_menus();

    for(int i = 0; i < NUM_MENUS; ++i) {
        // TODO: check support
        if(1) {
            menu[i].regenerate(menu_dir);
            free(menu);
            return 1;
        }
    }
    free(menu);

    return 0;
}
