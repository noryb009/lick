#include <cstring>

#include "catch2/catch.hpp"
#include "lick.h"
#include "boot-loader/9x.h"
#include "boot-loader/boot-loader-utils.h"
#include "boot-loader/nt.h"

#include "test-tools.h"

int compare_files(const char *a, const char *b) {
  if((a == NULL || b == NULL) && a != b)
    return 0;
  for(int i = 0, j = 0;; ++i, ++j) {
    int a_newline = 0, b_newline = 0;
    if(a[i] == '\n') {
      while(a[i] == '\n')
        ++i;
      a_newline = 1;
    }
    if(b[j] == '\n') {
      while(b[j] == '\n')
        ++j;
      b_newline = 1;
    }
    if(a[i] == '\0' && b[j] == '\0')
      return 1;
    else if(a_newline != b_newline)
      return 0;
    else if(a[i] != b[j])
      return 0;
  }
}

void timeout_inner(const char *orig, const char *key, const char *sep, const char *expect) {
  char *cpy = strdup2(orig);
  cpy = check_timeout(cpy, key, sep);
  REQUIRE(strcmp(cpy, expect) == 0);
  free(cpy);
}

TEST_CASE("test that timeout is correctly modified in .ini files", "[bootloader]") {
#define TIMEOUT_GEN(t) "[a]\nb=c,d\nto=WINDOWS," t "\ne=f,g\n"
  const char *two = TIMEOUT_GEN("2");
  const char *twenty_nine = TIMEOUT_GEN("29");
  const char *goal = TIMEOUT_GEN("5");
  const char *no_effect = TIMEOUT_GEN("--");
  timeout_inner("", "to", ",", "");
  timeout_inner(two, "to", ",", goal);
  timeout_inner(two, "to", "=", goal);
  timeout_inner(TIMEOUT_GEN("60"), "to", "=", goal);
  timeout_inner(goal, "to", "=", goal);
  timeout_inner(twenty_nine, "to", "=", twenty_nine);
  timeout_inner(no_effect, "to", "=", no_effect);
}

void bootloader_inner(char *(*fn)(char *, lickdir_t *), const char *str, lickdir_t *lick, const char *expect) {
  char *cpy = strdup2(str);
  char *after_fn = fn(cpy, lick);
  free(cpy);
  REQUIRE(after_fn);
  REQUIRE(compare_files(after_fn, expect));
  free(after_fn);
}



TEST_CASE("test that 9x bootloaders are correctly modified", "[bootloader]") {
  lickdir_t *lick_c = test_lick("C:\\lick");
  lickdir_t *lick_z = test_lick("Z:\\lick");

#define X_START(t, e) "[menu]\nmenuitem=WINDOWS,Start Windows\n" e "menudefault=WINDOWS," #t "\nmenucolor=7,0\n"
#define X_START_LICK(t) X_START(t, "menuitem=LICK," START_LOADER_DESC "\n")
#define X_WIN "[WINDOWS]\n"
#define X_WIN_CONTENT X_WIN "a=b\nc=\"d e\"\n"
#define X_LICK(d) "[LICK]\ndevice=" d ":\\pupl.exe\ninstall=" d ":\\pupl.exe\nshell=" d ":\\pupl.exe\n"

#define X_BASE(t) X_START(t, "") X_WIN
#define X_INST(t, d) X_START_LICK(t) X_LICK(d) X_WIN
#define X_BASE_CONTENT(t) X_START(t, "") X_WIN_CONTENT
#define X_INST_CONTENT(t, d) X_START_LICK(t) X_WIN_CONTENT X_LICK(d)
  // TODO: If lick is installed on Z drive, should pupldr be put there or C?
  bootloader_inner(install_to_config_sys, "", lick_c, X_INST(5, "C"));
  bootloader_inner(install_to_config_sys, "", lick_z, X_INST(5, "C"));
  bootloader_inner(install_to_config_sys, X_BASE_CONTENT(2), lick_z, X_INST_CONTENT(5, "C"));
  bootloader_inner(install_to_config_sys, X_BASE_CONTENT(12), lick_c, X_INST_CONTENT(12, "C"));
  bootloader_inner(uninstall_from_config_sys, X_INST_CONTENT(12, "C"), lick_c, X_BASE_CONTENT(12));
  bootloader_inner(uninstall_from_config_sys, X_INST_CONTENT(3, "C"), lick_c, X_BASE_CONTENT(3));
  bootloader_inner(uninstall_from_config_sys, X_INST_CONTENT(3, "C"), lick_z, X_BASE_CONTENT(3));
  bootloader_inner(uninstall_from_config_sys, X_INST(5, "C"), lick_z, X_BASE(5));
  bootloader_inner(uninstall_from_config_sys, X_INST(5, "C"), lick_c, X_BASE(5));
  free_lickdir(lick_c);
  free_lickdir(lick_z);
}

TEST_CASE("test that NT bootloaders are correctly modified", "[bootloader]") {
  lickdir_t *lick_c = test_lick("C:\\lick");
  lickdir_t *lick_z = test_lick("Z:\\lick");

#define NT_BASE(t) "[boot loader]\ntimeout=" #t "\ndefault=abc\n[operating systems]\nabc=\"abc /abc\""
#define NT_INST(t, d) NT_BASE(t) "\n" d ":\\pupldr=\"" START_LOADER_DESC "\""
  // TODO: If lick is installed on Z drive, should pupldr be put there or C?
  bootloader_inner(install_to_boot_ini, NT_BASE(10), lick_c, NT_INST(10, "C"));
  bootloader_inner(install_to_boot_ini, NT_BASE(10), lick_z, NT_INST(10, "C"));
  bootloader_inner(install_to_boot_ini, NT_BASE(60), lick_c, NT_INST(5, "C"));
  bootloader_inner(install_to_boot_ini, NT_BASE(.5), lick_c, NT_INST(.5, "C"));
  bootloader_inner(uninstall_from_boot_ini, NT_INST(10, "C"), lick_c, NT_BASE(10));
  bootloader_inner(uninstall_from_boot_ini, NT_INST(10, "C"), lick_z, NT_BASE(10));
  free_lickdir(lick_c);
  free_lickdir(lick_z);
}
