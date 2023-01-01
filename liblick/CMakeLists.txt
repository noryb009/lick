configure_file(
  "${PROJECT_SOURCE_DIR}/liblick/config.h.in"
  "${PROJECT_BINARY_DIR}/liblick/config.h"
)

set(sources
  "boot-loader.c"
  "boot-loader/9x.c"
  "boot-loader/me.c"
  "boot-loader/nt.c"
  "boot-loader/uefi.c"
  "boot-loader/boot-loader-utils.c"
  "boot-loader/vista.c"
  "distro.c"
  "distro/fatdog.c"
  "distro/puppy.c"
  "distro/distro-utils.c"
  "drives.c"
  "install.c"
  "lickdir.c"
  "llist.c"
  "llist-type.c"
  "menu.c"
  "menu/grub2.c"
  "menu/grub4dos.c"
  "menu/edit-flat-menu.c"
  "menu/menu-utils.c"
  "scandir.c"
  "system-info.c"
  "uniso.c"
  "utils/file-utils.c"
  "utils/fs-utils.c"
  "utils/id-utils.c"
  "utils/string-utils.c"
  "utils/system-utils.c"
)

add_library(lick STATIC ${sources})

target_include_directories(lick PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${PROJECT_BINARY_DIR})
target_compile_features(lick PUBLIC c_std_11)

add_dependencies(lick libarchive_external)
target_include_directories(lick PUBLIC ${CMAKE_BINARY_DIR}/cmake/libarchive/libarchive/include)
if (WIN32)
  target_link_libraries(lick PUBLIC ${CMAKE_BINARY_DIR}/cmake/libarchive/libarchive/lib/libarchive_static.a)
else()
  target_link_libraries(lick PUBLIC ${CMAKE_BINARY_DIR}/cmake/libarchive/libarchive/lib/libarchive.a)
endif()
