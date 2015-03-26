set(exe_name "tests")

set(sources
    "test.c")

set(inc_dirs
    ${inc_dirs}
    "${CMAKE_SOURCE_DIR}/liblick")

set(lick_dirs
    ${link_dirs}
    "${CMAKE_BINARY_DIR}/liblick")
set(libs
    ${libs}
    "lick"
    "archive")

if(WIN32)
    # TODO
    set(inc_dirs
        ${inc_dirs})
endif()

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})
