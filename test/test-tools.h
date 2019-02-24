#pragma once

#include <string>
#include <vector>

#include "lick.h"

lickdir_t *test_lick(const char *dir);

node_t *vector_to_list(const std::vector<void*> &v);
