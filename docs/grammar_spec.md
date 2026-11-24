# MiniLang Formal Grammar & Automata Specification

This document provides the complete formal language specification for **MiniLang**, including token definitions via regular expressions, deterministic finite automaton (DFA) state transitions, a normalized LL(1) context-free grammar, FIRST/FOLLOW set derivations, and the conflict-free LL(1) parsing table.

---

## 1. Tokens and Formal Regular Expressions

MiniLang tokens are partitioned into keywords, identifiers, numeric literals, arithmetic operators, comparison operators, assignment, and punctuation delimiters.

### 1.1 Token Categories & Regular Expressions

| Token Name | Token Type | Formal Regular Expression | Examples | Description |
|---|---|---|---|---|
| `KW_INT` | Keyword | `int` | `int` | Integer type keyword |
| `KW_IF` | Keyword | `if` | `if` | Conditional branch keyword |
| `KW_ELSE` | Keyword | `else` | `else` | Alternative branch keyword |
| `KW_WHILE` | Keyword | `while` | `while` | Loop construct keyword |
| `ID` | Identifier | `[a-zA-Z_][a-zA-Z0-9_]*` (excl. keywords) | `x`, `sum_1`, `_val` | Variable and symbol names |
| `NUM` | Numeric Literal | `[0-9]+` | `0`, `42`, `1000` | Non-negative integer literals |
| `PLUS` | Arithmetic Op | `\+` | `+` | Addition |
| `MINUS` | Arithmetic Op | `-` | `-` | Subtraction |
| `STAR` | Arithmetic Op | `\*` | `*` | Multiplication |
| `SLASH` | Arithmetic Op | `/` (not followed by `/`) | `/` | Division |
| `ASSIGN` | Assignment Op | `=` (not followed by `=`) | `=` | Variable assignment |
| `EQ` | Comparison Op | `==` | `==` | Equal to |
| `NEQ` | Comparison Op | `!=` | `!=` | Not equal to |
| `LT` | Comparison Op | `<` (not followed by `=`) | `<` | Strictly less than |
| `LE` | Comparison Op | `<=` | `<=` | Less than or equal to |
| `GT` | Comparison Op | `>` (not followed by `=`) | `>` | Strictly greater than |
| `GE` | Comparison Op | `>=` | `>=` | Greater than or equal to |
| `LPAREN` | Delimiter | `\(` | `(` | Left grouping parenthesis |
| `RPAREN` | Delimiter | `\)` | `)` | Right grouping parenthesis |
| `LBRACE` | Delimiter | `\{` | `{` | Left block brace |
| `RBRACE` | Delimiter | `\}` | `}` | Right block brace |
| `SEMICOLON` | Delimiter | `;` | `;` | Statement terminator |
| `EOF_TOKEN` | Special | `\0` / end of stream | `EOF` | End-of-file marker |

### 1.2 Skipped Lexemes

Whitespace and single-line comments are discarded by the lexer:

| Construct | Formal Regular Expression | Action |
|---|---|---|
| Whitespace | `[ \t\r\n]+` | Advance line/column positions; discard |
| Line Comment | `//[^\n]*` | Consume up to newline; discard |

---

## 2. Lexical DFA (Deterministic Finite Automaton)

The lexer executes an explicit DFA state machine (implemented using an `enum class State` and `switch` statements without regex libraries).

### 2.1 Formal Definition

- **States $S$:** `{ START, IN_ID, IN_NUM, SAW_SLASH, IN_COMMENT, SAW_ASSIGN, SAW_EXCLAMATION, SAW_LT, SAW_GT }`
- **Start State $s_0$:** `START`
- **Input Alphabet $\Sigma$:** ASCII characters (letters, digits, whitespace, punctuation, operators, `\0`)
- **Accepting States / Actions:** Each state emits a token or transitions deterministically.

### 2.2 State Transition Diagram

```mermaid
stateDiagram-v2
    [*] --> START

    START --> START: [ \t\r\n] (skip / count line & col)
    START --> IN_ID: [a-zA-Z_]
    START --> IN_NUM: [0-9]
    START --> SAW_SLASH: '/'
    START --> SAW_ASSIGN: '='
    START --> SAW_EXCLAMATION: '!'
    START --> SAW_LT: '<'
    START --> SAW_GT: '>'
    START --> [*]: '+', '-', '*', ';', '(', ')', '{', '}', EOF

    IN_ID --> IN_ID: [a-zA-Z0-9_]
    IN_ID --> [*]: other (retract, check keyword map, emit ID or KW_*)

    IN_NUM --> IN_NUM: [0-9]
    IN_NUM --> [*]: other (retract, emit NUM)

    SAW_SLASH --> IN_COMMENT: '/'
    SAW_SLASH --> [*]: other (retract, emit SLASH)

    IN_COMMENT --> IN_COMMENT: [^\n\0]
    IN_COMMENT --> START: '\n' or EOF (discard comment)

    SAW_ASSIGN --> [*]: '=' -> emit EQ
    SAW_ASSIGN --> [*]: other -> retract, emit ASSIGN

    SAW_EXCLAMATION --> [*]: '=' -> emit NEQ
    SAW_EXCLAMATION --> [*]: other -> Lexical Error

    SAW_LT --> [*]: '=' -> emit LE
    SAW_LT --> [*]: other -> retract, emit LT

    SAW_GT --> [*]: '=' -> emit GE
    SAW_GT --> [*]: other -> retract, emit GT
```

