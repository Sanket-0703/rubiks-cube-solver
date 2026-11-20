#include "cubecore/build_info.hpp"

#include <gtest/gtest.h>
#include <regex>
#include <string>
#include <string_view>

namespace {

using cubecore::BuildInfo;
using cubecore::buildInfo;
using cubecore::isBenchmarkBuild;

TEST(BuildInfo, VersionIsSemantic) {
    const std::string version{buildInfo().version};
    EXPECT_TRUE(std::regex_match(version, std::regex{R"(\d+\.\d+\.\d+)"})) << version;
}

TEST(BuildInfo, CompiledAsCpp20OrNewer) {
    EXPECT_GE(buildInfo().cplusplus, 202002L);
}

TEST(BuildInfo, FieldsAreFilledIn) {
    const BuildInfo info = buildInfo();
    EXPECT_FALSE(info.compilerId.empty());
    EXPECT_FALSE(info.compilerVersion.empty());
    EXPECT_FALSE(info.buildType.empty());
    EXPECT_FALSE(info.sanitizers.empty());
}

// Builds a BuildInfo with only the fields isBenchmarkBuild looks at. (Designated initialisers
// that skip fields trip gcc's and clang's -Wmissing-field-initializers, so start from {}.)
BuildInfo makeInfo(std::string_view buildType, std::string_view sanitizers, bool coverage) {
    BuildInfo info{};
    info.buildType = buildType;
    info.sanitizers = sanitizers;
    info.coverage = coverage;
    return info;
}

TEST(IsBenchmarkBuild, AcceptsOnlyPlainOptimisedBuilds) {
    EXPECT_TRUE(isBenchmarkBuild(makeInfo("Release", "none", false)));
    EXPECT_TRUE(isBenchmarkBuild(makeInfo("RelWithDebInfo", "none", false)));
    EXPECT_TRUE(isBenchmarkBuild(makeInfo("MinSizeRel", "none", false)));
    EXPECT_FALSE(isBenchmarkBuild(makeInfo("Debug", "none", false)));
}

TEST(IsBenchmarkBuild, RejectsInstrumentedBuilds) {
    EXPECT_FALSE(isBenchmarkBuild(makeInfo("Release", "address,undefined", false)));
    EXPECT_FALSE(isBenchmarkBuild(makeInfo("Release", "none", true)));
}

}  // namespace
