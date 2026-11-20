cmake_minimum_required(VERSION 3.25)

# Must run before project(): it may pick the macOS SDK (blocker B-1).
include(cmake/AppleSdkCheck.cmake)

project(
    RubiksCubeSolver
    VERSION 0.1.0
    DESCRIPTION "3x3 Rubik's Cube solver: BFS, DFS, IDDFS, Korf's IDA* with pattern databases"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(RCS_BUILD_TESTS "Build the unit tests" ON)
option(RCS_BUILD_BENCHMARKS "Build the benchmarks" ON)
option(RCS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(RCS_COVERAGE "Instrument our targets for llvm-cov source-based coverage (clang only)" OFF)
set(RCS_SANITIZERS "" CACHE STRING "Sanitizers to enable, e.g. \"address;undefined\" or \"thread\"")

# Sanitizers are global so dependencies are instrumented too; include it before fetching them.
include(cmake/Sanitizers.cmake)
include(cmake/TargetDefaults.cmake)
include(cmake/Dependencies.cmake)

add_subdirectory(cubecore)
add_subdirectory(apps/cubesolver)

if(RCS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(RCS_BUILD_BENCHMARKS)
    add_subdirectory(bench)
endif()
