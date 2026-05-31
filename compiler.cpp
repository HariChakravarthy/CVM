// ============================================================================
// CVM++ Compiler Implementation
// Walks the AST and emits bytecode into a std::vector<uint8_t>.
// Uses a simple flat variable-slot scheme (no scoping).
// ============================================================================

#include "compiler.h"
#include <cstring>
#include <stdexcept>

// ---- Helper: is an AST expression statically boolean? ---------------------
// Returns true when the expression is guaranteed to produce a bool (0 or 1)
// so the compiler can emit OP_PRINT_BOOL instead of OP_PRINT.
static bool isBoolExpr(const ASTPtr& node) {
    if (!node) return false;
    switch (node->type) {
        case NodeType::BOOL_LITERAL:
            return true;
        case NodeType::BINARY_EXPR: {
            auto* bin = static_cast<const BinaryExpr*>(node.get());
            switch (bin->op) {
                case BinOp::EQ:  case BinOp::NEQ:
                case BinOp::LT:  case BinOp::GT:
                case BinOp::LTE: case BinOp::GTE:
                case BinOp::AND: case BinOp::OR:
                    return true;
                default:
                    return false;
            }
        }
        case NodeType::UNARY_EXPR: {
            auto* un = static_cast<const UnaryExpr*>(node.get());
            return un->op == UnaryOp::NOT;
        }
        default:
            return false;
    }
}

Compiler::Compiler() : nextSlot_(0) {}

// ---- Emit helpers ----------------------------------------------------------

void Compiler::emitByte(uint8_t byte) {
    code_.push_back(byte);
}

void Compiler::emitOpCode(OpCode op) {
    emitByte(static_cast<uint8_t>(op));
}

void Compiler::emitInt32(int32_t value) {
    uint8_t bytes[4];
    std::memcpy(bytes, &value, 4);
    for (int i = 0; i < 4; i++) code_.push_back(bytes[i]);
}

void Compiler::emitUint16(uint16_t value) {
    uint8_t bytes[2];
    std::memcpy(bytes, &value, 2);
    code_.push_back(bytes[0]);
    code_.push_back(bytes[1]);
}

size_t Compiler::emitJump(OpCode jumpOp) {
    emitOpCode(jumpOp);
    size_t offset = code_.size();
    emitInt32(0);
    return offset;
}

void Compiler::patchJump(size_t offset) {
    int32_t target = static_cast<int32_t>(code_.size());
    std::memcpy(&code_[offset], &target, 4);
}

// ---- Variable management ---------------------------------------------------

uint16_t Compiler::resolveVar(const std::string& name, int line) {
    auto it = vars_.find(name);
    if (it == vars_.end()) {
        throw std::runtime_error(
            "Compile error (line " + std::to_string(line) +
            "): undefined variable '" + name + "'");
    }
    return it->second;
}

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

// ---- Public entry points ---------------------------------------------------

// File runner: full reset before compiling
std::vector<uint8_t> Compiler::compile(const std::vector<ASTPtr>& program) {
    code_.clear();
    vars_.clear();
    nextSlot_ = 0;

    for (auto& stmt : program) {
        compileNode(stmt);
    }

    emitOpCode(OP_HALT);
    return code_;
}

// REPL: only reset the code buffer — vars_ and nextSlot_ carry over from the
// previous line so that variables declared earlier remain visible
std::vector<uint8_t> Compiler::compileRepl(const std::vector<ASTPtr>& stmts) {
    code_.clear();          // fresh bytecode for this line
    // vars_ and nextSlot_ intentionally NOT reset

    for (auto& stmt : stmts) {
        compileNode(stmt);
    }

    emitOpCode(OP_HALT);
    return code_;
}

// ---- AST compilation -------------------------------------------------------

