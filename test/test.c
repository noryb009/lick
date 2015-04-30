#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "install-loader.h"
#include "uniso.h"
#include "utils.h"
#include "windows-info.h"

void print_info(win_info_t *info) {
    printf("family: %s\n", info->family_name);
    printf("version: %s\n", info->version_name);
    printf("architecture: %s\n", info->arch_name);
    printf("bios type: %s\n", info->bios_name);
}

void loud_test_install() {
    win_info_t info = get_windows_version_info();

    printf("check: %d\n", check_loader(&info));
    printf("install: %d\n", install_loader(&info, "D:\\lick"));
    printf("check: %d\n", check_loader(&info));
    getchar(); // to modify file
    printf("uninstall:%d\n", uninstall_loader(&info, "D:\\lick"));
    printf("check: %d\n", check_loader(&info));
}

void test_install() {
    win_info_t info = get_windows_version_info();

    install_loader(&info, "D:\\lick");
    uninstall_loader(&info, "D:\\lick");
}

int main(int argc, char* argv[]) {
    win_info_t info = get_windows_version_info();
    print_info(&info);

    test_install();

    return 0;
}
