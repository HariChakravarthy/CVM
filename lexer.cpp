// ============================================================================
// CVM++ Lexer Implementation
// Scans source code character-by-character, producing tokens.
// ============================================================================

#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source)
    : src_(source), pos_(0), line_(1) {}

// ---- Character helpers -----------------------------------------------------

bool Lexer::isAtEnd() const { return pos_ >= src_.size(); }

char Lexer::current() const { return isAtEnd() ? '\0' : src_[pos_]; }

char Lexer::peek() const {
    return (pos_ + 1 < src_.size()) ? src_[pos_ + 1] : '\0';
}

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') line_++;
    return c;
}

// Consume the current char only if it matches `expected`
bool Lexer::match(char expected) {
    if (isAtEnd() || src_[pos_] != expected) return false;
    advance();
    return true;
}

Token Lexer::makeToken(TokenType type, const std::string& val) {
    return Token{type, val, line_};
}

// ---- Whitespace & comment skipping ----------------------------------------

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = current();
        // Skip spaces, tabs, carriage returns, newlines
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        }
        // Single-line comments: // ... until end of line
        else if (c == '/' && peek() == '/') {
            while (!isAtEnd() && current() != '\n') advance();
        }
        else break;
    }
}

// ---- Token builders --------------------------------------------------------

// Read a multi-digit integer literal
Token Lexer::makeNumber() {
    size_t start = pos_;
    while (!isAtEnd() && std::isdigit(current())) advance();
    return makeToken(TokenType::NUMBER, src_.substr(start, pos_ - start));
}

// Read an identifier or keyword
Token Lexer::makeIdentifierOrKeyword() {
    size_t start = pos_;
    while (!isAtEnd() && (std::isalnum(current()) || current() == '_'))
        advance();

    std::string word = src_.substr(start, pos_ - start);

    // Check against all language keywords
    if (word == "let")   return makeToken(TokenType::KW_LET,   word);
    if (word == "if")    return makeToken(TokenType::KW_IF,    word);
    if (word == "else")  return makeToken(TokenType::KW_ELSE,  word);
    if (word == "while") return makeToken(TokenType::KW_WHILE, word);
    if (word == "print") return makeToken(TokenType::KW_PRINT, word);
    if (word == "input") return makeToken(TokenType::KW_INPUT, word);
    if (word == "true")  return makeToken(TokenType::TRUE_LIT, word);
    if (word == "false") return makeToken(TokenType::FALSE_LIT,word);

    return makeToken(TokenType::IDENTIFIER, word);
}

// ---- Main tokenize loop ----------------------------------------------------

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;

        char c = current();

        // Numbers
        if (std::isdigit(c))              { tokens.push_back(makeNumber());              continue; }
        // Identifiers / keywords
        if (std::isalpha(c) || c == '_')  { tokens.push_back(makeIdentifierOrKeyword()); continue; }

        // Single-char & multi-char operators / delimiters
        advance(); // consume the character
        switch (c) {
            case '+': tokens.push_back(makeToken(TokenType::PLUS,      "+")); break;
            case '-': tokens.push_back(makeToken(TokenType::MINUS,     "-")); break;
            case '*': tokens.push_back(makeToken(TokenType::STAR,      "*")); break;
            case '/': tokens.push_back(makeToken(TokenType::SLASH,     "/")); break;
            case '(': tokens.push_back(makeToken(TokenType::LPAREN,    "(")); break;
            case ')': tokens.push_back(makeToken(TokenType::RPAREN,    ")")); break;
            case '{': tokens.push_back(makeToken(TokenType::LBRACE,    "{")); break;
            case '}': tokens.push_back(makeToken(TokenType::RBRACE,    "}")); break;
            case ';': tokens.push_back(makeToken(TokenType::SEMICOLON, ";")); break;

            // Two-character tokens: ==, !=, <=, >=
            case '=':
                if (match('=')) tokens.push_back(makeToken(TokenType::EQUAL_EQUAL, "=="));
                else            tokens.push_back(makeToken(TokenType::EQUAL,       "="));
                break;
            case '!':
                if (match('=')) tokens.push_back(makeToken(TokenType::BANG_EQUAL,  "!="));
                else            tokens.push_back(makeToken(TokenType::BANG,        "!"));
                break;
            case '<':
                if (match('=')) tokens.push_back(makeToken(TokenType::LESS_EQUAL,    "<="));
                else            tokens.push_back(makeToken(TokenType::LESS,          "<"));
                break;
            case '>':
                if (match('=')) tokens.push_back(makeToken(TokenType::GREATER_EQUAL, ">="));
                else            tokens.push_back(makeToken(TokenType::GREATER,       ">"));
                break;

            default:
                throw std::runtime_error(
                    "Lexer error (line " + std::to_string(line_) +
                    "): unexpected character '" + std::string(1, c) + "'");
        }
    }

    tokens.push_back(makeToken(TokenType::END_OF_FILE, ""));
    return tokens;
}
