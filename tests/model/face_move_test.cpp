#include <gtest/gtest.h>
#include <set>
#include <string>
#include <string_view>

#include "cubecore/model/face.hpp"
#include "cubecore/model/move.hpp"

namespace {

using cubecore::Color;
using cubecore::Face;
using cubecore::kAllFaces;
using cubecore::kAllMoves;
using cubecore::Move;

TEST(Face, OppositePairs) {
    EXPECT_EQ(cubecore::opposite(Face::U), Face::D);
    EXPECT_EQ(cubecore::opposite(Face::R), Face::L);
    EXPECT_EQ(cubecore::opposite(Face::F), Face::B);
    for (const Face face : kAllFaces) {
        EXPECT_EQ(cubecore::opposite(cubecore::opposite(face)), face);
        EXPECT_NE(cubecore::opposite(face), face);
    }
}

TEST(Face, CharRoundTrip) {
    std::string letters;
    for (const Face face : kAllFaces) {
        letters += cubecore::toChar(face);
        EXPECT_EQ(cubecore::faceFromChar(cubecore::toChar(face)), face);
    }
    EXPECT_EQ(letters, "URFDLB");
    EXPECT_FALSE(cubecore::faceFromChar('x').has_value());
    EXPECT_FALSE(cubecore::faceFromChar('u').has_value());
}

TEST(Color, SolvedColoursAndLetters) {
    std::string letters;
    for (const Face face : kAllFaces) {
        const Color color = cubecore::solvedColor(face);
        letters += cubecore::toChar(color);
        EXPECT_EQ(cubecore::homeFace(color), face);
    }
    EXPECT_EQ(letters, "WRGYOB");  // white U, red R, green F, yellow D, orange L, blue B
}

TEST(Move, EighteenDistinctMovesWithNames) {
    std::set<std::string_view> names;
    for (const Move move : kAllMoves) {
        names.insert(cubecore::toString(move));
    }
    EXPECT_EQ(names.size(), cubecore::kMoveCount);
    EXPECT_EQ(cubecore::toString(Move::U), "U");
    EXPECT_EQ(cubecore::toString(Move::R2), "R2");
    EXPECT_EQ(cubecore::toString(Move::BPrime), "B'");
}

TEST(Move, FaceAndQuarterTurns) {
    EXPECT_EQ(cubecore::face(Move::FPrime), Face::F);
    EXPECT_EQ(cubecore::quarterTurns(Move::D), 1);
    EXPECT_EQ(cubecore::quarterTurns(Move::D2), 2);
    EXPECT_EQ(cubecore::quarterTurns(Move::DPrime), 3);
    for (const Move move : kAllMoves) {
        EXPECT_EQ(cubecore::makeMove(cubecore::face(move), cubecore::quarterTurns(move)), move);
    }
}

TEST(Move, MakeMoveNormalisesTurns) {
    EXPECT_EQ(cubecore::makeMove(Face::L, 5), Move::L);
    EXPECT_EQ(cubecore::makeMove(Face::L, -1), Move::LPrime);
    EXPECT_EQ(cubecore::makeMove(Face::L, 6), Move::L2);
}

TEST(Move, Inverse) {
    EXPECT_EQ(cubecore::inverse(Move::U), Move::UPrime);
    EXPECT_EQ(cubecore::inverse(Move::UPrime), Move::U);
    EXPECT_EQ(cubecore::inverse(Move::U2), Move::U2);
    for (const Move move : kAllMoves) {
        EXPECT_EQ(cubecore::inverse(cubecore::inverse(move)), move);
        EXPECT_EQ(cubecore::face(cubecore::inverse(move)), cubecore::face(move));
    }
}

TEST(Move, IsConstexpr) {
    static_assert(cubecore::inverse(Move::R) == Move::RPrime);
    static_assert(cubecore::kAllMoves[17] == Move::BPrime);
    static_assert(cubecore::opposite(Face::L) == Face::R);
    SUCCEED();
}

}  // namespace
