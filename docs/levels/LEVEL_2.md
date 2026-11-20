# Level 2 — The Cube Model (OOP)

Branch `level-2-cube-model`. Built **2026-11-21**.

Level 2 gives the project a cube. It defines the vocabulary (faces, colours, the 18 moves), an abstract `RubiksCube` interface, and the first model, `RubiksCube3dArray`, whose six face turns are written out by hand. Around it: a notation parser (`R U R' U'`), seeded scrambles that come out the same on every platform, 54-character facelet strings, and a terminal renderer. Two independent derivations check that the moves are right: the hand-written model and a 3D-geometry model in the tests must agree on every move.

---

## 1. The big picture

```
            cubecore/model/                         cubecore/notation/        cubecore/render/
 ┌──────────────────────────────────────┐    ┌────────────────────────┐   ┌──────────────┐
 │ face.hpp    Face, Color, opposite()  │    │ notation  parseMoves   │   │ net  render  │
 │ move.hpp    Move (18), inverse()     │◄───│           formatMoves  │   │      Net()   │
 │ rubiks_cube.hpp     «interface»      │    │           invertMoves  │   └──────┬───────┘
 │     ▲                                │    │ scramble  randomScramble│          │
 │     │ final                          │    └───────────┬────────────┘          │
 │ rubiks_cube_3d_array   faces_[6][3][3]│                │ SplitMix64 (util/)    │
 │ facelets.hpp  "UUUUUUUUURRR…"        │                ▼                       ▼
 └──────────────────────────────────────┘          apps/cubesolver: show · scramble · info
```

## 2. Conventions: getting the geometry right once

Every later level (bitboards, cubie model, pattern databases, Kociemba) depends on these definitions, so they're fixed here and tested hard.

### 2.1 Faces and colours

```cpp
enum class Face : std::uint8_t { U, R, F, D, L, B };            // URFDLB
enum class Color : std::uint8_t { White, Red, Green, Yellow, Orange, Blue };
```

- **URFDLB order** is the order Kociemba's facelet strings use (Level 9). It also puts opposite faces three apart, so `opposite(f) = (f + 3) % 6`, with no lookup table.
- Colours are listed in the same order, so the solved colour of face `f` is `Color(f)`: `solvedColor` and `homeFace` are just casts.
- Enums have an explicit `std::uint8_t` base. A sticker is one byte, which matters in Level 3 when we count bytes per state.

### 2.2 How each face's grid is oriented

Every face is a 3×3 grid **seen from outside the cube**, row 0 at the top and column 0 on the left, oriented as in the unfolded net:

```
            U          U: B side at the top, F side at the bottom
         L  F  R  B    L, F, R, B: U side at the top
            D          D: F side at the top, B side at the bottom
```

So for R, column 0 is next to F. For B, column 0 is next to R, because you're looking at the back from behind. This is the standard layout, and it's the one Kociemba's facelet format uses, so our facelet strings can go straight into other cube tools.

Because every face is seen from outside, "clockwise" means the same thing on every grid: `new[r][c] = old[2 - c][r]`. That one function, `rotateFaceClockwise`, serves all six faces.

### 2.3 The 18 moves

```cpp
enum class Move : std::uint8_t { U, U2, UPrime, R, R2, RPrime, F, ..., B, B2, BPrime };
```

Face-major layout: `index = 3 · face + (quarterTurns − 1)`. Everything is `constexpr` arithmetic on that index:

| Function | Example | How |
|---|---|---|
| `face(m)` | `face(FPrime) == F` | `index / 3` |
| `quarterTurns(m)` | U → 1, U2 → 2, U' → 3 | `index % 3 + 1` |
| `makeMove(f, t)` | `makeMove(L, -1) == L'` | `t` taken mod 4 |
| `inverse(m)` | U ↔ U', U2 ↔ U2 | `makeMove(face, 4 − turns)` |

Since they're `constexpr`, they can be checked at compile time: `static_assert(inverse(Move::R) == Move::RPrime)`.

This is the **half-turn metric** (HTM): U2 counts as one move. All the depth numbers on the resume (depth 8, depth 13) are in HTM.

## 3. The OOP design

### 3.1 `RubiksCube`: the interface

