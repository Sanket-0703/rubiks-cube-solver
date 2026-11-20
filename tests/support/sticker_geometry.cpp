#include "sticker_geometry.hpp"

#include <cstddef>
#include <stdexcept>

namespace cubetest {

namespace {

using cubecore::Face;

struct Vec {
    int x = 0;
    int y = 0;
    int z = 0;
};

int dot(Vec a, Vec b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec cross(Vec a, Vec b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec normalOf(Face face) {
    switch (face) {
        case Face::U:
            return {0, 1, 0};
        case Face::D:
            return {0, -1, 0};
        case Face::R:
            return {1, 0, 0};
        case Face::L:
            return {-1, 0, 0};
        case Face::F:
            return {0, 0, 1};
        case Face::B:
            return {0, 0, -1};
    }
    throw std::logic_error("bad face");
}

Face faceOfNormal(Vec n) {
    for (const Face face : cubecore::kAllFaces) {
        const Vec m = normalOf(face);
        if (m.x == n.x && m.y == n.y && m.z == n.z) {
            return face;
        }
    }
    throw std::logic_error("not a face normal");
}

/// Where sticker (face, row, col) sits in 3D. Each face's grid is seen from outside, as in the
/// net (see rubiks_cube.hpp), which fixes these formulas.
Vec positionOf(Face face, int row, int col) {
    switch (face) {
        case Face::U:
            return {col - 1, 1, row - 1};  // back row at the top
        case Face::D:
            return {col - 1, -1, 1 - row};  // front row at the top
        case Face::F:
            return {col - 1, 1 - row, 1};
        case Face::B:
            return {1 - col, 1 - row, -1};  // seen from behind: R side on the left
        case Face::R:
            return {1, 1 - row, 1 - col};  // F side on the left
        case Face::L:
            return {-1, 1 - row, col - 1};  // B side on the left
    }
    throw std::logic_error("bad face");
}

/// Inverse of positionOf: the (row, col) of a 3D position on `face`.
std::size_t stickerIndex(Face face, Vec p) {
    int row = 0;
    int col = 0;
    switch (face) {
        case Face::U:
            row = p.z + 1;
            col = p.x + 1;
            break;
        case Face::D:
            row = 1 - p.z;
            col = p.x + 1;
            break;
        case Face::F:
            row = 1 - p.y;
            col = p.x + 1;
            break;
        case Face::B:
            row = 1 - p.y;
            col = 1 - p.x;
            break;
        case Face::R:
            row = 1 - p.y;
            col = 1 - p.z;
            break;
        case Face::L:
            row = 1 - p.y;
            col = p.z + 1;
            break;
    }
    return cubecore::toIndex(face) * 9 + static_cast<std::size_t>(row * 3 + col);
}

/// Rotates v by -90 degrees about unit axis n: clockwise when seen from the tip of n, which is
/// how a face turn looks to someone facing that face. Rodrigues' formula with cos = 0, sin = -1.
Vec rotateClockwise(Vec v, Vec n) {
    const Vec c = cross(n, v);
    const int d = dot(n, v);
    return {n.x * d - c.x, n.y * d - c.y, n.z * d - c.z};
}

}  // namespace

Stickers solvedStickers() {
    Stickers s{};
    for (std::size_t i = 0; i < s.size(); ++i) {
        s[i] = cubecore::solvedColor(static_cast<Face>(i / 9));
    }
    return s;
}

void applyGeometric(Stickers& stickers, cubecore::Move move) {
    const Vec axis = normalOf(cubecore::face(move));
    for (int turn = 0; turn < cubecore::quarterTurns(move); ++turn) {
        const Stickers old = stickers;
        for (const Face face : cubecore::kAllFaces) {
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 3; ++col) {
                    const Vec position = positionOf(face, row, col);
                    if (dot(position, axis) != 1) {
                        continue;  // not in the turning layer
                    }
                    const Vec newPosition = rotateClockwise(position, axis);
                    const Face newFace = faceOfNormal(rotateClockwise(normalOf(face), axis));
                    stickers[stickerIndex(newFace, newPosition)] =
                        old[stickerIndex(face, position)];
                }
            }
        }
    }
}

}  // namespace cubetest
