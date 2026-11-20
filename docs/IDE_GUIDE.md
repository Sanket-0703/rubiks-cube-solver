# IDE Guide — VS Code for C++

Each level adds the IDE skills that level needs. Do the exercises yourself. They're how you learn the tools.

## Level 0 — Setup

### 1. Put `code` on your PATH
VS Code → `Cmd+Shift+P` → **Shell Command: Install 'code' command in PATH**. Open a new terminal and check:

```bash
code --version
```

### 2. Install the extensions

```bash
code --install-extension llvm-vs-code-extensions.vscode-clangd
```

```bash
code --install-extension ms-vscode.cmake-tools
```

```bash
code --install-extension vadimcn.vscode-lldb
```

| Extension | What it does |
|---|---|
| **clangd** | Code completion, go-to-definition, errors as you type, clang-tidy warnings inline. Reads `build/compile_commands.json` (made in Level 1) |
| **CMake Tools** | Picks a preset, builds, runs and debugs targets, runs tests in the Testing panel |
| **CodeLLDB** | The debugger: breakpoints, stepping, watching variables (Level 2) |

If the Microsoft **C/C++** extension is installed, set `"C_Cpp.intelliSenseEngine": "disabled"` so it doesn't fight clangd. You can keep it installed.

### 3. Point clangd at Homebrew's LLVM (optional, newer clangd)
Settings (`Cmd+,`) → search `clangd.path` → `/opt/homebrew/opt/llvm/bin/clangd`.

### 4. Open the project

```bash
code ~/rubiks-cube-solver
```

### Shortcuts to learn now
| Shortcut | Action |
|---|---|
| `Cmd+P` | Open a file by name |
| `Cmd+Shift+P` | Command palette (every command) |
| `` Ctrl+` `` | Show / hide the terminal |
| `Cmd+B` | Show / hide the sidebar |

## Level 1 — Presets, building, tests, navigation

The repo ships `.vscode/settings.json` (presets on, clangd as the C++ engine, format on save, CodeLLDB for debugging) and `.vscode/extensions.json`, which suggests the extensions when you open the folder.

### 1. Pick a preset and build
1. `code ~/rubiks-cube-solver`. CMake Tools asks for a **configure preset**: pick **Debug**. (Later: `Cmd+Shift+P` → **CMake: Select Configure Preset**.)
2. The status bar at the bottom now shows the preset, the build target and ⚙ **Build**. Click Build, or press `F7`.
3. Switch to **Release** and build. Notice it goes into `build/release/` and leaves `build/debug/` untouched.

### 2. Run tests from the Testing panel
1. Click the flask icon in the left bar. Expand `cubecore_tests` and you'll see the 5 GoogleTest tests and the 3 `cli.*` tests.
2. Run them all (▷▷ at the top), then run one on its own from the ▷ next to it.
3. Break one on purpose: in `tests/build_info_test.cpp` change `202002L` to `302002L`, run it, and read the failure message inline. Undo it.

### 3. Navigate with clangd
| Do this | Key |
|---|---|
| Go to definition: `buildInfo()` in `main.cpp` → `build_info.cpp` | `F12` (or `Cmd`+click) |
| Peek definition without leaving the file | `Option+F12` |
| Find all references to `isBenchmarkBuild` | `Shift+F12` |
| Switch between header and source | `Option+O` |
| Symbol in this file / in the whole project | `Cmd+Shift+O` / `Cmd+T` |
| Rename a symbol everywhere | `F2` |

### 4. See clang-tidy live
In `main.cpp`, rename `infoCmd` to `info_cmd`. clangd underlines it with `readability-identifier-naming`. Hover to read why, and use the 💡 quick fix (`Cmd+.`) to rename it back.

### 5. Terminal equivalents
```bash
cmake --workflow --preset debug
```

```bash
scripts/format.sh && scripts/tidy.sh
```

```bash
scripts/coverage.sh && open build/coverage/html/index.html
```
## Level 2 — Debugging with CodeLLDB

The repo now has `.vscode/launch.json` with two debug configurations. Both build first (`preLaunchTask: CMake: build`) using the **Debug** preset.

### 1. Step through a face turn
1. Open `cubecore/src/model/rubiks_cube_3d_array.cpp` and click left of the line number on the first line inside `turnR()`'s loop (`const Color up = u[i][2];`). A red dot appears.
2. Run and Debug panel (`Cmd+Shift+D`) → pick **cubesolver show (R U R' U')** → ▶ (or `F5`).
3. It stops in `turnR`. Look at the **Variables** panel: open `this` → `faces_` and you'll see the 6 × 3 × 3 sticker array, with `Color` values shown by name (`White`, `Green`, …).
4. Step over (`F10`) the four assignment lines and watch one column of three stickers move: F → U → B → D.
5. **Watch** panel → `+` → type `u[i][2]` and `b[2 - i][0]`. Step through `i = 0, 1, 2` and see why the B column is filled in reverse (`2 - i`).
6. Continue (`F5`) to finish, or Stop (`Shift+F5`).

### 2. Look up the call stack
At the breakpoint, the **Call Stack** panel shows `turnR` ← `RubiksCube3dArray::apply` ← `RubiksCube::applyMoves` ← `runShow` ← `main`. Click `applyMoves` to see which move of `R U R' U'` is being applied (`move` in Variables).

### 3. Debug a single test
1. **cubecore_tests (filter)** → ▶. It asks for a GoogleTest filter. The default is `CubeModelTest/0.RMovesFrontColumnUp`; try `*Superflip*`.
2. Set a breakpoint in the test body first, and you can inspect `cube` after each move.
3. Faster from the Testing panel: right-click a test → **Debug Test**.

### 4. Conditional breakpoint
Right-click the red dot → **Edit Breakpoint** → **Expression** `i == 2`. Now it only stops on the last sticker of each strip.

| Key | Action |
|---|---|
| `F5` | Start / continue |
| `F10` / `F11` / `Shift+F11` | Step over / into / out |
| `F9` | Toggle breakpoint |
| `Shift+F5` | Stop |
## Level 3 — (coming) Viewing memory and bits in the debugger, conditional breakpoints
## Level 5 — (coming) Debugging threads; profiling with Instruments (Time Profiler)
