set(exe_name "tests")

set(sources
    "test.c")

set(inc_dirs
    ${inc_dirs}
    ${PROJECT_BINARY_DIR}/libarchive/include
    ${PROJECT_SOURCE_DIR}/liblick
    ${PROJECT_BINARY_DIR}/liblick)

set(link_dirs
    ${link_dirs}
    ${PROJECT_BINARY_DIR}/libarchive/lib
    ${PROJECT_BINARY_DIR}/liblick)

if(WIN32)
    set(libs
        ${libs}
        lick_static
        archive_static)
else()
    set(libs
        ${libs}
        lick
        archive)
endif()

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})
