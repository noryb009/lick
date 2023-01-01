set(sources
  "arguments.c"
  "commands.c"
  "main.c"
  "program-status.c"
  "user-interface.c"
)

add_executable(lick-cli ${sources})

target_compile_features(lick-cli PUBLIC c_std_11)
target_link_libraries(lick-cli PRIVATE lick)

install(TARGETS lick-cli DESTINATION .)
