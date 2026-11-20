#include "cubecore/model/rubiks_cube_3d_array.hpp"

#include <cassert>
#include <cstddef>

namespace cubecore {

RubiksCube3dArray::RubiksCube3dArray() {
    for (const Face face : kAllFaces) {
        for (auto& row : grid(face)) {
            row.fill(solvedColor(face));
        }
    }
}

Color RubiksCube3dArray::getColor(Face face, std::size_t row, std::size_t col) const {
    assert(row < kSize && col < kSize);
    return faces_[toIndex(face)][row][col];
}

void RubiksCube3dArray::setColor(Face face, std::size_t row, std::size_t col, Color color) {
    assert(row < kSize && col < kSize);
    grid(face)[row][col] = color;
}

bool RubiksCube3dArray::isSolved() const {
    for (const FaceGrid& g : faces_) {
        const Color center = g[1][1];
        for (const auto& row : g) {
            for (const Color c : row) {
                if (c != center) {
                    return false;
                }
            }
        }
    }
    return true;
}

void RubiksCube3dArray::apply(Move move) {
    // The reference model keeps it simple: U2 is two U turns and U' is three.
    for (int i = 0; i < quarterTurns(move); ++i) {
        switch (face(move)) {
            case Face::U:
                turnU();
                break;
            case Face::R:
                turnR();
                break;
            case Face::F:
                turnF();
                break;
            case Face::D:
                turnD();
                break;
            case Face::L:
                turnL();
                break;
            case Face::B:
                turnB();
                break;
        }
    }
}

void RubiksCube3dArray::rotateFaceClockwise(Face face) {
    FaceGrid& g = grid(face);
    const FaceGrid old = g;
    // Clockwise: the left column becomes the top row, read bottom to top.
    for (std::size_t r = 0; r < kSize; ++r) {
        for (std::size_t c = 0; c < kSize; ++c) {
            g[r][c] = old[kSize - 1 - c][r];
        }
    }
}

// In each turn below, `i` walks along the moving strip of three stickers. Every line copies one
// strip into the next, going backwards around the ring so that only one strip needs a temporary.
// Where a strip is read in reverse (2 - i), that's because the two faces' grids are oriented
// differently in the net.

// U: the top rows of F, L, B and R. F's top row moves to L.
void RubiksCube3dArray::turnU() {
    rotateFaceClockwise(Face::U);
    FaceGrid& f = grid(Face::F);
    FaceGrid& r = grid(Face::R);
    FaceGrid& b = grid(Face::B);
    FaceGrid& l = grid(Face::L);
    const auto front = f[0];
    f[0] = r[0];
    r[0] = b[0];
    b[0] = l[0];
    l[0] = front;
}

// D: the bottom rows of F, R, B and L. F's bottom row moves to R.
void RubiksCube3dArray::turnD() {
    rotateFaceClockwise(Face::D);
    FaceGrid& f = grid(Face::F);
    FaceGrid& r = grid(Face::R);
    FaceGrid& b = grid(Face::B);
    FaceGrid& l = grid(Face::L);
    const auto front = f[2];
    f[2] = l[2];
    l[2] = b[2];
    b[2] = r[2];
    r[2] = front;
}

// R: the right columns of F, U and D, and the left column of B. F moves up to U, U to B, B to D.
void RubiksCube3dArray::turnR() {
    rotateFaceClockwise(Face::R);
    FaceGrid& u = grid(Face::U);
    FaceGrid& f = grid(Face::F);
    FaceGrid& d = grid(Face::D);
    FaceGrid& b = grid(Face::B);
    for (std::size_t i = 0; i < kSize; ++i) {
        const Color up = u[i][2];
        u[i][2] = f[i][2];
        f[i][2] = d[i][2];
        d[i][2] = b[2 - i][0];
        b[2 - i][0] = up;
    }
}

// L: the left columns of U, F and D, and the right column of B. U moves down to F, F to D, D to B.
void RubiksCube3dArray::turnL() {
    rotateFaceClockwise(Face::L);
    FaceGrid& u = grid(Face::U);
    FaceGrid& f = grid(Face::F);
    FaceGrid& d = grid(Face::D);
    FaceGrid& b = grid(Face::B);
    for (std::size_t i = 0; i < kSize; ++i) {
        const Color up = u[i][0];
        u[i][0] = b[2 - i][2];
        b[2 - i][2] = d[i][0];
        d[i][0] = f[i][0];
        f[i][0] = up;
    }
}

// F: U's bottom row, R's left column, D's top row, L's right column. U moves to R, R to D, D to L.
void RubiksCube3dArray::turnF() {
    rotateFaceClockwise(Face::F);
    FaceGrid& u = grid(Face::U);
    FaceGrid& r = grid(Face::R);
    FaceGrid& d = grid(Face::D);
    FaceGrid& l = grid(Face::L);
    for (std::size_t i = 0; i < kSize; ++i) {
        const Color up = u[2][i];
        u[2][i] = l[2 - i][2];
        l[2 - i][2] = d[0][2 - i];
        d[0][2 - i] = r[i][0];
        r[i][0] = up;
    }
}

// B: U's top row, L's left column, D's bottom row, R's right column. U moves to L, L to D, D to R.
void RubiksCube3dArray::turnB() {
    rotateFaceClockwise(Face::B);
    FaceGrid& u = grid(Face::U);
    FaceGrid& r = grid(Face::R);
    FaceGrid& d = grid(Face::D);
    FaceGrid& l = grid(Face::L);
    for (std::size_t i = 0; i < kSize; ++i) {
        const Color up = u[0][i];
        u[0][i] = r[i][2];
        r[i][2] = d[2][2 - i];
        d[2][2 - i] = l[2 - i][0];
        l[2 - i][0] = up;
    }
}

}  // namespace cubecore
