if(CMAKE_BUILD_TYPE STREQUAL "Debug")
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
    set(catch "catch-2.6.1")
    ExternalProject_Add(
        ${catch}
        PREFIX ${catch}
        URL https://github.com/catchorg/Catch2/releases/download/v2.6.1/catch.hpp
        URL_HASH SHA256=a53ef31cf0bd4c1038cdaf8a38d6c88ee762e6aba181f9534a026363bc73e430
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

endif()
