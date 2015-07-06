set(exe_name "lick-fltk")

set(sources
    backend.cxx
    drop-area.cxx
    frontend.cxx
    ipc.cxx
    ipc-lick.cxx)

find_package(FLTK REQUIRED)
FLTK_WRAP_UI(${exe_name} gui.fl)
set(sources
    ${sources}
    ${${exe_name}_FLTK_UI_SRCS})

set(inc_dirs
    ${inc_dirs}
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/libcdio/include
    ${PROJECT_SOURCE_DIR}/liblick)
set(link_dirs
    ${link_dirs}
    ${CMAKE_SOURCE_DIR}/libcdio/lib
    ${PROJECT_BINARY_DIR}/liblick)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS " -static")
    set(libs
        ${libs}
        fltk wsock32 comctl32
        lick_static
        iso9660
        cdio)
    set(sources
        WIN32
        ${sources})
else()
    set(libs
        ${libs}
        ${FLTK_LIBRARIES}
        lick
        iso9660
        cdio)
endif()

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})

install(TARGETS ${exe_name}
    DESTINATION .)
