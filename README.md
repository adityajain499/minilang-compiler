# MiniLang Automata Compiler (C++17)

A clean, modular, and minimal compiler front-end for **MiniLang** built in C++17 without external parser/lexer generator tools.

## Architecture & Features

1. **Phase 1 — Lexical Analyzer (Scanner):**
   - Explicit Deterministic Finite Automaton (DFA) state machine (`enum class DFAState` + `switch`).
   - Keyword lookup via `std::unordered_map` (`int`, `if`, `else`, `while`).
   - Exact 1-indexed `line` and `column` tracking for all tokens and lexical diagnostics.
   - Skips whitespace and `//` single-line comments.
   - Zero regex dependencies (no `<regex>`).

2. **Phase 2 — Recursive Descent Parser & AST:**
   - 1:1 mapping with the normalized LL(1) grammar in [`docs/grammar_spec.md`](docs/grammar_spec.md).
   - Strict operator precedence (`*`, `/` over `+`, `-`, and comparisons).
   - Left-associative binary expression trees: `(a - b) - c`.
   - **Panic-Mode Error Recovery:** Automatically skips tokens until synchronization boundaries (`;`, `}`, or statement keywords) to report multiple syntax errors in a single compilation run.
   - Memory management using `std::unique_ptr` for all AST nodes.

3. **Phase 3 — AST Visualization:**
   - **ASCII Tree Printer:** Human-readable indented tree printed directly to the terminal.
   - **Graphviz DOT Exporter:** Color-coded, directed AST graph written to `ast.dot`.
   - **PNG Renderer:** Helper script [`scripts/render_ast.sh`](scripts/render_ast.sh) uses Graphviz `dot -Tpng` to render `ast.png`.

---

## Directory Layout

```
.
├── Makefile               # Build script (clang++ -std=c++17 -Wall -Wextra -Iinclude)
├── PROJECT_SPEC.md        # Original project specification
├── README.md              # Project documentation
├── docs/
│   └── grammar_spec.md    # Formal tokens, DFA transitions, CFG, FIRST/FOLLOW, and LL(1) table
├── include/
│   ├── ast.hpp            # AST node classes & ASTVisitor interface
│   ├── lexer.hpp          # Lexer DFA definition
│   ├── parser.hpp         # Recursive descent parser & panic-mode recovery
│   ├── token.hpp          # Token class and TokenType enum
│   └── visualizer.hpp     # AsciiTreePrinter and DotExporter visitors
├── src/
│   ├── ast.cpp            # Visitor accept implementations
│   ├── lexer.cpp          # Character-by-character DFA scanner
│   ├── main.cpp           # CLI driver
│   ├── parser.cpp         # Recursive descent parsing methods
│   └── visualizer.cpp     # ASCII printer and DOT generator
├── tests/
│   ├── valid1.mini        # Valid test: arithmetic, precedence, if/else
│   ├── valid2.mini        # Valid test: while loop, accumulator, comparisons
│   ├── invalid1_lex.mini  # Invalid test: lexical error ('@')
│   └── invalid2_syntax.mini # Invalid test: syntax errors for panic recovery
└── scripts/
    └── render_ast.sh      # Bash script to render .dot files to .png
```

---

## Prerequisites

- **C++ Compiler:** `clang++` (or `g++`) with C++17 support.
- **Build Tool:** GNU `make`.
- **Graphviz (optional, for PNG rendering):** `dot` executable (install via `brew install graphviz` on macOS or `sudo apt-get install graphviz` on Ubuntu).

---

## Build Instructions

Compile the compiler binary `./minic`:

```bash
make clean && make
```

Build flags enabled: `-std=c++17 -Wall -Wextra -Iinclude`.

---

## Usage

Run the compiler on a `.mini` source file:

```bash
./minic <file.mini> [output.dot]
```

- When compilation succeeds:
  1. Displays the token stream `[Line:Col] (TOKEN_TYPE, "lexeme")`.
  2. Displays `Syntax Analysis: PASSED`.
  3. Prints the ASCII AST tree to the terminal.
  4. Exports the AST in Graphviz format to `ast.dot` (or your chosen `output.dot`).

- When errors occur:
  - Reports all lexical and syntax errors with exact line and column numbers.
  - Exits with non-zero exit code and does not create/overwrite `ast.dot`.

---

## Running the Test Suite

### 1. Valid Programs

```bash
# Valid Test 1: Arithmetic expressions & if-else
./minic tests/valid1.mini

# Valid Test 2: While loop with accumulator & comparisons
./minic tests/valid2.mini
```

### 2. Invalid Programs (Error Reporting & Recovery)

```bash
# Lexical error demonstration:
./minic tests/invalid1_lex.mini

# Panic-mode syntax recovery demonstration (detects multiple errors):
./minic tests/invalid2_syntax.mini
```

---

## Rendering the AST Graph (PNG)

After running a valid program, render `ast.dot` to `ast.png`:

```bash
./scripts/render_ast.sh ast.dot ast.png
```

Or run directly:

```bash
dot -Tpng ast.dot -o ast.png
```
