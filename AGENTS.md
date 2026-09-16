# AGENTS.md

## Project Overview

`hellowinui` is a C++23 WinUI 3 desktop application. It is a minimal
Windows App SDK project whose UI is built entirely in C++, with no XAML
files.

## Build System

- **Generator:** Ninja
- **CMake minimum:** 3.21
- **C++ standard:** C++23
- **Project version:** 0.1.0, defined in `CMakeLists.txt`
- **Platform:** Windows only

### Commands

Configure:

```sh
cmake -S . -B build -GNinja
```

Build:

```sh
ninja -C build
```

Run from a Developer PowerShell or a configured C++ build environment:

```powershell
build\hellowinui.exe
```

There is currently no test suite or test target. Add tests to the build
configuration when the project gains testable functionality.

### Dependencies and CMake Modules

The project uses `fmt` for text formatting and WinUI 3 through the Windows
App SDK. The dependencies are fetched at configure time, so the first
configuration requires network access. A matching Windows App Runtime 2.4
installation is also required to run the unpackaged application.

The `deps/` directory contains local CMake modules used by the build:

- `Findfmt.cmake` fetches and exposes the `fmt::fmt` target.
- `FindWinUI.cmake` fetches the Windows App SDK packages, generates C++/WinRT
  headers from their metadata, and exposes the `WinUI::WinUI` target.
- `Platform.cmake` detects the target platform and compiler.
- `Flags.cmake` defines platform libraries, preprocessor definitions, and
  compiler warning options.

`deps/` is added to `CMAKE_MODULE_PATH` before these modules are included.
To add another dependency:

1. Add a `Find<name>.cmake` module that provides `<name>::<name>`.
2. Add `find_package(<name> REQUIRED)` to `CMakeLists.txt`.
3. Link the dependency with `target_link_libraries()`.

### Targets

The project currently defines one executable target:

- **`hellowinui`**: the application target, with its entry point in
  `helloworld.cpp`

### CMake Configuration

`CMakeLists.txt` enables compile command generation, includes the platform
and compiler flag modules from `deps/`, and applies C++23 and the shared
warning options to the WinUI executable. Keep new targets consistent with
this configuration.

## Coding Conventions

- **Language:** C++23
- Use standard library types directly unless the project introduces a
  project-wide type-alias header.
- **Trailing return type** for function signatures (e.g. `auto fn() -> void`)
- Opening braces go on the same line as the declaration
- **4-space indentation**
- Do not add semicolons after namespace closing braces; class definitions
  require a semicolon
- `auto` for obvious types (e.g. `auto main(...) -> int`)

Example:

```cpp
#include "fmt/format.h"

auto main() -> int {
    fmt::print("Hello, World!\\n");
    return 0;
}
```

- **East const** (e.g. `char const*` not `const char*`)
- **Trailing return type** for all function definitions, including
  operators (e.g. `auto operator=(T&&) noexcept -> T&`)
- **Public members first** in class declarations, private members at the
  bottom
- **Private data members** start with `m_` (e.g. `m_state`)
- `<>` includes only for system headers (std, OS, etc.)
- `""` includes for third-party dependencies (e.g. `fmt`,
  `nlohmann/json`)
- **Naming:** `snake_case` for variables, functions, and classes
- **Naming:** `SCREAMING_SNAKE_CASE` only for macros and constants
- **Header extensions:** C++ headers use `.hpp`, C headers use `.h`
- Include order:
  1. C++ standard library headers (`<chrono>`, `<vector>`, etc.)
  2. _(blank line)_
  3. C standard library headers (`<stdlib.h>`, `<string.h>`, etc.)
  4. _(blank line)_
  5. OS-specific headers (Windows API, POSIX, etc.)
  6. _(blank line)_
  7. Third-party dependencies (`"fmt/core.h"`, etc.)
  8. _(blank line)_
  9. Local/project headers

## Shell Scripts

