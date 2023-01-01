#include <cstring>

#include "catch2/catch.hpp"
#include "distro/puppy.h"

#include "test-tools.h"

TEST_CASE("initrd with shortest name is chosen", "[distro]") {
  lickdir_t *lick = test_lick("C:\\lick");

  std::vector<void*> files = {
    strdup2("test-file"),
    strdup2("initrd-long-long-name.gz"),
    strdup2("initrd2.xz"),
    strdup2("initrd.gz"),
    strdup2("initrd3.xz"),
    strdup2("initrd-long-long.gz"),
  };
  string_node_t *file_list = reinterpret_cast<string_node_t *>(vector_to_list(files));
  distro_info_node_t *info = distro_puppy(file_list, "C:/puppy123", "Puppy 123", lick);

  for (distro_info_node_t *c = info; c != nullptr; c = c->next) {
    REQUIRE(strcmp(c->val->initrd, "C:/puppy123/initrd.gz") == 0);
  }

  free_distro_info_node_t(info);
  free_string_node_t(file_list);
  free_lickdir(lick);
}
