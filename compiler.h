#pragma once
// ============================================================================
// CVM++ Compiler Header
// Walks the AST and emits bytecode (std::vector<uint8_t>).
// ============================================================================

#include "parser.h"
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

// ---- Opcodes ---------------------------------------------------------------
// Each opcode is one byte. Some are followed by operands (noted in comments).

enum OpCode : uint8_t {
    OP_CONST,           // [4 bytes int32] push a constant integer
    OP_TRUE,            // push 1 (boolean true)
    OP_FALSE,           // push 0 (boolean false)

    OP_ADD,             // pop b, pop a, push a + b
    OP_SUB,             // pop b, pop a, push a - b
    OP_MUL,             // pop b, pop a, push a * b
    OP_DIV,             // pop b, pop a, push a / b

    OP_EQ,              // pop b, pop a, push (a == b)
    OP_NEQ,             // pop b, pop a, push (a != b)
    OP_LT,              // pop b, pop a, push (a < b)
    OP_GT,              // pop b, pop a, push (a > b)
    OP_LTE,             // pop b, pop a, push (a <= b)
    OP_GTE,             // pop b, pop a, push (a >= b)

    OP_NEG,             // pop a, push -a
    OP_NOT,             // pop a, push !a

    OP_LOAD,            // [2 bytes uint16] push value of variable slot
    OP_STORE,           // [2 bytes uint16] pop value, store in variable slot

    OP_PRINT,           // pop value, print to stdout
    OP_INPUT,           // read integer from stdin, push onto stack

    OP_JMP,             // [4 bytes uint32] unconditional jump to address
    OP_JMP_IF_FALSE,    // [4 bytes uint32] pop; if false (0), jump to address

    OP_POP,             // pop and discard top of stack
    OP_HALT             // stop execution
};

// ---- Compiler Class --------------------------------------------------------

class Compiler {
public:
    Compiler();

    // Compile a full program (list of AST statements) into bytecode
    std::vector<uint8_t> compile(const std::vector<ASTPtr>& program);

private:
    std::vector<uint8_t> code_;                        // output bytecode
    std::unordered_map<std::string, uint16_t> vars_;   // variable name → slot
    uint16_t nextSlot_;                                // next free variable slot

    // Emit helpers
    void emitByte(uint8_t byte);
    void emitOpCode(OpCode op);
    void emitInt32(int32_t value);
    void emitUint16(uint16_t value);

    // Jump helpers: emit placeholder, then patch later
    size_t emitJump(OpCode jumpOp);
    void   patchJump(size_t offset);

    // Resolve or allocate a variable slot
    uint16_t resolveVar(const std::string& name, int line);
    uint16_t declareVar(const std::string& name, int line);

    // AST walk
    void compileNode(const ASTPtr& node);
    void compileExpr(const ASTPtr& node);
};
