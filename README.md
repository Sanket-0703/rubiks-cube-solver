# Rubik's Cube Solver

A 3x3 Rubik's Cube solver in modern C++20, with several cube encodings (3D array, 1D array, bitboard, cubie) behind one OOP interface and a series of search algorithms:

- **Uninformed search:** BFS, depth-limited DFS, IDDFS, bidirectional BFS, multithreaded
- **Heuristic search:** Korf's IDA* with corner and edge pattern databases
- **Any scramble:** Kociemba's two-phase algorithm
- **Web demo:** the same core compiled to WebAssembly, with an animated 3D cube

> 🚧 Under construction, built level by level. See [docs/BUILD_PLAN.md](docs/BUILD_PLAN.md).

## Docs
- [Build plan](docs/BUILD_PLAN.md) — levels, checks, progress
- [Technical decisions](docs/TECHNICAL_DECISIONS.md) — choices, blockers, risks
- [IDE guide](docs/IDE_GUIDE.md) — VS Code for C++