### 2.3 Detailed State Transitions & Retract Policy

1. **`START`**
   - On `[ \t\r]`: Increment column, remain in `START`.
   - On `\n`: Increment line, reset column to 1, remain in `START`.
   - On `[a-zA-Z_]`: Append character to buffer, transition to `IN_ID`.
   - On `[0-9]`: Append character to buffer, transition to `IN_NUM`.
   - On `'/'`: Transition to `SAW_SLASH`.
   - On `'='`: Transition to `SAW_ASSIGN`.
   - On `'!'`: Transition to `SAW_EXCLAMATION`.
   - On `'<'`: Transition to `SAW_LT`.
   - On `'>'`: Transition to `SAW_GT`.
   - On single-char delimiters (`'+'`, `'-'`, `'*'`, `'('`, `')'`, `'{'`, `'}'`, `';'`): Emit respective token immediately (`PLUS`, `MINUS`, `STAR`, `LPAREN`, `RPAREN`, `LBRACE`, `RBRACE`, `SEMICOLON`).
   - On `\0` (EOF): Emit `EOF_TOKEN`.
   - Any other character: Emit lexical error with exact line and column.

2. **`IN_ID`**
   - On `[a-zA-Z0-9_]`: Append character, remain in `IN_ID`.
   - On any other character: Retract character (unget). Query keyword hash map:
     - `"int"` $\to$ `KW_INT`
     - `"if"` $\to$ `KW_IF`
     - `"else"` $\to$ `KW_ELSE`
     - `"while"` $\to$ `KW_WHILE`
     - otherwise $\to$ `ID`
     Emit token and return to `START`.

3. **`IN_NUM`**
   - On `[0-9]`: Append character, remain in `IN_NUM`.
   - On any other character: Retract character (unget), emit `NUM`, return to `START`.

4. **`SAW_SLASH`**
   - On `'/'`: Transition to `IN_COMMENT`.
   - On any other character: Retract character (unget), emit `SLASH` (`/`), return to `START`.

5. **`IN_COMMENT`**
   - On `[^\n\0]`: Consume character, remain in `IN_COMMENT`.
   - On `\n`: Increment line, reset column to 1, transition to `START`.
   - On `\0`: Transition to `START` (EOF terminates comment).

6. **`SAW_ASSIGN`**
   - On `'='`: Consume character, emit `EQ` (`==`), return to `START`.
   - On any other character: Retract character (unget), emit `ASSIGN` (`=`), return to `START`.

7. **`SAW_EXCLAMATION`**
   - On `'='`: Consume character, emit `NEQ` (`!=`), return to `START`.
   - On any other character: Lexical error (isolated `'!'` is illegal in MiniLang).

8. **`SAW_LT`**
   - On `'='`: Consume character, emit `LE` (`<=`), return to `START`.
   - On any other character: Retract character (unget), emit `LT` (`<`), return to `START`.

9. **`SAW_GT`**
   - On `'='`: Consume character, emit `GE` (`>=`), return to `START`.
   - On any other character: Retract character (unget), emit `GT` (`>`), return to `START`.

---

## 3. Context-Free Grammar (CFG) for MiniLang

MiniLang programs consist of a sequence of statements. The grammar below is strictly **left-recursion-free** and **left-factored**, making it suitable for top-down recursive descent LL(1) parsing.

### 3.1 Grammar Transformations Applied

1. **Elimination of Left Recursion:**
   - Arithmetic expressions (`Expr -> Expr + Term`) are converted into tail-recursive forms (`ArithExpr -> Term ArithExprTail`, `ArithExprTail -> + Term ArithExprTail | ε`).
   - Multiplication/division expressions are similarly transformed (`Term -> Factor TermTail`, `TermTail -> * Factor TermTail | ε`).
2. **Left Factoring:**
   - Optional variable initializations (`InitOpt -> = Expr | ε`).
   - Optional comparison operations (`ExprTail -> CompOp ArithExpr | ε`).
   - Optional else branches (`ElseOpt -> else Block | ε`).
   - Statement lists (`StmtList -> Stmt StmtList | ε`).
