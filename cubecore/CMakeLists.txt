# cubecore — the library that holds all the real code (cube models, search, pattern databases).
# The CLI, tests, benchmarks and (Level 10) the WebAssembly build all link against it.

# Build settings baked into the library, so `cubesolver info` reports how it was built.
list(JOIN RCS_SANITIZERS "," RCS_SANITIZERS_TEXT)
if(NOT RCS_SANITIZERS_TEXT)
    set(RCS_SANITIZERS_TEXT "none")
endif()
if(RCS_COVERAGE)
    set(RCS_COVERAGE_CPP true)
else()
    set(RCS_COVERAGE_CPP false)
endif()
configure_file(src/config.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/generated/config.hpp" @ONLY)

add_library(
    cubecore STATIC
    src/build_info.cpp
    src/model/move.cpp
    src/model/rubiks_cube.cpp
    src/model/rubiks_cube_3d_array.cpp
    src/model/facelets.cpp
    src/notation/notation.cpp
    src/notation/scramble.cpp
    src/render/net.cpp)
add_library(cubecore::cubecore ALIAS cubecore)

target_include_directories(
    cubecore
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_features(cubecore PUBLIC cxx_std_20)
rcs_configure_target(cubecore)
