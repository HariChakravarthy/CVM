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

enum OpCode : uint8_t {
    OP_CONST,
    OP_TRUE,
    OP_FALSE,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,      // remainder (a % b)

    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,

    OP_NEG,
    OP_NOT,

    OP_LOAD,
    OP_STORE,

    OP_PRINT,
    OP_PRINT_BOOL,  // print "true" / "false" for boolean results
    OP_INPUT,

    OP_JMP,
    OP_JMP_IF_FALSE,
    OP_JMP_IF_TRUE,  // short-circuit for ||

    OP_POP,
    OP_HALT
};

// ---- Compiler Class --------------------------------------------------------

class Compiler {
public:
    Compiler();

    // Compile a full program — resets all compiler state (file runner)
    std::vector<uint8_t> compile(const std::vector<ASTPtr>& program);

    // REPL: compile one or more statements WITHOUT resetting the variable table
    // so that variables declared in previous lines remain in scope
    std::vector<uint8_t> compileRepl(const std::vector<ASTPtr>& stmts);

private:
    std::vector<uint8_t> code_;
    std::unordered_map<std::string, uint16_t> vars_;
    uint16_t nextSlot_;

    void emitByte(uint8_t byte);
    void emitOpCode(OpCode op);
    void emitInt32(int32_t value);
    void emitUint16(uint16_t value);

    size_t emitJump(OpCode jumpOp);
    void   patchJump(size_t offset);

    uint16_t resolveVar(const std::string& name, int line);
    uint16_t declareVar(const std::string& name, int line);

    void compileNode(const ASTPtr& node);
    void compileExpr(const ASTPtr& node);
};