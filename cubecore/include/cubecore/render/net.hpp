#pragma once

#include <cstdint>
#include <string>

#include "cubecore/model/rubiks_cube.hpp"

namespace cubecore {

enum class NetStyle : std::uint8_t {
    Plain,    ///< Colour letters (W R G Y O B), for logs, tests and terminals without colour.
    Ansi256,  ///< Coloured squares using the 256-colour ANSI palette (has a real orange).
};

/// Draws the cube unfolded as a cross, with each face in the orientation described in
/// rubiks_cube.hpp:
///
///            W W W
///            W W W
///            W W W
///     O O O  G G G  R R R  B B B
///     O O O  G G G  R R R  B B B
///     O O O  G G G  R R R  B B B
///            Y Y Y
///            Y Y Y
///            Y Y Y
///
/// Every line ends with '\n'; no line has trailing spaces.
[[nodiscard]] std::string renderNet(const RubiksCube& cube, NetStyle style);

}  // namespace cubecore
