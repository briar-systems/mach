#ifndef MASM_OPERAND_H
#define MASM_OPERAND_H

#include <stdint.h>

// masm operand kinds
typedef enum MasmOperandKind
{
    MASM_OPERAND_NONE,
    MASM_OPERAND_REGISTER,
    MASM_OPERAND_IMM,
    MASM_OPERAND_MEMORY,
    MASM_OPERAND_SYMBOL,
    MASM_OPERAND_LABEL
} MasmOperandKind;

// register definition
typedef struct MasmRegister
{
    uint32_t id;
    uint8_t  size; // in bytes
} MasmRegister;

// memory reference: [base + index * scale + disp]
typedef struct MasmMemory
{
    MasmRegister base;
    MasmRegister index;
    uint8_t      scale; // 1, 2, 4, 8
    int64_t      disp;
    uint8_t      size;  // access size in bytes
} MasmMemory;

// masm operand
typedef struct MasmOperand
{
    MasmOperandKind kind;
    union
    {
        MasmRegister reg;       // MASM_OPERAND_REGISTER
        int64_t      imm;       // MASM_OPERAND_IMM
        MasmMemory   mem;       // MASM_OPERAND_MEMORY
        const char  *symbol;    // MASM_OPERAND_SYMBOL
        const char  *label;     // MASM_OPERAND_LABEL
    };
} MasmOperand;

// operand builders
MasmOperand masm_operand_none();
MasmOperand masm_operand_register(uint32_t id, uint8_t size);
MasmOperand masm_operand_imm(int64_t value);
MasmOperand masm_operand_memory(MasmRegister base, MasmRegister index, uint8_t scale, int64_t disp, uint8_t size);
MasmOperand masm_operand_symbol(const char *name);
MasmOperand masm_operand_label(const char *name);

#endif // MASM_OPERAND_H
