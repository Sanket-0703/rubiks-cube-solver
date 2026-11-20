#pragma once

#include <array>

#include "cubecore/model/face.hpp"
#include "cubecore/model/facelets.hpp"
#include "cubecore/model/move.hpp"

namespace cubetest {

/// A second, independent way to turn the cube, used to check every hand-written model.
///
/// Instead of listing which sticker goes where, it places all 54 stickers in 3D space
/// (x = right, y = up, z = towards you; each sticker has a position in {-1,0,1}^3 and an outward
/// normal) and turns a face by rotating every sticker in that layer -90 degrees about the face's
/// normal. If a hand-written move disagrees with plain geometry, the hand-written move is wrong.
using Stickers = std::array<cubecore::Color, cubecore::kStickerCount>;

/// Stickers of a solved cube, indexed face * 9 + row * 3 + col.
[[nodiscard]] Stickers solvedStickers();

/// Applies `move` to `stickers` by rotating them in 3D.
void applyGeometric(Stickers& stickers, cubecore::Move move);

}  // namespace cubetest
