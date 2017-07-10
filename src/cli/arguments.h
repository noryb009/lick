#pragma once

#include <lick.h>
#include "program-status.h"

typedef struct {
    int check_program;
    int try_uac;
    int ignore_errors;
    int me_check;
    int check_loader;
    int install_loader;
    int fix_loader;
    int check_fix_loader;
    string_node_t *install;
    string_node_t *uninstall;
    int uninstall_all;
    int reinstall;
} program_args_t;

void free_program_args(program_args_t *a);

void print_help();
program_args_t *handle_args(program_status_t *p, int argc, char **argv);
