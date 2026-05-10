<p align="center">
  <img src="https://img.shields.io/badge/C++-17-blue.svg?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++17" />
  <img src="https://img.shields.io/badge/Build-Passing-brightgreen?style=for-the-badge&logo=githubactions&logoColor=white" alt="Build" />
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge" alt="License" />
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=for-the-badge" alt="Platform" />
</p>

<h1 align="center">⚙️ CVM++</h1>

<p align="center">
  <strong>A Stack-Based Virtual Machine with a Custom Compiler — written in pure C++17</strong>
</p>

<p align="center">
  <em>Lexer → Parser → Compiler → Bytecode → VM — from source to execution in one pipeline.</em>
</p>

---

## 🧠 What is CVM++?

**CVM++** is a fully self-contained programming language implementation built from scratch in C++. It features a hand-written **lexer**, a **recursive-descent parser**, a **bytecode compiler**, and a **stack-based virtual machine** — all without any external dependencies.

Write `.cvm` scripts with variables, arithmetic, booleans, control flow, and I/O — and watch them execute through a real compilation pipeline.

---

## 🏗️ Architecture

```
┌──────────────────────────────────────────────────────────┐
│                     CVM++ Pipeline                       │
├──────────────────────────────────────────────────────────┤
│                                                          │
│   Source Code (.cvm)                                     │
│        │                                                 │
│        ▼                                                 │
│   ┌─────────┐    Scans characters into tokens            │
│   │  Lexer  │──────────────────────────────► Tokens       │
│   └─────────┘                                            │
│        │                                                 │
│        ▼                                                 │
│   ┌─────────┐    Recursive-descent parsing               │
│   │ Parser  │──────────────────────────────► AST          │
│   └─────────┘                                            │
│        │                                                 │
│        ▼                                                 │
│   ┌──────────┐   Walks AST, emits opcodes                │
│   │ Compiler │─────────────────────────────► Bytecode     │
│   └──────────┘                                           │
│        │                                                 │
│        ▼                                                 │
│   ┌─────────┐    Fetch-decode-execute loop               │
│   │   VM    │──────────────────────────────► Output       │
│   └─────────┘                                            │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

---

## 📁 Project Structure

```
CVM++/
├── lexer.h          # Token definitions & Lexer class declaration
├── lexer.cpp         # Tokenizer implementation
├── parser.h          # AST node hierarchy & Parser declaration
├── parser.cpp        # Recursive-descent parser
├── compiler.h        # Opcodes & Compiler declaration
├── compiler.cpp      # AST → Bytecode compilation
├── vm.h              # VM class declaration
├── vm.cpp            # Stack-based bytecode interpreter
├── main.cpp          # CLI entry point
├── CMakeLists.txt    # CMake build configuration
├── main.cvm          # Sample CVM++ script
└── README.md
```

> Each file has **one responsibility** — clean separation of concerns across the entire pipeline.

---

## 🚀 Getting Started

### Prerequisites

- **g++** (GCC 7+ with C++17 support) or any C++17 compatible compiler
- **CMake 3.10+** *(optional — can build with g++ directly)*

### Build with g++ (Quickest)

```bash
g++ -std=c++17 -Wall -Wextra -o cvm++ main.cpp lexer.cpp parser.cpp compiler.cpp vm.cpp
```

### Build with CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run a Script

```bash
./cvm++ main.cvm
```

On Windows:
```powershell
.\cvm++.exe main.cvm
```

---

## 📖 Language Reference

### Data Types

| Type      | Syntax            | Internal Representation |
|-----------|-------------------|------------------------|
| Integer   | `0`, `42`, `-5`   | 32-bit signed (`int32_t`) |
| Boolean   | `true`, `false`   | `1` / `0` |

---

### Variables

```javascript
let x = 10;          // declaration with initializer
let sum = x + 20;    // expressions allowed
x = x + 1;           // reassignment
```

---

### Operators

#### Arithmetic
| Operator | Description     | Example     | Result |
|----------|----------------|-------------|--------|
| `+`      | Addition        | `5 + 3`     | `8`    |
| `-`      | Subtraction     | `10 - 2`    | `8`    |
| `*`      | Multiplication  | `4 * 3`     | `12`   |
| `/`      | Division        | `15 / 3`    | `5`    |
| `-`      | Unary negation  | `-42`       | `-42`  |

#### Comparison
| Operator | Description        | Example      | Result |
|----------|--------------------|-------------|--------|
| `==`     | Equal to           | `10 == 10`  | `1`    |
| `!=`     | Not equal to       | `10 != 5`   | `1`    |
| `<`      | Less than          | `3 < 7`     | `1`    |
| `>`      | Greater than       | `9 > 2`     | `1`    |
| `<=`     | Less or equal      | `5 <= 5`    | `1`    |
| `>=`     | Greater or equal   | `3 >= 7`    | `0`    |

#### Logical
| Operator | Description   | Example   | Result |
|----------|--------------|-----------|--------|
| `!`      | Logical NOT  | `!true`   | `0`    |

---

### Control Flow

#### If / Else
```javascript
if (x == 10) {
    print 1;
} else {
    print 0;
}
```
> The `else` block is optional.

#### While Loop
```javascript
let i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
// Output: 0 1 2 3 4
```

> Any non-zero value is **truthy**. Zero is **falsy**.

---

### I/O

```javascript
print x;        // prints value of x, followed by newline
print 42;       // prints a literal