void Compiler::compileNode(const ASTPtr& node) {
    switch (node->type) {

    case NodeType::LET_STMT: {
        auto* let = static_cast<LetStmt*>(node.get());
        compileExpr(let->initializer);
        uint16_t slot = declareVar(let->name, let->line);
        emitOpCode(OP_STORE);
        emitUint16(slot);
        break;
    }

    case NodeType::ASSIGN_STMT: {
        auto* assign = static_cast<AssignStmt*>(node.get());
        compileExpr(assign->value);
        uint16_t slot = resolveVar(assign->name, assign->line);
        emitOpCode(OP_STORE);
        emitUint16(slot);
        break;
    }

    case NodeType::PRINT_STMT: {
        auto* pr = static_cast<PrintStmt*>(node.get());
        compileExpr(pr->expr);
        // Emit OP_PRINT_BOOL when we can prove the expression is boolean
        emitOpCode(isBoolExpr(pr->expr) ? OP_PRINT_BOOL : OP_PRINT);
        break;
    }

    case NodeType::INPUT_STMT: {
        auto* inp = static_cast<InputStmt*>(node.get());
        uint16_t slot = resolveVar(inp->varName, inp->line);
        emitOpCode(OP_INPUT);
        emitOpCode(OP_STORE);
        emitUint16(slot);
        break;
    }

    case NodeType::IF_STMT: {
        auto* ifs = static_cast<IfStmt*>(node.get());
        compileExpr(ifs->condition);
        size_t falseJump = emitJump(OP_JMP_IF_FALSE);
        compileNode(ifs->thenBlock);
        if (ifs->elseBlock) {
            size_t endJump = emitJump(OP_JMP);
            patchJump(falseJump);
            compileNode(ifs->elseBlock);
            patchJump(endJump);
        } else {
            patchJump(falseJump);
        }
        break;
    }

    case NodeType::WHILE_STMT: {
        auto* wh = static_cast<WhileStmt*>(node.get());
        int32_t loopStart = static_cast<int32_t>(code_.size());
        compileExpr(wh->condition);
        size_t exitJump = emitJump(OP_JMP_IF_FALSE);
        compileNode(wh->body);
        emitOpCode(OP_JMP);
        emitInt32(loopStart);
        patchJump(exitJump);
        break;
    }

    case NodeType::BLOCK: {
        auto* blk = static_cast<Block*>(node.get());
        for (auto& s : blk->statements) {
            compileNode(s);
        }
        break;
    }

    default:
        compileExpr(node);
        emitOpCode(OP_POP);
        break;
    }
}

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

        // Short-circuit AND: if left is false, skip right, result = 0
        if (bin->op == BinOp::AND) {
            compileExpr(bin->left);
            size_t falseJump = emitJump(OP_JMP_IF_FALSE); // pops left
            compileExpr(bin->right);
            size_t endJump = emitJump(OP_JMP);
            patchJump(falseJump);
            emitOpCode(OP_CONST); emitInt32(0); // left was false -> result 0
            patchJump(endJump);
            break;
        }

        // Short-circuit OR: if left is true, skip right, result = 1
        if (bin->op == BinOp::OR) {
            compileExpr(bin->left);
            size_t trueJump = emitJump(OP_JMP_IF_TRUE); // pops left
            compileExpr(bin->right);
            size_t endJump = emitJump(OP_JMP);
            patchJump(trueJump);
            emitOpCode(OP_CONST); emitInt32(1); // left was true -> result 1
            patchJump(endJump);
            break;
        }

        // All other binary ops: compile both sides then emit opcode
        compileExpr(bin->left);
        compileExpr(bin->right);
        switch (bin->op) {
            case BinOp::ADD: emitOpCode(OP_ADD); break;
            case BinOp::SUB: emitOpCode(OP_SUB); break;
            case BinOp::MUL: emitOpCode(OP_MUL); break;
            case BinOp::DIV: emitOpCode(OP_DIV); break;
            case BinOp::MOD: emitOpCode(OP_MOD); break;
            case BinOp::EQ:  emitOpCode(OP_EQ);  break;
            case BinOp::NEQ: emitOpCode(OP_NEQ); break;
            case BinOp::LT:  emitOpCode(OP_LT);  break;
            case BinOp::GT:  emitOpCode(OP_GT);  break;
            case BinOp::LTE: emitOpCode(OP_LTE); break;
            case BinOp::GTE: emitOpCode(OP_GTE); break;
            default: break; // AND/OR handled above
        }
        break;
    }

    case NodeType::UNARY_EXPR: {
        auto* un = static_cast<UnaryExpr*>(node.get());
        compileExpr(un->operand);
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