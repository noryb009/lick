#include "test-tools.h"

lickdir_t *test_lick(const char *dir) {
  return new_lickdir(dir[0],
      concat_strs(2, dir, "/entries"),
      concat_strs(2, dir, "/res"));
}

node_t *vector_to_list(const std::vector<void*> &v) {
  node_t *lst = nullptr;
  for (auto it = v.rbegin(); it != v.rend(); ++it) {
    lst = new_node(*it, lst);
  }
  return lst;
}
