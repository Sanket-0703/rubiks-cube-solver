#include "cubecore/model/rubiks_cube.hpp"

#include <cstddef>

#include "cubecore/model/face.hpp"

namespace cubecore {

bool RubiksCube::isSolved() const {
    for (const Face face : kAllFaces) {
        const Color center = centerColor(face);
        for (std::size_t r = 0; r < kSize; ++r) {
            for (std::size_t c = 0; c < kSize; ++c) {
                if (getColor(face, r, c) != center) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool sameStickers(const RubiksCube& a, const RubiksCube& b) {
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < RubiksCube::kSize; ++r) {
            for (std::size_t c = 0; c < RubiksCube::kSize; ++c) {
                if (a.getColor(face, r, c) != b.getColor(face, r, c)) {
                    return false;
                }
            }
        }
    }
    return true;
}

}  // namespace cubecore
