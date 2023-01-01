set(sources
  backend.cc
  drop-area.cc
  frontend.cc
  ipc.cc
  ipc-lick.cc
)

# TODO: swap when cmake fixes fltk_wrap_ui
#fltk_wrap_ui(lick-fltk gui.fl)
#set(sources
#    ${sources}
#    ${lick-fltk_FLTK_UI_SRCS})
set(gui_header ${CMAKE_CURRENT_BINARY_DIR}/gui.h)
set(gui_source ${CMAKE_CURRENT_BINARY_DIR}/gui.cc)
set(gui_fl ${CMAKE_CURRENT_LIST_DIR}/gui.fl)
add_custom_command(
  OUTPUT ${gui_source} ${gui_header}
  COMMAND fluid -h ${gui_header} -o ${gui_source} -c ${gui_fl}
  DEPENDS ${gui_fl}
)
set(sources
    ${sources}
    ${gui_source})


# TODO: set(CMAKE_EXE_LINKER_FLAGS " -static -Xlinker  --no-insert-timestamp")

add_executable(lick-fltk ${sources})
target_compile_features(lick-fltk PUBLIC cxx_std_11)
target_include_directories(lick-fltk PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(lick-fltk PRIVATE lick)

add_dependencies(lick-fltk external_fltk)
target_include_directories(lick-fltk PUBLIC ${CMAKE_BINARY_DIR}/cmake/fltk/fltk/include)
target_link_libraries(lick-fltk PUBLIC ${CMAKE_BINARY_DIR}/cmake/fltk/fltk/lib/libfltk.a)

target_include_directories(lick-fltk PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
  target_link_libraries(lick-fltk PRIVATE wsock32 comctl32)
endif()

install(TARGETS lick-fltk DESTINATION .)
