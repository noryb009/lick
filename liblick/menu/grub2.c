#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grub2.h"
#include "utils.h"
#include "../drives.h"
#include "../install.h"
#include "../lickdir.h"
#include "../utils.h"

// TODO

void grub2_write_entry(FILE *f, entry_t *e) {
}

int regenerate_grub2(lickdir_t *lick) {
    return 1;
}

int install_grub2(lickdir_t *lick) {
    return 1;
}

int uninstall_grub2(lickdir_t *lick) {
    return 1;
}

menu_t *get_grub2() {
    menu_t *menu = malloc(sizeof(menu_t));
    menu->regenerate = regenerate_grub2;
    menu->install = install_grub2;
    menu->uninstall = uninstall_grub2;
    return menu;
}
