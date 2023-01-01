#include <array>

#include "catch2/catch.hpp"
#include "lick.h"

static int num_frees = 0;
static void track_frees(void *) {
  num_frees++;
}

TEST_CASE("List operations work", "[llist]") {
  std::array<int, 5> nums;
  for (size_t i = 0; i < nums.size(); ++i) {
    nums[i] = i + 1;
  }
  node_t *lst = nullptr;
  for (size_t i = nums.size(); i-- > 0;) {
    lst = new_node(&nums[i], lst);
  }

  REQUIRE(list_length(lst) == nums.size());
  lst = list_reverse(lst);
  REQUIRE(list_length(lst) == nums.size());

  {
    int i = nums.size();
    for (node_t *n = lst; n != NULL; n = n->next, --i) {
      REQUIRE(*(const int *)n->val == i);
    }
  }

  free_list(lst, track_frees);
  REQUIRE(num_frees == nums.size());
}
