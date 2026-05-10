// ============================================================================
// CVM++ Main Entry Point
// CLI file runner: reads a .cvm script and pipes it through the full pipeline:
//   Source → Lexer → Tokens → Parser → AST → Compiler → Bytecode → VM
// ============================================================================

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

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
    // ---- Validate arguments ----
    if (argc < 2) {
        std::cerr << "CVM++ — Stack-Based Virtual Machine\n";
        std::cerr << "Usage: cvm++ <script.cvm>\n";
        return 1;
    }

    std::string filename = argv[1];

    try {
        // Step 1: Read source file
        std::string source = readFile(filename);

        // Step 2: Lexer — tokenize the source code
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        // Step 3: Parser — build Abstract Syntax Tree
        Parser parser(tokens);
        std::vector<ASTPtr> ast = parser.parse();

        // Step 4: Compiler — emit bytecode from AST
        Compiler compiler;
        std::vector<uint8_t> bytecode = compiler.compile(ast);

        // Step 5: VM — execute the bytecode
        VM vm;
        vm.execute(bytecode);

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
