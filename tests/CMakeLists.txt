include(GoogleTest)

add_executable(cubecore_tests build_info_test.cpp)
target_link_libraries(cubecore_tests PRIVATE cubecore::cubecore GTest::gtest_main)
rcs_configure_target(cubecore_tests)

# PRE_TEST: list the tests when ctest runs, not at build time, so building never has to start
# an instrumented binary.
gtest_discover_tests(cubecore_tests DISCOVERY_MODE PRE_TEST)

# Smoke tests for the CLI: the binary starts, parses arguments and reports its version.
add_test(NAME cli.version COMMAND cubesolver --version)
set_tests_properties(cli.version PROPERTIES PASS_REGULAR_EXPRESSION "^${PROJECT_VERSION}")

add_test(NAME cli.info COMMAND cubesolver info)
set_tests_properties(cli.info PROPERTIES PASS_REGULAR_EXPRESSION "build type +${CMAKE_BUILD_TYPE}")

add_test(NAME cli.requires_subcommand COMMAND cubesolver)
set_tests_properties(cli.requires_subcommand PROPERTIES WILL_FAIL TRUE)
