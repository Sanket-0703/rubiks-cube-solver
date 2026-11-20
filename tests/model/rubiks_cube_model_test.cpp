// Tests every cube model against the same rules. Level 3 adds its models to `CubeModels`, and
// every test here then runs against them too.

#include <array>
#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../support/sticker_geometry.hpp"
#include "cubecore/model/face.hpp"
#include "cubecore/model/facelets.hpp"
#include "cubecore/model/move.hpp"
#include "cubecore/model/rubiks_cube_3d_array.hpp"
#include "cubecore/notation/notation.hpp"
#include "cubecore/notation/scramble.hpp"

namespace {

using cubecore::Color;
using cubecore::Face;
using cubecore::kAllFaces;
using cubecore::kAllMoves;
using cubecore::Move;
using cubecore::parseMoves;
using cubecore::RubiksCube;

constexpr std::size_t kN = RubiksCube::kSize;

cubetest::Stickers stickersOf(const RubiksCube& cube) {
    cubetest::Stickers s{};
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < kN; ++r) {
            for (std::size_t c = 0; c < kN; ++c) {
                s[cubecore::toIndex(face) * 9 + r * 3 + c] = cube.getColor(face, r, c);
            }
        }
    }
    return s;
}

/// The three stickers of `face` in a row (row >= 0) or a column (col >= 0).
std::array<Color, 3> rowOf(const RubiksCube& cube, Face face, std::size_t row) {
    return {cube.getColor(face, row, 0), cube.getColor(face, row, 1), cube.getColor(face, row, 2)};
}
std::array<Color, 3> colOf(const RubiksCube& cube, Face face, std::size_t col) {
    return {cube.getColor(face, 0, col), cube.getColor(face, 1, col), cube.getColor(face, 2, col)};
}
std::array<Color, 3> all(Color c) {
    return {c, c, c};
}

/// How many times `sequence` must be repeated to return a solved cube to solved.
template <typename Cube>
int orderOf(const std::vector<Move>& sequence) {
    Cube cube;
    for (int n = 1; n <= 2000; ++n) {
        cube.applyMoves(sequence);
        if (cube.isSolved()) {
            return n;
        }
    }
    return -1;
}

template <typename Cube>
class CubeModelTest : public ::testing::Test {};

using CubeModels = ::testing::Types<cubecore::RubiksCube3dArray>;
TYPED_TEST_SUITE(CubeModelTest, CubeModels);

// ---------------------------------------------------------------- solved state

TYPED_TEST(CubeModelTest, StartsSolvedWithStandardColours) {
    const TypeParam cube;
    EXPECT_TRUE(cube.isSolved());
    EXPECT_EQ(cube.centerColor(Face::U), Color::White);
    EXPECT_EQ(cube.centerColor(Face::F), Color::Green);
    EXPECT_EQ(cube.centerColor(Face::R), Color::Red);
    EXPECT_EQ(cube.centerColor(Face::D), Color::Yellow);
    EXPECT_EQ(cube.centerColor(Face::L), Color::Orange);
    EXPECT_EQ(cube.centerColor(Face::B), Color::Blue);
}

TYPED_TEST(CubeModelTest, EveryMoveUnsolves) {
    for (const Move move : kAllMoves) {
        TypeParam cube;
        cube.apply(move);
        EXPECT_FALSE(cube.isSolved()) << cubecore::toString(move);
    }
}

// ---------------------------------------------------------------- algebra of moves

TYPED_TEST(CubeModelTest, FourQuarterTurnsAreIdentity) {
    for (const Face face : kAllFaces) {
        TypeParam cube;
        for (int i = 0; i < 4; ++i) {
            cube.apply(cubecore::makeMove(face, 1));
        }
        EXPECT_TRUE(cube.isSolved()) << cubecore::toChar(face);
    }
}

TYPED_TEST(CubeModelTest, MoveThenInverseIsIdentityFromAnyState) {
    TypeParam scrambled;
    scrambled.applyMoves(cubecore::randomScramble(30, 7));
    for (const Move move : kAllMoves) {
        TypeParam cube = scrambled;
        cube.apply(move);
        cube.apply(cubecore::inverse(move));
        EXPECT_EQ(cube, scrambled) << cubecore::toString(move);
    }
}

TYPED_TEST(CubeModelTest, HalfAndPrimeTurnsMatchRepeatedQuarterTurns) {
    TypeParam start;
    start.applyMoves(cubecore::randomScramble(25, 11));
    for (const Face face : kAllFaces) {
        const Move quarter = cubecore::makeMove(face, 1);
        TypeParam half = start;
        TypeParam twice = start;
        half.apply(cubecore::makeMove(face, 2));
        twice.apply(quarter);
        twice.apply(quarter);
        EXPECT_EQ(half, twice) << cubecore::toChar(face) << "2";

        TypeParam prime = start;
        TypeParam thrice = twice;
        prime.apply(cubecore::makeMove(face, 3));
        thrice.apply(quarter);
        EXPECT_EQ(prime, thrice) << cubecore::toChar(face) << "'";
    }
}

