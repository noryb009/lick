#pragma once

#include "program-status.h"

#define INVALID_INT -1
int ask_int();
int ask_bool(int default_val, char *try_again);

char *ask_iso();
int ask_install_loader(program_status_t *p);
int ask_uninstall_loader(program_status_t *p);
int auto_install(program_status_t *p, char *iso);

int main_menu(program_status_t *p);
int entry_submenu(program_status_t *p);
int ask_uninstall(program_status_t *p);
char *ask_drive(drive_node_t *drives);
