cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(proj_name "LICK")

project(${proj_name})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

if(WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
endif()

add_subdirectory(liblick)
add_subdirectory(test)
add_subdirectory(src)
