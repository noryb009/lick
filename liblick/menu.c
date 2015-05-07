#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "menu/utils.h"

#define MENU_TITLE   "title\t%s%s\n"
#define MENU_KERNEL  "\tkernel\t%s\n"
#define MENU_INITRD  "\tinitrd\t%s\n"
#define MENU_OPTIONS "\toptions\t%s\n"

void print_frag(FILE *f, char *name, char *suffix,
        uniso_status_t *info, char *ops) {
    fprintf(f, MENU_TITLE, name, suffix);
    if(info->kernel) {
        char *kernel = to_menu_path(info->kernel);
        fprintf(f, MENU_KERNEL, kernel);
        free(kernel);
    }
    if(info->initrd) {
        char *initrd = to_menu_path(info->initrd);
        fprintf(f, MENU_INITRD, initrd);
        free(initrd);
    }
    fprintf(f, MENU_OPTIONS, ops);
}

int write_menu_frag(char *dst, char *name, uniso_status_t *info, char *subdir) {
    FILE *f = fopen(dst, "w");
    if(!f)
        return 0;

    int len = strlen("pfix=fsck psubdir=") + 1;
    char *subdir_menu = NULL;
    if(subdir != NULL) {
        subdir_menu = to_menu_path(subdir);

        len += strlen(subdir_menu);
    }

    char options[len];
    options[len-1] = '\0';
    if(subdir != NULL)
        snprintf(options, len - 1, "pfix=fsck psubdir=%s", subdir_menu);
    else
        strcpy(options, "pfix=fsck");
    print_frag(f, name, "", info, options);

    fprintf(f, "\n");

    if(subdir != NULL)
        snprintf(options, len - 1, "pfix=ram psubdir=%s", subdir_menu);
    else
        strcpy(options, "pfix=ram");
    print_frag(f, name, " (no save file)", info, options);

    fclose(f);
    if(subdir_menu != NULL)
        free(subdir_menu);
    return 1;
}
