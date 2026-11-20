#include "cubecore/notation/scramble.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "cubecore/model/face.hpp"
#include "cubecore/util/split_mix64.hpp"

namespace cubecore {

std::vector<Move> randomScramble(std::size_t length, std::uint64_t seed) {
    SplitMix64 rng{seed};
    std::vector<Move> moves;
    moves.reserve(length);

    while (moves.size() < length) {
        const auto face = static_cast<Face>(rng.below(kFaceCount));
        const std::size_t n = moves.size();
        if (n >= 1 && cubecore::face(moves[n - 1]) == face) {
            continue;  // same face twice in a row
        }
        if (n >= 2 && cubecore::face(moves[n - 2]) == face &&
            cubecore::face(moves[n - 1]) == opposite(face)) {
            continue;  // X Y X with Y opposite X
        }
        const auto turns = static_cast<int>(rng.below(3)) + 1;
        moves.push_back(makeMove(face, turns));
    }
    return moves;
}

}  // namespace cubecore
