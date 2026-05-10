// ============================================================================
// CVM++ Compiler Implementation
// Walks the AST and emits bytecode into a std::vector<uint8_t>.
// Uses a simple flat variable-slot scheme (no scoping).
// ============================================================================

#include "compiler.h"
#include <cstring>   // for memcpy
#include <stdexcept>

Compiler::Compiler() : nextSlot_(0) {}

// ---- Emit helpers ----------------------------------------------------------

void Compiler::emitByte(uint8_t byte) {
    code_.push_back(byte);
}

void Compiler::emitOpCode(OpCode op) {
    emitByte(static_cast<uint8_t>(op));
}

// Write a 32-bit integer in little-endian order
void Compiler::emitInt32(int32_t value) {
    uint8_t bytes[4];
    std::memcpy(bytes, &value, 4);
    for (int i = 0; i < 4; i++) code_.push_back(bytes[i]);
}

// Write a 16-bit unsigned integer in little-endian order
void Compiler::emitUint16(uint16_t value) {
    uint8_t bytes[2];
    std::memcpy(bytes, &value, 2);
    code_.push_back(bytes[0]);
    code_.push_back(bytes[1]);
}

// Emit a jump instruction with a 4-byte placeholder; return offset of placeholder
size_t Compiler::emitJump(OpCode jumpOp) {
    emitOpCode(jumpOp);
    size_t offset = code_.size();       // remember where the placeholder starts
    emitInt32(0);                       // placeholder (will be patched later)
    return offset;
}

// Patch a previously emitted jump placeholder to point to the current position
void Compiler::patchJump(size_t offset) {
    int32_t target = static_cast<int32_t>(code_.size());
    std::memcpy(&code_[offset], &target, 4);
}

// ---- Variable management ---------------------------------------------------

// Look up an existing variable; throws if undeclared
uint16_t Compiler::resolveVar(const std::string& name, int line) {
    auto it = vars_.find(name);
    if (it == vars_.end()) {
        throw std::runtime_error(
            "Compile error (line " + std::to_string(line) +
            "): undefined variable '" + name + "'");
    }
    return it->second;
}

// Declare a new variable and assign it a slot
uint16_t Compiler::declareVar(const std::string& name, int line) {
    if (vars_.count(name)) {
        throw std::runtime_error(
            "Compile error (line " + std::to_string(line) +
            "): variable '" + name + "' already declared");
    }
    uint16_t slot = nextSlot_++;
    vars_[name] = slot;
    return slot;
}

// ---- Public entry point ----------------------------------------------------

std::vector<uint8_t> Compiler::compile(const std::vector<ASTPtr>& program) {
    code_.clear();
    vars_.clear();
    nextSlot_ = 0;

    // Compile each top-level statement
    for (auto& stmt : program) {
        compileNode(stmt);
    }

    // End the program
    emitOpCode(OP_HALT);
    return code_;
}

// ---- AST compilation -------------------------------------------------------