```cpp
class RubiksCube {
public:
    virtual ~RubiksCube() = default;
    virtual Color getColor(Face, size_t row, size_t col) const = 0;
    virtual void  setColor(Face, size_t row, size_t col, Color) = 0;
    virtual void  apply(Move) = 0;
    void applyMoves(std::span<const Move>);   // non-virtual: loops over apply()
    virtual bool isSolved() const;            // default via getColor; models may override
    Color centerColor(Face) const;
protected:
    RubiksCube(const RubiksCube&) = default;  // + move, assignment: protected
    ...
};
```

Design decisions:

- **Pure virtuals for the three things that depend on storage** (read a sticker, write a sticker, turn a face). Everything else is written **once in the base class** in terms of those: `applyMoves`, `isSolved`, `centerColor`, plus free functions `sameStickers`, `toFaceletString`, `loadFaceletString` and `renderNet`. A new model only has to implement three functions to get the whole toolkit.
- **`isSolved` is virtual with a default.** The generic version calls `getColor` 54 times through virtual dispatch. A model can override it with something faster. The bitboard in Level 3 will compare six integers.
- **Copy and move are protected.** `RubiksCube a = someBitboardCube;` would *slice*: it would copy the base part (nothing) and lose the stickers. Making the base's copy operations protected turns that into a compile error. Concrete models stay freely copyable, which solvers need.
- **A virtual destructor**, so deleting a model through a `RubiksCube*` is safe. `-Wnon-virtual-dtor` enforces this.
- **`final` on the concrete model.** When the compiler knows the exact type, it can call `apply` directly (devirtualisation). **The solvers (Level 4) will be templates over the concrete model** (`Solver<RubiksCubeBitboard>`), so the hot loop has no virtual calls and moves can be inlined. The virtual interface is for everything that isn't hot: printing, the CLI, tests comparing models. That's the central "OOP vs performance" trade-off of the project.

### 3.2 `RubiksCube3dArray`: the reference model

```cpp
std::array<std::array<std::array<Color, 3>, 3>, 6> faces_;   // faces_[face][row][col], 54 bytes
```

It's the most readable model, which is why it's the **reference** the faster Level 3 models are tested against. `apply(Move)` runs `turnX()` once for X, twice for X2, three times for X'. Simple and obviously correct; speed is Level 3's job.

Each `turnX()` does two things:
1. `rotateFaceClockwise(X)`: the 9 stickers on the face itself.
2. A 4-cycle of the 12 stickers on the ring around it: four strips of three, each moving to the next face.

Take `turnR()`:

```cpp
for (std::size_t i = 0; i < 3; ++i) {
    const Color up = u[i][2];
    u[i][2] = f[i][2];        // F's right column goes up to U
    f[i][2] = d[i][2];        // D's right column comes up to F
    d[i][2] = b[2 - i][0];    // B's left column comes down to D, upside down
    b[2 - i][0] = up;         // U's right column goes over the top to B, upside down
}
```

- The cycle is copied **backwards** (U ← F ← D ← B ← saved U), so only one temporary is needed.
- **`2 - i` is the subtle part.** B's grid is seen from behind, so a strip that goes over the top of the cube onto B arrives upside down: U's row 0 (at the back) ends up at B's row 2. Get one of these reversals wrong, and turning from solved still looks right, because every strip is one colour. That's why the tests below check the moves on *scrambled* cubes and against geometry.

## 4. Text formats

### 4.1 Notation: `parseMoves` / `formatMoves` / `invertMoves`

- Grammar: a face letter, then nothing, `2` or `'`. Spaces and commas are optional (`RUR'U'` works).
- Variants copied from websites are accepted: the typographic apostrophe `’` (3 bytes of UTF-8, handled with `starts_with`) and `U2'`.
- **Errors carry a byte position.** `NotationError` derives from `std::invalid_argument` and adds `position()`, and the CLI draws a caret:
  ```
  error: unexpected character 'X'
    R U X
        ^
  ```
- Known non-face moves get a specific message: wide moves (`r`), slices (`M E S`) and rotations (`x y z`) aren't supported yet.
- `invertMoves` reverses the sequence and inverts each move: (R U)⁻¹ = U' R'. It's the basis of "scramble, then apply the inverse, must be solved" tests.

### 4.2 Facelet strings

`UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`: the faces in URFDLB order, each read row by row, with each sticker written as the letter of the face whose colour it shows. `loadFaceletString` checks the length, the letters and that there are nine of each letter **before** painting anything, so a bad string leaves the cube unchanged (the strong exception guarantee). Whether a colouring can actually be reached by turning (no twisted corner, no flipped edge) is checked in Level 6.

### 4.3 The net renderer