3. **Unambiguous `if-else` Construction:**
   - MiniLang enforces `{ ... }` blocks for control-flow branches (`Block`). Because `else` is not in the `FOLLOW` set of a statement inside the block, the traditional "dangling else" ambiguity is completely eliminated at the grammar level.

### 3.2 Numbered Production Rules

| Rule # | Production Rule |
|---|---|
| **(1)** | `Program -> StmtList $` |
| **(2)** | `StmtList -> Stmt StmtList` |
| **(3)** | `StmtList -> ε` |
| **(4)** | `Stmt -> VarDecl` |
| **(5)** | `Stmt -> AssignStmt` |
| **(6)** | `Stmt -> IfStmt` |
| **(7)** | `Stmt -> WhileStmt` |
| **(8)** | `VarDecl -> int ID InitOpt ;` |
| **(9)** | `InitOpt -> = Expr` |
| **(10)** | `InitOpt -> ε` |
| **(11)** | `AssignStmt -> ID = Expr ;` |
| **(12)** | `IfStmt -> if ( Expr ) Block ElseOpt` |
| **(13)** | `ElseOpt -> else Block` |
| **(14)** | `ElseOpt -> ε` |
| **(15)** | `WhileStmt -> while ( Expr ) Block` |
| **(16)** | `Block -> { StmtList }` |
| **(17)** | `Expr -> ArithExpr ExprTail` |
| **(18)** | `ExprTail -> CompOp ArithExpr` |
| **(19)** | `ExprTail -> ε` |
| **(20)** | `CompOp -> ==` |
| **(21)** | `CompOp -> !=` |
| **(22)** | `CompOp -> <=` |
| **(23)** | `CompOp -> >=` |
| **(24)** | `CompOp -> <` |
| **(25)** | `CompOp -> >` |
| **(26)** | `ArithExpr -> Term ArithExprTail` |
| **(27)** | `ArithExprTail -> + Term ArithExprTail` |
| **(28)** | `ArithExprTail -> - Term ArithExprTail` |
| **(29)** | `ArithExprTail -> ε` |
| **(30)** | `Term -> Factor TermTail` |
| **(31)** | `TermTail -> * Factor TermTail` |
| **(32)** | `TermTail -> / Factor TermTail` |
| **(33)** | `TermTail -> ε` |
| **(34)** | `Factor -> ID` |
| **(35)** | `Factor -> NUM` |
| **(36)** | `Factor -> ( Expr )` |

---

## 4. FIRST and FOLLOW Sets

### 4.1 Summary Table

| Non-Terminal | FIRST Set | FOLLOW Set |
|---|---|---|
| `Program` | `{ int, if, while, ID, $ }` | `{ $ }` |
| `StmtList` | `{ int, if, while, ID, ε }` | `{ }, $ }` |
| `Stmt` | `{ int, if, while, ID }` | `{ int, if, while, ID, }, $ }` |
| `VarDecl` | `{ int }` | `{ int, if, while, ID, }, $ }` |
| `InitOpt` | `{ =, ε }` | `{ ; }` |
| `AssignStmt` | `{ ID }` | `{ int, if, while, ID, }, $ }` |
| `IfStmt` | `{ if }` | `{ int, if, while, ID, }, $ }` |
| `ElseOpt` | `{ else, ε }` | `{ int, if, while, ID, }, $ }` |
| `WhileStmt` | `{ while }` | `{ int, if, while, ID, }, $ }` |
| `Block` | `{ { }` | `{ else, int, if, while, ID, }, $ }` |
| `Expr` | `{ (, ID, NUM }` | `{ ), ; }` |
| `ExprTail` | `{ ==, !=, <, <=, >, >=, ε }` | `{ ), ; }` |
| `CompOp` | `{ ==, !=, <, <=, >, >= }` | `{ (, ID, NUM }` |
| `ArithExpr` | `{ (, ID, NUM }` | `{ ==, !=, <, <=, >, >=, ), ; }` |
| `ArithExprTail` | `{ +, -, ε }` | `{ ==, !=, <, <=, >, >=, ), ; }` |
| `Term` | `{ (, ID, NUM }` | `{ +, -, ==, !=, <, <=, >, >=, ), ; }` |
| `TermTail` | `{ *, /, ε }` | `{ +, -, ==, !=, <, <=, >, >=, ), ; }` |
| `Factor` | `{ (, ID, NUM }` | `{ *, /, +, -, ==, !=, <, <=, >, >=, ), ; }` |

### 4.2 Derivation Verification

