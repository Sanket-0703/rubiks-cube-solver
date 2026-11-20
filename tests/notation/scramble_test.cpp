#include "cubecore/notation/scramble.hpp"

#include <array>
#include <cstdint>
#include <gtest/gtest.h>

#include "cubecore/model/face.hpp"
#include "cubecore/notation/notation.hpp"

namespace {

using cubecore::face;
using cubecore::randomScramble;

TEST(RandomScramble, HasRequestedLength) {
    EXPECT_TRUE(randomScramble(0, 1).empty());
    EXPECT_EQ(randomScramble(1, 1).size(), 1U);
    EXPECT_EQ(randomScramble(25, 1).size(), 25U);
}

TEST(RandomScramble, SameSeedSameScramble) {
    EXPECT_EQ(randomScramble(30, 1234), randomScramble(30, 1234));
    EXPECT_NE(randomScramble(30, 1234), randomScramble(30, 1235));
}

TEST(RandomScramble, IsIdenticalOnEveryPlatform) {
    // Golden values. If these differ between macOS and Linux in CI, scrambles (and so benchmark
    // inputs) wouldn't be reproducible across machines.
    EXPECT_EQ(cubecore::formatMoves(randomScramble(10, 42)), "R2 U L R' F' L' R' F' D U");
    EXPECT_EQ(cubecore::formatMoves(randomScramble(8, 0)), "R L2 U' F' R2 D B2 D'");
}

TEST(RandomScramble, NoRedundantNeighbours) {
    for (std::uint64_t seed = 0; seed < 500; ++seed) {
        const auto s = randomScramble(40, seed);
        for (std::size_t i = 1; i < s.size(); ++i) {
            ASSERT_NE(face(s[i]), face(s[i - 1])) << "seed " << seed << " at " << i;
            if (i >= 2) {
                const bool xyx = face(s[i]) == face(s[i - 2]) &&
                                 face(s[i - 1]) == cubecore::opposite(face(s[i]));
                ASSERT_FALSE(xyx) << "seed " << seed << " at " << i;
            }
        }
    }
}

TEST(RandomScramble, UsesEveryMove) {
    std::array<int, cubecore::kMoveCount> seen{};
    for (const auto move : randomScramble(2000, 5)) {
        ++seen[cubecore::toIndex(move)];
    }
    for (std::size_t i = 0; i < seen.size(); ++i) {
        // 2000 moves over 18 kinds: about 111 each. 60 would be wildly unlikely.
        EXPECT_GT(seen[i], 60) << cubecore::toString(cubecore::kAllMoves[i]);
    }
}

}  // namespace
