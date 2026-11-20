#include "cubecore/notation/notation.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cubecore/model/face.hpp"

namespace cubecore {

namespace {

constexpr std::string_view kTypographicApostrophe = "’";  // ’ (3 bytes in UTF-8)

/// Length of an apostrophe at text[pos], or 0 if there isn't one.
std::size_t apostropheAt(std::string_view text, std::size_t pos) {
    if (pos < text.size() && text[pos] == '\'') {
        return 1;
    }
    if (text.substr(pos).starts_with(kTypographicApostrophe)) {
        return kTypographicApostrophe.size();
    }
    return 0;
}

[[noreturn]] void rejectToken(std::string_view text, std::size_t pos) {
    const char c = text[pos];
    if (std::string_view{"urfdlb"}.find(c) != std::string_view::npos) {
        throw NotationError(
            std::string{"wide move '"} + c + "' is not supported; use face turns U R F D L B", pos);
    }
    if (std::string_view{"MESxyz"}.find(c) != std::string_view::npos) {
        throw NotationError(std::string{"slice moves and cube rotations ('"} + c +
                                "') are not supported; use face turns U R F D L B",
                            pos);
    }
    if (c == '2' || c == '\'' || apostropheAt(text, pos) > 0) {
        throw NotationError("modifier without a face letter before it", pos);
    }
    throw NotationError(std::string{"unexpected character '"} + c + "'", pos);
}

}  // namespace

std::vector<Move> parseMoves(std::string_view text) {
    std::vector<Move> moves;
    std::size_t pos = 0;
    while (pos < text.size()) {
        const char c = text[pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',') {
            ++pos;
            continue;
        }

        const std::optional<Face> face = faceFromChar(c);
        if (!face) {
            rejectToken(text, pos);
        }
        ++pos;

        int turns = 1;
        if (pos < text.size() && text[pos] == '2') {
            turns = 2;
            ++pos;
            pos += apostropheAt(text, pos);  // "U2'" is still a half turn
        } else if (const std::size_t len = apostropheAt(text, pos); len > 0) {
            turns = 3;
            pos += len;
        }
        moves.push_back(makeMove(*face, turns));
    }
    return moves;
}

std::string formatMoves(std::span<const Move> moves) {
    std::string text;
    for (const Move move : moves) {
        if (!text.empty()) {
            text += ' ';
        }
        text += toString(move);
    }
    return text;
}

std::vector<Move> invertMoves(std::span<const Move> moves) {
    std::vector<Move> inverted(moves.rbegin(), moves.rend());
    std::ranges::transform(inverted, inverted.begin(), [](Move m) { return inverse(m); });
    return inverted;
}

}  // namespace cubecore
