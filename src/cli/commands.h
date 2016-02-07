#pragma once

#include "program-status.h"

void handle_error(program_status_t *p);
int install_iso(program_status_t *p, char *iso);
int uninstall_id(program_status_t *p, char *id);
