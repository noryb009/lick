#include <stdio.h>
#include <stdlib.h>

#include "menu.h"
#include "menu/grub4dos.h"

#define MENU_TITLE   "title\t%s%s"
#define MENU_LINUX   "\tlinux\t%s"
#define MENU_INITRD  "\tinitrd\t%s"
#define MENU_OPTIONS "\toptions\t%s"

int print_frag(FILE *f, char *name, char *suffix,
        uniso_status_t *info, char *ops) {
    // TODO: file format
    fprintf(f, MENU_TITLE, name, suffix);
    if(info->kernel)
        fprintf(f, MENU_LINUX, info->kernel);
    if(info->initrd)
        fprintf(f, MENU_INITRD, info->initrd);
    fprintf(f, MENU_OPTIONS, ops);
}

int write_menu_frag(char *dst, char *name, uniso_status_t *info) {
    FILE *f = fopen(dst, "w");
    if(!f)
        return 0;

     // TODO: psubdir
    print_frag(f, name, "", info, "pfix=fsck");
    fprintf(f, "\n");
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

int regenerate_menu() {
    menu_t *menu = get_menus();

    for(int i = 0; i < NUM_MENUS; ++i) {
        // TODO: check support
        if(1) {
            menu[i].regenerate();
            free(menu);
            return 1;
        }
    }
    free(menu);

    return 0;
}