- Always use `#!/bin/sh` shebang for shell scripts
- Scripts must be POSIX compliant (no bashisms)
- When providing commands to users:
  - Windows/PowerShell: use `` ` `` for line continuation
  - Unix/Linux/macOS: use `\` for line continuation

## Commit Messages

- Follow the 50/72 rule:
  - Subject line: max 50 characters
  - Body lines: wrapped at 72 characters
- Use conventional commit prefixes (`feat:`, `fix:`, `docs:`, `chore:`,
  `ci:`, etc.)
- Separate subject from body with a blank line
- Keep messages concise: no one wants to read a novel in the log
- Always include a `Co-Authored-By:` trailer listing every agent or
  model that contributed to the commit. Use one line per co-author:

  ```
  Co-Authored-By: qwen (lmstudio/qwen3.6-27b-mtp): wrote tests + build
  Co-Authored-By: luna (openai/gpt-5.6-luna): reviewed edge cases
  ```

Example:

```
feat: add stopwatch timer

Replace Hello World with a live stopwatch that prints elapsed time
in HH:MM:SS.mmm format, updating every 10ms with color output.
```

## Versioning

This project follows [Semantic Versioning](https://semver.org). The
version is set in the top-level `CMakeLists.txt` (`project(hellowinui
VERSION ...)`).

- **Patch** (`x.y.Z`): bump on every commit that changes code. No
  commit is too small.
- **Minor** (`x.Y.z`): bump when adding a feature or significant new
  capability.
- **Major** (`X.y.z`): bump on breaking API changes.

## Documentation (Markdown)

- Wrap normal text and lists at **max 80 columns** (for readability in
  terminals and editors).
- **Exceptions**: Tables and code blocks (` ``` `) can exceed 80
  columns when formatting requires it (e.g. trees, alignment).
- Use standard Markdown: `**bold**`, `` `inline code` ``, `##` headings,
  `-` or numbered lists, fenced code blocks with language hints
  (` ```cpp `, ` ```sh `).
- Keep examples concise, up-to-date, and self-documenting.
- This file (`AGENTS.md`) follows its own rules.
- Do not use em dashes (`—`). Use a colon or rewrite the sentence.

### Auto-Formatting with oxfmt

- All markdown files are formatted with **oxfmt**
- **Always run `npx oxfmt --write .` after editing any markdown file**
  - This is mandatory. Do not skip it. Run it before finishing your
    response
- Use `npx oxfmt --check .` to verify formatting before committing
- Config is in `.oxfmtrc.json` at the project root
- Do not manually fight the formatter. Let oxfmt handle spacing,
  line breaks, and list alignment

## Source Layout

```text
hellowinui/
  CMakeLists.txt      # Build configuration
  helloworld.cpp      # Application entry point
  winuiex.cpp         # Full-featured layout demo
  wui/                # Header-only declarative WinUI helpers (wui.hpp)
  deps/               # Dependency, platform, and compiler CMake modules
  build/              # Generated build tree, not source-controlled
  AGENTS.md           # Instructions for coding agents
```

## Behavioral Guidelines

Reduce common LLM coding mistakes. These guidelines bias toward caution
over speed. For trivial tasks, use judgment.

### 1. Think Before Coding

Don't assume. Don't hide confusion. Surface tradeoffs.

Before implementing:

- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them, don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

### 2. Simplicity First

Minimum code that solves the problem. Nothing speculative.

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?"
If yes, simplify.

### 3. Surgical Changes

Touch only what you must. Clean up only your own mess.

When editing existing code:

- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it, don't delete it.

When your changes create orphans:

- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

Every changed line should trace directly to the user's request.

### 4. Goal-Driven Execution

Define success criteria. Loop until verified.

Transform tasks into verifiable goals:

- "Add validation" means: write tests for invalid inputs, then make
  them pass.
- "Fix the bug" means: write a test that reproduces it, then make it
  pass.
- "Refactor X" means: ensure tests pass before and after.

For multi-step tasks, state a brief plan:

```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria
("make it work") require constant clarification.

## Platform Support

`Platform.cmake` detects Windows, Linux, macOS, iOS, Android, and Emscripten.
The WinUI application itself is Windows-only, and `CMakeLists.txt` rejects
non-Windows configurations.
