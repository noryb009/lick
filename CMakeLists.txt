cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(proj_name "LICK")

project(${proj_name})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
endif()

# TODO: only build needed targets
include(ExternalProject)
ExternalProject_Add(
    LibArchive-3-1-2
    URL http://libarchive.org/downloads/libarchive-3.1.2.tar.gz
    URL_MD5 efad5a503f66329bb9d2f4308b5de98a
    CMAKE_ARGS
        -DACL:BOOL=FALSE -DBSDCPIO:BOOL=FALSE -DBSDTAR:BOOL=FALSE
        -DBZ2LIB:BOOL=FALSE -DEXPAT:BOOL=FALSE -DICONV:BOOL=FALSE
        -DLIBICONV-PREFIX:BOOL=FALSE -DLZMA:BOOL=FALSE -DLZMADEC:BOOL=FALSE
        -DLZO2:BOOL=FALSE -DNETTLE:BOOL=FALSE -DOPENSSL:BOOL=FALSE
        -DPOSIX-REGEX-LIB:BOOL=FALSE -DXATTR:BOOL=FALSE -DXML2:BOOL=FALSE
        -DZLIB:BOOL=FALSE
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/libarchive
        -DCMAKE_C_FLAGS=-static-libgcc)

add_subdirectory(liblick)
add_subdirectory(test)
add_subdirectory(src)
