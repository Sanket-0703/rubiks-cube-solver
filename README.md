# Rubik's Cube Solver

A 3x3 Rubik's Cube solver in modern C++20, with several cube encodings (3D array, 1D array, bitboard, cubie) behind one OOP interface and a series of search algorithms:

- **Uninformed search:** BFS, depth-limited DFS, IDDFS, bidirectional BFS, multithreaded
- **Heuristic search:** Korf's IDA* with corner and edge pattern databases
- **Any scramble:** Kociemba's two-phase algorithm
- **Web demo:** the same core compiled to WebAssembly, with an animated 3D cube

> 🚧 Under construction, built level by level. See [docs/BUILD_PLAN.md](docs/BUILD_PLAN.md).

## Build and test

Requires CMake ≥ 3.25, Ninja and a C++20 compiler (clang ≥ 16, gcc ≥ 13, or AppleClang).

```bash
cmake --workflow --preset debug      # configure + build + test
cmake --workflow --preset release    # -O3 + LTO, for benchmarks
cmake --workflow --preset asan       # AddressSanitizer + UBSan
./build/release/apps/cubesolver/cubesolver info
```

## Try it

```bash
cubesolver show --scramble "R U R' U'"     # draw the cube after a sequence
cubesolver scramble --length 20 --seed 42  # a repeatable random scramble
cubesolver show -s "$(cubesolver scramble --seed 7)" --facelets
```

Code checks: `scripts/format.sh --check`, `scripts/tidy.sh`, `scripts/coverage.sh --fail-under 85`.

## Docs
- [Build plan](docs/BUILD_PLAN.md) — levels, checks, progress
- [Technical decisions](docs/TECHNICAL_DECISIONS.md) — choices, blockers, risks
- [IDE guide](docs/IDE_GUIDE.md) — VS Code for C++
- Level explainers: [Level 1](docs/levels/LEVEL_1.md) · [Level 2](docs/levels/LEVEL_2.md)
