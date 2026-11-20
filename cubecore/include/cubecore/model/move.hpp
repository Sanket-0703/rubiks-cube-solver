#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "cubecore/model/face.hpp"

namespace cubecore {

/// The 18 face turns of the half-turn metric. Each face has three: a clockwise quarter turn (U),
/// a half turn (U2) and an anticlockwise quarter turn (U'), always as seen looking at that face.
/// Laid out face-major, so a move's index is 3 * face + (quarter turns - 1).
// clang-format off
enum class Move : std::uint8_t {
    U, U2, UPrime,
    R, R2, RPrime,
    F, F2, FPrime,
    D, D2, DPrime,
    L, L2, LPrime,
    B, B2, BPrime,
};
// clang-format on

inline constexpr std::size_t kMoveCount = 18;

[[nodiscard]] constexpr std::size_t toIndex(Move move) noexcept {
    return static_cast<std::size_t>(move);
}

inline constexpr std::array<Move, kMoveCount> kAllMoves = [] {
    std::array<Move, kMoveCount> moves{};
    for (std::size_t i = 0; i < kMoveCount; ++i) {
        moves[i] = static_cast<Move>(i);
    }
    return moves;
}();

/// The face a move turns.
[[nodiscard]] constexpr Face face(Move move) noexcept {
    return static_cast<Face>(toIndex(move) / 3);
}

/// Clockwise quarter turns: 1 for U, 2 for U2, 3 for U'.
[[nodiscard]] constexpr int quarterTurns(Move move) noexcept {
    return static_cast<int>(toIndex(move) % 3) + 1;
}

/// The move that turns `face` clockwise by `turns` quarter turns. `turns` is taken mod 4 and must
/// not be a multiple of 4 (that would be no move at all).
[[nodiscard]] constexpr Move makeMove(Face face, int turns) noexcept {
    const int normalized = ((turns % 4) + 4) % 4;  // 1, 2 or 3
    return static_cast<Move>(toIndex(face) * 3 + static_cast<std::size_t>(normalized - 1));
}

/// The move that undoes `move`: U ↔ U', U2 ↔ U2.
[[nodiscard]] constexpr Move inverse(Move move) noexcept {
    return makeMove(face(move), 4 - quarterTurns(move));
}

/// Singmaster notation: "U", "U2", "U'".
[[nodiscard]] std::string_view toString(Move move) noexcept;

}  // namespace cubecore
