set(lib_name "lick")
set(lib_static_name "lick_static")

set(sources
    "drives.c"
    "install.c"
    "install-loader.c"
    "install-loader/9x.c"
    "install-loader/nt.c"
    "install-loader/vista.c"
    "install-loader/utils.c"
    "lickdir.c"
    "llist.c"
    "menu.c"
    "menu/grub4dos.c"
    "menu/utils.c"
    "scandir.c"
    "uniso.c"
    "utils.c"
    "windows-info.c")

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
