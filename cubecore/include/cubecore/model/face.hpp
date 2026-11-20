#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace cubecore {

/// The six faces, in the URFDLB order used by Kociemba-style facelet strings. Opposite faces are
/// three apart (U–D, R–L, F–B), so opposite(f) is just (f + 3) mod 6.
enum class Face : std::uint8_t { U, R, F, D, L, B };

inline constexpr std::size_t kFaceCount = 6;
inline constexpr std::array<Face, kFaceCount> kAllFaces{Face::U, Face::R, Face::F,
                                                        Face::D, Face::L, Face::B};

[[nodiscard]] constexpr std::size_t toIndex(Face face) noexcept {
    return static_cast<std::size_t>(face);
}

[[nodiscard]] constexpr Face opposite(Face face) noexcept {
    return static_cast<Face>((toIndex(face) + 3) % kFaceCount);
}

/// 'U', 'R', 'F', 'D', 'L' or 'B'.
[[nodiscard]] constexpr char toChar(Face face) noexcept {
    return std::string_view{"URFDLB"}[toIndex(face)];
}

/// The face for 'U', 'R', 'F', 'D', 'L' or 'B'; nothing for any other character.
[[nodiscard]] constexpr std::optional<Face> faceFromChar(char c) noexcept {
    const std::size_t i = std::string_view{"URFDLB"}.find(c);
    if (i == std::string_view::npos) {
        return std::nullopt;
    }
    return static_cast<Face>(i);
}

/// Sticker colours in the Western scheme (white on top, green in front), listed in face order so
/// a solved face f has colour Color(f).
enum class Color : std::uint8_t { White, Red, Green, Yellow, Orange, Blue };

/// The colour of `face` on a solved cube.
[[nodiscard]] constexpr Color solvedColor(Face face) noexcept {
    return static_cast<Color>(toIndex(face));
}

/// The face whose solved colour is `color`.
[[nodiscard]] constexpr Face homeFace(Color color) noexcept {
    return static_cast<Face>(static_cast<std::size_t>(color));
}

/// 'W', 'R', 'G', 'Y', 'O' or 'B'.
[[nodiscard]] constexpr char toChar(Color color) noexcept {
    return std::string_view{"WRGYOB"}[static_cast<std::size_t>(color)];
}

}  // namespace cubecore
