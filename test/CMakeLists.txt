set(exe_name "tests")

set(sources
    "test.c")

set(inc_dirs
    ${inc_dirs}
    ${CMAKE_BINARY_DIR}/libarchive/include
    "${PROJECT_SOURCE_DIR}/liblick")

set(link_dirs
    ${link_dirs}
    ${CMAKE_BINARY_DIR}/libarchive/lib
    "${PROJECT_SOURCE_DIR}/liblick")

set(libs
    ${libs}
    "lick"
    "archive")

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})
