#ifndef MACH_H

#include <stdbool.h>
#include <stdint.h>

#define FRAMES_MAX 64
#define REGISTERS_COUNT 256  // Number of registers per frame
#define CONSTANTS_MAX 65536  // Maximum constants pool size

typedef uint8_t reg_t;       // Register identifier type
typedef uint64_t val_t;      // Value type (NaN boxed)

// Keep your existing value representation (NaN boxing)
// ... existing NaN boxing code ...

// Instruction format - 32 bits total
typedef struct {
    uint8_t op;       // Operation code
    reg_t dest;       // Destination register
    reg_t src1;       // Source register 1
    reg_t src2;       // Source register 2
} Instruction;

// Operation codes for register machine
typedef enum {
    // Register operations
    OP_NOP,      // No operation
    OP_MOV,      // Move value between registers: MOV dest, src
    OP_LOADK,    // Load constant: LOADK dest, const_idx
    
    // Arithmetic (3-register operations)
    OP_ADD,      // Add: ADD dest, src1, src2
    OP_SUB,      // Subtract: SUB dest, src1, src2
    OP_MUL,      // Multiply: MUL dest, src1, src2
    OP_DIV,      // Divide: DIV dest, src1, src2
    OP_MOD,      // Modulo: MOD dest, src1, src2
    
    // Arithmetic (2-register operations)
    OP_NEG,      // Negate: NEG dest, src
    
    // Bitwise operations
    OP_B_AND,    // Bitwise AND: B_AND dest, src1, src2
    OP_B_OR,     // Bitwise OR: B_OR dest, src1, src2
    OP_B_XOR,    // Bitwise XOR: B_XOR dest, src1, src2
    OP_B_NOT,    // Bitwise NOT: B_NOT dest, src
    OP_B_SHL,    // Shift left: B_SHL dest, src1, src2
    OP_B_SHR,    // Shift right: B_SHR dest, src1, src2
    
    // Comparison operations
    OP_EQ,       // Equal: EQ dest, src1, src2
    OP_NEQ,      // Not equal: NEQ dest, src1, src2
    OP_GT,       // Greater than: GT dest, src1, src2
    OP_GTE,      // Greater than or equal: GTE dest, src1, src2
    OP_LT,       // Less than: LT dest, src1, src2
    OP_LTE,      // Less than or equal: LTE dest, src1, src2
    
    // Logical operations
    OP_L_AND,    // Logical AND: L_AND dest, src1, src2
    OP_L_OR,     // Logical OR: L_OR dest, src1, src2
    OP_L_NOT,    // Logical NOT: L_NOT dest, src
    
    // Control flow
    OP_JMP,      // Jump: JMP offset
    OP_JZ,       // Jump if zero: JZ src, offset
    OP_JNZ,      // Jump if not zero: JNZ src, offset
    OP_CALL,     // Call: CALL dest, src, nargs
    OP_RET,      // Return: RET src
} OpCode;

// Function structure (code container)
typedef struct {
    Instruction* code;        // Array of instructions
    int code_len;            // Length of code array
    int code_cap;            // Capacity of code array
    
    val_t* constants;        // Constants pool
    int const_len;
    int const_cap;
    
    int* lines;              // Line number information
} Function;

// Chunk structure (compilation unit)
typedef struct {
    Function* functions;     // Array of functions
    int func_len;
    int func_cap;
    
    int main_func;           // Index of main function
} Chunk;

// Call frame for function calls
typedef struct {
    Instruction* return_addr;// Return address
    reg_t base_reg;          // Base register for this frame
    int func_idx;            // Function index
} CallFrame;

// VM structure
typedef struct {
    Chunk* chunk;            // Current chunk
    Instruction* ip;         // Instruction pointer
    
    val_t* registers;        // Register array
    CallFrame* frames;       // Call frames
    int frame_count;         // Current frame count
} VM;

typedef enum {
    VM_RUN_OK,
    VM_RUN_ERR_COMPILE,
    VM_RUN_ERR_RUNTIME
} VMRunResult;

// Chunk manipulation
void chunk_init(Chunk* chunk);
void chunk_free(Chunk* chunk);
int chunk_add_function(Chunk* chunk);
int chunk_add_constant(Chunk* chunk, Function* func, val_t val);
void chunk_write_instruction(Chunk* chunk, Function* func, Instruction inst, int line);

// Instruction creation helpers
Instruction instruction_create(OpCode op, reg_t dest, reg_t src1, reg_t src2);
Instruction instruction_3reg(OpCode op, reg_t dest, reg_t src1, reg_t src2);
Instruction instruction_2reg(OpCode op, reg_t dest, reg_t src);
Instruction instruction_jump(OpCode op, int offset);
Instruction instruction_loadk(reg_t dest, uint16_t const_idx);

// Debugging
void chunk_disassemble(Chunk* chunk, const char* name);
void function_disassemble(Chunk* chunk, Function* func, const char* name);
int instruction_disassemble(Chunk* chunk, Function* func, Instruction* instr, int offset);

// VM operations
void vm_init(VM* vm);
void vm_free(VM* vm);
VMRunResult vm_run(VM* vm, Chunk* chunk);

// Register access helpers
static inline val_t vm_get_register(VM* vm, reg_t reg) {
    return vm->registers[reg];
}

static inline void vm_set_register(VM* vm, reg_t reg, val_t val) {
    vm->registers[reg] = val;
}

#endif // MACH_H