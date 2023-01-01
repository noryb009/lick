FetchContent_Declare(
  Catch2
  URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.10.tar.gz
  URL_HASH SHA256=d54a712b7b1d7708bc7a819a8e6e47b2fde9536f487b89ccbca295072a7d9943
  DOWNLOAD_EXTRACT_TIMESTAMP false
  OVERRIDE_FIND_PACKAGE
)
find_package(Catch2)
