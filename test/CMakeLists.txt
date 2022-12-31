set(exe_name "tests")

set(sources
    main.cc)

set(sources
    ${sources}
    test-bootloader.cc
    test-list.cc
    test-puppy.cc
    test-tools.cc
)

set(inc_dirs
    ${inc_dirs}
    ${PROJECT_BINARY_DIR}/libarchive/include
    ${PROJECT_SOURCE_DIR}/liblick
    ${PROJECT_BINARY_DIR}/liblick
    ${PROJECT_SOURCE_DIR}/test)

set(link_dirs
    ${link_dirs}
    ${PROJECT_BINARY_DIR}/libarchive/lib)

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

include(ExternalProject)
set(catch "catch-2.13.9")
ExternalProject_Add(
    ${catch}
    PREFIX ${catch}
    URL https://github.com/catchorg/Catch2/releases/download/v2.13.9/catch.hpp
    URL_HASH SHA256=27da57c7a06d09be8dd81fab7246b79e7892b6ae7e4e49ba8631f1d5a955e3fc
    DOWNLOAD_NO_EXTRACT TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)
set(external_projects ${external_projects} ${catch})
set(inc_dirs
    ${inc_dirs}
    ${PROJECT_BINARY_DIR}/test/${catch}/src)
  message(${PROJECT_BINARY_DIR}/test/${catch}/src)
  message(${PROJECT_BINARY_DIR}/test/${catch}/src)

include_directories(${inc_dirs})
link_directories(${link_dirs})
add_executable(${exe_name} ${sources})
target_link_libraries(${exe_name} ${libs})
add_dependencies(${exe_name} ${catch})

add_test(NAME LickTests COMMAND ${exe_name})
