# Build Plan — Level by Level

Updated **2026-11-16**.

Ground rules
- **Local first.** Everything builds and runs on this Mac (Apple M5, 10 cores, 16 GB). The only thing published is the web demo, in Level 10.
- **Level 0.** Nothing exists yet. A feature is `[x]` only once it's built, tested, benchmarked where it matters, and checked by you.
- **Every level ends with a Check** you run yourself, in the terminal or in the browser.
- **Tests as we go.** GoogleTest unit tests and property tests (e.g. any move done 4 times returns the cube to where it started, and every cube model agrees after any sequence) in every level. CI runs ASan/UBSan builds. A coverage gate of **85%+** on `cubecore` runs in CI from Level 1.
- **Numbers come from benchmarks.** The resume claims are checked by `bench/` on this Mac (median / p95 / worst case over a fixed set of seeded scrambles). If a measurement disagrees with the resume, the resume wording changes, not the benchmark.
- **One branch and one PR per level.** Each level starts on a fresh branch from `main` named `level-N-<topic>` and goes up as a draft PR so CI runs. You review and merge it.
- **Every level ends with its own explainer.** `docs/levels/LEVEL_N.md` walks through every class and function built in that level and why it was built that way, and ends with "Explain it in an interview".
- **Learn the IDE on the way.** VS Code with clangd, CMake Tools and CodeLLDB. Each level adds the IDE skills it needs to [IDE_GUIDE.md](IDE_GUIDE.md).

## Tech stack

| Layer | Stack |
|---|---|
| Language | C++20 (Apple clang 21 locally; clang + gcc on Linux in CI) |
| Build | CMake 4 + Ninja, `CMakePresets.json` (debug / release / asan / coverage) |
| Libraries | GoogleTest, Google Benchmark, CLI11. Fetched with CMake `FetchContent` at pinned versions |
| Quality | clang-format, clang-tidy (Homebrew LLVM 23), llvm-cov coverage, GitHub Actions on macOS and Ubuntu |
| Web demo | Emscripten (WebAssembly) + React + TypeScript (Vite) + three.js, on GitHub Pages |
| IDE | VS Code + clangd + CMake Tools + CodeLLDB |

## Target layout

```
rubiks-cube-solver/
├── CMakeLists.txt, CMakePresets.json
├── cubecore/                    the library — all the real code
│   ├── include/cubecore/
│   │   ├── model/   RubiksCube (abstract), RubiksCube3dArray, RubiksCube1dArray,
│   │   │            RubiksCubeBitboard, CubieCube, Move, Face, Color
│   │   ├── notation/  "R U R' U2" parser/printer, seeded scramble generator
│   │   ├── search/    Solver<Cube> + BFS, DFS, IDDFS, BidirectionalBFS, IDAStar, (TwoPhase)
│   │   └── pdb/       permutation ranking, NibbleArray, PatternDatabase (corner, edge)
│   └── src/
├── apps/cubesolver/             CLI: solve, scramble, bench, build-pdb, show
├── tests/                       GoogleTest
├── bench/                       Google Benchmark + resume-claim harness (CSV out)
├── web/                         Level 10: WASM bindings + React/three.js demo
└── docs/                        this plan, decisions, IDE guide, level docs, BENCHMARKS.md
```

Resume claims and where each gets built:

| Resume claim | Level |
|---|---|
| 3x3 state space modelled in OOP C++ | 2 (abstract `RubiksCube` + 3D array), 3, 6 |
| Bitwise, compact array encodings | 3 (1D array, bitboard), 6 (cubie model, ranking with bit operations), 7 (4-bit nibble arrays) |
| BFS, DFS and IDDFS solvers | 4 |
| Depth 8 in under 3 s | 5 (bidirectional BFS, multithreaded IDDFS, measured) |
| Corner/edge pattern-database heuristics | 6, 7 |
| Korf's IDA*, depth 13 in under 10 s | 8 (measured) |

Extra features beyond the resume: bidirectional BFS, multithreaded search, a CLI, seeded scrambles, PDB files with checksums, a Kociemba two-phase solver (Level 9), and a WebAssembly 3D demo (Level 10).

