#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "menu/grub4dos.h"

#define MENU_TITLE   "title\t%s%s\n"
#define MENU_KERNEL   "\tkernel\t%s\n"
#define MENU_INITRD  "\tinitrd\t%s\n"
#define MENU_OPTIONS "\toptions\t%s\n"

int print_frag(FILE *f, char *name, char *suffix,
        uniso_status_t *info, char *ops) {
    fprintf(f, MENU_TITLE, name, suffix);
    if(info->kernel)
        fprintf(f, MENU_KERNEL, info->kernel);
    if(info->initrd)
        fprintf(f, MENU_INITRD, info->initrd);
    fprintf(f, MENU_OPTIONS, ops);
}

int write_menu_frag(char *dst, char *name, uniso_status_t *info, char *subdir) {
    FILE *f = fopen(dst, "w");
    if(!f)
        return 0;

    int len = 20;
    if(subdir != NULL) {
        len += strlen(subdir);
    }

    char options[len];
    options[len-1] = '\0';
    if(subdir != NULL)
        snprintf(options, len - 1, "pfix=fsck psubdir=%s", subdir);
    else
        strcpy(options, "pfix=fsck");
    print_frag(f, name, "", info, options);

    fprintf(f, "\n");

    if(subdir != NULL)
        snprintf(options, len - 1, "pfix=ram psubdir=%s", subdir);
    else
        strcpy(options, "pfix=ram");
    print_frag(f, name, " (no save file)", info, options);

    fclose(f);
    return 1;
}
