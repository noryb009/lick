#include <cstdint>

#include "catch2/catch.hpp"

int fib(const uint64_t n) {
  uint64_t a = 1;
  uint64_t b = 0;
  for (uint64_t i = 0; i < n; ++i) {
    const uint64_t c = a + b;
    a = b;
    b = c;
  }
  return b;
}

TEST_CASE("Fib is correct", "[fib]") {
  REQUIRE(fib(0) == 0);
  REQUIRE(fib(1) == 1);
  REQUIRE(fib(2) == 1);
  REQUIRE(fib(3) == 2);
  REQUIRE(fib(4) == 3);
  REQUIRE(fib(10) == 55);
}
