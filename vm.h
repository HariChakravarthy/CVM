#pragma once
// ============================================================================
// CVM++ Virtual Machine Header
// Stack-based bytecode interpreter.
// ============================================================================

#include "compiler.h"
#include <vector>
#include <cstdint>

class VM {
public:
    VM();

    // Execute a compiled bytecode program (resets all state)
    void execute(const std::vector<uint8_t>& bytecode);

    // REPL: execute a chunk WITHOUT resetting globals (variables persist)
    void executeChunk(const std::vector<uint8_t>& bytecode);

    // REPL: initialise global storage once before the REPL loop starts
    void initGlobals();

private:
    std::vector<int32_t> stack_;     // operand stack
    std::vector<int32_t> globals_;   // variable storage (indexed by slot)
    const uint8_t* code_;            // pointer to bytecode array
    size_t pc_;                      // program counter

    // Shared fetch-decode-execute loop used by both execute() and executeChunk()
    void runLoop();

    // Stack operations
    void     push(int32_t value);
    int32_t  pop();
    int32_t  top() const;

    // Bytecode reading helpers (advance pc)
    uint8_t  readByte();
    int32_t  readInt32();
    uint16_t readUint16();
};