#include "cubecore/render/net.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "cubecore/model/face.hpp"

namespace cubecore {

namespace {

constexpr std::size_t kN = RubiksCube::kSize;

/// 256-colour palette indices, in Color order: white, red, green, yellow, orange, blue.
constexpr std::array<int, kFaceCount> kAnsi256{255, 160, 28, 226, 208, 21};

/// One sticker: "W " in plain style, a coloured two-character square in ANSI style.
std::string sticker(Color color, NetStyle style) {
    if (style == NetStyle::Plain) {
        return std::string{toChar(color)} + ' ';
    }
    return "\x1b[48;5;" + std::to_string(kAnsi256[static_cast<std::size_t>(color)]) + "m  \x1b[0m";
}

/// The blank space a missing face takes up in the cross: three stickers, two columns each (in
/// both styles).
constexpr std::string_view kBlankFace = "      ";

void rtrimInto(std::string& out, std::string line) {
    while (!line.empty() && line.back() == ' ') {
        line.pop_back();
    }
    out += line;
    out += '\n';
}

/// Rows of the cross: which faces appear in each band, left to right (nullopt = blank).
using Band = std::array<std::optional<Face>, 4>;
constexpr std::array<Band, 3> kBands{{
    {std::nullopt, Face::U, std::nullopt, std::nullopt},
    {Face::L, Face::F, Face::R, Face::B},
    {std::nullopt, Face::D, std::nullopt, std::nullopt},
}};

}  // namespace

std::string renderNet(const RubiksCube& cube, NetStyle style) {
    std::string out;
    for (const Band& band : kBands) {
        for (std::size_t r = 0; r < kN; ++r) {
            std::string line;
            for (std::size_t slot = 0; slot < band.size(); ++slot) {
                if (slot > 0) {
                    line += ' ';  // one space between faces
                }
                const std::optional<Face> face = band[slot];
                if (!face) {
                    line += kBlankFace;
                    continue;
                }
                for (std::size_t c = 0; c < kN; ++c) {
                    line += sticker(cube.getColor(*face, r, c), style);
                }
            }
            rtrimInto(out, std::move(line));
        }
    }
    return out;
}

}  // namespace cubecore
