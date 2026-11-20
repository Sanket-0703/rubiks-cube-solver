#include "cubecore/build_info.hpp"

#include "config.hpp"

namespace cubecore {

BuildInfo buildInfo() noexcept {
    return BuildInfo{
        .version = config::kVersion,
        .compilerId = config::kCompilerId,
        .compilerVersion = config::kCompilerVersion,
        .buildType = config::kBuildType,
        .sanitizers = config::kSanitizers,
        .coverage = config::kCoverage,
        .cplusplus = __cplusplus,
    };
}

bool isBenchmarkBuild(const BuildInfo& info) noexcept {
    const bool optimised = info.buildType == "Release" || info.buildType == "RelWithDebInfo" ||
                           info.buildType == "MinSizeRel";
    return optimised && info.sanitizers == "none" && !info.coverage;
}

}  // namespace cubecore
