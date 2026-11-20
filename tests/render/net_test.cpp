#include "cubecore/render/net.hpp"

#include <gtest/gtest.h>
#include <string>

#include "cubecore/model/rubiks_cube_3d_array.hpp"

namespace {

using cubecore::NetStyle;
using cubecore::renderNet;
using cubecore::RubiksCube3dArray;

TEST(RenderNet, PlainSolvedCube) {
    const std::string expected =
        "       W W W\n"
        "       W W W\n"
        "       W W W\n"
        "O O O  G G G  R R R  B B B\n"
        "O O O  G G G  R R R  B B B\n"
        "O O O  G G G  R R R  B B B\n"
        "       Y Y Y\n"
        "       Y Y Y\n"
        "       Y Y Y\n";
    EXPECT_EQ(renderNet(RubiksCube3dArray{}, NetStyle::Plain), expected);
}

TEST(RenderNet, PlainAfterR) {
    // R: F's right column goes up to U, so U's right column turns green; B's left turns white.
    RubiksCube3dArray cube;
    cube.apply(cubecore::Move::R);
    const std::string expected =
        "       W W G\n"
        "       W W G\n"
        "       W W G\n"
        "O O O  G G Y  R R R  W B B\n"
        "O O O  G G Y  R R R  W B B\n"
        "O O O  G G Y  R R R  W B B\n"
        "       Y Y B\n"
        "       Y Y B\n"
        "       Y Y B\n";
    EXPECT_EQ(renderNet(cube, NetStyle::Plain), expected);
}

TEST(RenderNet, AnsiUsesColourCodesAndResets) {
    const std::string net = renderNet(RubiksCube3dArray{}, NetStyle::Ansi256);
    EXPECT_NE(net.find("\x1b[48;5;208m"), std::string::npos);  // orange
    EXPECT_NE(net.find("\x1b[0m"), std::string::npos);
    EXPECT_EQ(net.find('W'), std::string::npos);  // squares, not letters
    int lines = 0;
    for (const char c : net) {
        lines += c == '\n' ? 1 : 0;
    }
    EXPECT_EQ(lines, 9);
}

}  // namespace
