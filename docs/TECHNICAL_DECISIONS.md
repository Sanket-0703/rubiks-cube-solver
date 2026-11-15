# Technical Decisions, Blockers & Risks

Updated **2026-11-16**. Each decision has a recommendation. Items marked **⚠ Needs approval** are ones I won't act on without your explicit yes.

## 1. Blockers

| ID | Blocker | Status |
|---|---|---|
| B-1 | **The macOS 27 SDK can't be linked.** Command Line Tools 26.6 (`ld-1267`) are installed next to a newer `MacOSX27.0.sdk`, which `xcrun` picks by default. That SDK's `.tbd` files list an `arm64e.x1` architecture the linker doesn't know, so every link fails with `tapi error: malformed file … unknown architecture`. | **Worked around.** Build against `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk` (→ 26.5) by setting `SDKROOT`. Level 1's presets will pass it too. Real fix: update the Command Line Tools to 27 once Software Update offers them (not listed today), then drop the workaround. |

## 2. Decisions

### D-01 · Language standard — **C++20** ✅
C++20 gives `<bit>` (`std::rotl`, `std::popcount`, `std::countr_zero`), concepts (to constrain `Solver<Cube>`), `std::span` and `std::jthread`. C++23 would add little here, and Apple's libc++ doesn't have all of it yet.

### D-02 · Build system — **CMake + Ninja + presets** ✅
CMake is what most C++ jobs use, CLion and VS Code both read it, and Emscripten (Level 10) supports it through `emcmake`. Presets keep the build commands short and the same everywhere.

### D-03 · IDE — **VS Code** (clangd + CMake Tools + CodeLLDB) ✅
Already installed. clangd (rather than Microsoft's C/C++ IntelliSense) gives the same diagnostics as clang-tidy. CLion remains an option.

### D-04 · Cube models — several representations behind one abstract class ✅
- `RubiksCube` is an abstract base with virtual moves. It's used for readability, the CLI and tests.
- Solvers are **templates** over the concrete model (`Solver<RubiksCubeBitboard>`), so the hot loop never makes a virtual call and moves can be inlined. This is the main OOP-vs-performance trade-off to talk about in interviews: the interface stays polymorphic, and the search loop uses compile-time polymorphism.
- The sticker models (3D array, 1D array, bitboard) show how the encodings compare. The cubie model (Level 6) exists because pattern databases need a fast index for each state.

### D-05 · Making depth 8 under 3 s true ✅
- Positions at an exact distance from solved (counting half turns): d6 7.6M, d7 101M, **d8 1.33 billion**. Plain BFS can't store depth 8 in 16 GB, so the BFS claim is met with **bidirectional BFS** (about 43k states on each side at depth 4).
- DFS/IDDFS at depth 8 visit up to about 1.4 billion nodes even with move pruning. One core manages roughly 0.2 billion bitboard moves per second (to be measured), so we **split the search across the 10 cores**.
- Plain BFS is still built and benchmarked to show where it runs out of memory.

### D-06 · Pattern databases ✅
Korf's 1997 setup: the corner PDB plus two 6-edge PDBs, h = max of the three. That uses about 82 MB on disk, and building them takes minutes. Depth 13 should then take milliseconds, so the "under 10 s" claim has a large margin. 7-edge PDBs (2 × 255 MB) are optional.

### D-07 · GitHub repository  ⚠ Needs approval
- **Recommendation:** a public repo named `rubiks-cube-solver` under your account, created when Level 1 is pushed. The README links it from your resume.
- PDB files and build output are **never committed** (they're in `.gitignore`); CI rebuilds what it needs.

### D-08 · Web demo hosting (Level 10)  ⚠ Needs approval at Level 10
GitHub Pages, deployed from CI. No backend and no running cost.

## 3. Risks

| ID | Risk | Mitigation |
|---|---|---|
| R-1 | Benchmarks on this Mac don't reproduce the resume numbers | Measure in Levels 5 and 8; adjust the resume wording to the measured numbers (Level 11) |
| R-2 | Bitboard side-strip masks are easy to get subtly wrong | The 3D-array model is the reference; property tests compare every model after random sequences |
| R-3 | Building the PDBs is slow or uses too much RAM | 4-bit storage; build level by level over the index instead of keeping a queue; multithreaded; progress output |
| R-4 | Data races in the multithreaded search | Build `tsan` presets; read-only shared PDBs; per-thread cube copies; atomics only for found/node counts |
| R-5 | Large PDBs in the browser | Web demo uses Kociemba (small tables) + IDDFS; Korf's IDA* is CLI-only |
