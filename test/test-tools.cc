#include "test-tools.h"

lickdir_t *test_lick(const char *dir) {
  return new_lickdir(dir[0],
      concat_strs(2, dir, "/entries"),
      concat_strs(2, dir, "/res"));
}