TYPED_TEST(CubeModelTest, ScrambleThenInverseSolves) {
    for (std::uint64_t seed = 0; seed < 20; ++seed) {
        const auto scramble = cubecore::randomScramble(40, seed);
        TypeParam cube;
        cube.applyMoves(scramble);
        cube.applyMoves(cubecore::invertMoves(scramble));
        EXPECT_TRUE(cube.isSolved()) << "seed " << seed;
    }
}

TYPED_TEST(CubeModelTest, OppositeFacesCommute) {
    // U and D never touch the same stickers, so their order doesn't matter (same for R/L, F/B).
    TypeParam start;
    start.applyMoves(cubecore::randomScramble(25, 3));
    for (const Move a : kAllMoves) {
        for (const Move b : kAllMoves) {
            if (cubecore::face(b) != cubecore::opposite(cubecore::face(a))) {
                continue;
            }
            TypeParam ab = start;
            TypeParam ba = start;
            ab.apply(a);
            ab.apply(b);
            ba.apply(b);
            ba.apply(a);
            EXPECT_EQ(ab, ba) << cubecore::toString(a) << ' ' << cubecore::toString(b);
        }
    }
}

// ---------------------------------------------------------------- known group facts
// These are properties of the real cube. A model that moved stickers to the wrong place, or
// twisted them, would get them wrong.

TYPED_TEST(CubeModelTest, KnownSequenceOrders) {
    EXPECT_EQ(orderOf<TypeParam>(parseMoves("R U R' U'")), 6);  // "sexy move"
    EXPECT_EQ(orderOf<TypeParam>(parseMoves("R U")), 105);
    EXPECT_EQ(orderOf<TypeParam>(parseMoves("R U'")), 63);
    EXPECT_EQ(orderOf<TypeParam>(parseMoves("R2 U2")), 6);
    EXPECT_EQ(orderOf<TypeParam>(parseMoves("F B'")), 4);
}

TYPED_TEST(CubeModelTest, CentresNeverMove) {
    TypeParam cube;
    cube.applyMoves(cubecore::randomScramble(100, 5));
    for (const Face face : kAllFaces) {
        EXPECT_EQ(cube.centerColor(face), cubecore::solvedColor(face));
    }
}

TYPED_TEST(CubeModelTest, EveryColourStaysOnNineStickers) {
    TypeParam cube;
    cube.applyMoves(cubecore::randomScramble(100, 6));
    std::array<int, 6> counts{};
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < kN; ++r) {
            for (std::size_t c = 0; c < kN; ++c) {
                ++counts[static_cast<std::size_t>(cube.getColor(face, r, c))];
            }
        }
    }
    for (const int count : counts) {
        EXPECT_EQ(count, 9);
    }
}

TYPED_TEST(CubeModelTest, CheckerboardPattern) {
    // R2 L2 U2 D2 F2 B2: on every face the centre and corners keep their colour and the four
    // edges show the opposite face's colour.
    TypeParam cube;
    cube.applyMoves(parseMoves("R2 L2 U2 D2 F2 B2"));
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < kN; ++r) {
            for (std::size_t c = 0; c < kN; ++c) {
                const bool edge = (r + c) % 2 == 1;
                const Face expected = edge ? cubecore::opposite(face) : face;
                EXPECT_EQ(cube.getColor(face, r, c), cubecore::solvedColor(expected))
                    << cubecore::toChar(face) << r << c;
            }
        }
    }
}

TYPED_TEST(CubeModelTest, Superflip) {
    // A 20-move superflip: every corner solved, every edge in place but flipped.
    TypeParam cube;
    cube.applyMoves(parseMoves("U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2"));
    for (const Face face : kAllFaces) {
        for (std::size_t r = 0; r < kN; ++r) {
            for (std::size_t c = 0; c < kN; ++c) {
                const bool edge = (r + c) % 2 == 1;
                const Color color = cube.getColor(face, r, c);
                if (edge) {
                    EXPECT_NE(color, cubecore::solvedColor(face))
                        << cubecore::toChar(face) << r << c;
                } else {
                    EXPECT_EQ(color, cubecore::solvedColor(face))
                        << cubecore::toChar(face) << r << c;
                }
            }
        }
    }
}

// ---------------------------------------------------------------- each face turn, by hand
// What a real cube (white top, green front) shows after one turn from solved.

TYPED_TEST(CubeModelTest, UMovesFrontRowToLeft) {
    TypeParam cube;
    cube.apply(Move::U);
    EXPECT_EQ(rowOf(cube, Face::F, 0), all(Color::Red));
    EXPECT_EQ(rowOf(cube, Face::L, 0), all(Color::Green));
    EXPECT_EQ(rowOf(cube, Face::B, 0), all(Color::Orange));
    EXPECT_EQ(rowOf(cube, Face::R, 0), all(Color::Blue));
    EXPECT_EQ(rowOf(cube, Face::F, 1), all(Color::Green));  // middle layer untouched
}

TYPED_TEST(CubeModelTest, DMovesFrontRowToRight) {
    TypeParam cube;
    cube.apply(Move::D);
    EXPECT_EQ(rowOf(cube, Face::F, 2), all(Color::Orange));
    EXPECT_EQ(rowOf(cube, Face::R, 2), all(Color::Green));
    EXPECT_EQ(rowOf(cube, Face::B, 2), all(Color::Red));
    EXPECT_EQ(rowOf(cube, Face::L, 2), all(Color::Blue));
}

