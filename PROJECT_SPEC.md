# Project Specification: MiniLang Automata Compiler (Simplified C++17)

## Tech Stack & Build
- **Language:** C++17 (`-std=c++17 -Wall -Wextra`)
- **Build System:** Simple `Makefile` (no CMake)
- **Design:** Clean, readable `class`-based OOP using `std::unique_ptr` for AST nodes and standard containers (`std::vector`, `std::unordered_map`).
- **Visualization:** Simple ASCII tree in terminal + Graphviz `.dot` export (rendered to `ast.png` via `dot -Tpng`).

## Scope of MiniLang (Keep Minimal)
Only support:
- `int` variable declarations and assignments
- Arithmetic expressions with operator precedence (`+`, `-`, `*`, `/`, parentheses)
- Comparisons (`==`, `!=`, `<=`, `>=`, `<`, `>`)
- `if` / `else` blocks and `while` loops
- Single-line comments (`//`) and whitespace skipping

## Architecture
1. **Phase 1 — Lexer (`include/token.hpp`, `include/lexer.hpp`, `src/lexer.cpp`)**
   - Explicit DFA state machine (enum states + switch/loop). **Do NOT use `<regex>`.**
   - Keyword lookup table (`std::unordered_map`).
   - Exact `line` and `column` tracking for every token and lexical errors.

2. **Phase 2 — Parser (`include/parser.hpp`, `src/parser.cpp`)**
   - **Recursive Descent Parser** (one clear function per grammar rule) based on a left-recursion-free, left-factored LL(1) CFG.
   - **Simple Panic-Mode Error Recovery:** Print exact line/column error, then skip tokens until `;` or `}` so multiple errors are reported in one run.

3. **Phase 3 — AST & Visualizer (`include/ast.hpp`, `src/ast.cpp`)**
   - Build a clean in-memory AST for valid programs.
   - Print an indented ASCII tree and export a `.dot` file.
   - Include `scripts/render_ast.sh` to run `dot -Tpng ast.dot -o ast.png`.

4. **Deliverables**
   - CLI executable `./minic <file.mini>`
   - `docs/grammar_spec.md` (Regexes, DFA descriptions, normalized CFG, FIRST/FOLLOW sets, LL(1) table)
   - `tests/` (2 valid and 2 invalid `.mini` scripts)