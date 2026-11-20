#pragma once

#include <cstddef>
#include <span>

#include "cubecore/model/face.hpp"
#include "cubecore/model/move.hpp"

namespace cubecore {

/// The interface every cube model implements, whatever its storage (3D array, 1D array, bitboard).
///
/// Sticker coordinates: each face is a 3x3 grid seen from outside the cube, row 0 at the top and
/// column 0 on the left, oriented as in the unfolded net
///
///            U                   U: B side at the top, F side at the bottom
///         L  F  R  B             L, F, R, B: U side at the top
///            D                   D: F side at the top, B side at the bottom
///
/// This is the same layout Kociemba-style facelet strings use.
///
/// Code that needs speed (the solvers) is written as templates over a concrete model, so moves
/// are called directly and inlined. This virtual interface is for everything else: printing,
/// facelet strings, tests that compare models, and the CLI.
class RubiksCube {
public:
    static constexpr std::size_t kSize = 3;  ///< Stickers per row and per column.

    virtual ~RubiksCube() = default;

    /// Colour of the sticker at (row, col) on `face`. row and col are 0..2.
    [[nodiscard]] virtual Color getColor(Face face, std::size_t row, std::size_t col) const = 0;

    /// Paints one sticker. Used to load a cube from a facelet string; moves never need it.
    virtual void setColor(Face face, std::size_t row, std::size_t col, Color color) = 0;

    /// Applies one face turn.
    virtual void apply(Move move) = 0;

    /// Applies moves in order.
    void applyMoves(std::span<const Move> moves) {
        for (const Move move : moves) {
            apply(move);
        }
    }

    /// True when every face is a single colour. Models may override this with a faster check.
    [[nodiscard]] virtual bool isSolved() const;

    /// The centre sticker of `face`. Face turns never move centres.
    [[nodiscard]] Color centerColor(Face face) const { return getColor(face, 1, 1); }

protected:
    // Copying or moving through the base would slice off the derived model's stickers, so these
    // are only for derived classes to use. Concrete models are freely copyable.
    RubiksCube() = default;
    RubiksCube(const RubiksCube&) = default;
    RubiksCube(RubiksCube&&) = default;
    RubiksCube& operator=(const RubiksCube&) = default;
    RubiksCube& operator=(RubiksCube&&) = default;
};

/// True when both cubes show the same colour on every sticker, even if they are different models.
[[nodiscard]] bool sameStickers(const RubiksCube& a, const RubiksCube& b);

}  // namespace cubecore
