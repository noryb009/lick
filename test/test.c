#include <assert.h>
#include <boot-loader/9x.h>
#include <boot-loader/nt.h>
#include <boot-loader/utils.h>
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
    installed_node_t *l = list_installed(lick);
    for(installed_node_t *n = l; n != NULL; n = n->next) {
        installed_t *i = n->val;
        printf("ID: %s, NAME: %s\n", i->id, i->name);
    }
    free_installed_node_t(l);
}

lickdir_t *test_lick(const char *d) {
    return new_lickdir(d[0], concat_strs(2, d, "/entries"),
            concat_strs(2, d, "/res"));
}

void install_iso(char *iso, char *to) {
    sys_info_t *info = get_system_info();
    loader_t *loader = get_loader(info);
    lickdir_t *lick = test_lick("C:\\lick");
    menu_t *menu = get_menu(loader);

    printf("Before install:\n");
    print_installed(lick);
    distro_t *distro = get_distro_by_key("puppy");
    install("myID", "myNAME", distro, iso, to, lick, menu);
    free_distro(distro);
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

    printf("check: %d\n", check_loader(loader));
    printf("install: %d\n", install_loader(loader, info, lick));
    printf("check: %d\n", check_loader(loader));
    getchar(); // to modify file
    install_iso(iso, "C:\\CLI");
    printf("uninstall:%d\n", uninstall_loader(loader, 0, info, lick));
    printf("check: %d\n", check_loader(loader));

    free(lick);
    free(loader);
    free(info);
}

void test_install() {
    sys_info_t *info = get_system_info();
    loader_t *loader = get_loader(info);
    lickdir_t *lick = test_lick("C:\\lick");

    install_loader(loader, info, lick);
    uninstall_loader(loader, 0, info, lick);

    free(lick);
    free(loader);
    free(info);
}

void print_drives() {
    drive_node_t *drv = all_drives();
    for(drive_node_t *n = drv; n != NULL; n = n->next) {
        drive_t *d = n->val;
        printf("- %s\n", d->path);
    }
    free_drive_node_t(drv);
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
    lst = list_reverse(lst);
    assert(list_length(lst) == 5);

    free_list(lst, NULL);
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

void timeout_inner(const char *orig, char *key, char *sep, const char *expect) {
    char *cpy = strdup2(orig);
    cpy = check_timeout(cpy, key, sep);
    assert(strcmp(cpy, expect) == 0);
    free(cpy);
}

void test_timeout() {
#define TIMEOUT_GEN(t) "[a]\nb=c,d\nto=WINDOWS," t "\ne=f,g\n"
    const char *two = TIMEOUT_GEN("2");
    const char *twenty_nine = TIMEOUT_GEN("29");
    const char *goal = TIMEOUT_GEN("5");
    const char *no_effect = TIMEOUT_GEN("--");
    timeout_inner("", "to", ",", "");
    timeout_inner(two, "to", ",", goal);
    timeout_inner(two, "to", "=", goal);
    timeout_inner(TIMEOUT_GEN("60"), "to", "=", goal);
    timeout_inner(goal, "to", "=", goal);
    timeout_inner(twenty_nine, "to", "=", twenty_nine);
    timeout_inner(no_effect, "to", "=", no_effect);
}

void bootloader_inner(char *(*fn)(char *, lickdir_t *), const char *str, lickdir_t *lick, const char *expect) {
    char *cpy = strdup2(str);
    char *after_fn = fn(cpy, lick);
    free(cpy);
    assert(after_fn);
    //printf(";;;;;\n%s-----\n%s;;;;;\n", after_fn, expect);
    assert(compare_files(after_fn, expect));
    free(after_fn);
}

void test_bootloader_9x() {
    lickdir_t *lick_c = test_lick("C:\\lick");
    lickdir_t *lick_z = test_lick("Z:\\lick");

#define X_START(t, e) "[menu]\nmenuitem=WINDOWS,Start Windows\n" e "menudefault=WINDOWS," #t "\nmenucolor=7,0\n"
#define X_START_LICK(t) X_START(t, "menuitem=LICK," START_LOADER_DESC "\n")
#define X_WIN "[WINDOWS]\n"
#define X_WIN_CONTENT X_WIN "a=b\nc=\"d e\"\n"
#define X_LICK(d) "[LICK]\ndevice=" d ":\\pupl.exe\ninstall=" d ":\\pupl.exe\nshell=" d ":\\pupl.exe\n"

#define X_BASE(t) X_START(t, "") X_WIN
#define X_INST(t, d) X_START_LICK(t) X_LICK(d) X_WIN
#define X_BASE_CONTENT(t) X_START(t, "") X_WIN_CONTENT
#define X_INST_CONTENT(t, d) X_START_LICK(t) X_WIN_CONTENT X_LICK(d)
    bootloader_inner(install_to_config_sys, "", lick_c, X_INST(5, "C"));
    bootloader_inner(install_to_config_sys, "", lick_z, X_INST(5, "Z"));
    bootloader_inner(install_to_config_sys, X_BASE_CONTENT(2), lick_z, X_INST_CONTENT(5, "Z"));
    bootloader_inner(install_to_config_sys, X_BASE_CONTENT(12), lick_c, X_INST_CONTENT(12, "C"));
    bootloader_inner(uninstall_from_config_sys, X_INST_CONTENT(12, "C"), lick_c, X_BASE_CONTENT(12));
    bootloader_inner(uninstall_from_config_sys, X_INST_CONTENT(3, "Z"), lick_c, X_BASE_CONTENT(3));
    bootloader_inner(uninstall_from_config_sys, X_INST_CONTENT(3, "Z"), lick_z, X_BASE_CONTENT(3));
    bootloader_inner(uninstall_from_config_sys, X_INST(5, "Z"), lick_z, X_BASE(5));
    bootloader_inner(uninstall_from_config_sys, X_INST(5, "C"), lick_c, X_BASE(5));
    free_lickdir(lick_c);
    free_lickdir(lick_z);
}

void test_bootloader_nt() {
    lickdir_t *lick_c = test_lick("C:\\lick");
    lickdir_t *lick_z = test_lick("Z:\\lick");

#define NT_BASE(t) "[boot loader]\ntimeout=" #t "\ndefault=abc\n[operating systems]\nabc=\"abc /abc\""
#define NT_INST(t, d) NT_BASE(t) "\n" d ":\\pupldr=\"" START_LOADER_DESC "\""
    bootloader_inner(install_to_boot_ini, NT_BASE(10), lick_c, NT_INST(10, "C"));
    bootloader_inner(install_to_boot_ini, NT_BASE(10), lick_z, NT_INST(10, "Z"));
    bootloader_inner(install_to_boot_ini, NT_BASE(60), lick_c, NT_INST(5, "C"));
    bootloader_inner(install_to_boot_ini, NT_BASE(.5), lick_c, NT_INST(.5, "C"));
    bootloader_inner(uninstall_from_boot_ini, NT_INST(10, "C"), lick_c, NT_BASE(10));
    bootloader_inner(uninstall_from_boot_ini, NT_INST(10, "D"), lick_z, NT_BASE(10));
    free_lickdir(lick_c);
    free_lickdir(lick_z);
}

void test_bootloader() {
    test_timeout();
    test_bootloader_9x();
    test_bootloader_nt();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
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
