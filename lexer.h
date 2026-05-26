#pragma once
// ============================================================================
// CVM++ Lexer Header — Tokenizer
// Converts raw source text into a stream of tokens for the parser.
// ============================================================================

#include <string>
#include <vector>
#include <stdexcept>

// Every distinct token the language understands
enum class TokenType {
    // Literals
    NUMBER,              // integer literal  (42, 0, 1000)
    IDENTIFIER,          // variable name    (x, sum, counter)
    TRUE_LIT,            // boolean true
    FALSE_LIT,           // boolean false

    // Arithmetic
    PLUS, MINUS, STAR, SLASH, PERCENT,  // + - * / %

    // Assignment & comparison
    EQUAL,               // =
    EQUAL_EQUAL,         // ==
    BANG_EQUAL,          // !=
    LESS, GREATER,       // < >
    LESS_EQUAL,          // <=
    GREATER_EQUAL,       // >=

    // Logical
    BANG,                // !
    AND_AND,             // &&
    PIPE_PIPE,           // ||

    // Delimiters
    LPAREN, RPAREN,      // ( )
    LBRACE, RBRACE,      // { }
    SEMICOLON,           // ;

    // Keywords
    KW_LET, KW_IF, KW_ELSE, KW_WHILE, KW_PRINT, KW_INPUT,

    // Sentinel
    END_OF_FILE
};

// A single token produced by the lexer
struct Token {
    TokenType   type;
    std::string value;   // raw text of the token
    int         line;    // source line (1-based)
};

// Lexer: scans source string and produces token list
class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string src_;
    size_t      pos_;
    int         line_;

    bool isAtEnd()  const;
    char current()  const;
    char peek()     const;
    char advance();
    bool match(char expected);

    void  skipWhitespaceAndComments();
    Token makeNumber();
    Token makeIdentifierOrKeyword();
    Token makeToken(TokenType type, const std::string& val);
};
