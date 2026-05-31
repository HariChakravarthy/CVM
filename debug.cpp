// ============================================================================
// CVM++ Debug Utilities Implementation
// ============================================================================

#include "debug.h"
#include <iostream>
#include <string>
#include <cstring>

// ============================================================================
// AST PRINTER
// ============================================================================

static void printNode(const ASTNode* node, const std::string& prefix, bool isLast);

static std::string branch(bool isLast) {
    return isLast ? "+-- " : "|-- ";
}

static std::string indent(const std::string& prefix, bool isLast) {
    return prefix + (isLast ? "    " : "|   ");
}


static void printNode(const ASTNode* node, const std::string& prefix, bool isLast) {
    if (!node) {
        std::cout << prefix << branch(isLast) << "(null)\n";
        return;
    }

    std::string next = indent(prefix, isLast);

    switch (node->type) {

    case NodeType::NUMBER_LITERAL: {
        auto* n = static_cast<const NumberLiteral*>(node);
        std::cout << prefix << branch(isLast)
                  << "NumberLiteral(" << n->value << ")  [line " << n->line << "]\n";
        break;
    }
    case NodeType::BOOL_LITERAL: {
        auto* b = static_cast<const BoolLiteral*>(node);
        std::cout << prefix << branch(isLast)
                  << "BoolLiteral(" << (b->value ? "true" : "false")
                  << ")  [line " << b->line << "]\n";
        break;
    }
    case NodeType::IDENTIFIER: {
        auto* id = static_cast<const Identifier*>(node);
        std::cout << prefix << branch(isLast)
                  << "Identifier('" << id->name << "')  [line " << id->line << "]\n";
        break;
    }
    case NodeType::BINARY_EXPR: {
        auto* bin = static_cast<const BinaryExpr*>(node);
        static const char* opNames[] = {
            "ADD(+)","SUB(-)","MUL(*)","DIV(/)",
            "EQ(==)","NEQ(!=)","LT(<)","GT(>)","LTE(<=)","GTE(>=)"
        };
        std::cout << prefix << branch(isLast)
                  << "BinaryExpr " << opNames[static_cast<int>(bin->op)]
                  << "  [line " << bin->line << "]\n";
        printNode(bin->left.get(),  next, false);
        printNode(bin->right.get(), next, true);
        break;
    }
    case NodeType::UNARY_EXPR: {
        auto* un = static_cast<const UnaryExpr*>(node);
        const char* opName = (un->op == UnaryOp::NEG) ? "NEG(-)" : "NOT(!)";
        std::cout << prefix << branch(isLast)
                  << "UnaryExpr " << opName << "  [line " << un->line << "]\n";
        printNode(un->operand.get(), next, true);
        break;
    }
    case NodeType::LET_STMT: {
        auto* let = static_cast<const LetStmt*>(node);
        std::cout << prefix << branch(isLast)
                  << "LetStmt('" << let->name << "')  [line " << let->line << "]\n";
        printNode(let->initializer.get(), next, true);
        break;
    }
    case NodeType::ASSIGN_STMT: {
        auto* asgn = static_cast<const AssignStmt*>(node);
        std::cout << prefix << branch(isLast)
                  << "AssignStmt('" << asgn->name << "')  [line " << asgn->line << "]\n";
        printNode(asgn->value.get(), next, true);
        break;
    }
    case NodeType::PRINT_STMT: {
        auto* pr = static_cast<const PrintStmt*>(node);
        std::cout << prefix << branch(isLast)
                  << "PrintStmt  [line " << pr->line << "]\n";
        printNode(pr->expr.get(), next, true);
        break;
    }
    case NodeType::INPUT_STMT: {
        auto* inp = static_cast<const InputStmt*>(node);
        std::cout << prefix << branch(isLast)
                  << "InputStmt('" << inp->varName << "')  [line " << inp->line << "]\n";
        break;
    }
    case NodeType::IF_STMT: {
        auto* ifs = static_cast<const IfStmt*>(node);
        std::cout << prefix << branch(isLast)
                  << "IfStmt  [line " << ifs->line << "]\n";
        std::cout << next << "|-- [condition]\n";
        printNode(ifs->condition.get(), next + "|   ", true);
        std::cout << next << (ifs->elseBlock ? "|-- " : "+-- ") << "[then]\n";
        printNode(ifs->thenBlock.get(), next + (ifs->elseBlock ? "|   " : "    "), true);
        if (ifs->elseBlock) {
            std::cout << next << "+-- [else]\n";
            printNode(ifs->elseBlock.get(), next + "    ", true);
        }
        break;
    }
    case NodeType::WHILE_STMT: {
        auto* wh = static_cast<const WhileStmt*>(node);
        std::cout << prefix << branch(isLast)
                  << "WhileStmt  [line " << wh->line << "]\n";
        std::cout << next << "|-- [condition]\n";
        printNode(wh->condition.get(), next + "|   ", true);
        std::cout << next << "+-- [body]\n";
        printNode(wh->body.get(), next + "    ", true);
        break;
    }
    case NodeType::BLOCK: {
        auto* blk = static_cast<const Block*>(node);
        std::cout << prefix << branch(isLast)
                  << "Block(" << blk->statements.size() << " stmts)  [line " << blk->line << "]\n";
        for (size_t i = 0; i < blk->statements.size(); i++) {
            printNode(blk->statements[i].get(), next, i == blk->statements.size() - 1);
        }
        break;
    }
    default:
        std::cout << prefix << branch(isLast) << "UnknownNode\n";
    }
}

