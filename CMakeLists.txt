cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(proj_name "LICK")
set(VERSION_MAJOR "0")
set(VERSION_MINOR "2")
set(VERSION_PATCH "0")

project(${proj_name})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

# TODO: only build needed targets
include(ExternalProject)
ExternalProject_Add(
    libarchive-3.1.2
    URL http://libarchive.org/downloads/libarchive-3.1.2.tar.gz
    URL_MD5 efad5a503f66329bb9d2f4308b5de98a
    # ignore if the system has zlib; otherwise zlib needs to be linked in
    PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/res-build/libarchive-no-system-zlib.patch
    # remove wincrypt calls; enables Win95 support (before OEM2)
          COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/res-build/libarchive-no-wincrypt.patch
    CMAKE_ARGS
        -DENABLE_NETTLE=OFF -DENABLE_OPENSSL=OFF -DENABLE_TAR=OFF
        -DENABLE_CPIO=OFF -DENABLE_XATTR=OFF -DENABLE_ACL=OFF
        -DENABLE_ICONV=OFF -DENABLE_TEST=OFF
        -DENABLE_LZMA=OFF -DENABLE_ZLIB=OFF -DENABLE_BZip2=OFF
        -DENABLE_EXPAT=OFF -DENABLE_PCREPOSIX=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libarchive
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_C_FLAGS=-static-libgcc)

add_subdirectory(liblick)
add_subdirectory(test)
add_subdirectory(src)

install(DIRECTORY res
    DESTINATION .)
install(DIRECTORY
    DESTINATION menu)
install(DIRECTORY
    DESTINATION entries)
install(FILES LICENSE
    DESTINATION .
    RENAME LICENSE.txt)

# CPack
set(CPACK_GENERATOR "${CPACK_GENERATOR};ZIP")
set(CPACK_PACKAGING_INSTALL_PREFIX "")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${proj_name}")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "Luke Lorimer (noryb009)")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "lick")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
if(WIN32 AND NOT UNIX)
    set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
    #set(CPACK_PACKAGE_ICON "")
    set(CPACK_NSIS_MENU_LINKS
        "lick-cli.exe" "LICK Command Line Interface"
        "lick-fltk.exe" "LICK")
    set(CPACK_NSIS_DISPLAY_NAME "LICK")
    set(CPACK_NSIS_HELP_LINK "https://github.com/noryb009/lick")
    set(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_NSIS_HELP_LINK}")
    set(CPACK_NSIS_CONTACT "noryb009@gmail.com")
    # TODO: uninstall all
    #set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
endif()

include(CPack)
