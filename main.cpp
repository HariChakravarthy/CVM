// ============================================================================
// CVM++ Main Entry Point
// CLI file runner: reads a .cvm script and pipes it through the full pipeline:
//   Source → Lexer → Tokens → Parser → AST → Compiler → Bytecode → VM
//
// Usage:
//   cvm++ <script.cvm>          — run normally
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

// Read an entire file into a string
static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Error: could not open file '" + path + "'");
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    // ---- Parse arguments ----
    bool debugMode    = false;
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

    if (filename.empty()) {
        std::cerr << "CVM++ — Stack-Based Virtual Machine\n";
        std::cerr << "Usage: cvm++ [-d|--debug] <script.cvm>\n";
        std::cerr << "\n";
        std::cerr << "  -d, --debug    Print AST and bytecode before execution\n";
        return 1;
    }

    try {
        // Step 1: Read source file
        std::string source = readFile(filename);

        if (debugMode) {
            std::cout << "+======================================+\n";
            std::cout << "|       CVM++ Debug Mode               |\n";
            std::cout << "+======================================+\n";
            std::cout << "File: " << filename << "\n";
            std::cout << "Source:\n";
            std::cout << "  " ;
            for (char c : source) {
                std::cout << c;
                if (c == '\n') std::cout << "  ";
            }
            std::cout << "\n";
        }

        // Step 2: Lexer — tokenize the source code
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        // Step 3: Parser — build Abstract Syntax Tree
        Parser parser(tokens);
        std::vector<ASTPtr> ast = parser.parse();

        // [DEBUG] Print AST
        if (debugMode) {
            printAST(ast);
        }

        // Step 4: Compiler — emit bytecode from AST
        Compiler compiler;
        std::vector<uint8_t> bytecode = compiler.compile(ast);

        // [DEBUG] Disassemble bytecode
        if (debugMode) {
            disassemble(bytecode);
            std::cout << "+======================================+\n";
            std::cout << "|          Execution Output            |\n";
            std::cout << "+======================================+\n";
        }

        // Step 5: VM — execute the bytecode
        VM vm;
        vm.execute(bytecode);

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}