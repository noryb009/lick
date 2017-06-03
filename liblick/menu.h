/**
 * @file
 * @brief a plugin system to manage boot loader menus
 *
 * Each boot loader has an associated menu structure. There are three parts to
 * the menu system:
 * 1) the distro-specific generator
 * 2) the menu-specific generator
 * 3) the menu-type writer
 *
 * The distro-specific generator is given a list of files extracted from the
 * given ISO, and returns what file is the kernel, the initrd, boot options,
 * and more.
 * The menu-specific generator is given this information and creates an
 * entry for it's menu.
 * Finally, this is passed to the menu-type writer, which places this entry
 * into the menu, and removes it on uninstall.
 *
 * For example, the Puppy Linux generator is given files, and will return the
 * .../vmlinuz, .../initrd.gz and "psubdir=... pfix=..." as the kernel, initrd
 * and boot options, respectively. This is passed to the grub4dos generator,
 * which creates a section containing:
 * \code{.unparsed}
 * title Puppy Linux
 * kernel .../vmlinuz psubdir=... pfix=...
 * initrd .../initrd.gz
 * boot
 * \endcode
 * This is then passed to the flat-menu writer, which will place it after the
 * other entries in menu.lst.
 */

#pragma once

#include "distro.h"
#include "lickdir.h"
#include "llist.h"

typedef int (*menu_install_f)(lickdir_t *lick);
typedef int (*menu_uninstall_f)(lickdir_t *lick);
typedef int (*menu_fix_f)(lickdir_t *lick);
typedef int (*menu_check_fix_f)(lickdir_t *lick);
typedef char *(*menu_gen_section_f)(distro_info_t *info);
typedef int (*menu_append_section_f)(const char *id, const char *section,
        lickdir_t *lick);
typedef int (*menu_remove_section_f)(const char *id, lickdir_t *lick);

/**
 * @brief Hooks to call boot loader menu plugins
 */
typedef struct {
    /// install hook
    menu_install_f install;
    /// uninstall hook
    menu_uninstall_f uninstall;
    /// fix the bootloader when the system does not boot LICK
    menu_fix_f fix_loader;
    /// check if the bootloader fix has been applied
    menu_check_fix_f check_fix_loader;
    // TODO: header hook
    /// generate section for the given distro info
    menu_gen_section_f gen_section;
    /// append section to the menu
    menu_append_section_f append_section;
    /// remove sections from the menu
    menu_remove_section_f remove_section;
} menu_t;

/**
 * @brief create a new menu
 * @return a new menu_t, with the given info filled in. Must be freed using
 * free_menu
 */
menu_t *new_menu(menu_install_f i, menu_uninstall_f u,
        menu_fix_f f, menu_check_fix_f c,
        menu_gen_section_f g, menu_append_section_f a, menu_remove_section_f r);

/**
 * @brief frees a menu_t and all data associated with it
 * @param m the menu to free
 */
void free_menu(menu_t *m);

/**
 * @brief add an entry to a menu file
 * @param files a list of files extracted from the ISO
 * @param dst the destination folder
 * @param distro handles to functions for the distribution
 * @param id the ID of the installation
 * @param name the human-friendly name of the installation
 * @param lick the LICK directory
 * @param menu the menu being used
 * @return 1 on success, 0 on error
 */
int install_menu(node_t *files, const char *dst, distro_t *distro,
        const char *id, const char *name, lickdir_t *lick, menu_t *menu);
int uninstall_menu(const char *id, lickdir_t *lick, menu_t *menu);
