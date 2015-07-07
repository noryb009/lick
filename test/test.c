#include <assert.h>
#include <boot-loader/9x.h>
#include <boot-loader/nt.h>
#include <lick.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void print_info(sys_info_t *info) {
    printf("family: %s\n", info->family_name);
    printf("version: %s\n", info->version_name);
    printf("architecture: %s\n", info->arch_name);
    printf("bios type: %s\n", info->bios_name);
}

void print_installed(lickdir_t *lick) {
    node_t *l = list_installed(lick);
    for(node_t *n = l; n != NULL; n = n->next) {
        installed_t *i = n->val;
        printf("ID: %s, NAME: %s\n", i->id, i->name);
    }
    free_list_installed(l);
}

lickdir_t *test_lick(const char *d) {
    return new_lickdir(strdup2(d), concat_strs(2, d, "/entries"),
            concat_strs(2, d, "/menu"), concat_strs(2, d, "/res"));
}

void install_iso(char *iso, char *to) {
    sys_info_t *info = get_system_info();
    loader_t *loader = get_loader(info);
    lickdir_t *lick = test_lick("C:\\lick");
    menu_t *menu = get_menu(loader);

    printf("Before install:\n");
    print_installed(lick);
    install("myID", "myNAME", iso, to, lick, menu);
    print_installed(lick);
    printf("After install:\n");
    getchar();
    uninstall("myID", lick, menu);
    print_installed(lick);
    printf("After uninstall:\n");

    free(menu);
    free(lick);
    free(loader);
    free(info);
}

void loud_test_install(char *iso) {
    sys_info_t *info = get_system_info();
    loader_t *loader = get_loader(info);
    lickdir_t *lick = test_lick("C:\\lick");

    printf("check: %d\n", check_loader(loader, info));
    printf("install: %d\n", install_loader(loader, info, lick));
    printf("check: %d\n", check_loader(loader, info));
    getchar(); // to modify file
    install_iso("puppy_cli.iso", "C:\\CLI");
    printf("uninstall:%d\n", uninstall_loader(loader, info, lick));
    printf("check: %d\n", check_loader(loader, info));

    free(lick);
    free(loader);
    free(info);
}

void test_install() {
    sys_info_t *info = get_system_info();
    loader_t *loader = get_loader(info);
    lickdir_t *lick = test_lick("C:\\lick");

    install_loader(loader, info, lick);
    uninstall_loader(loader, info, lick);

    free(lick);
    free(loader);
    free(info);
}

void print_drives() {
    node_t *drv = all_drives();
    for(node_t *n = drv; n != NULL; n = n->next) {
        drive_t *d = n->val;
        printf("- %s\n", d->path);
    }
    free_drive_list(drv);
}

int even(int *i) {
    return (*i % 2 == 0);
}

void test_list() {
    int a=1, b=2, c=3, d=4, e=5;
    node_t *lst =
        new_node(
            &a, new_node(&b, new_node(&c, new_node(&d, new_node(&e, NULL)))));

    assert(list_length(lst) == 5);

    free_list(lst, free_nothing);
}

int compare_files(const char *a, const char *b) {
    if((a == NULL || b == NULL) && a != b)
        return 0;
    for(int i = 0, j = 0;; ++i, ++j) {
        int a_newline = 0, b_newline = 0;
        if(a[i] == '\n') {
            while(a[i] == '\n')
                ++i;
            a_newline = 1;
        }
        if(b[j] == '\n') {
            while(b[j] == '\n')
                ++j;
            b_newline = 1;
        }
        if(a[i] == '\0' && b[j] == '\0')
            return 1;
        else if(a_newline != b_newline)
            return 0;
        else if(a[i] != b[j])
            return 0;
    }
}

int bootloader_inner(char *(*fn)(char *, lickdir_t *), const char *str, lickdir_t *lick, const char *expect) {
    char *cpy = strdup2(str);
    char *after_fn = fn(cpy, lick);
    free(cpy);
    if(!after_fn)
        return 0;
    int ret = compare_files(after_fn, expect);
    free(after_fn);
    return ret;
}

void test_bootloader_9x() {
    lickdir_t *lick_c = test_lick("C:\\lick");
    lickdir_t *lick_z = test_lick("Z:\\lick");
    const char *empty = "";
    const char *base = "[menu]\nmenuitem=WINDOWS,Start Windows\nmenudefault=WINDOWS,10\nmenucolor=7,0\n[WINDOWS]";
    const char *base_inst = "[menu]\nmenuitem=WINDOWS,Start Windows\nmenuitem=LICK, Start Puppy Linux\nmenudefault=WINDOWS,10\nmenucolor=7,0\n[LICK]\ndevice=C:\\pupl.exe\ninstall=C:\\pupl.exe\nshell=C:\\pupl.exe\n[WINDOWS]";
    const char *base_inst_z = "[menu]\nmenuitem=WINDOWS,Start Windows\nmenuitem=LICK, Start Puppy Linux\nmenudefault=WINDOWS,10\nmenucolor=7,0\n[LICK]\ndevice=Z:\\pupl.exe\ninstall=Z:\\pupl.exe\nshell=Z:\\pupl.exe\n[WINDOWS]";
    const char *base_inst_after = "[menu]\nmenuitem=WINDOWS,Start Windows\nmenuitem=LICK, Start Puppy Linux\nmenudefault=WINDOWS,10\nmenucolor=7,0\n[WINDOWS]\n[LICK]\ndevice=C:\\pupl.exe\ninstall=C:\\pupl.exe\nshell=C:\\pupl.exe";
    assert(bootloader_inner(install_to_config_sys, empty, lick_c, base_inst));
    assert(bootloader_inner(install_to_config_sys, empty, lick_z, base_inst_z));
    assert(bootloader_inner(install_to_config_sys, base, lick_c, base_inst_after));
    assert(bootloader_inner(uninstall_from_config_sys, base_inst, lick_c, base));
    assert(bootloader_inner(uninstall_from_config_sys, base_inst_z, lick_z, base));
    assert(bootloader_inner(uninstall_from_config_sys, base_inst_after, lick_c, base));
    free_lickdir(lick_c);
    free_lickdir(lick_z);
}

void test_bootloader_nt() {
    lickdir_t *lick_c = test_lick("C:\\lick");
    lickdir_t *lick_z = test_lick("Z:\\lick");

#define NT_BASE "[boot loader]\ntimeout=10\ndefault=abc\n[operating systems]\nabc=\"abc /abc\""
    const char *base = NT_BASE;
    const char *base_inst = NT_BASE "\nC:\\pupldr=\"Start Puppy Linux\"";
    const char *base_inst_z = NT_BASE "\nZ:\\pupldr=\"Start Puppy Linux\"";
    assert(bootloader_inner(install_to_boot_ini, base, lick_c, base_inst));
    assert(bootloader_inner(install_to_boot_ini, base, lick_z, base_inst_z));
    assert(bootloader_inner(uninstall_from_boot_ini, base_inst, lick_c, base));
    assert(bootloader_inner(uninstall_from_boot_ini, base_inst_z, lick_z, base));
    free_lickdir(lick_c);
    free_lickdir(lick_z);
}

void test_bootloader() {
    test_bootloader_9x(); test_bootloader_nt();
}

int main(int argc, char* argv[]) {
    sys_info_t *info = get_system_info();
    print_info(info);
    free_sys_info(info);

    test_list();

    /*if(argc < 2)
        test_install();
    else
        loud_test_install(argv[1]);*/

    test_bootloader();

    //print_drives();

    return 0;
}
