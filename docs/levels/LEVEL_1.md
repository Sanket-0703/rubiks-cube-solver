# Level 1 — Project Skeleton & CI

Branch `level-1-skeleton`. Built **2026-11-21**.

Level 1 has no cube code yet. It builds the setup that every later level relies on: one command builds and tests the project in six configurations, strict warnings and static analysis run on every file, coverage is measured, and CI repeats all of it on macOS and Linux for every pull request.

---

## 1. The big picture

```
                 CMakePresets.json  (debug · release · asan · tsan · coverage)
                          │
                          ▼
 CMakeLists.txt ──► cmake/AppleSdkCheck.cmake   picks a macOS SDK that links (blocker B-1)
       │            cmake/Sanitizers.cmake      -fsanitize=… for everything
       │            cmake/TargetDefaults.cmake  rcs_configure_target(): warnings, -Werror, coverage
       │            cmake/Dependencies.cmake    CLI11, GoogleTest, Google Benchmark (pinned + SHA-256)
       │
       ├── cubecore/          static library  ◄── all real code lives here
       ├── apps/cubesolver/   CLI  ───────────────┐
       ├── tests/             cubecore_tests + CLI smoke tests (ctest)
       └── bench/             cubecore_bench      │
                                                  ▼
                           links cubecore::cubecore
```

The rule from now on: **all logic goes in `cubecore`**. The CLI, the tests, the benchmarks and (Level 10) the WebAssembly build are thin layers that link against it. That means everything is testable, and the browser runs exactly the same code as the terminal.

