include(CTest)

set(sources
  main.cc
)

set(sources
  ${sources}
  test-bootloader.cc
  test-list.cc
  test-puppy.cc
  test-tools.cc
)

add_executable(tests ${sources})
target_include_directories(tests PRIVATE .)
target_compile_features(tests PUBLIC cxx_std_17)
target_link_libraries(tests PRIVATE lick Catch2)

add_test(NAME LickTests COMMAND tests)
