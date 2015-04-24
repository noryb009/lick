#include "grub4dos.h"

int regenerate_grub4dos() {
    return 1;
}

menu_t get_grub4dos() {
    menu_t menu;
    menu.regenerate = regenerate_grub4dos;
    return menu;
}