## 2. The build: `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25)
include(cmake/AppleSdkCheck.cmake)   # before project()!
project(RubiksCubeSolver VERSION 0.1.0 ... LANGUAGES CXX)
```

- **`cmake_minimum_required(3.25)`** — the oldest CMake that supports everything we use: `SYSTEM` in FetchContent and version-6 presets (workflow presets).
- **The SDK check runs before `project()`** because `project()` is where CMake detects and tests the compiler. By then the SDK must already be chosen.
- `CMAKE_CXX_STANDARD 20` + `STANDARD_REQUIRED` + `EXTENSIONS OFF` → exactly `-std=c++20`. No `gnu++20` extensions, so code that builds here also builds with gcc and Emscripten.
- `CMAKE_EXPORT_COMPILE_COMMANDS` writes `compile_commands.json`, the list of exact compiler flags per file. clangd (the editor) and clang-tidy both read it.
- **Options**, all prefixed `RCS_` so they never clash with a dependency's options: `RCS_BUILD_TESTS`, `RCS_BUILD_BENCHMARKS`, `RCS_WARNINGS_AS_ERRORS`, `RCS_COVERAGE`, `RCS_SANITIZERS`.

### 2.1 `cmake/AppleSdkCheck.cmake` — blocker B-1, fixed automatically

Your Mac has Command Line Tools 26.6 but also a macOS 27 SDK, and the 26.6 linker can't read that SDK. In Level 0 we fixed this by hand with `SDKROOT`. Now CMake does it itself:

1. If we're not on a Mac, or `CMAKE_OSX_SYSROOT` / `SDKROOT` is already set → do nothing.
2. Link `int main() { return 0; }` with the default SDK. If it links → do nothing. (This is what happens on GitHub's macOS runners, and on your Mac once the tools are updated.)
3. Otherwise, try every installed `MacOSX*.*.sdk` from newest to oldest and **cache the first one that links** in `CMAKE_OSX_SYSROOT`.

Because the result is cached, the check runs once per build folder. On your Mac it chooses `MacOSX26.5.sdk` and prints why.

### 2.2 `cmake/Sanitizers.cmake`

`RCS_SANITIZERS="address;undefined"` becomes `-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all`.

- It uses `add_compile_options` (global) **on purpose**, and it's included *before* the dependencies are fetched. GoogleTest is instrumented too. Mixing instrumented and plain code can cause false reports, especially with ThreadSanitizer.
- `-fno-sanitize-recover=all`: UBSan normally prints a report and carries on. This makes it stop, so a test fails and CI goes red.
- ASan + TSan together is refused with a clear error, because the runtimes are incompatible.

### 2.3 `cmake/TargetDefaults.cmake` — `rcs_configure_target()`

Every target **we** write calls `rcs_configure_target(<target>)`. It adds:

| Flag | Catches |
|---|---|
| `-Wall -Wextra -Wpedantic` | the standard set + non-portable extensions |
| `-Wshadow` | an inner variable hiding an outer one (easy with `face`, `row`, `col`) |
| `-Wconversion -Wsign-conversion` | silent narrowing, e.g. `uint64_t` → `uint8_t`, `int` → `size_t`. Essential for bit-packing code. |
| `-Wnon-virtual-dtor` | a polymorphic base class without a virtual destructor (`RubiksCube` in Level 2) |
| `-Woverloaded-virtual` | a derived function that hides a base virtual instead of overriding it |
| `-Wold-style-cast` | `(int)x` — use `static_cast` so casts are visible and searchable |
| `-Wcast-align -Wdouble-promotion -Wimplicit-fallthrough` | alignment bugs, accidental `double` maths, missing `break` |
| `-Werror` (when `RCS_WARNINGS_AS_ERRORS`, on in every preset) | a warning can't be ignored |

Coverage flags (`-fprofile-instr-generate -fcoverage-mapping`) are added here too, so only our code is measured. Dependencies never get these flags. Their warnings aren't ours to fix.

### 2.4 `cmake/Dependencies.cmake` — FetchContent

| Library | Version | Used for |
|---|---|---|
| CLI11 | 2.7.2 | `cubesolver` subcommands and flags (header-only) |
| GoogleTest | 1.18.0 | unit tests (`GTest::gtest_main`) |
| Google Benchmark | 1.10.0 | micro-benchmarks and the resume-claim harness |

- **Release tarball + `URL_HASH SHA256=…`**: the build fails if GitHub ever serves different bytes. This is a supply-chain safeguard, and it's faster than a `git clone`.
- **`SYSTEM`**: their headers are included with `-isystem`, so our strict warnings never fire inside their code.
- GoogleTest and Benchmark are only downloaded when tests / benchmarks are enabled. The sanitizer and coverage presets skip Benchmark.

## 3. `CMakePresets.json`

Presets put every build configuration in one checked-in file. VS Code (CMake Tools), CLion and CI all read the same file. Every preset builds into its own folder `build/<preset>/`, so switching between them never forces a rebuild.

| Preset | Build type | Extra | Use it for |
|---|---|---|---|
| `debug` | Debug | — | day to day; clangd reads its `compile_commands.json` |
| `release` | Release (`-O3`) | LTO (`CMAKE_INTERPROCEDURAL_OPTIMIZATION`) | benchmarks, real solves |
| `asan` | Debug | ASan + UBSan | memory bugs, undefined behaviour |
| `tsan` | Debug | ThreadSanitizer | data races (Level 5 onwards) |
| `coverage` | Debug | llvm-cov instrumentation | `scripts/coverage.sh` |

Each has a matching **build** and **test** preset. Test presets fail if no tests are found (`noTestsAction: error`), so an empty test run can never pass silently. **Workflow presets** chain configure → build → test:

```bash
cmake --workflow --preset debug
```

## 4. The code

### 4.1 `cubecore`: `BuildInfo`

`cubecore/src/config.hpp.in` is a template. At configure time CMake fills in `@PROJECT_VERSION@`, `@CMAKE_BUILD_TYPE@`, the compiler and the sanitizers, and writes `build/<preset>/cubecore/generated/config.hpp`. That header is **private** to `cubecore`: only `build_info.cpp` includes it, so changing build settings recompiles one file.

`buildInfo()` returns those values in a `BuildInfo` struct. `isBenchmarkBuild()` is true only for an optimised build without sanitizers or coverage. Level 5's benchmark harness will **refuse to report resume numbers** from any other build, so a Debug or ASan timing can never get into `BENCHMARKS.md` by mistake.

### 4.2 `cubesolver` CLI

```
cubesolver --version     → 0.1.0
cubesolver info          → version, compiler, C++ standard, build type, sanitizers, hw threads
cubesolver               → error: a subcommand is required (exit code ≠ 0)
```

- CLI11: `require_subcommand(1)` makes the app subcommand-based from the start. `show`, `solve`, `bench` and `build-pdb` arrive in later levels.
- `main` wraps everything in `try/catch (const std::exception&)`, so an error prints a message and exits with code 1 instead of calling `std::terminate()`. clang-tidy's `bugprone-exception-escape` required this.

## 5. Tests

- `tests/build_info_test.cpp` — 5 GoogleTest tests: the version is `x.y.z`, `__cplusplus ≥ 202002`, the fields are filled in, and `isBenchmarkBuild` accepts or rejects the right builds. A small `makeInfo(buildType, sanitizers, coverage)` helper starts from `BuildInfo{}` and sets only the fields under test.
  - **Lesson from the Linux rehearsal:** the first version used C++20 designated initialisers that skipped fields (`BuildInfo{.buildType = "Release"}`). AppleClang 21 accepts that silently, but gcc 14 and clang 18 warn (`-Wmissing-field-initializers`), and with `-Werror` the Linux build failed. That's exactly why CI builds with three compilers. `BuildInfo{}` (empty braces) value-initialises every field and never warns.
- `gtest_discover_tests(... DISCOVERY_MODE PRE_TEST)`: ctest lists each `TEST` as its own test, with the list taken when ctest runs rather than at build time. Building never has to start a sanitizer-instrumented binary.
- **CLI smoke tests** with plain `add_test`: `cli.version` (output starts with the version), `cli.info` (reports the right build type), `cli.requires_subcommand` (`WILL_FAIL`: running without a subcommand *must* fail).

## 6. Benchmark

`bench/placeholder_bench.cpp` proves Google Benchmark works and previews Level 3. It turns one face, with its 8 stickers stored either as 8 bytes or packed in one `uint64_t`:

| | Time (release, M5) |
|---|---|
| `BM_FaceTurnByteArray` — move 8 bytes in a loop | 1.49 ns |
| `BM_FaceTurnBitboard` — `std::rotl(face, 16)` | 0.29 ns |

That's **5× faster** for the same face turn, because one 64-bit rotate instruction does the work of eight byte moves. This is the trick behind the bitboard cube in Level 3.

`benchmark::DoNotOptimize(face)` stops the compiler from seeing the result is unused and deleting the whole loop. Without it, both benchmarks would measure 0 ns.

## 7. Code quality tools

| File | Tool | Run it |
|---|---|---|
| `.clang-format` | clang-format 23 (Google style, 4-space indent, 100 columns, includes sorted and grouped) | `scripts/format.sh` (fix) · `--check` (CI) |
| `.clang-tidy` | clang-tidy 23: bugprone, cert, clang-analyzer, concurrency, cppcoreguidelines, misc, modernize, performance, portability, readability, plus naming rules | `scripts/tidy.sh` |
| `.clangd` | tells clangd to use `build/debug/compile_commands.json` | automatic in VS Code |
| `.editorconfig` | UTF-8, LF line endings, final newline, indentation | automatic in most editors |
| `scripts/coverage.sh` | llvm-cov | `scripts/coverage.sh --fail-under 85` |

**Naming rules** (`readability-identifier-naming`) enforce one style: `PascalCase` types, `camelCase` functions, variables and members, a `_` suffix on private members, `kPascalCase` constants, and `lower_case` namespaces. Benchmark functions (`BM_…`) are exempt.

**Disabled checks are listed with a reason** at the top of `.clang-tidy`. The main one is unchecked array indexing: indexing fixed-size arrays with computed values *is* the cube model. Checked access in the hot loop would cost speed, so ASan and tests catch out-of-bounds access instead. Two folder-level overrides exist:
- `bench/.clang-tidy`: Google Benchmark's `for (auto _ : state)` never reads `_` by design.
- `apps/.clang-tidy`: the static analyzer follows `CLI11_PARSE` into CLI11's own code and reports two known false positives there. They stay in the report because their path starts in `main.cpp`, so the header filter can't drop them. Only the CLI layer turns these two off; `cubecore` keeps every analyzer check.

**Coverage** is clang's *source-based* coverage, which is more precise than gcov. Running the tests writes a `.profraw` file per process (`LLVM_PROFILE_FILE=…/%p.profraw`). `llvm-profdata merge` combines them, and `llvm-cov report/show` produces the table and an HTML report with per-branch counts. The tools must match the compiler that built the code, so on macOS they come from `xcrun`. The **85% gate on `cubecore` lines runs from day one**.

## 8. CI: `.github/workflows/ci.yml`

| Job | Runs |
|---|---|
| `build-test` (7 jobs) | macOS AppleClang: debug, asan, release · Ubuntu gcc-14: debug, asan · Ubuntu clang-18: debug, release. Each runs `cmake --workflow --preset …`; release jobs also do a benchmark smoke run |
| `format` | clang-format **23.1.3 from pip**, the same major version as local, so CI and your machine format code identically |
| `tidy` | clang-tidy 23.1.3 from pip on the debug build |
| `coverage` | clang-18 build, ≥ 85% line gate, summary in the job page, HTML report uploaded as an artifact |

- Three compilers (AppleClang, gcc, clang) catch non-portable code. gcc and clang warn about different things.
- `fail-fast: false`: one failing job doesn't cancel the others, so you see every failure at once.
- `concurrency` cancels an older run on the same branch when you push again.
- `permissions: contents: read`: the workflow's token can't write to the repo.
- Before pushing, the Ubuntu jobs were rehearsed locally in an `ubuntu:24.04` Docker container (gcc-14 debug + asan, clang-18 debug, pip clang-format and clang-tidy 23.1.3, clang-18 coverage). The rehearsal caught the gcc/clang `-Wmissing-field-initializers` failure described in §5.

## 9. What happens when you run `cmake --workflow --preset debug`

1. **Configure**: the SDK check (cached after the first run) → `project()` detects AppleClang 21 → FetchContent downloads and verifies the three libraries (first run only) → `config.hpp` is generated → Ninja build files and `compile_commands.json` are written to `build/debug/`.
2. **Build**: Ninja compiles `cubecore`, `cubesolver`, `cubecore_tests`, `cubecore_bench` and GoogleTest in parallel on all 10 cores, with `-Werror`.
3. **Test**: ctest asks `cubecore_tests` for its list of tests, then runs the 5 unit tests and 3 CLI smoke tests. It prints output only for tests that fail.

## 10. Explain it in an interview

> **"How is the project built?"**
> CMake with Ninja and presets. All the logic is in one static library, `cubecore`. The CLI, the tests, the benchmarks and a WebAssembly build are thin layers on top of it. Presets give one command per configuration (debug, release with LTO, ASan+UBSan, TSan, coverage), and CI runs the same presets.

> **"How do you keep C++ code safe?"**
> Several layers: strict warnings as errors (including `-Wconversion`, which matters for bit-packing), clang-tidy with the static analyzer and core-guidelines checks, sanitizer builds (ASan, UBSan, TSan) that fail on the first report, and three compilers in CI. Every disabled check is documented with a reason.

> **"Why apply sanitizers globally but warnings per target?"**
> Sanitizers have to instrument *all* the code, including dependencies, or you get false reports and missed bugs. Warnings are only useful on code you own, and dependencies are compiled as system headers so they can't break our `-Werror` build.

> **"How do you make dependencies reproducible?"**
> Each one is pinned to a release tarball and checked against its SHA-256 at configure time. If the bytes change, the build fails.

> **"How do you know your benchmark numbers are real?"**
> The library knows how it was built. The harness refuses to report numbers unless the build is optimised and has no sanitizers or coverage. `DoNotOptimize` stops the compiler from deleting the measured work. Every result records the compiler and build settings.

## 11. Files in this level

| File | Purpose |
|---|---|
| `CMakeLists.txt` | top-level build, options, subprojects |
| `CMakePresets.json` | debug / release / asan / tsan / coverage: configure, build, test, workflow |
| `cmake/AppleSdkCheck.cmake` | picks a macOS SDK that links (B-1) |
| `cmake/Sanitizers.cmake` | global sanitizer flags |
| `cmake/TargetDefaults.cmake` | `rcs_configure_target()`: warnings, `-Werror`, coverage |
| `cmake/Dependencies.cmake` | CLI11, GoogleTest, Google Benchmark |
| `cubecore/CMakeLists.txt`, `cubecore/src/config.hpp.in` | the library and its generated config |
| `cubecore/include/cubecore/build_info.hpp`, `cubecore/src/build_info.cpp` | `BuildInfo`, `isBenchmarkBuild` |
| `apps/cubesolver/main.cpp` | CLI: `--version`, `info` |
| `tests/build_info_test.cpp`, `tests/CMakeLists.txt` | unit + CLI smoke tests |
| `bench/placeholder_bench.cpp` | byte array vs bitboard face turn |
| `.clang-format`, `.clang-tidy`, `apps/.clang-tidy`, `bench/.clang-tidy`, `.clangd`, `.editorconfig` | code style and analysis |
| `.vscode/settings.json`, `.vscode/extensions.json` | VS Code: CMake presets, clangd, CodeLLDB |
| `scripts/format.sh`, `scripts/tidy.sh`, `scripts/coverage.sh` | the same checks locally and in CI |
| `.github/workflows/ci.yml` | CI |
