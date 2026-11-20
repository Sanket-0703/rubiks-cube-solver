#include "cubecore/model/facelets.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>

#include "cubecore/model/face.hpp"

namespace cubecore {

namespace {

constexpr std::size_t kStickersPerFace = RubiksCube::kSize * RubiksCube::kSize;

}  // namespace

std::string toFaceletString(const RubiksCube& cube) {
    std::string facelets;
    facelets.reserve(kStickerCount);
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < RubiksCube::kSize; ++r) {
            for (std::size_t c = 0; c < RubiksCube::kSize; ++c) {
                facelets += toChar(homeFace(cube.getColor(face, r, c)));
            }
        }
    }
    return facelets;
}

void loadFaceletString(RubiksCube& cube, std::string_view facelets) {
    if (facelets.size() != kStickerCount) {
        throw std::invalid_argument("a facelet string has 54 characters, got " +
                                    std::to_string(facelets.size()));
    }

    // Parse and check everything before painting anything, so a bad string leaves the cube
    // unchanged.
    std::array<Face, kStickerCount> parsed{};
    std::array<std::size_t, kFaceCount> counts{};
    for (std::size_t i = 0; i < kStickerCount; ++i) {
        const std::optional<Face> face = faceFromChar(facelets[i]);
        if (!face) {
            throw std::invalid_argument("facelet " + std::to_string(i) + " is '" +
                                        std::string{facelets[i]} + "'; expected one of URFDLB");
        }
        parsed[i] = *face;
        ++counts[toIndex(*face)];
    }
    for (const Face face : kAllFaces) {
        if (counts[toIndex(face)] != kStickersPerFace) {
            throw std::invalid_argument("expected 9 '" + std::string{toChar(face)} +
                                        "' facelets, got " + std::to_string(counts[toIndex(face)]));
        }
    }

    std::size_t i = 0;
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < RubiksCube::kSize; ++r) {
            for (std::size_t c = 0; c < RubiksCube::kSize; ++c) {
                cube.setColor(face, r, c, solvedColor(parsed[i++]));
            }
        }
    }
}

}  // namespace cubecore
