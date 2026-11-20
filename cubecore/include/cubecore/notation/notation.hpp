#pragma once

#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cubecore/model/move.hpp"

namespace cubecore {

/// Thrown by parseMoves. position() is the byte offset in the input where parsing failed, so the
/// caller can point at it.
class NotationError : public std::invalid_argument {
public:
    NotationError(const std::string& message, std::size_t position)
        : std::invalid_argument(message), position_(position) {}

    [[nodiscard]] std::size_t position() const noexcept { return position_; }

private:
    std::size_t position_;
};

/// Parses Singmaster notation for the 18 face turns: a face letter (U R F D L B) followed by
/// nothing (clockwise), 2 (half turn) or ' (anticlockwise). Spaces and commas between moves are
/// optional, so "R U R' U'" and "RUR'U'" are the same. Also accepted, because they appear in
/// sequences copied from websites: the typographic apostrophe ’ and "2'" (same as 2).
/// Wide moves, slice moves and cube rotations (r, M, x, ...) are rejected with an explanation.
[[nodiscard]] std::vector<Move> parseMoves(std::string_view text);

/// Formats moves as space-separated Singmaster notation: "R U R' U'".
[[nodiscard]] std::string formatMoves(std::span<const Move> moves);

/// The sequence that undoes `moves`: each move inverted, in reverse order. (R U)⁻¹ = U' R'.
[[nodiscard]] std::vector<Move> invertMoves(std::span<const Move> moves);

}  // namespace cubecore