Legend: `[x]` done · `[ ]` to do

---

## Level 0 — Development environment ✅ (IDE setup is yours to do)

- [x] Project folder `~/rubiks-cube-solver`
- [x] Homebrew: CMake 4.5.0, Ninja 1.13.2, LLVM 23.1.5 (keg-only: used for `clang-format` / `clang-tidy`; Apple clang 21 stays the compiler)
- [x] Apple clang 21 builds a C++20 program (`<bit>`, `std::rotl`, `std::popcount`, threads) with Release and with ASan + UBSan
- [x] SDK workaround — see [B-1](TECHNICAL_DECISIONS.md#1-blockers): builds use `SDKROOT=…/MacOSX.sdk` (26.5)
- [x] `gh` signed in
- [x] Plan docs: this file, [TECHNICAL_DECISIONS.md](TECHNICAL_DECISIONS.md), [IDE_GUIDE.md](IDE_GUIDE.md)
- [x] Git repository on `main` with `.gitignore`
- [ ] GitHub repository (public) — created when we push Level 1 (your approval)
- [ ] IDE: install VS Code extensions and the `code` command ([IDE_GUIDE.md](IDE_GUIDE.md), Level 0)

**Check:** `cmake --version`, `ninja --version`, `$(brew --prefix llvm)/bin/clang-tidy --version` all print versions; the smoke test prints `hw threads: 10`. ✅

## Level 1 — Project skeleton + CI ✅ locally (CI runs once the repo is on GitHub; IDE exercises are yours to do)

- [x] `CMakeLists.txt` for `cubecore` (static library), `cubesolver` (CLI), `tests`, `bench`; strict warnings (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wold-style-cast …`) as errors on our targets only
- [x] `CMakePresets.json`: `debug`, `release` (`-O3` + LTO), `asan` (ASan + UBSan), `tsan`, `coverage`, plus workflow presets (`cmake --workflow --preset debug`)
- [x] Blocker B-1 handled by CMake itself (`cmake/AppleSdkCheck.cmake`): no more `SDKROOT` by hand
- [x] CLI11 2.7.2, GoogleTest 1.18.0, Google Benchmark 1.10.0 via `FetchContent`, pinned to release tarballs with SHA-256 checks
- [x] `.clang-format`, `.clang-tidy` (with naming rules), `.clangd`, `.editorconfig`, `.vscode/` settings; `scripts/format.sh`, `scripts/tidy.sh`, `scripts/coverage.sh`
- [x] `cubecore::BuildInfo` + `isBenchmarkBuild()`; `cubesolver --version` and `cubesolver info`
- [x] 5 unit tests + 3 CLI smoke tests; passing in debug, release, asan, tsan and coverage builds; 0 warnings; clang-tidy clean
- [x] Coverage gate **85% of `cubecore` lines from day one** (now 100%)
- [x] First benchmark: one face turn takes 1.49 ns as a byte array and 0.29 ns as a bitboard (`std::rotl`)
- [x] GitHub Actions: macOS AppleClang (debug, asan, release) · Ubuntu gcc-14 (debug, asan) · Ubuntu clang-18 (debug, release) · clang-format · clang-tidy · coverage; `actionlint` clean
- [x] Level doc: [LEVEL_1.md](levels/LEVEL_1.md)
- [ ] GitHub repo + draft PR, CI green (needs your approval: D-07)
- [ ] IDE: configure with CMake Tools, run a test from the Testing panel, clangd navigation ([IDE_GUIDE.md](IDE_GUIDE.md), Level 1)

**Check:** `cmake --workflow --preset debug` green locally ✅, and green in CI on the draft PR.

## Level 2 — Cube model (OOP) ✅ locally (PR + CI pending; IDE exercises and the real-cube check are yours to do)

- [x] `Face` in URFDLB order (opposite = +3 mod 6), `Color` (W R G Y O B, Western scheme), `Move`: the 18 face turns with `face()`, `quarterTurns()`, `makeMove()`, `inverse()`, all `constexpr`
- [x] Abstract `RubiksCube`: pure virtual `getColor` / `setColor` / `apply(Move)`; `applyMoves`, `isSolved` (overridable), `centerColor`; protected copy operations against slicing
- [x] `RubiksCube3dArray` (`std::array<FaceGrid, 6>`, 54 bytes, `final`): the reference model with all six turns written out by hand
- [x] Facelet strings (Kociemba format, URFDLB): `toFaceletString` / `loadFaceletString` with validation
- [x] Notation: `parseMoves` (accepts `RUR'U'`, `’`, `U2'`; explains wide/slice/rotation moves; errors carry a position), `formatMoves`, `invertMoves`
- [x] Seeded scrambles that are identical on every platform: `SplitMix64` (checked against the reference implementation) + `randomScramble` (no same face twice, no X Y X with opposite faces)
- [x] Net renderer: plain letters or 256-colour ANSI; CLI `--color auto|always|never` honours `isatty` and `NO_COLOR`
- [x] CLI: `cubesolver show --scramble … [--facelets]`, `cubesolver scramble [--length N] [--seed S]`; notation errors drawn with a caret
- [x] Tests: 73 (64 GoogleTest + 9 CLI). Typed tests ready for the Level 3 models. Algebra (X⁴, X·X⁻¹, X2 = X·X, opposite faces commute), real-cube facts (orders 6 / 105 / 63, checkerboard, superflip), each face by hand, and **a 3D-geometry reference model** that must match all 18 moves and 200 random sequences
- [x] Passing in debug, release, asan, tsan on macOS and gcc-14 / clang-18 on Linux (Docker rehearsal); clang-tidy clean; coverage 100% of lines, 94% of branches
- [x] Level doc: [LEVEL_2.md](levels/LEVEL_2.md)
- [ ] Draft PR, CI green
- [ ] IDE: debugging with CodeLLDB ([IDE_GUIDE.md](IDE_GUIDE.md), Level 2)

**Check:** hold a solved cube with white on top and green in front, do `R U R' U'`, and compare it with `cubesolver show --scramble "R U R' U'"`.

## Level 3 — Compact encodings

- [ ] `RubiksCube1dArray` (`char[54]`, index = face·9 + row·3 + col)
- [ ] `RubiksCubeBitboard`: 6 × `uint64_t`, 8 stickers × 8 bits per face (centres implied). A face's own turn is `std::rotl` by 16 bits; side strips moved with masks
- [ ] `operator==` and `std::hash` for each model (for `unordered_set` / `unordered_map`)
- [ ] Property tests: 10,000 random seeded sequences → all three models show the same colours
- [ ] Benchmarks: moves per second and bytes per state for each model → `docs/BENCHMARKS.md`
- [ ] Level doc

**Check:** benchmarks show the bitboard as the fastest model, and the cross-model property test passes under ASan.

## Level 4 — Uninformed search: BFS, DFS, IDDFS

- [ ] `Solver<Cube>` interface: `solve(cube) → Solution{moves, nodes, time}`, plus a node/depth limit and cancellation
- [ ] Move pruning: never turn the same face twice in a row; turn opposite faces in one fixed order (cuts the branching factor from 18 to ~13.35)
- [ ] `BFSSolver` (visited set + parent map), `DFSSolver` (depth-limited), `IDDFSSolver`
- [ ] Each solver is a template over the cube model, so the same algorithm runs on every model
- [ ] CLI: `cubesolver solve --algo bfs|dfs|iddfs --scramble "…"` printing the solution, node count and time
- [ ] Tests: every solver finds an optimal solution for every scramble of depth ≤ 5 in a seeded set; each returned solution really solves the cube
- [ ] Level doc

**Check:** solve a depth-5 scramble with each algorithm and compare node counts.

## Level 5 — Depth 8, fast (resume claim #1)

- [ ] `BidirectionalBFSSolver`: search outward from the scramble and from solved at the same time and meet in the middle (depth 8 = 4 + 4)
- [ ] Multithreaded DFS/IDDFS: split the work at the root across the 10 cores (`std::jthread`), with an atomic "found" flag to stop early
- [ ] Measure memory and know the limits: plain BFS stops at depth 6–7 (1.3 billion states at depth 8)
- [ ] Benchmark harness: 100 seeded depth-8 scrambles per algorithm → CSV + table in `BENCHMARKS.md`
- [ ] Level doc

**Check:** `cubesolver bench --claim depth8` reports median / p95 / max under 3 s for BFS (bidirectional), DFS and IDDFS.

## Level 6 — Cubie model + indexing

- [ ] `CubieCube`: corner permutation + orientation (8 × mod 3), edge permutation + orientation (12 × mod 2), moves as permutation tables
- [ ] Conversion between sticker models and the cubie model (with checks that the cube is solvable: parity, twist, flip)
- [ ] Permutation ranking (Lehmer code) in O(n) using a bitmask + `std::popcount`
- [ ] `NibbleArray`: 2 values per byte
- [ ] Tests: rank → unrank round trip for every permutation, conversion round trips
- [ ] Level doc

**Check:** a tool prints the corner index for solved = 0 and that rank/unrank round-trips all 40,320 corner permutations.

## Level 7 — Pattern databases

- [ ] Corner PDB: 8!·3⁷ = 88,179,840 states → 42 MB at 4 bits; max depth 11
- [ ] Two 6-edge PDBs: 12!/6!·2⁶ = 42,577,920 states each → 20 MB each
- [ ] Built by BFS over the index (multithreaded, level by level), with progress output
- [ ] File format: header (magic, version, kind, size) + data + checksum; `cubesolver build-pdb`; `mmap` loading
- [ ] Tests: depth counts per level match the known published distribution for the corner PDB
- [ ] Level doc

**Check:** `cubesolver build-pdb --all` finishes in a few minutes, and loading the files is near-instant afterwards.

## Level 8 — Korf's IDA* (resume claim #2)

- [ ] `IDAStarSolver`: f = g + h with h = max(corner, edge A, edge B), threshold raised to the smallest f that went over it
- [ ] Optimisations: move pruning, inline heuristic lookups, no allocation in the hot loop, optional root parallelism
- [ ] Optional: 7-edge PDBs (510M states, 255 MB each) for a stronger heuristic
- [ ] Benchmark: seeded depth-10…16 scrambles → median / p95 / max, nodes expanded, nodes/s
- [ ] Level doc

**Check:** `cubesolver bench --claim depth13` reports under 10 s (expected well under 1 s).

## Level 9 — ➕ Kociemba two-phase solver

- [ ] Phase 1 to the ⟨U, D, R2, L2, F2, B2⟩ subgroup, phase 2 to solved; coordinate move tables and pruning tables (a few MB)
- [ ] Solves any random state in about 20 moves within milliseconds
- [ ] Tests on 1,000 random-state scrambles
- [ ] Level doc

**Check:** scramble a real cube, type it in, and solve it with the printed moves.

## Level 10 — ➕ WebAssembly 3D demo

- [ ] `cubecore` compiled with Emscripten; bindings for apply / solve / scramble
- [ ] React + TS + three.js: animated 3D cube, scramble input or paint-the-stickers, pick an algorithm, play / step / reverse through the solution, see nodes and time
- [ ] Solving runs in a Web Worker; heavy PDBs stay CLI-only (Kociemba + IDDFS run in the browser)
- [ ] Deployed to GitHub Pages from CI
- [ ] Level doc

**Check:** open the Pages URL on your phone and solve a scramble.

## Level 11 — Hardening & polish

- [ ] Branch protection on `main`: every CI job required to merge
- [ ] README: architecture, benchmark tables/charts, how to reproduce the resume numbers
- [ ] Release workflow: macOS / Linux `cubesolver` binaries
- [ ] Final resume wording based on the measured numbers
- [ ] Level doc

**Check:** a fresh clone → one preset → all tests and benchmarks reproduce the README numbers.
