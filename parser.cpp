// ============================================================================
// CVM++ Parser Implementation — Recursive-Descent
// Grammar (lowest → highest precedence):
//   program        → statement* EOF
//   statement      → letStmt | printStmt | inputStmt | ifStmt | whileStmt
//                    | block | assignStmt | exprStmt
//   letStmt        → "let" IDENT "=" expression ";"
//   assignStmt     → IDENT "=" expression ";"
//   printStmt      → "print" expression ";"
//   inputStmt      → "input" IDENT ";"
//   ifStmt         → "if" "(" expression ")" block ("else" block)?
//   whileStmt      → "while" "(" expression ")" block
//   block          → "{" statement* "}"
//   expression     → equality
//   equality       → comparison (("==" | "!=") comparison)*
//   comparison     → addition (("<" | ">" | "<=" | ">=") addition)*
//   addition       → multiplication (("+" | "-") multiplication)*
//   multiplication → unary (("*" | "/") unary)*
//   unary          → ("-" | "!") unary | primary
//   primary        → NUMBER | "true" | "false" | IDENT | "(" expression ")"
// ============================================================================

#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), pos_(0) {}

// ---- Token helpers ---------------------------------------------------------

const Token& Parser::current() const  { return tokens_[pos_]; }
const Token& Parser::previous() const { return tokens_[pos_ - 1]; }
bool Parser::isAtEnd() const          { return current().type == TokenType::END_OF_FILE; }

bool Parser::check(TokenType t) const {
    return !isAtEnd() && current().type == t;
}

// If current token matches, consume it and return true
bool Parser::match(TokenType t) {
    if (check(t)) { pos_++; return true; }
    return false;
}

// Consume a token of the expected type, or throw an error
Token Parser::consume(TokenType t, const std::string& errMsg) {
    if (check(t)) { Token tok = current(); pos_++; return tok; }
    throw std::runtime_error(
        "Parse error (line " + std::to_string(current().line) + "): " + errMsg);
}

// ---- Top-level parse -------------------------------------------------------

std::vector<ASTPtr> Parser::parse() {
    std::vector<ASTPtr> program;
    while (!isAtEnd()) {
        program.push_back(statement());
    }
    return program;
}

// ---- Statement parsing -----------------------------------------------------

ASTPtr Parser::statement() {
    // Dispatch on the current token type
    if (check(TokenType::KW_LET))   return letStatement();
    if (check(TokenType::KW_PRINT)) return printStatement();
    if (check(TokenType::KW_INPUT)) return inputStatement();
    if (check(TokenType::KW_IF))    return ifStatement();
    if (check(TokenType::KW_WHILE)) return whileStatement();
    if (check(TokenType::LBRACE))   return block();

    // Check for assignment: IDENTIFIER followed by '='
    if (check(TokenType::IDENTIFIER)) {
        // Look ahead to see if this is assignment (=) vs expression
        if (pos_ + 1 < tokens_.size() &&
            tokens_[pos_ + 1].type == TokenType::EQUAL) {
            Token name = consume(TokenType::IDENTIFIER, "expected variable name");
            consume(TokenType::EQUAL, "expected '='");
            ASTPtr val = expression();
            consume(TokenType::SEMICOLON, "expected ';' after assignment");
            return std::make_unique<AssignStmt>(name.value, std::move(val), name.line);
        }
    }

    // Fall through: expression statement (e.g. a function call or bare expr)
    ASTPtr expr = expression();
    consume(TokenType::SEMICOLON, "expected ';' after expression");
    // We wrap it in a print just to avoid silent discard;
    // alternatively could just evaluate and pop.
    // For now we'll return it wrapped in a block with one statement that pops.
    // Actually, let's just return the expression as-is; the compiler will emit OP_POP.
    return expr;
}

// let x = expression;
ASTPtr Parser::letStatement() {
    int ln = current().line;
    consume(TokenType::KW_LET, "expected 'let'");
    Token name = consume(TokenType::IDENTIFIER, "expected variable name after 'let'");
    consume(TokenType::EQUAL, "expected '=' in let declaration");
    ASTPtr init = expression();
    consume(TokenType::SEMICOLON, "expected ';' after let declaration");
    return std::make_unique<LetStmt>(name.value, std::move(init), ln);
}

// print expression;
ASTPtr Parser::printStatement() {
    int ln = current().line;
    consume(TokenType::KW_PRINT, "expected 'print'");
    ASTPtr expr = expression();
    consume(TokenType::SEMICOLON, "expected ';' after print");
    return std::make_unique<PrintStmt>(std::move(expr), ln);
}

// input varname;
ASTPtr Parser::inputStatement() {
    int ln = current().line;
    consume(TokenType::KW_INPUT, "expected 'input'");
    Token name = consume(TokenType::IDENTIFIER, "expected variable name after 'input'");
    consume(TokenType::SEMICOLON, "expected ';' after input");
    return std::make_unique<InputStmt>(name.value, ln);
}

