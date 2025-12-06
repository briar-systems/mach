#include "compiler/masm/isa/x86_64.h"
#include <stdbool.h>
#include <stdint.h>

MasmOperand masm_x86_reg(MasmX86Reg reg, uint8_t size)
{
    return masm_operand_register((uint32_t)reg, size);
}

static void emit_byte(uint8_t *buf, size_t *off, size_t cap, uint8_t b)
{
    if (*off < cap)
    {
        buf[*off] = b;
    }
    (*off)++;
}

static void emit_imm32(uint8_t *buf, size_t *off, size_t cap, int32_t v)
{
    for (int i = 0; i < 4; i++)
    {
        emit_byte(buf, off, cap, (uint8_t)((v >> (i * 8)) & 0xFF));
    }
}

static void emit_imm64(uint8_t *buf, size_t *off, size_t cap, int64_t v)
{
    for (int i = 0; i < 8; i++)
    {
        emit_byte(buf, off, cap, (uint8_t)((v >> (i * 8)) & 0xFF));
    }
}

static uint8_t reg_low(uint32_t r)
{
    return (uint8_t)(r & 7);
}

static void emit_rex(uint8_t *buf, size_t *off, size_t cap, bool w, bool r, bool x, bool b)
{
    uint8_t rex = 0x40 | (w ? 8 : 0) | (r ? 4 : 0) | (x ? 2 : 0) | (b ? 1 : 0);
    if (rex != 0x40)
    {
        emit_byte(buf, off, cap, rex);
    }
    else if (w || r || x || b)
    {
        emit_byte(buf, off, cap, rex);
    }
}

static uint8_t encode_modrm(uint8_t mod, uint8_t reg, uint8_t rm)
{
    return (uint8_t)(mod | ((reg & 7) << 3) | (rm & 7));
}

static uint8_t encode_sib(uint8_t scale_bits, uint8_t index, uint8_t base)
{
    return (uint8_t)((scale_bits << 6) | ((index & 7) << 3) | (base & 7));
}

static void emit_mem(uint8_t *buf, size_t *off, size_t cap, uint32_t base, uint32_t index, uint8_t scale, int32_t disp, uint8_t reg_field)
{
    bool has_index = index != 0 || scale != 0;
    if (scale == 0)
    {
        scale = 1;
    }

    uint8_t mod;
    if (!has_index && disp == 0 && (base & 7) != 5 && base != MASM_X86_RBP && base != MASM_X86_R13)
    {
        mod = 0x00;
    }
    else if (disp >= -128 && disp <= 127)
    {
        mod = 0x40;
    }
    else
    {
        mod = 0x80;
    }

    if (has_index || (base & 7) == 4)
    {
        emit_byte(buf, off, cap, encode_modrm(mod, reg_field, 0x04));

        uint8_t scale_bits = 0;
        if (scale == 2)
        {
            scale_bits = 1;
        }
        else if (scale == 4)
        {
            scale_bits = 2;
        }
        else if (scale == 8)
        {
            scale_bits = 3;
        }

        emit_byte(buf, off, cap, encode_sib(scale_bits, has_index ? index : 4, base));
    }
    else
    {
        emit_byte(buf, off, cap, encode_modrm(mod, reg_field, base));
    }

    if (mod == 0x40)
    {
        emit_byte(buf, off, cap, (int8_t)disp);
    }
    else if (mod == 0x80 || (!has_index && ((base & 7) == 5)))
    {
        emit_imm32(buf, off, cap, disp);
    }
}

