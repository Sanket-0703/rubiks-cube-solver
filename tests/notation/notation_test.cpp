#include "cubecore/notation/notation.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "cubecore/model/move.hpp"

namespace {

using cubecore::Move;
using cubecore::NotationError;
using cubecore::parseMoves;

std::vector<Move> moves(std::initializer_list<Move> list) {
    return list;
}

/// Position of the NotationError thrown for `text`, or -1 if it parsed.
long errorPosition(const std::string& text) {
    try {
        (void)parseMoves(text);
    } catch (const NotationError& e) {
        return static_cast<long>(e.position());
    }
    return -1;
}

TEST(ParseMoves, AllEighteenMovesRoundTrip) {
    const std::string text = "U U2 U' R R2 R' F F2 F' D D2 D' L L2 L' B B2 B'";
    const auto parsed = parseMoves(text);
    ASSERT_EQ(parsed.size(), cubecore::kMoveCount);
    for (std::size_t i = 0; i < parsed.size(); ++i) {
        EXPECT_EQ(parsed[i], cubecore::kAllMoves[i]);
    }
    EXPECT_EQ(cubecore::formatMoves(parsed), text);
}

TEST(ParseMoves, SpacingIsOptional) {
    const auto expected = moves({Move::R, Move::U, Move::RPrime, Move::UPrime});
    EXPECT_EQ(parseMoves("R U R' U'"), expected);
    EXPECT_EQ(parseMoves("RUR'U'"), expected);
    EXPECT_EQ(parseMoves("  R,U, R'\tU'\n"), expected);
}

TEST(ParseMoves, EmptyInputIsNoMoves) {
    EXPECT_TRUE(parseMoves("").empty());
    EXPECT_TRUE(parseMoves("   ").empty());
}

TEST(ParseMoves, AcceptsCopiedNotationVariants) {
    EXPECT_EQ(parseMoves("R’ U’"), moves({Move::RPrime, Move::UPrime}));  // R’ U’
    EXPECT_EQ(parseMoves("U2' F2’"), moves({Move::U2, Move::F2}));
}

TEST(ParseMoves, RejectsWideSliceAndRotationWithPosition) {
    EXPECT_EQ(errorPosition("R u"), 2);
    EXPECT_EQ(errorPosition("R U M"), 4);
    EXPECT_EQ(errorPosition("x R"), 0);
}

TEST(ParseMoves, RejectsStrayModifiersAndUnknownCharacters) {
    EXPECT_EQ(errorPosition("R U X"), 4);
    EXPECT_EQ(errorPosition("R ' U"), 2);
    EXPECT_EQ(errorPosition("2R"), 0);
    EXPECT_EQ(errorPosition("R ’"), 2);
}

TEST(ParseMoves, ErrorMessagesExplain) {
    try {
        (void)parseMoves("R u");
        FAIL();
    } catch (const NotationError& e) {
        EXPECT_NE(std::string{e.what()}.find("wide move"), std::string::npos) << e.what();
    }
    try {
        (void)parseMoves("y");
        FAIL();
    } catch (const NotationError& e) {
        EXPECT_NE(std::string{e.what()}.find("rotations"), std::string::npos) << e.what();
    }
}

TEST(FormatMoves, EmptyAndSingle) {
    EXPECT_EQ(cubecore::formatMoves({}), "");
    EXPECT_EQ(cubecore::formatMoves(moves({Move::D2})), "D2");
}

TEST(InvertMoves, ReversesAndInverts) {
    EXPECT_EQ(cubecore::invertMoves(parseMoves("R U F2 D'")), parseMoves("D F2 U' R'"));
    EXPECT_TRUE(cubecore::invertMoves({}).empty());
}

}  // namespace
