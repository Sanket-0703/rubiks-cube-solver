#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "cubecore/model/rubiks_cube.hpp"

namespace cubecore {

inline constexpr std::size_t kStickerCount = 54;

/// The whole cube as 54 characters: the faces in URFDLB order, each read row by row (in the
/// orientation described in rubiks_cube.hpp). Each sticker is written as the letter of the face
/// whose colour it has, so a solved cube is
///
///     UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB
///
/// This is the format Kociemba's solver and most cube tools read and write.
[[nodiscard]] std::string toFaceletString(const RubiksCube& cube);

/// Paints `cube` from a facelet string. Throws std::invalid_argument unless the string has
/// exactly 54 characters from "URFDLB" and nine of each. (Whether that colouring is a cube you
/// could actually reach by turning, with no twisted corner or flipped edge, is checked in Level 6.)
void loadFaceletString(RubiksCube& cube, std::string_view facelets);

}  // namespace cubecore
