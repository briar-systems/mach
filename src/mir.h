#ifndef MIR_H
#define MIR_H

#include "ast.h"
#include "target.h"
#include "type.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct MIRInst MIRInst;
typedef struct MIRValue MIRValue;
typedef struct MIRBlock MIRBlock;
typedef struct MIRFunction MIRFunction;
typedef struct MIRModule MIRModule;
typedef struct MIRContext MIRContext;

typedef enum
{
    MIR_NOP, // No operation

    MIR_ALLOCA, // Allocate stack memory
    MIR_LOAD,   // Load from memory
    MIR_STORE,  // Store to memory
    MIR_OFFSET, // Calculate pointer offset (for struct fields, array indexing)

    MIR_ADD, // Addition
    MIR_SUB, // Subtraction
    MIR_MUL, // Multiplication
    MIR_DIV, // Division
    MIR_MOD, // Modulo
    MIR_NEG, // Negation

    MIR_AND, // Bitwise AND
    MIR_OR,  // Bitwise OR
    MIR_XOR, // Bitwise XOR
    MIR_SHL, // Shift left
    MIR_SHR, // Shift right
    MIR_NOT, // Bitwise NOT

    MIR_EQ, // Equal
    MIR_NE, // Not equal
    MIR_LT, // Less than
    MIR_LE, // Less than or equal
    MIR_GT, // Greater than
    MIR_GE, // Greater than or equal

    MIR_JMP,  // Unconditional jump
    MIR_CJMP, // Conditional jump
    MIR_RET,  // Return from function
    MIR_CALL, // Function call

    MIR_CAST, // Type conversion

    MIR_CONST_INT,   // Integer constant
    MIR_CONST_FLOAT, // Float constant
    MIR_CONST_PTR,   // Pointer constant (e.g. null)
    MIR_CONST_STR,   // String constant

    MIR_PHI, // PHI node for SSA form
} MIROpcode;

typedef enum
{
    MIR_VAL_ERROR, // Error value

    MIR_VAL_NONE,     // No value
    MIR_VAL_REGISTER, // Virtual register
    MIR_VAL_GLOBAL,   // Global variable
    MIR_VAL_CONSTANT, // Constant value
    MIR_VAL_BLOCK,    // Basic block (for jumps)
    MIR_VAL_FUNCTION, // Function reference
} MIRValueKind;

struct MIRValue
{
    MIRValueKind kind;
    Type *type;

    union
    {
        struct
        {
            const char *message;
        } err;

        struct
        {
            unsigned int id;
        } reg;

        struct
        {
            char *name;
        } global;

        struct
        {
            union
            {
                long long i;
                double f;
                char *s;
            };
        } constant;

        MIRBlock *block;
        MIRFunction *function;
    };
};

struct MIRInst
{
    MIROpcode opcode;
    MIRValue result;
    MIRValue *operands;
    int operand_count;

    MIRInst *next;
    MIRInst *prev;

    MIRBlock *parent;

    Node *source_node;
};

struct MIRBlock
{
    MIRFunction *parent;

    MIRBlock **predecessors;
    int predecessor_count;
    MIRBlock **successors;
    int successor_count;

    MIRInst *first;
    MIRInst *last;
    bool is_entry;
    bool is_exit;

    char *label;
};

struct MIRFunction
{
    MIRModule *parent;

    char *name;
    Type *type;

    MIRBlock **blocks;
    int block_count;
    MIRValue *parameters;
    int parameter_count;

    MIRBlock *entry_block;

    int reg_count;
};

struct MIRModule
{
    MIRContext *context;
    Target target;

    char *name;

    MIRFunction **functions;
    int function_count;
};

struct MIRContext
{
    MIRModule **modules;
    int module_count;

    int string_constant_count;
    char **string_constants;
};

bool mir_context_init(MIRContext *context);
void mir_context_free(MIRContext *context);
bool mir_context_add(MIRContext *context, MIRModule *module);

bool mir_module_init(MIRModule *module, MIRContext *context, const char *name, Target target);
void mir_module_free(MIRModule *module);
bool mir_module_add_function(MIRModule *module, MIRFunction *function);

bool mir_function_init(MIRFunction *function, MIRModule *module, const char *name, Type *type);
void mir_function_free(MIRFunction *function);
MIRValue mir_function_add_parameter(MIRFunction *function, Type *type);
bool mir_function_add_block(MIRFunction *function, MIRBlock *block);

bool mir_block_init(MIRBlock *block, MIRFunction *function, const char *label);
void mir_block_free(MIRBlock *block);
bool mir_block_add_successor(MIRBlock *block, MIRBlock *successor);
bool mir_block_add_predecessor(MIRBlock *block, MIRBlock *predecessor);

bool mir_inst_init(MIRInst *inst, MIRBlock *block, MIROpcode opcode);
void mir_inst_free(MIRInst *inst);
void mir_inst_remove(MIRInst *inst);
void mir_inst_insert_after(MIRInst *inst, MIRInst *after);
void mir_inst_insert_before(MIRInst *inst, MIRInst *before);
bool mir_inst_add_operand(MIRInst *inst, MIRValue operand);

MIRValue mir_value_error(const char *message);
MIRValue mir_value_register(Type *type, unsigned int id);
MIRValue mir_value_global(Type *type, const char *name);
MIRValue mir_value_block(MIRBlock *block);
MIRValue mir_value_function(MIRFunction *function);

MIRValue mir_const_int(Type *type, long long value);
MIRValue mir_const_float(Type *type, double value);
MIRValue mir_const_ptr(Type *type, void *value);
MIRValue mir_const_str(MIRContext *context, const char *value);

MIRValue mir_create_register(MIRFunction *function, Type *type);

MIRValue mir_build_alloca(MIRBlock *block, Type *type);
MIRValue mir_build_load(MIRBlock *block, MIRValue ptr);
bool mir_build_store(MIRBlock *block, MIRValue ptr, MIRValue value);
MIRValue mir_build_offset(MIRBlock *block, MIRValue base, MIRValue index, size_t element_size);
MIRValue mir_build_field_offset(MIRBlock *block, MIRValue base, int field_index);

MIRValue mir_build_binary(MIRBlock *block, MIROpcode opcode, MIRValue left, MIRValue right);
MIRValue mir_build_unary(MIRBlock *block, MIROpcode opcode, MIRValue operand);

bool mir_build_jmp(MIRBlock *block, MIRBlock *target);
bool mir_build_cjmp(MIRBlock *block, MIRValue condition, MIRBlock *true_target, MIRBlock *false_target);
bool mir_build_ret(MIRBlock *block, MIRValue value);
MIRValue mir_build_call(MIRBlock *block, MIRValue function, MIRValue *args, int arg_count);

MIRValue mir_build_cast(MIRBlock *block, MIRValue value, Type *target_type);

void mir_compute_cfg(MIRFunction *function);
void mir_validate(MIRModule *module);
void mir_optimize(MIRModule *module);

void mir_print_module(MIRModule *module);
void mir_print_function(MIRFunction *function);
void mir_print_block(MIRBlock *block);
void mir_print_inst(MIRInst *inst);

const char *mir_opcode_name(MIROpcode opcode);
void mir_dump_dot(MIRFunction *function, const char *filename); // Dump CFG in GraphViz format

#endif // MIR_H
