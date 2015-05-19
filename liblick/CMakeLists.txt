set(lib_name "lick")
set(lib_static_name "lick_static")

set(sources
    "boot-loader.c"
    "boot-loader/9x.c"
    "boot-loader/nt.c"
    "boot-loader/uefi.c"
    "boot-loader/utils.c"
    "boot-loader/vista.c"
    "drives.c"
    "id-utils.c"
    "install.c"
    "lickdir.c"
    "llist.c"
    "menu.c"
    "menu/grub2.c"
    "menu/grub4dos.c"
    "menu/utils.c"
    "scandir.c"
    "system-info.c"
    "uniso.c"
    "utils.c")

set(inc_dirs
    ${inc_dirs}
    ${CMAKE_BINARY_DIR}/libarchive/include)

set(link_dirs
    ${link_dirs}
    ${CMAKE_BINARY_DIR}/libarchive/lib
    ${CMAKE_BINARY_DIR}/libarchive/bin)

set(libs
    ${libs}
    archive)

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_library(${lib_name} SHARED ${sources})
add_library(${lib_static_name} STATIC ${sources})
target_link_libraries(${lib_name} ${libs})
