// Level 1 placeholder: proves Google Benchmark is wired up. It also previews the trick behind the
// Level 3 bitboard cube: a face's 8 stickers packed into one uint64_t (8 bits each), so turning
// that face clockwise is a single 16-bit rotation instead of moving 8 bytes one by one.

#include <array>
#include <benchmark/benchmark.h>
#include <bit>
#include <cstdint>

namespace {

void BM_FaceTurnByteArray(benchmark::State& state) {
    std::array<std::uint8_t, 8> face{0, 1, 2, 3, 4, 5, 6, 7};
    for (auto _ : state) {
        // Clockwise quarter turn: every sticker moves two places around the ring.
        const std::uint8_t a = face[6];
        const std::uint8_t b = face[7];
        for (std::size_t i = 7; i >= 2; --i) {
            face[i] = face[i - 2];
        }
        face[0] = a;
        face[1] = b;
        benchmark::DoNotOptimize(face);
    }
}
BENCHMARK(BM_FaceTurnByteArray);

void BM_FaceTurnBitboard(benchmark::State& state) {
    std::uint64_t face = 0x0706050403020100ULL;
    for (auto _ : state) {
        face = std::rotl(face, 16);
        benchmark::DoNotOptimize(face);
    }
}
BENCHMARK(BM_FaceTurnBitboard);

}  // namespace
