include(GoogleTest)

add_executable(
    cubecore_tests
    build_info_test.cpp
    model/face_move_test.cpp
    model/rubiks_cube_model_test.cpp
    model/facelets_test.cpp
    notation/notation_test.cpp
    notation/scramble_test.cpp
    render/net_test.cpp
    util/split_mix64_test.cpp
    support/sticker_geometry.cpp)
target_link_libraries(cubecore_tests PRIVATE cubecore::cubecore GTest::gtest_main)
rcs_configure_target(cubecore_tests)

# PRE_TEST: list the tests when ctest runs, not at build time, so building never has to start
# an instrumented binary.
gtest_discover_tests(cubecore_tests DISCOVERY_MODE PRE_TEST)

# Smoke tests for the CLI: the binary starts, parses arguments and does the right thing.
add_test(NAME cli.version COMMAND cubesolver --version)
set_tests_properties(cli.version PROPERTIES PASS_REGULAR_EXPRESSION "^${PROJECT_VERSION}")

add_test(NAME cli.info COMMAND cubesolver info)
set_tests_properties(cli.info PROPERTIES PASS_REGULAR_EXPRESSION "build type +${CMAKE_BUILD_TYPE}")

add_test(NAME cli.requires_subcommand COMMAND cubesolver)
set_tests_properties(cli.requires_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli.show.solved COMMAND cubesolver show --color never)
set_tests_properties(cli.show.solved PROPERTIES PASS_REGULAR_EXPRESSION "Solved: yes")

add_test(NAME cli.show.scramble_and_back
         COMMAND cubesolver show --color never --scramble "R U R' U' U R U' R'")
set_tests_properties(cli.show.scramble_and_back PROPERTIES PASS_REGULAR_EXPRESSION "Solved: yes")

add_test(NAME cli.show.facelets COMMAND cubesolver show --color never --facelets -s "U")
set_tests_properties(cli.show.facelets PROPERTIES PASS_REGULAR_EXPRESSION
                     "Facelets: UUUUUUUUUBBBRRRRRR")

add_test(NAME cli.show.bad_notation COMMAND cubesolver show --scramble "R U X")
set_tests_properties(cli.show.bad_notation PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli.scramble.seeded COMMAND cubesolver scramble --seed 42 --length 10)
set_tests_properties(cli.scramble.seeded PROPERTIES PASS_REGULAR_EXPRESSION
                     "^R2 U L R' F' L' R' F' D U")

# Five moves. (CMake regexes have no {n} repetition, so the pattern is spelled out.)
set(_move "[URFDLB][2']?")
add_test(NAME cli.scramble.random COMMAND cubesolver scramble -n 5)
set_tests_properties(cli.scramble.random PROPERTIES PASS_REGULAR_EXPRESSION
                     "${_move} ${_move} ${_move} ${_move} ${_move}\n")
