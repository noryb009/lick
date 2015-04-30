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
    loader_t *loader = get_loader(&info);

    printf("check: %d\n", check_loader(loader, &info));
    printf("install: %d\n", install_loader(loader, &info, "D:\\lick\\menu", "D:\\lick\\res"));
    printf("check: %d\n", check_loader(loader, &info));
    getchar(); // to modify file
    printf("uninstall:%d\n", uninstall_loader(loader, &info, "D:\\lick\\menu", "D:\\lick\\res"));
    printf("check: %d\n", check_loader(loader, &info));

    free(loader);
}

void test_install() {
    win_info_t info = get_windows_version_info();
    loader_t *loader = get_loader(&info);

    install_loader(loader, &info, "D:\\lick\\menu", "D:\\lick\\res");
    uninstall_loader(loader, &info, "D:\\lick\\menu", "D:\\lick\\res");

    free(loader);
}

int main(int argc, char* argv[]) {
    win_info_t info = get_windows_version_info();
    print_info(&info);

    test_install();

    return 0;
}