`renderNet(cube, NetStyle::Plain | NetStyle::Ansi256)` draws the cross. ANSI style uses the 256-colour palette (`ESC[48;5;Nm`), because the basic 16 colours have no orange. The CLI picks a style with `--color auto|always|never`. `auto` means colour only when stdout is a terminal (`isatty`) and the `NO_COLOR` environment variable isn't set (the no-color.org convention).

## 5. Seeded scrambles: `SplitMix64` + `randomScramble`

**The problem:** a benchmark says "100 scrambles of depth 8, seed 1…100". Those must be *the same scrambles* on your Mac, on Linux CI and on anyone's machine. `std::mt19937_64` is fully specified, but `std::uniform_int_distribution` is not: libc++ and libstdc++ turn the same engine output into different numbers.

**The fix:** a 20-line SplitMix64 generator (Steele, Lea & Flood) plus our own `below(bound)`, all fixed-width integer maths:

```cpp
state += 0x9E3779B97F4A7C15;                       // Weyl sequence
z = (state ^ (state >> 30)) * 0xBF58476D1CE4E5B9;  // two multiply-xorshift mixing rounds
z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
return z ^ (z >> 31);
```

`below(bound)` avoids **modulo bias**. 2⁶⁴ isn't a multiple of 6, so `next() % 6` would give 0–3 very slightly more often. Rejecting the `2⁶⁴ mod bound` smallest values makes the result exactly uniform. `(0 - bound) % bound` computes `2⁶⁴ mod bound` without overflow, using unsigned wrap-around.

The tests check the first three outputs against Vigna's reference implementation, and a `static_assert` proves it works at compile time. Golden scramble values (`seed 42 → R2 U L R' F' L' R' F' D U`) pass on macOS/libc++ and Linux/libstdc++.

`randomScramble(length, seed)` follows the official random-move rules: never the same face twice in a row (U U' cancels out) and never `X Y X` with Y opposite X (U D U is just U2 D). Otherwise a "length 20" scramble would really be shorter.

## 6. Testing: how we know the moves are right

73 tests (64 GoogleTest + 9 CLI). The cube tests are **typed tests**:

```cpp
using CubeModels = ::testing::Types<cubecore::RubiksCube3dArray>;
TYPED_TEST_SUITE(CubeModelTest, CubeModels);
```

In Level 3, adding `RubiksCube1dArray` and `RubiksCubeBitboard` to that list runs *every* cube test against the new models, with no new test code.

Four independent kinds of evidence:

| Evidence | Tests | What it catches |
|---|---|---|
| **Algebra** | X⁴ = id, X·X⁻¹ = id from scrambled states, X2 = X·X, X' = X³, opposite faces commute, scramble · inverse = solved | moves that aren't proper rotations or inverses |
| **Known facts about the real cube** | order of `R U R' U'` = 6, `R U` = 105, `R U'` = 63, `R2 U2` = 6; the checkerboard pattern; the 20-move superflip (all corners solved, all 12 edges flipped); centres never move; 9 stickers of each colour | stickers moved to the wrong place or twisted. These numbers are properties of the real puzzle: a model with one reversed strip gets a different order |
| **Each turn by hand** | after U, F's top row is red; after R, U's right column is green; … (all 6 faces) | the wrong direction or the wrong neighbour |
| **3D geometry** | every one of the 18 moves, and 200 random 25-move sequences, sticker for sticker | any transcription error in the hand-written strips |

### The geometry check (`tests/support/sticker_geometry.cpp`)

This is a completely different way to turn the cube. Each sticker gets a 3D position in {−1, 0, 1}³ and an outward normal (x right, y up, z towards you). A face turn rotates every sticker whose position lies in that layer by −90° about the face's normal, using Rodrigues' formula with cos = 0 and sin = −1:

```
v' = n (n · v) − n × v
```

Then the new position and normal are converted back to (face, row, col). No table of "which sticker goes where" is written by hand; it all comes out of the geometry. If the hand-written `turnB` disagreed with it on even one sticker, `RandomSequencesMatchGeometry` would fail. They agree on all 18 moves and 5,000 random moves.

## 7. Code quality changes

- clang-tidy: three style checks turned off, with reasons in `.clang-tidy`. `readability-math-missing-parentheses` fired on every `face * 9 + row * 3 + col`, `modernize-use-designated-initializers` wanted `{.x=0, .y=1, .z=0}` for 3D vectors, and `readability-trailing-comma` duplicates clang-format's job.
- Real findings fixed: `NetStyle` got a `std::uint8_t` base (`performance-enum-size`). `loadFaceletString` no longer reads an optional value without checking it (`bugprone-unchecked-optional-access`): it now parses once into an array and paints from that.
- Coverage: **100% of lines and 94% of branches** in `cubecore`.

