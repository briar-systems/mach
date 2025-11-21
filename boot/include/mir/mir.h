#ifndef MIR_H
#define MIR_H

#include "frontend/type.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// forward declarations
typedef struct MirInstruction MirInstruction;
typedef struct MirBasicBlock  MirBasicBlock;
typedef struct MirModule      MirModule;
typedef struct MirData        MirData;

// MIR opcodes - universal instructions
typedef enum MirOpcode
{
    // data movement
    MIR_OP_MOV,   // mov.T dst, src
    MIR_OP_LOAD,  // load.T dst, [addr]
    MIR_OP_STORE, // store.T [addr], src
    MIR_OP_LEA,   // lea.ptr dst, label

    // arithmetic
    MIR_OP_ADD, // add.T dst, a, b
    MIR_OP_SUB, // sub.T dst, a, b
    MIR_OP_MUL, // mul.T dst, a, b
    MIR_OP_DIV, // div.T dst, a, b
    MIR_OP_MOD, // mod.T dst, a, b
    MIR_OP_NEG, // neg.T dst, src

    // bitwise
    MIR_OP_AND, // and.T dst, a, b
    MIR_OP_OR,  // or.T dst, a, b
    MIR_OP_XOR, // xor.T dst, a, b
    MIR_OP_NOT, // not.T dst, src
    MIR_OP_SHL, // shl.T dst, val, shift
    MIR_OP_SHR, // shr.T dst, val, shift (logical)
    MIR_OP_SAR, // sar.T dst, val, shift (arithmetic)

    // comparison
    MIR_OP_CMP_EQ, // cmp.T.eq dst, a, b
    MIR_OP_CMP_NE, // cmp.T.ne dst, a, b
    MIR_OP_CMP_LT, // cmp.T.lt dst, a, b
    MIR_OP_CMP_LE, // cmp.T.le dst, a, b
    MIR_OP_CMP_GT, // cmp.T.gt dst, a, b
    MIR_OP_CMP_GE, // cmp.T.ge dst, a, b

    // type conversions
    MIR_OP_ZEXT,    // zext.T1.T2 dst, src (zero extend)
    MIR_OP_SEXT,    // sext.T1.T2 dst, src (sign extend)
    MIR_OP_TRUNC,   // trunc.T1.T2 dst, src (truncate)
    MIR_OP_BITCAST, // bitcast.T1.T2 dst, src (reinterpret)

    // control flow
    MIR_OP_JMP,   // jmp label
    MIR_OP_BR,    // br.T cond, true_label, false_label
    MIR_OP_CALL,  // call label
    MIR_OP_RET,   // ret
    MIR_OP_UNREACHABLE,

    // stack
    MIR_OP_ALLOCA, // alloca.T dst, size

    // architecture-specific (namespace-prefixed in text form)
    MIR_OP_ARCH_SYSCALL, // x86.syscall
    MIR_OP_ARCH_SVC,     // arm.svc immediate
    MIR_OP_ARCH_ECALL,   // riscv.ecall
    MIR_OP_ARCH_HLT,     // x86.hlt
} MirOpcode;

// operand kinds
typedef enum MirOperandKind
{
    MIR_OPERAND_NONE,   // no operand
    MIR_OPERAND_VREG,   // virtual register (%name or %number)
    MIR_OPERAND_PREG,   // physical register (rax, rdi, x0, etc.)
    MIR_OPERAND_IMM,    // immediate value (integer)
    MIR_OPERAND_LABEL,  // label reference
    MIR_OPERAND_MEMORY, // memory operand [base+offset]
} MirOperandKind;

// physical register (architecture-specific)
typedef enum MirPhysicalReg
{
    // x86_64 registers
    MIR_PREG_RAX,
    MIR_PREG_RBX,
    MIR_PREG_RCX,
    MIR_PREG_RDX,
    MIR_PREG_RSI,
    MIR_PREG_RDI,
    MIR_PREG_RBP,
    MIR_PREG_RSP,
    MIR_PREG_R8,
    MIR_PREG_R9,
    MIR_PREG_R10,
    MIR_PREG_R11,
    MIR_PREG_R12,
    MIR_PREG_R13,
    MIR_PREG_R14,
    MIR_PREG_R15,

    // ARM64 registers (future)
    // RISC-V registers (future)
} MirPhysicalReg;

