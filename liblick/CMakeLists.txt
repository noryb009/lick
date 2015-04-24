set(lib_name "lick")
set(lib_shared_name "lick-shared")

set(sources
    "drives.c"
    "install.c"
    "install-loader.c"
    "install-loader/9x.c"
    "install-loader/nt.c"
    "install-loader/vista.c"
    "llist.c"
    "menu.c"
    "uniso.c"
    "utils.c"
    "windows-info.c")

set(inc_dirs) # TODO

if(WIN32)
    # TODO
    set(inc_dirs
        ${inc_dirs})
endif()

include_directories(${inc_dirs})

add_library(${lib_name} STATIC ${sources})
#add_library(${lib_shared_name} SHARED ${sources})
