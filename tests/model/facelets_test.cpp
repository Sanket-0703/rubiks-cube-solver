#include "cubecore/model/facelets.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

#include "cubecore/model/rubiks_cube_3d_array.hpp"
#include "cubecore/notation/scramble.hpp"

namespace {

using cubecore::loadFaceletString;
using cubecore::RubiksCube3dArray;
using cubecore::toFaceletString;

constexpr const char* kSolved = "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB";

TEST(Facelets, SolvedCube) {
    EXPECT_EQ(toFaceletString(RubiksCube3dArray{}), kSolved);
}

TEST(Facelets, RoundTripsAScrambledCube) {
    RubiksCube3dArray original;
    original.applyMoves(cubecore::randomScramble(30, 99));

    RubiksCube3dArray loaded;
    loadFaceletString(loaded, toFaceletString(original));
    EXPECT_EQ(loaded, original);
}

TEST(Facelets, KnownStateAfterU) {
    // U moves F's top row to L, so L shows F (green) on top, etc.
    RubiksCube3dArray cube;
    cube.apply(cubecore::Move::U);
    EXPECT_EQ(toFaceletString(cube),
              "UUUUUUUUU"    // U
              "BBBRRRRRR"    // R: top row from B
              "RRRFFFFFF"    // F: top row from R
              "DDDDDDDDD"    // D
              "FFFLLLLLL"    // L: top row from F
              "LLLBBBBBB");  // B: top row from L
}

TEST(Facelets, RejectsWrongLength) {
    RubiksCube3dArray cube;
    EXPECT_THROW(loadFaceletString(cube, "UUU"), std::invalid_argument);
}

TEST(Facelets, RejectsUnknownLetter) {
    std::string bad = kSolved;
    bad[10] = 'X';
    RubiksCube3dArray cube;
    EXPECT_THROW(loadFaceletString(cube, bad), std::invalid_argument);
}

TEST(Facelets, RejectsWrongColourCountsAndLeavesCubeUnchanged) {
    std::string bad = kSolved;
    bad[0] = 'R';  // ten R, eight U
    RubiksCube3dArray cube;
    try {
        loadFaceletString(cube, bad);
        FAIL() << "expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        EXPECT_NE(std::string{e.what()}.find("expected 9 'U'"), std::string::npos) << e.what();
    }
    EXPECT_TRUE(cube.isSolved());
}

}  // namespace
