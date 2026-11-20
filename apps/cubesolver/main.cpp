#include <CLI/CLI.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include "cubecore/build_info.hpp"

namespace {

void printInfo(const cubecore::BuildInfo& info) {
    std::cout << "cubesolver " << info.version << '\n'
              << "  compiler      " << info.compilerId << ' ' << info.compilerVersion << '\n'
              << "  C++ standard  " << info.cplusplus << '\n'
              << "  build type    " << info.buildType << '\n'
              << "  sanitizers    " << info.sanitizers << '\n'
              << "  coverage      " << (info.coverage ? "on" : "off") << '\n'
              << "  hw threads    " << std::thread::hardware_concurrency() << '\n'
              << "  benchmarkable " << (cubecore::isBenchmarkBuild(info) ? "yes" : "no") << '\n';
}

}  // namespace

// The signature of main is fixed by the standard, so argv can't be made const.
int main(int argc, char** argv) {  // NOLINT(misc-const-correctness)
    try {
        const cubecore::BuildInfo info = cubecore::buildInfo();

        CLI::App app{"Rubik's Cube solver: BFS, DFS, IDDFS and Korf's IDA* with pattern databases",
                     "cubesolver"};
        app.set_version_flag("-V,--version", std::string{info.version});
        app.require_subcommand(1);

        CLI::App* infoCmd = app.add_subcommand("info", "Show how this binary was built");
        infoCmd->callback([&info] { printInfo(info); });

        CLI11_PARSE(app, argc, argv);
        return 0;
    } catch (const std::exception& e) {
        // Anything a subcommand throws ends here: print it and exit non-zero, never terminate().
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