## 8. The CLI

```bash
cubesolver show --scramble "R U R' U'"           # net in colour (if a terminal) + "Solved: no"
cubesolver show -s "R U R' U'" --color never --facelets
cubesolver scramble --length 20 --seed 42         # R2 U L R' F' L' R' F' D U ...
cubesolver scramble                               # random seed, printed to stderr
cubesolver show -s "$(cubesolver scramble --seed 7)"
```

`scramble` prints only the moves to stdout. The random seed goes to stderr, so `$(...)` captures just the moves and you can still repeat a run.

## 9. Explain it in an interview

> **"How did you model the cube?"**
> An abstract `RubiksCube` class with three pure virtuals: read a sticker, write a sticker, apply a move. Everything else (isSolved, printing, facelet strings, comparing two models) is written once against that interface. The first model is a 6×3×3 array with each face turn written by hand: rotate the face's own 3×3 grid, then cycle four strips of three stickers around it. Later models swap in compact encodings behind the same interface.

> **"Isn't virtual dispatch slow for a solver?"**
> Yes, which is why the solvers don't use it. The concrete models are `final`, and the solvers are templates over the concrete type, so moves are direct, inlinable calls. The virtual interface is only for code that isn't hot. It's the classic choice between runtime and compile-time polymorphism, made per use.

> **"How do you know your move implementation is correct?"**
> Four independent checks. Group algebra (every move to the 4th power is the identity, opposite faces commute). Known properties of the real puzzle: the order of `R U` is 105, and the superflip and checkerboard come out right. Per-face expectations. And a second model built from 3D geometry, where each sticker has a position and a normal and a turn is a 90° rotation matrix. The hand-written model must match it sticker for sticker on random sequences. A strip copied in the wrong direction passes the solved-state checks but fails the geometry check.

> **"Why write your own random number generator?"**
> Reproducibility. `std::uniform_int_distribution` isn't specified exactly, so libc++ and libstdc++ give different numbers from the same seed, and the benchmark scrambles would differ between macOS and Linux. SplitMix64 with rejection sampling is a few lines of fixed-width integer maths, identical everywhere and free of modulo bias. CI checks golden values on both platforms.

> **"Why protected copy constructors on the base class?"**
> To prevent slicing. Copying a derived cube into a `RubiksCube` by value would silently drop its data. With the base's copy operations protected, that's a compile error, and the concrete classes are still copyable.

## 10. Files in this level

| File | Purpose |
|---|---|
| `cubecore/include/cubecore/model/face.hpp` | `Face`, `Color`, `opposite`, `solvedColor`, char conversions |
| `cubecore/include/cubecore/model/move.hpp`, `src/model/move.cpp` | `Move` (18), `face`, `quarterTurns`, `makeMove`, `inverse`, `toString` |
| `cubecore/include/cubecore/model/rubiks_cube.hpp`, `src/model/rubiks_cube.cpp` | the abstract interface, `isSolved`, `sameStickers` |
| `cubecore/include/cubecore/model/rubiks_cube_3d_array.hpp`, `src/model/rubiks_cube_3d_array.cpp` | the reference model |
| `cubecore/include/cubecore/model/facelets.hpp`, `src/model/facelets.cpp` | 54-character facelet strings |
| `cubecore/include/cubecore/notation/notation.hpp`, `src/notation/notation.cpp` | parse / format / invert move sequences |
| `cubecore/include/cubecore/notation/scramble.hpp`, `src/notation/scramble.cpp` | seeded random-move scrambles |
| `cubecore/include/cubecore/util/split_mix64.hpp` | portable seeded random numbers |
| `cubecore/include/cubecore/render/net.hpp`, `src/render/net.cpp` | plain and ANSI net |
| `apps/cubesolver/main.cpp` | `show`, `scramble` subcommands |
| `tests/model/*`, `tests/notation/*`, `tests/render/*`, `tests/util/*` | unit tests |
| `tests/support/sticker_geometry.*` | the 3D-geometry reference model |
| `tests/CMakeLists.txt` | +6 CLI tests (9 in total) |
| `.vscode/launch.json` | debug `cubesolver show` and filtered tests with CodeLLDB |
| `.clang-tidy` | three style checks off, with reasons |
