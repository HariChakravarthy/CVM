<div align="center">

```
 ██████╗██╗   ██╗███╗   ███╗    ██╗  ██╗ ██╗
██╔════╝██║   ██║████╗ ████║    ╚██╗██╔╝ ██║
██║     ██║   ██║██╔████╔██║     ╚███╔╝  ██║
██║     ╚██╗ ██╔╝██║╚██╔╝██║     ██╔██╗  ╚═╝
╚██████╗ ╚████╔╝ ██║ ╚═╝ ██║    ██╔╝ ██╗ ██╗
 ╚═════╝  ╚═══╝  ╚═╝     ╚═╝    ╚═╝  ╚═╝ ╚═╝
```

**A Stack-Based Virtual Machine with a Custom Compiler — written in pure C++17**

*From raw source text to executed bytecode — no dependencies, no compromises.*

---

[![C++17](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/17)
[![Build](https://img.shields.io/badge/Build-Passing-2ea44f?style=for-the-badge&logo=githubactions&logoColor=white)]()
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)]()
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=for-the-badge)]()
[![IIT Guwahati](https://img.shields.io/badge/IIT-Guwahati-003087?style=for-the-badge)]()

</div>

---

## What is CVM++?

CVM++ is a fully self-contained programming language implementation built from scratch in C++17. It features a hand-written **Lexer**, a **Recursive-Descent Parser**, a **Bytecode Compiler**, and a **Stack-Based Virtual Machine** — all without any external libraries or dependencies.

Write `.cvm` scripts with variables, arithmetic, booleans, control flow, and I/O — and watch them travel through a real, transparent compilation pipeline from source text all the way to execution.

> Built as part of the **IIT Guwahati Coding Club — Even Semester Projects 2026**

---

## Demo

```
Source Code (.cvm)
      |
      |  Lexer       — characters  ->  tokens
      |  Parser      — tokens      ->  AST
      |  Compiler    — AST         ->  bytecode
      |  VM          — bytecode    ->  output
      v
   Result
```

Run the calculator in debug mode to see the full pipeline live:

```bash
./cvm++.exe -d calculator.cvm
```

```
+======================================+
|       CVM++ Debug Mode               |
+======================================+
File: calculator.cvm

+======================================+
|         Abstract Syntax Tree         |
+======================================+
Program (7 statements)
|-- LetStmt('a')  [line 1]
|   +-- NumberLiteral(0)  [line 1]
|-- InputStmt('a')  [line 5]
+-- IfStmt  [line 10]
    |-- [condition]
    |   +-- BinaryExpr EQ(==)  [line 10]
    ...

+======================================+
|           Bytecode Dump              |
+======================================+
  OFFSET  OPCODE               OPERANDS
  ------  -------------------  --------
  000000  OP_CONST             0
  000005  OP_STORE             slot=0
  000024  OP_INPUT
  000036  OP_LOAD              slot=2
  000044  OP_EQ
  000045  OP_JMP_IF_FALSE      -> 63
  000057  OP_PRINT
  000098  OP_HALT

+======================================+
|          Execution Output            |
+======================================+
>> 10
>> 5
>> 1
15
```

---

## Architecture

```
+----------------------------------------------------------+
|                     CVM++ Pipeline                       |
+----------------------------------------------------------+
|                                                          |
|   Source Code (.cvm)                                     |
|        |                                                 |
|        v                                                 |
|   +---------+                                            |
|   |  Lexer  |   Scans characters --> Tokens              |
|   +---------+   lexer.h / lexer.cpp                      |
|        |                                                 |
|        v                                                 |
|   +---------+                                            |
|   | Parser  |   Tokens --> Abstract Syntax Tree          |
|   +---------+   parser.h / parser.cpp                    |
|        |                                                 |
|        v                                                 |
|   +----------+                                           |
|   | Compiler |   AST --> Bytecode (uint8_t[])            |
|   +----------+   compiler.h / compiler.cpp               |
|        |                                                 |
|        v                                                 |
|   +---------+                                            |
|   |   VM    |   Fetch -> Decode -> Execute loop          |
|   +---------+   vm.h / vm.cpp                            |
|        |                                                 |
|        v                                                 |
|     Output                                               |
+----------------------------------------------------------+
```

---

## Project Structure

```
CVM++/
|
|-- lexer.h          Token definitions & Lexer class
|-- lexer.cpp        Character-by-character tokenizer
|
|-- parser.h         AST node hierarchy & Parser class
|-- parser.cpp       Recursive-descent parser
|
|-- compiler.h       Opcode enum & Compiler class
|-- compiler.cpp     AST -> flat bytecode emitter
|
|-- vm.h             VM class declaration
|-- vm.cpp           Stack-based fetch-decode-execute loop
|
|-- debug.h          Debug utilities header
|-- debug.cpp        AST printer + bytecode disassembler
|
|-- main.cpp         CLI entry point (-d / --debug flag)
|-- CMakeLists.txt   CMake build config
|-- run.bat          One-click Windows build script
|
|-- calculator.cvm   Sample: interactive calculator
|-- truth_machine.cvm Sample: classic truth machine
|-- main.cvm         Sample: countdown demo
|
|-- README.md        You are here
```

> Each file has **one responsibility** — clean separation of concerns across the full pipeline.

---

## Getting Started

### Prerequisites

- **g++** with C++17 support (GCC 7+ or Clang 5+)
- **CMake 3.10+** *(optional)*

### Option 1 — One-Click Build (Windows)

Double-click `run.bat` or run from terminal:

```powershell
.\run.bat
```

This compiles everything and leaves a terminal open ready to use.

### Option 2 — Build with g++ directly

```bash
g++ -std=c++17 -Wall -o cvm++.exe main.cpp lexer.cpp parser.cpp compiler.cpp vm.cpp debug.cpp
```

### Option 3 — Build with CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

---

## Usage

```bash
# Run a script normally
./cvm++.exe <script.cvm>

# Run in debug mode — shows AST + bytecode + output
./cvm++.exe -d <script.cvm>
./cvm++.exe --debug <script.cvm>
```

### Quick Examples

```bash
./cvm++.exe calculator.cvm       # interactive calculator
./cvm++.exe truth_machine.cvm    # truth machine (0 exits, 1 loops forever)
./cvm++.exe -d calculator.cvm    # full debug view of the pipeline
```

---

## Language Reference

### Data Types

| Type    | Syntax            | Stored As         |
|---------|-------------------|-------------------|
| Integer | `0`, `42`, `-5`   | `int32_t`         |
| Boolean | `true`, `false`   | `1` / `0`         |

### Variables

```javascript
let x = 10;         // declare with initializer
let sum = x + 20;   // expressions allowed
x = x + 1;          // reassignment
```

### Operators

| Category   | Operators                        |
|------------|----------------------------------|
| Arithmetic | `+`  `-`  `*`  `/`              |
| Comparison | `==`  `!=`  `<`  `>`  `<=`  `>=`|
| Logical    | `!`                              |
| Unary      | `-` (negation)                   |

### Control Flow

```javascript
// If / Else
if (x >= 90) {
    print 1;
} else {
    print 0;
}

// While loop
let i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

### I/O

```javascript
print x;      // prints value followed by newline
input x;      // reads an integer from stdin into x
```

### Comments

```javascript
// This is a single-line comment
let x = 10;   // inline comment
```

### Syntax Rules

| Rule       | Detail                                 |
|------------|----------------------------------------|
| Statements | End with semicolons `;`                |
| Blocks     | Wrapped in curly braces `{ }`          |
| Conditions | Wrapped in parentheses `( )`           |
| Output     | All values print as integers (`true` → `1`) |

---

## Sample Programs

### Calculator

```javascript
let a = 0;
let b = 0;
let choice = 0;

input a;
input b;
input choice;

// 1=add  2=subtract  3=multiply
if (choice == 1) {
    print a + b;
} else {
    if (choice == 2) {
        print a - b;
    } else {
        print a * b;
    }
}
```

```
>> 10
>> 3
>> 2
7
```

### Truth Machine

```javascript
let n = 0;
input n;

if (n == 0) {
    print 0;
} else {
    while (1) {
        print 1;
    }
}
```

```
>> 0       >> 1
0          1 1 1 1 1 ... (forever)
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

### Grade Checker

```javascript
let score = 0;
input score;

if (score >= 90) {
    print 1;
} else {
    if (score >= 80) {
        print 2;
    } else {
        if (score >= 70) {
            print 3;
        } else {
            print 4;
        }
    }
}
```

---

## Bytecode & Opcodes

The compiler emits a flat `std::vector<uint8_t>`. All 23 opcodes:

| Opcode           | Hex    | Operands    | Description                    |
|------------------|--------|-------------|--------------------------------|
| `OP_CONST`       | `0x00` | `int32`     | Push constant integer          |
| `OP_TRUE`        | `0x01` | —           | Push `1`                       |
| `OP_FALSE`       | `0x02` | —           | Push `0`                       |
| `OP_ADD`         | `0x03` | —           | Pop b, a → push `a + b`        |
| `OP_SUB`         | `0x04` | —           | Pop b, a → push `a - b`        |
| `OP_MUL`         | `0x05` | —           | Pop b, a → push `a * b`        |
| `OP_DIV`         | `0x06` | —           | Pop b, a → push `a / b`        |
| `OP_EQ`          | `0x07` | —           | Push `a == b`                  |
| `OP_NEQ`         | `0x08` | —           | Push `a != b`                  |
| `OP_LT`          | `0x09` | —           | Push `a < b`                   |
| `OP_GT`          | `0x0A` | —           | Push `a > b`                   |
| `OP_LTE`         | `0x0B` | —           | Push `a <= b`                  |
| `OP_GTE`         | `0x0C` | —           | Push `a >= b`                  |
| `OP_NEG`         | `0x0D` | —           | Push `-a`                      |
| `OP_NOT`         | `0x0E` | —           | Push `!a`                      |
| `OP_LOAD`        | `0x0F` | `uint16`    | Push variable from slot        |
| `OP_STORE`       | `0x10` | `uint16`    | Pop → store in slot            |
| `OP_PRINT`       | `0x11` | —           | Pop → print to stdout          |
| `OP_INPUT`       | `0x12` | —           | Read int → push                |
| `OP_JMP`         | `0x13` | `uint32`    | Unconditional jump             |
| `OP_JMP_IF_FALSE`| `0x14` | `uint32`    | Pop → jump if `0`              |
| `OP_POP`         | `0x15` | —           | Discard top of stack           |
| `OP_HALT`        | `0x16` | —           | Stop execution                 |

---

## How It Works — Under the Hood

### 1. Lexer (`lexer.cpp`)
Scans source code character by character. Handles multi-character tokens (`==`, `!=`, `<=`, `>=`), skips whitespace and `//` comments, and maps keywords like `let`, `if`, `while` to their token types.

### 2. Parser (`parser.cpp`)
A **recursive-descent parser** that builds an AST. Operator precedence is encoded directly in the grammar:

```
expression
  -> equality
     -> comparison
        -> addition
           -> multiplication
              -> unary
                 -> primary
```

### 3. Compiler (`compiler.cpp`)
Walks the AST and emits bytecode into a flat array. Manages a **variable slot table** (name → index) and handles **jump patching** — forward jumps are emitted with a placeholder address, then patched once the target offset is known.

### 4. VM (`vm.cpp`)
A classic **fetch-decode-execute loop**. Maintains an operand stack (`std::vector<int32_t>`) and a globals array for variable storage. Every expression leaves exactly one value on the stack.

### 5. Debug Mode (`debug.cpp`)
The `-d` flag activates two tools built on top of the pipeline: an **AST pretty-printer** that walks the node tree with indented branches, and a **bytecode disassembler** that reads the raw `uint8_t` array and prints each opcode with its operands and byte offset.

---

## Error Handling

CVM++ gives clear, line-numbered error messages at every stage:

```
Lexer error   (line 5):  unexpected character '@'
Parse error   (line 8):  expected ';' after expression
Compile error (line 12): undefined variable 'z'
VM error:                division by zero
VM error:                stack underflow
```

---

## License

This project is open source under the [MIT License](LICENSE).

---

<div align="center">

Built with care in pure C++17 — IIT Guwahati Coding Club, 2026.

</div>