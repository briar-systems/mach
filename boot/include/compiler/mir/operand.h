#ifndef MIR_OPERAND_H
#define MIR_OPERAND_H

#include <stdint.h>

// mir operand kinds
typedef enum MIROperandKind
{
    MIR_OPERAND_NONE,
    MIR_OPERAND_VALUE,   // ssa value reference
    MIR_OPERAND_IMM_INT, // immediate integer
    MIR_OPERAND_IMM_FLT, // immediate float
    MIR_OPERAND_GLOBAL,  // global symbol reference
    MIR_OPERAND_BLOCK,   // basic block reference
} MIROperandKind;

// mir operand
typedef struct MIROperand
{
    MIROperandKind kind;
    union
    {
        uint32_t    value_id;     // MIR_OPERAND_VALUE
        int64_t     imm_int;      // MIR_OPERAND_IMM_INT
        double      imm_flt;      // MIR_OPERAND_IMM_FLT
        const char *global_name;  // MIR_OPERAND_GLOBAL
        uint32_t    block_id;     // MIR_OPERAND_BLOCK
    };
} MIROperand;

// operand builders
MIROperand mir_operand_none();
MIROperand mir_operand_value(uint32_t value_id);
MIROperand mir_operand_imm_int(int64_t value);
MIROperand mir_operand_imm_flt(double value);
MIROperand mir_operand_global(const char *name);
MIROperand mir_operand_block(uint32_t block_id);

#endif // MIR_OPERAND_H
