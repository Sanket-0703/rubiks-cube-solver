#include <CLI/CLI.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "cubecore/build_info.hpp"
#include "cubecore/model/facelets.hpp"
#include "cubecore/model/move.hpp"
#include "cubecore/model/rubiks_cube_3d_array.hpp"
#include "cubecore/notation/notation.hpp"
#include "cubecore/notation/scramble.hpp"
#include "cubecore/render/net.hpp"

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

/// --color auto: colour only when writing to a terminal and NO_COLOR (no-color.org) isn't set.
cubecore::NetStyle chooseStyle(const std::string& color) {
    if (color == "always") {
        return cubecore::NetStyle::Ansi256;
    }
    if (color == "never") {
        return cubecore::NetStyle::Plain;
    }
    const bool noColorSet = std::getenv("NO_COLOR") != nullptr;  // NOLINT(concurrency-mt-unsafe)
    const bool terminal = isatty(STDOUT_FILENO) != 0;
    return (terminal && !noColorSet) ? cubecore::NetStyle::Ansi256 : cubecore::NetStyle::Plain;
}

/// Parses a scramble, or prints the error with a caret under the offending character.
std::optional<std::vector<cubecore::Move>> parseOrReport(const std::string& text) {
    try {
        return cubecore::parseMoves(text);
    } catch (const cubecore::NotationError& e) {
        std::cerr << "error: " << e.what() << '\n'
                  << "  " << text << '\n'
                  << "  " << std::string(e.position(), ' ') << "^\n";
        return std::nullopt;
    }
}

struct ShowOptions {
    std::string scramble;
    std::string color = "auto";
    bool facelets = false;
};

int runShow(const ShowOptions& options) {
    const auto moves = parseOrReport(options.scramble);
    if (!moves) {
        return 1;
    }
    cubecore::RubiksCube3dArray cube;
    cube.applyMoves(*moves);

    std::cout << "Scramble: " << (moves->empty() ? "(none)" : cubecore::formatMoves(*moves)) << " ("
              << moves->size() << (moves->size() == 1 ? " move" : " moves") << ")\n\n"
              << cubecore::renderNet(cube, chooseStyle(options.color)) << '\n'
              << "Solved: " << (cube.isSolved() ? "yes" : "no") << '\n';
    if (options.facelets) {
        std::cout << "Facelets: " << cubecore::toFaceletString(cube) << '\n';
    }
    return 0;
}

struct ScrambleOptions {
    std::size_t length = 20;
    std::optional<std::uint64_t> seed;
};

int runScramble(const ScrambleOptions& options) {
    std::uint64_t seed = 0;
    if (options.seed) {
        seed = *options.seed;
    } else {
        std::random_device device;
        seed = (std::uint64_t{device()} << 32U) | device();
        // stderr, so `$(cubesolver scramble)` captures only the moves.
        std::cerr << "seed " << seed << " (pass --seed " << seed << " to repeat)\n";
    }
    std::cout << cubecore::formatMoves(cubecore::randomScramble(options.length, seed)) << '\n';
    return 0;
}

}  // namespace

// The signature of main is fixed by the standard, so argv can't be made const.
int main(int argc, char** argv) {  // NOLINT(misc-const-correctness)
    try {
        const cubecore::BuildInfo info = cubecore::buildInfo();
        int exitCode = 0;

        CLI::App app{"Rubik's Cube solver: BFS, DFS, IDDFS and Korf's IDA* with pattern databases",
                     "cubesolver"};
        app.set_version_flag("-V,--version", std::string{info.version});
        app.require_subcommand(1);

        CLI::App* infoCmd = app.add_subcommand("info", "Show how this binary was built");
        infoCmd->callback([&info] { printInfo(info); });

        ShowOptions show;
        CLI::App* showCmd = app.add_subcommand("show", "Apply a scramble and draw the cube");
        showCmd->add_option("-s,--scramble", show.scramble, "Moves, e.g. \"R U R' U'\"");
        showCmd->add_option("--color", show.color, "Coloured output")
            ->check(CLI::IsMember({"auto", "always", "never"}))
            ->capture_default_str();
        showCmd->add_flag("--facelets", show.facelets,
                          "Also print the 54-character facelet string");
        showCmd->callback([&] { exitCode = runShow(show); });

        ScrambleOptions scramble;
        CLI::App* scrambleCmd = app.add_subcommand("scramble", "Print a random scramble");
        scrambleCmd->add_option("-n,--length", scramble.length, "Number of moves")
            ->check(CLI::Range(std::size_t{0}, std::size_t{1000}))
            ->capture_default_str();
        scrambleCmd->add_option("--seed", scramble.seed, "Seed, for a repeatable scramble");
        scrambleCmd->callback([&] { exitCode = runScramble(scramble); });

        CLI11_PARSE(app, argc, argv);
        return exitCode;
    } catch (const std::exception& e) {
        // Anything a subcommand throws ends here: print it and exit non-zero, never terminate().
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