// if (condition) block [else block]
ASTPtr Parser::ifStatement() {
    int ln = current().line;
    consume(TokenType::KW_IF, "expected 'if'");
    consume(TokenType::LPAREN, "expected '(' after 'if'");
    ASTPtr cond = expression();
    consume(TokenType::RPAREN, "expected ')' after if condition");
    ASTPtr thenB = block();

    ASTPtr elseB = nullptr;
    if (match(TokenType::KW_ELSE)) {
        elseB = block();
    }

    return std::make_unique<IfStmt>(
        std::move(cond), std::move(thenB), std::move(elseB), ln);
}

// while (condition) block
ASTPtr Parser::whileStatement() {
    int ln = current().line;
    consume(TokenType::KW_WHILE, "expected 'while'");
    consume(TokenType::LPAREN, "expected '(' after 'while'");
    ASTPtr cond = expression();
    consume(TokenType::RPAREN, "expected ')' after while condition");
    ASTPtr body_ = block();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body_), ln);
}

// { statement* }
ASTPtr Parser::block() {
    int ln = current().line;
    consume(TokenType::LBRACE, "expected '{'");
    std::vector<ASTPtr> stmts;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        stmts.push_back(statement());
    }
    consume(TokenType::RBRACE, "expected '}'");
    return std::make_unique<Block>(std::move(stmts), ln);
}

// ---- Expression parsing (precedence climbing) ------------------------------

ASTPtr Parser::expression() {
    return equality();
}

// equality → comparison (("==" | "!=") comparison)*
ASTPtr Parser::equality() {
    ASTPtr left = comparison();
    while (check(TokenType::EQUAL_EQUAL) || check(TokenType::BANG_EQUAL)) {
        Token op = current(); pos_++;
        ASTPtr right = comparison();
        BinOp binOp = (op.type == TokenType::EQUAL_EQUAL) ? BinOp::EQ : BinOp::NEQ;
        left = std::make_unique<BinaryExpr>(
            binOp, std::move(left), std::move(right), op.line);
    }
    return left;
}

// comparison → addition (("<" | ">" | "<=" | ">=") addition)*
ASTPtr Parser::comparison() {
    ASTPtr left = addition();
    while (check(TokenType::LESS) || check(TokenType::GREATER) ||
           check(TokenType::LESS_EQUAL) || check(TokenType::GREATER_EQUAL)) {
        Token op = current(); pos_++;
        ASTPtr right = addition();
        BinOp binOp;
        switch (op.type) {
            case TokenType::LESS:          binOp = BinOp::LT;  break;
            case TokenType::GREATER:       binOp = BinOp::GT;  break;
            case TokenType::LESS_EQUAL:    binOp = BinOp::LTE; break;
            case TokenType::GREATER_EQUAL: binOp = BinOp::GTE; break;
            default: binOp = BinOp::LT; // unreachable
        }
        left = std::make_unique<BinaryExpr>(
            binOp, std::move(left), std::move(right), op.line);
    }
    return left;
}

// addition → multiplication (("+" | "-") multiplication)*
ASTPtr Parser::addition() {
    ASTPtr left = multiplication();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = current(); pos_++;
        ASTPtr right = multiplication();
        BinOp binOp = (op.type == TokenType::PLUS) ? BinOp::ADD : BinOp::SUB;
        left = std::make_unique<BinaryExpr>(
            binOp, std::move(left), std::move(right), op.line);
    }
    return left;
}

// multiplication → unary (("*" | "/") unary)*
ASTPtr Parser::multiplication() {
    ASTPtr left = unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        Token op = current(); pos_++;
        ASTPtr right = unary();
        BinOp binOp = (op.type == TokenType::STAR) ? BinOp::MUL : BinOp::DIV;
        left = std::make_unique<BinaryExpr>(
            binOp, std::move(left), std::move(right), op.line);
    }
    return left;
}

// unary → ("-" | "!") unary | primary
ASTPtr Parser::unary() {
    if (match(TokenType::MINUS)) {
        int ln = previous().line;
        ASTPtr operand = unary();
        return std::make_unique<UnaryExpr>(UnaryOp::NEG, std::move(operand), ln);
    }
    if (match(TokenType::BANG)) {
        int ln = previous().line;
        ASTPtr operand = unary();
        return std::make_unique<UnaryExpr>(UnaryOp::NOT, std::move(operand), ln);
    }
    return primary();
}

// primary → NUMBER | "true" | "false" | IDENTIFIER | "(" expression ")"
ASTPtr Parser::primary() {
    // Number literal
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberLiteral>(
            std::stoi(previous().value), previous().line);
    }
    // Boolean literals
    if (match(TokenType::TRUE_LIT)) {
        return std::make_unique<BoolLiteral>(true, previous().line);
    }
    if (match(TokenType::FALSE_LIT)) {
        return std::make_unique<BoolLiteral>(false, previous().line);
    }
    // Variable reference
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<Identifier>(previous().value, previous().line);
    }
    // Parenthesized expression
    if (match(TokenType::LPAREN)) {
        ASTPtr expr = expression();
        consume(TokenType::RPAREN, "expected ')' after expression");
        return expr;
    }

    throw std::runtime_error(
        "Parse error (line " + std::to_string(current().line) +
        "): unexpected token '" + current().value + "'");
}