input x;        // reads an integer from stdin into x
                // displays >> prompt
```

---

### Comments

```javascript
// This is a single-line comment
let x = 10;  // inline comment
```

---

### Syntax Rules

| Rule | Detail |
|------|--------|
| Statements | End with **semicolons** `;` |
| Blocks | Use **curly braces** `{ }` |
| Conditions | Must be wrapped in **parentheses** `( )` |
| Output | All values print as integers (`true` → `1`) |

---

## 🧪 Example Programs

### Hello Numbers
```javascript
let a = 10;
let b = 20;
print a + b;    // 30
print a * b;    // 200
```

### FizzBuzz-style Countdown
```javascript
let n = 0;
input n;
while (n > 0) {
    print n;
    n = n - 1;
}
```

### Conditional Logic
```javascript
let score = 85;
if (score >= 90) {
    print 1;       // A grade
} else {
    if (score >= 80) {
        print 2;   // B grade
    } else {
        print 3;   // C grade
    }
}
// Output: 2
```

### Sum of First N Numbers
```javascript
let n = 10;
let sum = 0;
let i = 1;
while (i <= n) {
    sum = sum + i;
    i = i + 1;
}
print sum;    // 55
```

---

## ⚡ Bytecode & Opcodes

The compiler emits bytecode into a flat `std::vector<uint8_t>`. Here are all 24 opcodes:

| Opcode | Hex | Operands | Description |
|--------|-----|----------|-------------|
| `OP_CONST` | `0x00` | `int32` | Push constant integer |
| `OP_TRUE` | `0x01` | — | Push `1` |
| `OP_FALSE` | `0x02` | — | Push `0` |
| `OP_ADD` | `0x03` | — | Pop b, a → push `a + b` |
| `OP_SUB` | `0x04` | — | Pop b, a → push `a - b` |
| `OP_MUL` | `0x05` | — | Pop b, a → push `a * b` |
| `OP_DIV` | `0x06` | — | Pop b, a → push `a / b` |
| `OP_EQ` | `0x07` | — | Pop b, a → push `a == b` |
| `OP_NEQ` | `0x08` | — | Pop b, a → push `a != b` |
| `OP_LT` | `0x09` | — | Pop b, a → push `a < b` |
| `OP_GT` | `0x0A` | — | Pop b, a → push `a > b` |
| `OP_LTE` | `0x0B` | — | Pop b, a → push `a <= b` |
| `OP_GTE` | `0x0C` | — | Pop b, a → push `a >= b` |
| `OP_NEG` | `0x0D` | — | Pop a → push `-a` |
| `OP_NOT` | `0x0E` | — | Pop a → push `!a` |
| `OP_LOAD` | `0x0F` | `uint16` | Push variable from slot |
| `OP_STORE` | `0x10` | `uint16` | Pop → store in slot |
| `OP_PRINT` | `0x11` | — | Pop → print to stdout |
| `OP_INPUT` | `0x12` | — | Read int → push |
| `OP_JMP` | `0x13` | `uint32` | Unconditional jump |
| `OP_JMP_IF_FALSE` | `0x14` | `uint32` | Pop → jump if `0` |
| `OP_POP` | `0x15` | — | Discard top of stack |
| `OP_HALT` | `0x16` | — | Stop execution |

---

## 🧩 How It Works (Under the Hood)

### 1. Lexer (`lexer.cpp`)
Scans source code character by character. Handles multi-character tokens (`==`, `!=`, `<=`, `>=`), skips whitespace and `//` comments, and maps keywords to their token types.

### 2. Parser (`parser.cpp`)
A **recursive-descent parser** that builds a tree of AST nodes. Operator precedence is encoded in the grammar structure:

```
expression → equality → comparison → addition → multiplication → unary → primary
```

### 3. Compiler (`compiler.cpp`)
Walks the AST and emits bytecode. Manages a **variable slot table** (name → index mapping) and handles **jump patching** for `if/else` and `while` control flow.

### 4. VM (`vm.cpp`)
A classic **fetch-decode-execute loop**. Maintains an operand stack (`std::vector<int32_t>`) and a globals array for variable storage. Processes each opcode sequentially until `OP_HALT`.

---

## 🛡️ Error Handling

CVM++ provides clear error messages with **line numbers** at every stage:

```
Lexer error (line 5): unexpected character '@'
Parse error (line 8): expected ';' after expression
Compile error (line 12): undefined variable 'z'
VM error: division by zero
VM error: stack underflow
```

---

## 📜 License

This project is open source under the [MIT License](LICENSE).

---

<p align="center">
  Built with ❤️ in pure C++17 — no dependencies, no compromises.
</p>
