#pragma once

#include <cassert>
#include <cstdint>

namespace cubecore {

/// SplitMix64 (Steele, Lea & Flood, 2014): a tiny, fast, well-mixed 64-bit generator.
///
/// Why not std::mt19937_64 + std::uniform_int_distribution? The engine is portable, but the
/// distribution is not: libc++ (macOS) and libstdc++ (Linux) turn the same engine output into
/// different numbers. A seeded scramble must be the same move list everywhere, or benchmark
/// results couldn't be reproduced on another machine. Everything here is fixed-width integer
/// maths, so the output is identical on every platform and compiler.
class SplitMix64 {
public:
    explicit constexpr SplitMix64(std::uint64_t seed) noexcept : state_(seed) {}

    /// The next 64 random bits.
    constexpr std::uint64_t next() noexcept {
        state_ += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = state_;
        z = (z ^ (z >> 30U)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27U)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31U);
    }

    /// A uniformly distributed number in [0, bound). bound must be > 0.
    ///
    /// Plain next() % bound would be slightly biased towards small results (2^64 isn't a
    /// multiple of bound). Rejecting the few values below 2^64 mod bound removes the bias.
    constexpr std::uint64_t below(std::uint64_t bound) noexcept {
        assert(bound > 0);
        const std::uint64_t threshold = (0 - bound) % bound;  // == 2^64 mod bound
        while (true) {
            const std::uint64_t r = next();
            if (r >= threshold) {
                return r % bound;
            }
        }
    }

private:
    std::uint64_t state_;
};

}  // namespace cubecore
