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

## Level 1 — (coming) CMake Tools presets, building, running tests from the Testing panel, clangd navigation
## Level 2 — (coming) Debugging with CodeLLDB: breakpoints in a move function, watching the sticker array
## Level 3 — (coming) Viewing memory and bits in the debugger, conditional breakpoints
## Level 5 — (coming) Debugging threads; profiling with Instruments (Time Profiler)
