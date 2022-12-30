add_subdirectory(cli)

# Only build fltk if compiling for Windows.
if (WIN32)
  add_subdirectory(fltk)
endif()