TYPED_TEST(CubeModelTest, RMovesFrontColumnUp) {
    TypeParam cube;
    cube.apply(Move::R);
    EXPECT_EQ(colOf(cube, Face::U, 2), all(Color::Green));
    EXPECT_EQ(colOf(cube, Face::F, 2), all(Color::Yellow));
    EXPECT_EQ(colOf(cube, Face::D, 2), all(Color::Blue));
    EXPECT_EQ(colOf(cube, Face::B, 0), all(Color::White));
}

TYPED_TEST(CubeModelTest, LMovesFrontColumnDown) {
    TypeParam cube;
    cube.apply(Move::L);
    EXPECT_EQ(colOf(cube, Face::U, 0), all(Color::Blue));
    EXPECT_EQ(colOf(cube, Face::F, 0), all(Color::White));
    EXPECT_EQ(colOf(cube, Face::D, 0), all(Color::Green));
    EXPECT_EQ(colOf(cube, Face::B, 2), all(Color::Yellow));
}

TYPED_TEST(CubeModelTest, FMovesTopRowToRight) {
    TypeParam cube;
    cube.apply(Move::F);
    EXPECT_EQ(rowOf(cube, Face::U, 2), all(Color::Orange));
    EXPECT_EQ(colOf(cube, Face::R, 0), all(Color::White));
    EXPECT_EQ(rowOf(cube, Face::D, 0), all(Color::Red));
    EXPECT_EQ(colOf(cube, Face::L, 2), all(Color::Yellow));
}

TYPED_TEST(CubeModelTest, BMovesTopRowToLeft) {
    TypeParam cube;
    cube.apply(Move::B);
    EXPECT_EQ(rowOf(cube, Face::U, 0), all(Color::Red));
    EXPECT_EQ(colOf(cube, Face::L, 0), all(Color::White));
    EXPECT_EQ(rowOf(cube, Face::D, 2), all(Color::Orange));
    EXPECT_EQ(colOf(cube, Face::R, 2), all(Color::Yellow));
}

TYPED_TEST(CubeModelTest, FaceletSnapshotAfterSexyMove) {
    // Turning from solved can't show whether a strip arrives reversed (it's one colour); the
    // geometry tests below can. This snapshot pins the exact result of "R U R' U'" so any change
    // shows up as a readable diff. The Level 2 check compares the same state with a real cube.
    TypeParam cube;
    cube.applyMoves(parseMoves("R U R' U'"));
    EXPECT_EQ(cubecore::toFaceletString(cube),
              "UULUUFUUFRRUBRRURRFFDFFUFFFDDRDDDDDDBLLLLLLLLBRRBBBBBB");
}

// ---------------------------------------------------------------- against 3D geometry

TYPED_TEST(CubeModelTest, EveryMoveMatchesGeometryFromSolved) {
    for (const Move move : kAllMoves) {
        TypeParam cube;
        cube.apply(move);
        cubetest::Stickers expected = cubetest::solvedStickers();
        cubetest::applyGeometric(expected, move);
        EXPECT_EQ(stickersOf(cube), expected) << cubecore::toString(move);
    }
}

TYPED_TEST(CubeModelTest, RandomSequencesMatchGeometry) {
    for (std::uint64_t seed = 0; seed < 200; ++seed) {
        TypeParam cube;
        cubetest::Stickers expected = cubetest::solvedStickers();
        for (const Move move : cubecore::randomScramble(25, seed)) {
            cube.apply(move);
            cubetest::applyGeometric(expected, move);
        }
        ASSERT_EQ(stickersOf(cube), expected) << "seed " << seed;
    }
}

// ---------------------------------------------------------------- copying and comparing

TYPED_TEST(CubeModelTest, CopiesAreIndependent) {
    const TypeParam a;
    TypeParam b = a;
    b.apply(Move::R);
    EXPECT_TRUE(a.isSolved());
    EXPECT_FALSE(a == b);
}

TYPED_TEST(CubeModelTest, SameStickersWorksThroughTheInterface) {
    TypeParam a;
    TypeParam b;
    a.applyMoves(parseMoves("F2 D' L"));
    b.applyMoves(parseMoves("F2 D' L"));
    const RubiksCube& ra = a;
    const RubiksCube& rb = b;
    EXPECT_TRUE(cubecore::sameStickers(ra, rb));
    b.apply(Move::U);
    EXPECT_FALSE(cubecore::sameStickers(ra, rb));
}

TYPED_TEST(CubeModelTest, BaseClassIsSolvedAgreesWithModel) {
    // RubiksCube::isSolved (the generic version through getColor) and the model's own override
    // must agree.
    TypeParam cube;
    EXPECT_TRUE(cube.RubiksCube::isSolved());
    cube.apply(Move::F);
    EXPECT_FALSE(cube.RubiksCube::isSolved());
    EXPECT_FALSE(cube.isSolved());
}

}  // namespace