int masm_x86_encode(MasmInstruction inst, uint8_t *buffer, size_t size)
{
    size_t offset = 0;

    switch (inst.opcode)
    {
    case MASM_OP_RET:
        emit_byte(buffer, &offset, size, 0xC3);
        break;

    case MASM_OP_SYSCALL:
        emit_byte(buffer, &offset, size, 0x0F);
        emit_byte(buffer, &offset, size, 0x05);
        break;

    case MASM_OP_PUSH:
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t r = inst.operands[0].reg.id;
            emit_rex(buffer, &offset, size, false, false, false, r >= 8);
            emit_byte(buffer, &offset, size, (uint8_t)(0x50 + reg_low(r)));
        }
        break;

    case MASM_OP_POP:
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t r = inst.operands[0].reg.id;
            emit_rex(buffer, &offset, size, false, false, false, r >= 8);
            emit_byte(buffer, &offset, size, (uint8_t)(0x58 + reg_low(r)));
        }
        break;

    case MASM_OP_MOV:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint8_t  sz  = inst.operands[0].reg.size;
            int64_t  imm = inst.operands[1].imm;
            emit_rex(buffer, &offset, size, sz == 8, false, false, dst >= 8);
            emit_byte(buffer, &offset, size, (uint8_t)(0xB8 + reg_low(dst)));
            if (sz == 8)
            {
                emit_imm64(buffer, &offset, size, imm);
            }
            else
            {
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            bool     w   = inst.operands[0].reg.size == 8;
            emit_rex(buffer, &offset, size, w, src >= 8, false, dst >= 8);
            emit_byte(buffer, &offset, size, 0x89);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(src), reg_low(dst)));
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_MEMORY)
        {
            uint32_t dst    = inst.operands[0].reg.id;
            uint8_t  sz     = inst.operands[0].reg.size;
            bool     w      = sz == 8;
            uint8_t  opcode = (sz == 1) ? 0x8A : 0x8B;
            emit_rex(buffer, &offset, size, w, dst >= 8, inst.operands[1].mem.index.id >= 8, inst.operands[1].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, opcode);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, dst);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_MEMORY && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t src    = inst.operands[1].reg.id;
            uint8_t  sz     = inst.operands[1].reg.size;
            bool     w      = sz == 8;
            uint8_t  opcode = (sz == 1) ? 0x88 : 0x89;
            emit_rex(buffer, &offset, size, w, src >= 8, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, opcode);
            emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id, inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, src);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_MEMORY && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint8_t  sz   = inst.operands[0].mem.size;
            uint32_t base = inst.operands[0].mem.base.id;
            int32_t  disp = (int32_t)inst.operands[0].mem.disp;
            int64_t  imm  = inst.operands[1].imm;
            bool     w    = sz == 8;

            if (sz == 8 || sz == 4)
            {
                emit_rex(buffer, &offset, size, w, false, false, base >= 8);
                emit_byte(buffer, &offset, size, 0xC7);
            }
            else
            {
                emit_rex(buffer, &offset, size, false, false, false, base >= 8);
                emit_byte(buffer, &offset, size, 0xC6);
            }

            uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
            if (disp == 0 && base != MASM_X86_RBP && base != MASM_X86_R13)
            {
                mod = 0x00;
            }

            emit_byte(buffer, &offset, size, encode_modrm(mod, 0, reg_low(base)));
            if (mod == 0x40)
            {
                emit_byte(buffer, &offset, size, (int8_t)disp);
            }
            else if (mod == 0x80 || base == MASM_X86_RBP || base == MASM_X86_R13)
            {
                emit_imm32(buffer, &offset, size, disp);
            }

            if (sz == 1)
            {
                emit_byte(buffer, &offset, size, (int8_t)imm);
            }
            else
            {
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        break;

    case MASM_OP_LEA:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_MEMORY)
        {
            uint32_t dst = inst.operands[0].reg.id;
            bool     w   = inst.operands[0].reg.size == 8;
            emit_rex(buffer, &offset, size, w, dst >= 8, inst.operands[1].mem.index.id >= 8, inst.operands[1].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, 0x8D);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, dst);
        }
        break;

    case MASM_OP_ADD:
    case MASM_OP_SUB:
    case MASM_OP_AND:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t reg   = inst.operands[0].reg.id;
            int64_t  imm   = inst.operands[1].imm;
            bool     w     = inst.operands[0].reg.size == 8;
            uint8_t  subop = (inst.opcode == MASM_OP_ADD) ? 0 : (inst.opcode == MASM_OP_AND ? 4 : 5);
            emit_rex(buffer, &offset, size, w, false, false, reg >= 8);
            if (imm >= -128 && imm <= 127)
            {
                emit_byte(buffer, &offset, size, 0x83);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(reg)));
                emit_byte(buffer, &offset, size, (int8_t)imm);
            }
            else
            {
                emit_byte(buffer, &offset, size, 0x81);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(reg)));
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        break;

    case MASM_OP_CMP:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t lhs = inst.operands[0].reg.id;
            uint32_t rhs = inst.operands[1].reg.id;
            emit_rex(buffer, &offset, size, true, rhs >= 8, false, lhs >= 8);
            emit_byte(buffer, &offset, size, 0x39);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(rhs), reg_low(lhs)));
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t reg = inst.operands[0].reg.id;
            int64_t  imm = inst.operands[1].imm;
            bool     w   = inst.operands[0].reg.size == 8;
            emit_rex(buffer, &offset, size, w, false, false, reg >= 8);
            emit_byte(buffer, &offset, size, 0x81);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, 7, reg_low(reg)));
            emit_imm32(buffer, &offset, size, (int32_t)imm);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[0].reg.size == 1 && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t reg       = inst.operands[0].reg.id;
            int64_t  imm       = inst.operands[1].imm;
            bool     rex_b     = reg >= 8;
            bool     rex_force = reg >= 4 && reg <= 7;
            if (rex_b || rex_force)
            {
                emit_rex(buffer, &offset, size, false, false, false, rex_b);
            }
            emit_byte(buffer, &offset, size, 0x80);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, 7, reg_low(reg)));
            emit_byte(buffer, &offset, size, (int8_t)imm);
        }
        break;

    case MASM_OP_TEST:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t a = inst.operands[0].reg.id;
            uint32_t b = inst.operands[1].reg.id;
            emit_rex(buffer, &offset, size, true, b >= 8, false, a >= 8);
            emit_byte(buffer, &offset, size, 0x85);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(b), reg_low(a)));
        }
        break;

    case MASM_OP_CALL:
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_IMM)
        {
            // relative call
            emit_byte(buffer, &offset, size, 0xE8);
            emit_imm32(buffer, &offset, size, (int32_t)inst.operands[0].imm);
        }
        else if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t reg = inst.operands[0].reg.id;
            emit_rex(buffer, &offset, size, true, false, false, reg >= 8);
            emit_byte(buffer, &offset, size, 0xFF);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, 2, reg_low(reg)));
        }
        break;

    case MASM_OP_SETE:
    case MASM_OP_SETNE:
    case MASM_OP_SETL:
    case MASM_OP_SETG:
    case MASM_OP_SETLE:
    case MASM_OP_SETGE:
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t reg    = inst.operands[0].reg.id;
            uint8_t  opcode = 0x90;
            switch (inst.opcode)
            {
            case MASM_OP_SETE:
                opcode = 0x94;
                break;
            case MASM_OP_SETNE:
                opcode = 0x95;
                break;
            case MASM_OP_SETL:
                opcode = 0x9C;
                break;
            case MASM_OP_SETG:
                opcode = 0x9F;
                break;
            case MASM_OP_SETLE:
                opcode = 0x9E;
                break;
            case MASM_OP_SETGE:
                opcode = 0x9D;
                break;
            default:
                break;
            }
            bool rex_b     = reg >= 8;
            bool rex_force = reg >= 4 && reg <= 7;
            if (rex_b || rex_force)
            {
                emit_rex(buffer, &offset, size, false, false, false, rex_b);
            }
            emit_byte(buffer, &offset, size, 0x0F);
            emit_byte(buffer, &offset, size, opcode);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, 0, reg_low(reg)));
        }
        break;

    case MASM_OP_LABEL:
        break;

    default:
        break;
    }

    return (int)offset;
}
