set(exe_name "tests")

set(sources
    "test.c")

set(inc_dirs
    ${inc_dirs}
    ${CMAKE_SOURCE_DIR}/libcdio/include
    ${PROJECT_SOURCE_DIR}/liblick)

set(link_dirs
    ${link_dirs}
    ${CMAKE_SOURCE_DIR}/libcdio/lib
    ${PROJECT_BINARY_DIR}/liblick)

if(WIN32)
    set(libs
        ${libs}
        lick_static
        iso9660
        cdio)
else()
    set(libs
        ${libs}
        lick
        iso9660
        cdio)
endif()

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})
