set(exe_name "lick-fltk")

set(sources
    backend.cc
    drop-area.cc
    frontend.cc
    ipc.cc
    ipc-lick.cc)

# TODO: swap when cmake fixes fltk_wrap_ui
#fltk_wrap_ui(${exe_name} gui.fl)
#set(sources
#    ${sources}
#    ${${exe_name}_FLTK_UI_SRCS})
set(gui_header ${CMAKE_CURRENT_BINARY_DIR}/gui.h)
set(gui_source ${CMAKE_CURRENT_BINARY_DIR}/gui.cc)
set(gui_fl ${CMAKE_CURRENT_LIST_DIR}/gui.fl)
message("${CMAKE_CURRENT_LIST_DIR}/gui.fl")
add_custom_command(
    OUTPUT ${gui_source}
    COMMAND fluid -h ${gui_header} -o ${gui_source} -c ${gui_fl}
    DEPENDS ${gui_fl})
set(sources
    ${sources}
#    ${${exe_name}_FLTK_UI_SRCS})
    ${gui_source})
set(inc_dirs
    ${inc_dirs}
    ${CMAKE_CURRENT_BINARY_DIR})


set(inc_dirs
    ${inc_dirs}
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_BINARY_DIR}/fltk/include
    ${PROJECT_BINARY_DIR}/libarchive/include
    ${PROJECT_SOURCE_DIR}/liblick
    ${PROJECT_BINARY_DIR}/liblick)
set(link_dirs
    ${link_dirs}
    ${PROJECT_BINARY_DIR}/fltk/lib
    ${PROJECT_BINARY_DIR}/libarchive/lib
    ${PROJECT_BINARY_DIR}/liblick)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS " -static")
    set(libs
        ${libs}
        fltk wsock32 comctl32
        lick_static
        archive_static)
    set(sources
        WIN32
        ${sources})
else()
    set(libs
        ${libs}
        ${FLTK_LIBRARIES}
        lick
        archive)
endif()

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})

install(TARGETS ${exe_name}
    DESTINATION .)
