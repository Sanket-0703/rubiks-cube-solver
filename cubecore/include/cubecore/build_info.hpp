#pragma once

#include <string_view>

namespace cubecore {

/// How this copy of cubecore was built. Printed by `cubesolver info` and recorded next to
/// every benchmark result, so a number can always be traced back to the build that produced it.
struct BuildInfo {
    std::string_view version;          ///< Project version, e.g. "0.1.0".
    std::string_view compilerId;       ///< "AppleClang", "Clang" or "GNU".
    std::string_view compilerVersion;  ///< e.g. "21.0.0".
    std::string_view buildType;        ///< "Debug", "Release", ...
    std::string_view sanitizers;       ///< e.g. "address,undefined", or "none".
    bool coverage = false;             ///< Built with coverage instrumentation.
    long cplusplus = 0;                ///< Value of __cplusplus (202002 for C++20).
};

/// Returns the build settings baked in at configure time.
[[nodiscard]] BuildInfo buildInfo() noexcept;

/// True for an optimised build (Release / RelWithDebInfo / MinSizeRel) without sanitizers or
/// coverage. Benchmarks refuse to report resume numbers from any other build.
[[nodiscard]] bool isBenchmarkBuild(const BuildInfo& info) noexcept;

}  // namespace cubecore
