#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "cubecore/model/move.hpp"

namespace cubecore {

/// A random sequence of `length` face turns, the same for the same seed on every platform.
///
/// Like official random-move scrambles, it never turns the same face twice in a row (U U' would
/// cancel) and never does X Y X where X and Y are opposite faces (U D U is just U2 D). Both would
/// make the scramble shorter than it looks.
///
/// A scramble of length n is at most n moves from solved, but can be fewer (some sequences have
/// shortcuts). The optimal solvers from Level 4 on report the true distance.
[[nodiscard]] std::vector<Move> randomScramble(std::size_t length, std::uint64_t seed);

}  // namespace cubecore
