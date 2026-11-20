#pragma once

#include <array>
#include <cstddef>

#include "cubecore/model/face.hpp"
#include "cubecore/model/move.hpp"
#include "cubecore/model/rubiks_cube.hpp"

namespace cubecore {

/// The reference model: stickers stored exactly as you picture them, `faces_[face][row][col]`,
/// with each of the six clockwise face turns written out by hand. It is the easiest model to
/// read and check against a real cube. The faster models (Level 3) are tested against it.
///
/// Size: 6 x 3 x 3 = 54 bytes (one byte per sticker, centres included).
class RubiksCube3dArray final : public RubiksCube {
public:
    using FaceGrid = std::array<std::array<Color, kSize>, kSize>;

    /// A solved cube.
    RubiksCube3dArray();

    [[nodiscard]] Color getColor(Face face, std::size_t row, std::size_t col) const override;
    void setColor(Face face, std::size_t row, std::size_t col, Color color) override;
    void apply(Move move) override;
    [[nodiscard]] bool isSolved() const override;

    friend bool operator==(const RubiksCube3dArray& a, const RubiksCube3dArray& b) {
        return a.faces_ == b.faces_;
    }

private:
    [[nodiscard]] FaceGrid& grid(Face face) { return faces_[toIndex(face)]; }

    /// Turns the 3x3 grid of `face` a quarter turn clockwise (its own stickers only).
    void rotateFaceClockwise(Face face);

    // One clockwise quarter turn of each face: the face's own grid, plus the ring of 12 stickers
    // on the four neighbouring faces.
    void turnU();
    void turnR();
    void turnF();
    void turnD();
    void turnL();
    void turnB();

    std::array<FaceGrid, kFaceCount> faces_{};
};

}  // namespace cubecore