// Compile a statement node
void Compiler::compileNode(const ASTPtr& node) {
    switch (node->type) {

    // ---- let x = expr; ----
    case NodeType::LET_STMT: {
        auto* let = static_cast<LetStmt*>(node.get());
        compileExpr(let->initializer);              // push initializer value
        uint16_t slot = declareVar(let->name, let->line);
        emitOpCode(OP_STORE);                       // store into new slot
        emitUint16(slot);
        break;
    }

    // ---- x = expr; ----
    case NodeType::ASSIGN_STMT: {
        auto* assign = static_cast<AssignStmt*>(node.get());
        compileExpr(assign->value);                 // push new value
        uint16_t slot = resolveVar(assign->name, assign->line);
        emitOpCode(OP_STORE);                       // overwrite slot
        emitUint16(slot);
        break;
    }

    // ---- print expr; ----
    case NodeType::PRINT_STMT: {
        auto* pr = static_cast<PrintStmt*>(node.get());
        compileExpr(pr->expr);                      // push value
        emitOpCode(OP_PRINT);                       // pop & print
        break;
    }

    // ---- input x; ----
    case NodeType::INPUT_STMT: {
        auto* inp = static_cast<InputStmt*>(node.get());
        uint16_t slot = resolveVar(inp->varName, inp->line);
        emitOpCode(OP_INPUT);                       // read int, push
        emitOpCode(OP_STORE);                       // pop into var slot
        emitUint16(slot);
        break;
    }

    // ---- if (cond) { ... } else { ... } ----
    case NodeType::IF_STMT: {
        auto* ifs = static_cast<IfStmt*>(node.get());

        compileExpr(ifs->condition);                // evaluate condition

        // Jump past the then-block if condition is false
        size_t falseJump = emitJump(OP_JMP_IF_FALSE);

        compileNode(ifs->thenBlock);                // compile then-block

        if (ifs->elseBlock) {
            // Jump past else-block after executing then-block
            size_t endJump = emitJump(OP_JMP);
            patchJump(falseJump);                   // false → else-block start
            compileNode(ifs->elseBlock);
            patchJump(endJump);                     // end of else
        } else {
            patchJump(falseJump);                   // no else; false → after then
        }
        break;
    }

    // ---- while (cond) { ... } ----
    case NodeType::WHILE_STMT: {
        auto* wh = static_cast<WhileStmt*>(node.get());

        int32_t loopStart = static_cast<int32_t>(code_.size());  // loop top

        compileExpr(wh->condition);                 // evaluate condition
        size_t exitJump = emitJump(OP_JMP_IF_FALSE); // exit if false

        compileNode(wh->body);                      // compile body

        // Jump back to loop top
        emitOpCode(OP_JMP);
        emitInt32(loopStart);

        patchJump(exitJump);                        // exit → after loop
        break;
    }

    // ---- { stmts... } ----
    case NodeType::BLOCK: {
        auto* blk = static_cast<Block*>(node.get());
        for (auto& s : blk->statements) {
            compileNode(s);
        }
        break;
    }

    // ---- Bare expression statement → evaluate and discard ----
    default:
        compileExpr(node);
        emitOpCode(OP_POP);
        break;
    }
}

// Compile an expression node (leaves one value on the stack)
void Compiler::compileExpr(const ASTPtr& node) {
    switch (node->type) {

    case NodeType::NUMBER_LITERAL: {
        auto* num = static_cast<NumberLiteral*>(node.get());
        emitOpCode(OP_CONST);
        emitInt32(num->value);
        break;
    }

    case NodeType::BOOL_LITERAL: {
        auto* bl = static_cast<BoolLiteral*>(node.get());
        emitOpCode(bl->value ? OP_TRUE : OP_FALSE);
        break;
    }

    case NodeType::IDENTIFIER: {
        auto* id = static_cast<Identifier*>(node.get());
        uint16_t slot = resolveVar(id->name, id->line);
        emitOpCode(OP_LOAD);
        emitUint16(slot);
        break;
    }

    case NodeType::BINARY_EXPR: {
        auto* bin = static_cast<BinaryExpr*>(node.get());
        compileExpr(bin->left);                     // push left operand
        compileExpr(bin->right);                    // push right operand
        // Emit the matching arithmetic/comparison opcode
        switch (bin->op) {
            case BinOp::ADD: emitOpCode(OP_ADD); break;
            case BinOp::SUB: emitOpCode(OP_SUB); break;
            case BinOp::MUL: emitOpCode(OP_MUL); break;
            case BinOp::DIV: emitOpCode(OP_DIV); break;
            case BinOp::EQ:  emitOpCode(OP_EQ);  break;
            case BinOp::NEQ: emitOpCode(OP_NEQ); break;
            case BinOp::LT:  emitOpCode(OP_LT);  break;
            case BinOp::GT:  emitOpCode(OP_GT);  break;
            case BinOp::LTE: emitOpCode(OP_LTE); break;
            case BinOp::GTE: emitOpCode(OP_GTE); break;
        }
        break;
    }

    case NodeType::UNARY_EXPR: {
        auto* un = static_cast<UnaryExpr*>(node.get());
        compileExpr(un->operand);                   // push operand
        switch (un->op) {
            case UnaryOp::NEG: emitOpCode(OP_NEG); break;
            case UnaryOp::NOT: emitOpCode(OP_NOT); break;
        }
        break;
    }

    default:
        throw std::runtime_error(
            "Compile error (line " + std::to_string(node->line) +
            "): invalid expression node");
    }
}
