#pragma once
// ============================================================================
// CVM++ Parser Header — Recursive-Descent Parser
// Converts a token stream into an Abstract Syntax Tree (AST).
// ============================================================================

#include "lexer.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

// ---- AST Node Types --------------------------------------------------------

enum class NodeType {
    NUMBER_LITERAL, BOOL_LITERAL, IDENTIFIER,
    BINARY_EXPR, UNARY_EXPR,
    LET_STMT, ASSIGN_STMT, PRINT_STMT, INPUT_STMT,
    IF_STMT, WHILE_STMT, BLOCK
};

// Binary operator kinds
enum class BinOp { ADD, SUB, MUL, DIV, EQ, NEQ, LT, GT, LTE, GTE };

// Unary operator kinds
enum class UnaryOp { NEG, NOT };

// ---- AST Node Hierarchy ----------------------------------------------------

// Base class for all AST nodes
struct ASTNode {
    NodeType type;
    int line;
    virtual ~ASTNode() = default;
protected:
    ASTNode(NodeType t, int l) : type(t), line(l) {}
};

using ASTPtr = std::unique_ptr<ASTNode>;

// Integer literal: 42
struct NumberLiteral : ASTNode {
    int32_t value;
    NumberLiteral(int32_t v, int ln)
        : ASTNode(NodeType::NUMBER_LITERAL, ln), value(v) {}
};

// Boolean literal: true / false
struct BoolLiteral : ASTNode {
    bool value;
    BoolLiteral(bool v, int ln)
        : ASTNode(NodeType::BOOL_LITERAL, ln), value(v) {}
};

// Variable reference: x
struct Identifier : ASTNode {
    std::string name;
    Identifier(const std::string& n, int ln)
        : ASTNode(NodeType::IDENTIFIER, ln), name(n) {}
};

// Binary expression: left op right
struct BinaryExpr : ASTNode {
    BinOp   op;
    ASTPtr  left;
    ASTPtr  right;
    BinaryExpr(BinOp o, ASTPtr l, ASTPtr r, int ln)
        : ASTNode(NodeType::BINARY_EXPR, ln),
          op(o), left(std::move(l)), right(std::move(r)) {}
};

// Unary expression: op operand
struct UnaryExpr : ASTNode {
    UnaryOp op;
    ASTPtr  operand;
    UnaryExpr(UnaryOp o, ASTPtr e, int ln)
        : ASTNode(NodeType::UNARY_EXPR, ln), op(o), operand(std::move(e)) {}
};

// Variable declaration: let x = expr;
struct LetStmt : ASTNode {
    std::string name;
    ASTPtr      initializer;
    LetStmt(const std::string& n, ASTPtr init, int ln)
        : ASTNode(NodeType::LET_STMT, ln),
          name(n), initializer(std::move(init)) {}
};

// Variable assignment: x = expr;
struct AssignStmt : ASTNode {
    std::string name;
    ASTPtr      value;
    AssignStmt(const std::string& n, ASTPtr v, int ln)
        : ASTNode(NodeType::ASSIGN_STMT, ln),
          name(n), value(std::move(v)) {}
};

// Print statement: print expr;
struct PrintStmt : ASTNode {
    ASTPtr expr;
    PrintStmt(ASTPtr e, int ln)
        : ASTNode(NodeType::PRINT_STMT, ln), expr(std::move(e)) {}
};

// Input statement: input x;
struct InputStmt : ASTNode {
    std::string varName;
    InputStmt(const std::string& n, int ln)
        : ASTNode(NodeType::INPUT_STMT, ln), varName(n) {}
};

// If/else statement: if (cond) { ... } else { ... }
struct IfStmt : ASTNode {
    ASTPtr condition;
    ASTPtr thenBlock;
    ASTPtr elseBlock;   // may be nullptr
    IfStmt(ASTPtr cond, ASTPtr then_, ASTPtr else_, int ln)
        : ASTNode(NodeType::IF_STMT, ln),
          condition(std::move(cond)),
          thenBlock(std::move(then_)),
          elseBlock(std::move(else_)) {}
};

// While loop: while (cond) { ... }
struct WhileStmt : ASTNode {
    ASTPtr condition;
    ASTPtr body;
    WhileStmt(ASTPtr cond, ASTPtr b, int ln)
        : ASTNode(NodeType::WHILE_STMT, ln),
          condition(std::move(cond)), body(std::move(b)) {}
};

// Block: { stmt1; stmt2; ... }
struct Block : ASTNode {
    std::vector<ASTPtr> statements;
    Block(std::vector<ASTPtr> stmts, int ln)
        : ASTNode(NodeType::BLOCK, ln), statements(std::move(stmts)) {}
};

// ---- Parser Class ----------------------------------------------------------

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // Parse the entire program into a list of top-level statements
    std::vector<ASTPtr> parse();

private:
    std::vector<Token> tokens_;
    size_t pos_;

    // Token navigation
    const Token& current() const;
    const Token& previous() const;
    bool  isAtEnd() const;
    bool  check(TokenType t) const;
    bool  match(TokenType t);
    Token consume(TokenType t, const std::string& errMsg);

    // Recursive-descent grammar rules (ordered by precedence)
    ASTPtr statement();
    ASTPtr letStatement();
    ASTPtr printStatement();
    ASTPtr inputStatement();
    ASTPtr ifStatement();
    ASTPtr whileStatement();
    ASTPtr block();

    // Expression rules (lowest to highest precedence)
    ASTPtr expression();
    ASTPtr equality();
    ASTPtr comparison();
    ASTPtr addition();
    ASTPtr multiplication();
    ASTPtr unary();
    ASTPtr primary();
};
