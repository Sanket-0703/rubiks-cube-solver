#include "cubecore/model/move.hpp"

#include <array>
#include <string_view>

namespace cubecore {

std::string_view toString(Move move) noexcept {
    static constexpr std::array<std::string_view, kMoveCount> kNames{
        "U", "U2", "U'", "R", "R2", "R'", "F", "F2", "F'",
        "D", "D2", "D'", "L", "L2", "L'", "B", "B2", "B'",
    };
    return kNames[toIndex(move)];
}

}  // namespace cubecore
