// ============================================================================
// CVM++ Main Entry Point
//
// Usage:
//   cvm++                       — launch interactive REPL
//   cvm++ <script.cvm>          — run a script file normally
//   cvm++ -d <script.cvm>       — debug mode: print AST + bytecode, then run
//   cvm++ --debug <script.cvm>  — same as -d
// ============================================================================

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "debug.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ---- Helpers ---------------------------------------------------------------

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Error: could not open file '" + path + "'");
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ---- REPL ------------------------------------------------------------------
//
// How it works:
//   - Compiler keeps its variable table (vars_ / nextSlot_) across lines via
//     compileRepl(), so variables declared on one line are visible on the next.
//   - VM keeps its globals_ array across lines via executeChunk(), so stored
//     values survive between prompts.
//   - Each line gets a fresh operand stack (executeChunk clears stack_).
//   - Errors on one line are caught and printed; the session continues.

static void runRepl() {
    std::cout << "CVM++ REPL  (type 'exit' or 'quit' to leave)\n";
    std::cout << "--------------------------------------------\n";

    Compiler compiler;
    VM       vm;
    vm.initGlobals();   // set up global storage once

    std::string line;
    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) break;      // EOF (Ctrl-D / Ctrl-Z)

        // Trim leading/trailing whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;      // blank line
        line = line.substr(start);

        if (line == "exit" || line == "quit") break;

        try {
            Lexer               lexer(line);
            std::vector<Token>  tokens = lexer.tokenize();

            Parser              parser(tokens);
            std::vector<ASTPtr> ast = parser.parse();

            std::vector<uint8_t> bytecode = compiler.compileRepl(ast);

            vm.executeChunk(bytecode);

        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << "\n";
            // Session continues — don't exit on error
        }
    }

    std::cout << "\nBye!\n";
}

// ---- File runner -----------------------------------------------------------

static void runFile(const std::string& filename, bool debugMode) {
    std::string source = readFile(filename);

    if (debugMode) {
        std::cout << "+======================================+\n";
        std::cout << "|       CVM++ Debug Mode               |\n";
        std::cout << "+======================================+\n";
        std::cout << "File: " << filename << "\n";
        std::cout << "Source:\n  ";
        for (char c : source) {
            std::cout << c;
            if (c == '\n') std::cout << "  ";
        }
        std::cout << "\n";
    }

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::vector<ASTPtr> ast = parser.parse();

    if (debugMode) printAST(ast);

    Compiler compiler;
    std::vector<uint8_t> bytecode = compiler.compile(ast);

    if (debugMode) {
        disassemble(bytecode);
        std::cout << "+======================================+\n";
        std::cout << "|          Execution Output            |\n";
        std::cout << "+======================================+\n";
    }

    VM vm;
    vm.execute(bytecode);
}

// ---- Entry point -----------------------------------------------------------

int main(int argc, char* argv[]) {
    bool        debugMode = false;
    std::string filename;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-d" || arg == "--debug") {
            debugMode = true;
        } else if (filename.empty()) {
            filename = arg;
        } else {
            std::cerr << "Error: unexpected argument '" << arg << "'\n";
            return 1;
        }
    }

    // No filename → launch REPL
    if (filename.empty()) {
        runRepl();
        return 0;
    }

    try {
        runFile(filename, debugMode);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}