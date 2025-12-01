#include "compiler/masm/isa/x86_64.h"
#include <stdio.h>

MasmOperand masm_x86_reg(MasmX86Reg reg, uint8_t size)
{
    return masm_operand_register((uint32_t)reg, size);
}

static void emit_byte(uint8_t *buffer, size_t *offset, size_t size, uint8_t byte)
{
    if (*offset < size)
    {
        buffer[(*offset)++] = byte;
    }
}

static void emit_imm64(uint8_t *buffer, size_t *offset, size_t size, int64_t imm)
{
    for (int i = 0; i < 8; i++)
    {
        emit_byte(buffer, offset, size, (imm >> (i * 8)) & 0xFF);
    }
}

static void emit_imm32(uint8_t *buffer, size_t *offset, size_t size, int32_t imm)
{
    for (int i = 0; i < 4; i++)
    {
        emit_byte(buffer, offset, size, (imm >> (i * 8)) & 0xFF);
    }
}

int masm_x86_encode(MasmInstruction inst, uint8_t *buffer, size_t size)
{
    size_t offset = 0;
    
    if (inst.opcode == MASM_OP_RET)
    {
        emit_byte(buffer, &offset, size, 0xC3);
    }
    else if (inst.opcode == MASM_OP_SYSCALL)
    {
        emit_byte(buffer, &offset, size, 0x0F);
        emit_byte(buffer, &offset, size, 0x05);
    }
    else if (inst.opcode == MASM_OP_CALL)
    {
        // CALL rel32 (E8 cd)
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            emit_byte(buffer, &offset, size, 0xE8);
            // placeholder for relative offset, will be patched later or resolved if we knew it
            // for now emit 0 and assume we need linking/patching
            emit_imm32(buffer, &offset, size, 0); 
        }
    }
    else if (inst.opcode == MASM_OP_MOV)
    {
        // MOV reg, imm
        if (inst.operand_count == 2 && 
            inst.operands[0].kind == MASM_OPERAND_REGISTER && 
            inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t reg = inst.operands[0].reg.id;
            int64_t  imm = inst.operands[1].imm;
            
            // REX.W + B8+rd io
            // for 64-bit immediate to 64-bit register: 48 B8+rd imm64
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0xB8 + (reg & 7));
                emit_imm64(buffer, &offset, size, imm);
            }
            // for 32-bit immediate to 32-bit register: B8+rd imm32
            else if (inst.operands[0].reg.size == 4)
            {
                emit_byte(buffer, &offset, size, 0xB8 + (reg & 7));
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        // MOV reg, reg
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
             uint32_t dst = inst.operands[0].reg.id;
             uint32_t src = inst.operands[1].reg.id;
             
             // REX.W + 89 /r
             // MOV r/m64, r64
             if (inst.operands[0].reg.size == 8 && inst.operands[1].reg.size == 8)
             {
                 emit_byte(buffer, &offset, size, 0x48);
                 emit_byte(buffer, &offset, size, 0x89);
                 // ModR/M: 11 src dst
                 uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                 emit_byte(buffer, &offset, size, modrm);
             }
        }
    }
    else if (inst.opcode == MASM_OP_LABEL)
    {
        // 0 bytes
    }
    
    return offset;
}