void printAST(const std::vector<ASTPtr>& program) {
    std::cout << "\n+======================================+\n";
    std::cout <<   "|         Abstract Syntax Tree         |\n";
    std::cout <<   "+======================================+\n";
    std::cout << "Program (" << program.size() << " statement"
              << (program.size() == 1 ? "" : "s") << ")\n";
    for (size_t i = 0; i < program.size(); i++) {
        printNode(program[i].get(), "", i == program.size() - 1);
    }
    std::cout << "\n";
}

// ============================================================================
// BYTECODE DISASSEMBLER
// ============================================================================

static int32_t readI32(const std::vector<uint8_t>& code, size_t pc) {
    int32_t v;
    std::memcpy(&v, &code[pc], 4);
    return v;
}

static uint16_t readU16(const std::vector<uint8_t>& code, size_t pc) {
    uint16_t v;
    std::memcpy(&v, &code[pc], 2);
    return v;
}

void disassemble(const std::vector<uint8_t>& code) {
    std::cout << "+======================================+\n";
    std::cout <<   "|           Bytecode Dump              |\n";
    std::cout <<   "+======================================+\n";
    std::cout << "  OFFSET  OPCODE               OPERANDS\n";
    std::cout << "  ------  -------------------  --------\n";

    size_t pc = 0;
    while (pc < code.size()) {
        uint8_t op = code[pc];
        // Print offset
        std::cout << "  " ;
        // pad offset to 6 chars
        std::string off = std::to_string(pc);
        while (off.size() < 6) off = "0" + off;
        std::cout << off << "  ";

        switch (static_cast<OpCode>(op)) {
        case OP_CONST: {
            int32_t val = readI32(code, pc + 1);
            std::cout << "OP_CONST             " << val << "\n";
            pc += 5; break;
        }
        case OP_TRUE:  std::cout << "OP_TRUE\n";  pc++; break;
        case OP_FALSE: std::cout << "OP_FALSE\n"; pc++; break;

        case OP_ADD:   std::cout << "OP_ADD\n";   pc++; break;
        case OP_SUB:   std::cout << "OP_SUB\n";   pc++; break;
        case OP_MUL:   std::cout << "OP_MUL\n";   pc++; break;
        case OP_DIV:   std::cout << "OP_DIV\n";   pc++; break;
        case OP_MOD:   std::cout << "OP_MOD\n";   pc++; break;

        case OP_EQ:    std::cout << "OP_EQ\n";    pc++; break;
        case OP_NEQ:   std::cout << "OP_NEQ\n";   pc++; break;
        case OP_LT:    std::cout << "OP_LT\n";    pc++; break;
        case OP_GT:    std::cout << "OP_GT\n";    pc++; break;
        case OP_LTE:   std::cout << "OP_LTE\n";   pc++; break;
        case OP_GTE:   std::cout << "OP_GTE\n";   pc++; break;

        case OP_NEG:   std::cout << "OP_NEG\n";   pc++; break;
        case OP_NOT:   std::cout << "OP_NOT\n";   pc++; break;

        case OP_LOAD: {
            uint16_t slot = readU16(code, pc + 1);
            std::cout << "OP_LOAD              slot=" << slot << "\n";
            pc += 3; break;
        }
        case OP_STORE: {
            uint16_t slot = readU16(code, pc + 1);
            std::cout << "OP_STORE             slot=" << slot << "\n";
            pc += 3; break;
        }
        case OP_PRINT:      std::cout << "OP_PRINT\n";      pc++; break;
        case OP_PRINT_BOOL: std::cout << "OP_PRINT_BOOL\n"; pc++; break;
        case OP_INPUT:      std::cout << "OP_INPUT\n";      pc++; break;

        case OP_JMP: {
            int32_t target = readI32(code, pc + 1);
            std::cout << "OP_JMP               -> " << target << "\n";
            pc += 5; break;
        }
        case OP_JMP_IF_FALSE: {
            int32_t target = readI32(code, pc + 1);
            std::cout << "OP_JMP_IF_FALSE      -> " << target << "\n";
            pc += 5; break;
        }
        case OP_JMP_IF_TRUE: {
            int32_t target = readI32(code, pc + 1);
            std::cout << "OP_JMP_IF_TRUE       -> " << target << "\n";
            pc += 5; break;
        }
        case OP_POP:  std::cout << "OP_POP\n";  pc++; break;
        case OP_HALT: std::cout << "OP_HALT\n"; pc++; break;

        default:
            std::cout << "??? (0x" << std::hex << (int)op << std::dec << ")\n";
            pc++;
        }
    }
    std::cout << "\n";
}