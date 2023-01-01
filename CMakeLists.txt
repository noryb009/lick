cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

include(FetchContent)

set(proj_name "LICK")
set(VERSION_MAJOR "1")
set(VERSION_MINOR "3")
set(VERSION_PATCH "4")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

project(LICK LANGUAGES C CXX)

if ("${CMAKE_TOOLCHAIN_FILE}" MATCHES "")
  set(TOOLCHAIN_FILE "")
else()
  get_filename_component(TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
endif()

if(WIN32)
    # `--no-insert-timestamp` is needed for deterministic builds.
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -Xlinker --no-insert-timestamp")
endif()

# TODO: Use -Os
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -static-libgcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -static-libgcc")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -Os")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -Os")

add_subdirectory(cmake)

add_subdirectory(liblick)

include(CTest)
add_subdirectory(test)

add_subdirectory(src)

install(DIRECTORY res
    DESTINATION .)
install(FILES README.md
    DESTINATION .
    RENAME README.txt)
install(FILES LICENSE
    DESTINATION .
    RENAME LICENSE.txt)

# CPack
set(CPACK_GENERATOR "${CPACK_GENERATOR};ZIP")
set(CPACK_PACKAGING_INSTALL_PREFIX "")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${proj_name}")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${VERSION}")
set(CPACK_PACKAGE_VENDOR "Luke Lorimer (noryb009)")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "lick")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
if(WIN32 AND NOT UNIX)
    set(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
    #set(CPACK_PACKAGE_ICON "")
    set(CPACK_NSIS_DISPLAY_NAME "LICK")
    set(CPACK_NSIS_HELP_LINK "https://github.com/noryb009/lick")
    set(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_NSIS_HELP_LINK}")
    set(CPACK_NSIS_CONTACT "noryb009@gmail.com")

    set(CPACK_NSIS_MENU_LINKS
        "lick-cli.exe" "LICK Command Line Interface"
        "lick-fltk.exe" "LICK")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "..\\\\lick-fltk.exe")

    # on reinstall, uninstall boot loader, then install overtop
    set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
        "
        Var /GLOBAL lick_reinstall_boot
        Var /GLOBAL lick_refix_boot
        Var /GLOBAL lick_exit_code
        StrCpy $lick_reinstall_boot 0
        ; if old install exists
        IfFileExists '$INSTDIR\\\\lick-cli.exe' 0 lick_after_preinstall
          ; if is a valid install
          nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -s --check-program'
          Pop $lick_exit_code
          StrCmp '$lick_exit_code' '0' 0 lick_after_preinstall
          ; if bootloader is installed
          nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -sc'
          Pop $lick_exit_code
          StrCmp '$lick_exit_code' '1' 0 lick_after_preinstall
          ; if the fix has been applied to the bootloader
          nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -s --check-fix-loader'
          Pop $lick_refix_boot
          ; uninstall loader
          nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -s --uninstall-loader --reinstall'
          StrCpy $lick_reinstall_boot 1
        lick_after_preinstall:")

    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "
        StrCmp '$lick_reinstall_boot' '0' lick_after_install
          nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -s --install-loader'
          StrCmp '$lick_refix_boot' '2' 0 lick_after_install
            nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -s --fix-loader'
        lick_after_install:")

    # offer to uninstall all on uninstall
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "
        MessageBox MB_YESNO 'Uninstall bootloader and all currently installed Linux versions?' /SD IDNO IDNO lick_after_uninstall
          nsExec::Exec '\\\"$INSTDIR\\\\lick-cli.exe\\\" -s --uninstall-all'
        lick_after_uninstall:")
endif()

include(CPack)
