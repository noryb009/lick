# LICK Documentation

LICK is a method to install Linux distributions from inside Windows.
It can install directly from an ISO file, bypassing the need to burn it
to a CD.
Additionally, it installs a bootloader (either GRUB4DOS or GRUB2), so that
the user doesn't need to do so themselves.

LICK also includes a library, liblick, that can be used in other projects.
It is released under the MIT licence. This documentation describes liblick.

## Useful headers

* @ref lick.h :
  Include this file, and all other headers will be included.
* @ref install.h :
  The main file to interact with.
  Installs, uninstalls, and lists distribution installations.
* @ref boot-loader.h :
  Installs and uninstalls the GRUB boot loader.
* @ref id-utils.h :
  Check if an id is valid.
* @ref drives.h :
  Lists system drives.
* @ref system-info.h :
  Get information about the system.

## Outline of a Program

Below is a summary of what the LICK CLI interface does to use liblick.
To see an example of how to use the following functions, see src/cli.c
in the LICK source code.

### Setup

1.  Use @ref get_system_info and @ref get_lickdir to get information about
    the system and the lick configuration directory, respectively.
2.  Use @ref get_loader to get the system loader. [1]
4.  Use @ref list_installed to get a list of installed distributions.

[1]: Due to how the bootloader of Windows ME works, you should use
     @ref check_loader_me_patch to check if the suggested patch has
     been installed. If not, you should prompt the user to do so.


### Pre-installation of a Distribution

1.  Optionally use @ref gen_id and @ref gen_name to convert an ISO file path
    to a valid ID and name, respectively.
2.  Use @ref is_valid_id to check if an ID is valid. Any name is valid.
3.  Use @ref all_drives to get a list of all system drives, and use
    [drive_t.path](@ref drive_t#path) to get their names. You might want to
    suggest the drive [lickdir_t.drive](@ref lickdir_t#drive), which LICK
    is installed on.

### Installation of a Distribution

1.  If @ref check_loader returns false, use @ref install_loader
    to install GRUB.
2.  Use @ref install to install the distribution.
    If you want to show a progress bar, use @ref install_cb

### Uninstall a Distribution

1.  Use @ref list_installed to get a list of all installed distributions.
2.  Uninstall one using @ref uninstall.
3.  If this is the last installation, you may want to use @ref uninstall_loader