// MIR operand
typedef struct MirOperand
{
    MirOperandKind kind;
    union
    {
        uint32_t      vreg_id;       // MIR_OPERAND_VREG
        MirPhysicalReg preg;          // MIR_OPERAND_PREG
        int64_t       immediate;     // MIR_OPERAND_IMM
        const char   *label;         // MIR_OPERAND_LABEL
        struct
        {
            uint32_t base_vreg; // base virtual register
            int32_t  offset;    // constant offset
        } memory;               // MIR_OPERAND_MEMORY
    };
} MirOperand;

// MIR instruction
struct MirInstruction
{
    MirOpcode     opcode;
    Type         *type;        // instruction type (i64, ptr, etc.)
    Type         *type2;       // secondary type (for conversions)
    MirOperand    operands[3]; // dst, src1, src2 (max 3 operands)
    size_t        operand_count;
    MirInstruction *next;
};

// MIR basic block
struct MirBasicBlock
{
    const char     *label;            // block label
    bool            is_exported;      // pub label (visible to linker)
    MirInstruction *instructions;    // instruction list
    MirInstruction *last_instruction; // tail pointer for O(1) append
    size_t          instruction_count;
    MirBasicBlock  *next;
};

// MIR data declaration kind
typedef enum MirDataKind
{
    MIR_DATA_VAL,      // val NAME: type = value (read-only)
    MIR_DATA_VAR,      // var NAME: type = value (mutable)
    MIR_DATA_VAR_UNINIT, // var NAME: type (uninitialized)
    MIR_DATA_EXT,      // ext NAME: type (external data)
    MIR_DATA_EXT_LABEL // ext NAME (external label)
} MirDataKind;

// MIR data declaration
struct MirData
{
    MirDataKind kind;
    const char *name;
    Type       *type; // NULL for ext labels
    union
    {
        int64_t     int_value;
        double      float_value;
        const char *string_value; // for string literals
        struct
        {
            int64_t *values;
            size_t   count;
        } array_int;
        struct
        {
            double *values;
            size_t  count;
        } array_float;
    } init_value;
    MirData *next;
};

// MIR module (compilation unit)
struct MirModule
{
    const char    *name;         // module name
    MirData       *data;         // data declarations
    MirData       *last_data;    // tail pointer
    MirBasicBlock *blocks;       // basic blocks
    MirBasicBlock *last_block;   // tail pointer
    uint32_t       next_vreg_id; // virtual register counter
    uint32_t       next_label_id; // anonymous label counter
};

// module management
MirModule      *mir_module_create(const char *name);
void            mir_module_destroy(MirModule *module);
MirBasicBlock  *mir_module_add_block(MirModule *module, const char *label, bool is_exported);
MirData        *mir_module_add_data(MirModule *module, MirDataKind kind, const char *name, Type *type);
uint32_t        mir_module_alloc_vreg(MirModule *module);
const char     *mir_module_gen_label(MirModule *module, const char *prefix);

// block management
MirBasicBlock  *mir_block_create(const char *label, bool is_exported);
void            mir_block_destroy(MirBasicBlock *block);
void            mir_block_add_instruction(MirBasicBlock *block, MirInstruction *inst);

// instruction builders
MirInstruction *mir_inst_create_0op(MirOpcode opcode);
MirInstruction *mir_inst_create_1op(MirOpcode opcode, Type *type, MirOperand dst);
MirInstruction *mir_inst_create_2op(MirOpcode opcode, Type *type, MirOperand dst, MirOperand src);
MirInstruction *mir_inst_create_3op(MirOpcode opcode, Type *type, MirOperand dst, MirOperand src1, MirOperand src2);
void            mir_inst_destroy(MirInstruction *inst);

// operand builders
MirOperand mir_operand_none(void);
MirOperand mir_operand_vreg(uint32_t id);
MirOperand mir_operand_preg(MirPhysicalReg preg);
MirOperand mir_operand_imm(int64_t value);
MirOperand mir_operand_label(const char *label);
MirOperand mir_operand_memory(uint32_t base_vreg, int32_t offset);

// physical register helpers
MirPhysicalReg  mir_preg_from_name(const char *name);
const char     *mir_preg_to_name(MirPhysicalReg preg);

#endif // MIR_H
