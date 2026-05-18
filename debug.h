#pragma once
// ============================================================================
// CVM++ Debug Utilities
// AST pretty-printer and bytecode disassembler for the --debug / -d flag.
// ============================================================================

#include "parser.h"
#include "compiler.h"
#include <vector>
#include <cstdint>

// Print the full AST to stdout with indentation
void printAST(const std::vector<ASTPtr>& program);

// Disassemble bytecode to stdout, showing each opcode + operands
void disassemble(const std::vector<uint8_t>& code);