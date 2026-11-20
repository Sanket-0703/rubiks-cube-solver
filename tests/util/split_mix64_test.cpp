#include "cubecore/util/split_mix64.hpp"

#include <array>
#include <cstdint>
#include <gtest/gtest.h>

namespace {

using cubecore::SplitMix64;

TEST(SplitMix64, MatchesReferenceImplementation) {
    // First outputs of Vigna's reference splitmix64.c for seed 0.
    SplitMix64 rng{0};
    EXPECT_EQ(rng.next(), 0xE220A8397B1DCDAFULL);
    EXPECT_EQ(rng.next(), 0x6E789E6AA1B965F4ULL);
    EXPECT_EQ(rng.next(), 0x06C45D188009454FULL);
}

TEST(SplitMix64, IsConstexpr) {
    constexpr std::uint64_t kFirst = [] {
        SplitMix64 rng{0};
        return rng.next();
    }();
    static_assert(kFirst == 0xE220A8397B1DCDAFULL);
    SUCCEED();
}

TEST(SplitMix64, BelowStaysInRangeAndCoversIt) {
    SplitMix64 rng{7};
    std::array<int, 6> counts{};
    for (int i = 0; i < 6000; ++i) {
        const std::uint64_t r = rng.below(6);
        ASSERT_LT(r, 6U);
        ++counts[r];
    }
    for (const int count : counts) {
        EXPECT_GT(count, 850);  // expected 1000 each
        EXPECT_LT(count, 1150);
    }
}

TEST(SplitMix64, BelowOneIsAlwaysZero) {
    SplitMix64 rng{1};
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(rng.below(1), 0U);
    }
}

}  // namespace