For each production $A \to \alpha$ where $\alpha \Rightarrow^* \varepsilon$:
1. **`StmtList`**: $\text{FIRST}(StmtList \setminus \{\varepsilon\}) \cap \text{FOLLOW}(StmtList) = \{ int, if, while, ID \} \cap \{ \}, \$ \} = \emptyset$.
2. **`InitOpt`**: $\text{FIRST}(= Expr) \cap \text{FOLLOW}(InitOpt) = \{ = \} \cap \{ ; \} = \emptyset$.
3. **`ElseOpt`**: $\text{FIRST}(else\ Block) \cap \text{FOLLOW}(ElseOpt) = \{ else \} \cap \{ int, if, while, ID, \}, \$ \} = \emptyset$.
4. **`ExprTail`**: $\text{FIRST}(CompOp\ ArithExpr) \cap \text{FOLLOW}(ExprTail) = \{ ==, !=, <, <=, >, >= \} \cap \{ ), ; \} = \emptyset$.
5. **`ArithExprTail`**: $\text{FIRST}(+ \dots) \cap \text{FIRST}(- \dots) \cap \text{FOLLOW}(ArithExprTail) = \{ + \} \cap \{ - \} \cap \{ ==, \dots, ), ; \} = \emptyset$.
6. **`TermTail`**: $\text{FIRST}(* \dots) \cap \text{FIRST}(/ \dots) \cap \text{FOLLOW}(TermTail) = \{ * \} \cap \{ / \} \cap \{ +, -, \dots, ), ; \} = \emptyset$.

All intersection tests are empty. Hence, the grammar satisfies the necessary and sufficient conditions for LL(1).

---

## 5. LL(1) Parsing Table

In the table below, rows correspond to non-terminals, columns correspond to terminals, and entries `P#` indicate the uniquely selected production rule from Section 3.2. Empty cells represent syntax errors.

| Non-Terminal | `int` | `if` | `else` | `while` | `ID` | `NUM` | `+` | `-` | `*` | `/` | `==` | `!=` | `<` | `<=` | `>` | `>=` | `=` | `;` | `(` | `)` | `{` | `}` | `$` |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **`Program`** | P1 | P1 | | P1 | P1 | | | | | | | | | | | | | | | | | | P1 |
| **`StmtList`** | P2 | P2 | | P2 | P2 | | | | | | | | | | | | | | | | | P3 | P3 |
| **`Stmt`** | P4 | P6 | | P7 | P5 | | | | | | | | | | | | | | | | | | |
| **`VarDecl`** | P8 | | | | | | | | | | | | | | | | | | | | | | |
| **`InitOpt`** | | | | | | | | | | | | | | | | | P9 | P10 | | | | | |
| **`AssignStmt`** | | | | | P11 | | | | | | | | | | | | | | | | | | |
| **`IfStmt`** | | P12 | | | | | | | | | | | | | | | | | | | | | |
| **`ElseOpt`** | P14 | P14 | P13 | P14 | P14 | | | | | | | | | | | | | | | | | P14 | P14 |
| **`WhileStmt`** | | | | P15 | | | | | | | | | | | | | | | | | | | |
| **`Block`** | | | | | | | | | | | | | | | | | | | | | P16 | | |
| **`Expr`** | | | | | P17 | P17 | | | | | | | | | | | | | P17 | | | | |
| **`ExprTail`** | | | | | | | | | | | P18 | P18 | P18 | P18 | P18 | P18 | | P19 | | P19 | | | |
| **`CompOp`** | | | | | | | | | | | P20 | P21 | P24 | P22 | P25 | P23 | | | | | | | |
| **`ArithExpr`** | | | | | P26 | P26 | | | | | | | | | | | | | P26 | | | | |
| **`ArithExprTail`** | | | | | | | P27 | P28 | | | P29 | P29 | P29 | P29 | P29 | P29 | | P29 | | P29 | | | |
| **`Term`** | | | | | P30 | P30 | | | | | | | | | | | | | P30 | | | | |
| **`TermTail`** | | | | | | | P33 | P33 | P31 | P32 | P33 | P33 | P33 | P33 | P33 | P33 | | P33 | | P33 | | | |
| **`Factor`** | | | | | P34 | P35 | | | | | | | | | | | | | P36 | | | | |

### 5.1 LL(1) Conflict Analysis
- **Total Conflicts:** Exactly **0** (no multiple-definition cells).
- Every cell contains at most one production.
- Deterministic, predictive parsing is guaranteed without backtracking.

---

## 6. Panic-Mode Synchronization Tokens (Phase 2 Preview)

To support multiple error reports per compiler invocation without crashing or looping indefinitely, recursive descent parsing functions in Phase 2 synchronize on well-defined tokens:

1. **Statement Synchronization Tokens:**
   - **Terminator:** `;` (advance past `;` to recover at the next statement).
   - **Block Boundary:** `}` (synchronize to recover at the enclosing block level).
2. **Expression Synchronization Tokens:**
   - `)` or `;`.
