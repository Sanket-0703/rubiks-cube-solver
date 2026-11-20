# cubesolver — the command-line app. Subcommands arrive level by level:
# info (Level 1), show (2), solve (4), bench (5), build-pdb (7).

add_executable(cubesolver main.cpp)
target_link_libraries(cubesolver PRIVATE cubecore::cubecore CLI11::CLI11)
rcs_configure_target(cubesolver)
