// ============================================================================
// CVM++ Virtual Machine Implementation
// Fetch-decode-execute loop over the bytecode produced by the compiler.
// ============================================================================

#include "vm.h"
#include <iostream>
#include <cstring>
#include <stdexcept>

VM::VM() : code_(nullptr), pc_(0) {}

// ---- Stack operations ------------------------------------------------------

void VM::push(int32_t value) {
    stack_.push_back(value);
}

int32_t VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("VM error: stack underflow");
    }
    int32_t val = stack_.back();
    stack_.pop_back();
    return val;
}

int32_t VM::top() const {
    if (stack_.empty()) {
        throw std::runtime_error("VM error: stack underflow (top)");
    }
    return stack_.back();
}

// ---- Bytecode reading helpers ----------------------------------------------

uint8_t VM::readByte() {
    return code_[pc_++];
}

int32_t VM::readInt32() {
    int32_t value;
    std::memcpy(&value, &code_[pc_], 4);
    pc_ += 4;
    return value;
}

uint16_t VM::readUint16() {
    uint16_t value;
    std::memcpy(&value, &code_[pc_], 2);
    pc_ += 2;
    return value;
}

// ---- Public entry points ---------------------------------------------------

// Full reset — used by the file runner
void VM::execute(const std::vector<uint8_t>& bytecode) {
    code_ = bytecode.data();
    pc_   = 0;
    stack_.clear();
    globals_.clear();
    globals_.resize(256, 0);
    runLoop();
}

// REPL: call once before the loop to set up global storage
void VM::initGlobals() {
    globals_.clear();
    globals_.resize(256, 0);
}

// REPL: run one chunk; globals persist between calls so variables survive
void VM::executeChunk(const std::vector<uint8_t>& bytecode) {
    code_ = bytecode.data();
    pc_   = 0;
    stack_.clear();                                   // fresh stack each line
    if (globals_.size() < 256) globals_.resize(256, 0); // keep existing globals
    runLoop();
}

// ---- Shared fetch-decode-execute loop -------------------------------------

void VM::runLoop() {
    while (true) {
        uint8_t instruction = readByte();

        switch (static_cast<OpCode>(instruction)) {

        // ---- Constants & literals ----

        case OP_CONST: {
            int32_t val = readInt32();
            push(val);
            break;
        }
        case OP_TRUE:  push(1); break;
        case OP_FALSE: push(0); break;

        // ---- Arithmetic ----

        case OP_ADD: { int32_t b = pop(), a = pop(); push(a + b); break; }
        case OP_SUB: { int32_t b = pop(), a = pop(); push(a - b); break; }
        case OP_MUL: { int32_t b = pop(), a = pop(); push(a * b); break; }
        case OP_DIV: {
            int32_t b = pop(), a = pop();
            if (b == 0) throw std::runtime_error("VM error: division by zero");
            push(a / b);
            break;
        }

        // ---- Comparisons ----

        case OP_EQ:  { int32_t b = pop(), a = pop(); push(a == b ? 1 : 0); break; }
        case OP_NEQ: { int32_t b = pop(), a = pop(); push(a != b ? 1 : 0); break; }
        case OP_LT:  { int32_t b = pop(), a = pop(); push(a <  b ? 1 : 0); break; }
        case OP_GT:  { int32_t b = pop(), a = pop(); push(a >  b ? 1 : 0); break; }
        case OP_LTE: { int32_t b = pop(), a = pop(); push(a <= b ? 1 : 0); break; }
        case OP_GTE: { int32_t b = pop(), a = pop(); push(a >= b ? 1 : 0); break; }

        // ---- Unary ----

        case OP_NEG: { push(-pop()); break; }
        case OP_NOT: { push(pop() == 0 ? 1 : 0); break; }

        // ---- Variables ----

        case OP_LOAD: {
            uint16_t slot = readUint16();
            if (slot >= globals_.size()) globals_.resize(slot + 1, 0);
            push(globals_[slot]);
            break;
        }
        case OP_STORE: {
            uint16_t slot = readUint16();
            int32_t  val  = pop();
            if (slot >= globals_.size()) globals_.resize(slot + 1, 0);
            globals_[slot] = val;
            break;
        }

        // ---- I/O ----

        case OP_PRINT: {
            int32_t val = pop();
            std::cout << val << "\n";
            break;
        }
        case OP_INPUT: {
            int32_t val = 0;
            std::cout << ">> ";
            if (!(std::cin >> val)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                std::cerr << "Warning: invalid input, using 0\n";
                val = 0;
            }
            push(val);
            break;
        }

        // ---- Control flow ----

        case OP_JMP: {
            int32_t target = readInt32();
            pc_ = static_cast<size_t>(target);
            break;
        }
        case OP_JMP_IF_FALSE: {
            int32_t target = readInt32();
            int32_t cond   = pop();
            if (cond == 0) pc_ = static_cast<size_t>(target);
            break;
        }

        // ---- Stack management ----

        case OP_POP: pop(); break;

        // ---- Halt ----

        case OP_HALT: return;

        default:
            throw std::runtime_error(
                "VM error: unknown opcode " + std::to_string(instruction) +
                " at pc=" + std::to_string(pc_ - 1));
        }
    }
}