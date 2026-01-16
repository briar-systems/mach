#include "compiler/masm/isa/x86_64.h"
#include "compiler/masm/isa/spec.h"
#include "compiler/masm/abi/sysv64.h"
#include <string.h>
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

static bool operand_is_xmm(MasmOperand op)
{
    return op.kind == MASM_OPERAND_REGISTER && op.reg.size == 16;
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

static void emit_rex_force(uint8_t *buf, size_t *off, size_t cap, bool force, bool w, bool r, bool x, bool b)
{
    // x86_64 quirk: spl/bpl/sil/dil require a REX prefix to select the low-byte
    // register encoding. this is a "presence" requirement; it must NOT set REX.R/B.
    uint8_t rex = 0x40 | (w ? 8 : 0) | (r ? 4 : 0) | (x ? 2 : 0) | (b ? 1 : 0);
    if (force || rex != 0x40)
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

    case MASM_OP_X86_SYSCALL:
        emit_byte(buffer, &offset, size, 0x0F);
        emit_byte(buffer, &offset, size, 0x05);
        break;

    case MASM_OP_X86_MOVQ:
        // movd/movq between xmm and r/m32|r/m64:
        //  - xmm <- r/m : 66 0F 6E /r (REX.W selects 64-bit)
        //  - r/m <- xmm : 66 0F 7E /r (REX.W selects 64-bit)
        if (inst.operand_count == 2)
        {
            MasmOperand a = inst.operands[0];
            MasmOperand b = inst.operands[1];

            // xmm <- r/m
            if (operand_is_xmm(a) && (b.kind == MASM_OPERAND_REGISTER || b.kind == MASM_OPERAND_MEMORY))
            {
                uint8_t rm_size = (b.kind == MASM_OPERAND_REGISTER) ? b.reg.size : b.mem.size;
                if (rm_size != 4 && rm_size != 8)
                {
                    break;
                }

                bool w     = rm_size == 8;
                bool rex_r = a.reg.id >= 8;

                emit_byte(buffer, &offset, size, 0x66);

                if (b.kind == MASM_OPERAND_REGISTER)
                {
                    uint32_t rm = b.reg.id;
                    emit_rex(buffer, &offset, size, w, rex_r, false, rm >= 8);
                    emit_byte(buffer, &offset, size, 0x0F);
                    emit_byte(buffer, &offset, size, 0x6E);
                    emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(a.reg.id), reg_low(rm)));
                }
                else
                {
                    bool rex_x = b.mem.index.id >= 8;
                    bool rex_b = b.mem.base.id >= 8;
                    emit_rex(buffer, &offset, size, w, rex_r, rex_x, rex_b);
                    emit_byte(buffer, &offset, size, 0x0F);
                    emit_byte(buffer, &offset, size, 0x6E);
                    emit_mem(buffer, &offset, size, b.mem.base.id, b.mem.index.id, b.mem.scale, (int32_t)b.mem.disp, reg_low(a.reg.id));
                }
            }
            // r/m <- xmm
            else if ((a.kind == MASM_OPERAND_REGISTER || a.kind == MASM_OPERAND_MEMORY) && operand_is_xmm(b))
            {
                uint8_t rm_size = (a.kind == MASM_OPERAND_REGISTER) ? a.reg.size : a.mem.size;
                if (rm_size != 4 && rm_size != 8)
                {
                    break;
                }

                bool w     = rm_size == 8;
                bool rex_r = b.reg.id >= 8;

                emit_byte(buffer, &offset, size, 0x66);

                if (a.kind == MASM_OPERAND_REGISTER)
                {
                    uint32_t rm = a.reg.id;
                    emit_rex(buffer, &offset, size, w, rex_r, false, rm >= 8);
                    emit_byte(buffer, &offset, size, 0x0F);
                    emit_byte(buffer, &offset, size, 0x7E);
                    emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(b.reg.id), reg_low(rm)));
                }
                else
                {
                    bool rex_x = a.mem.index.id >= 8;
                    bool rex_b = a.mem.base.id >= 8;
                    emit_rex(buffer, &offset, size, w, rex_r, rex_x, rex_b);
                    emit_byte(buffer, &offset, size, 0x0F);
                    emit_byte(buffer, &offset, size, 0x7E);
                    emit_mem(buffer, &offset, size, a.mem.base.id, a.mem.index.id, a.mem.scale, (int32_t)a.mem.disp, reg_low(b.reg.id));
                }
            }
        }
        break;

    case MASM_OP_X86_UCOMISD:
        // ucomisd xmm1, xmm2/m64: 66 0F 2E /r
        // compares two f64 values and sets EFLAGS (ZF, PF, CF)
        if (inst.operand_count == 2 && operand_is_xmm(inst.operands[0]))
        {
            MasmOperand a = inst.operands[0];
            MasmOperand b = inst.operands[1];

            bool rex_r = a.reg.id >= 8;

            emit_byte(buffer, &offset, size, 0x66);

            if (operand_is_xmm(b))
            {
                // xmm, xmm
                bool rex_b = b.reg.id >= 8;
                if (rex_r || rex_b)
                {
                    emit_rex(buffer, &offset, size, false, rex_r, false, rex_b);
                }
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x2E);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(a.reg.id), reg_low(b.reg.id)));
            }
            else if (b.kind == MASM_OPERAND_MEMORY)
            {
                // xmm, m64
                bool rex_x = b.mem.index.id >= 8;
                bool rex_b = b.mem.base.id >= 8;
                if (rex_r || rex_x || rex_b)
                {
                    emit_rex(buffer, &offset, size, false, rex_r, rex_x, rex_b);
                }
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x2E);
                emit_mem(buffer, &offset, size, b.mem.base.id, b.mem.index.id, b.mem.scale, (int32_t)b.mem.disp, reg_low(a.reg.id));
            }
        }
        break;

    case MASM_OP_CQO:
        // sign-extend RAX into RDX:RAX
        emit_rex(buffer, &offset, size, true, false, false, false);
        emit_byte(buffer, &offset, size, 0x99);
        break;

    case MASM_OP_IDIV:
    case MASM_OP_DIV:
        if (inst.operand_count == 1)
        {
            uint8_t subopcode = inst.opcode == MASM_OP_IDIV ? 7 : 6; // /7 for idiv, /6 for div
            if (inst.operands[0].kind == MASM_OPERAND_REGISTER)
            {
                uint32_t rm = inst.operands[0].reg.id;
                bool     w  = inst.operands[0].reg.size == 8;
                emit_rex(buffer, &offset, size, w, false, false, rm >= 8);
                emit_byte(buffer, &offset, size, 0xF7);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subopcode, reg_low(rm)));
            }
            else if (inst.operands[0].kind == MASM_OPERAND_MEMORY)
            {
                bool w = inst.operands[0].mem.size == 8;
                emit_rex(buffer, &offset, size, w, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
                emit_byte(buffer, &offset, size, 0xF7);
                emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id, inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, subopcode);
            }
        }
        break;

    case MASM_OP_IMUL:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            // imul r64, r/m64  -> 0F AF /r
            bool w = inst.operands[0].reg.size == 8;
            if (inst.operands[1].kind == MASM_OPERAND_REGISTER)
            {
                uint32_t dst = inst.operands[0].reg.id;
                uint32_t src = inst.operands[1].reg.id;
                emit_rex(buffer, &offset, size, w, src >= 8, false, dst >= 8);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0xAF);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst), reg_low(src)));
            }
            else if (inst.operands[1].kind == MASM_OPERAND_MEMORY)
            {
                uint32_t dst = inst.operands[0].reg.id;
                emit_rex(buffer, &offset, size, w, dst >= 8, inst.operands[1].mem.index.id >= 8, inst.operands[1].mem.base.id >= 8);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0xAF);
                emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, reg_low(dst));
            }
        }
        else if (inst.operand_count == 3 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            // imul r64, r/m64, imm8/32  -> 6B/69 /r imm
            bool w = inst.operands[0].reg.size == 8;
            uint32_t dst = inst.operands[0].reg.id;
            uint8_t  opcode = 0x69;
            int64_t  imm = 0;

            if (inst.operands[2].kind == MASM_OPERAND_IMM)
            {
                imm    = inst.operands[2].imm;
                opcode = (imm >= -128 && imm <= 127) ? 0x6B : 0x69;
            }

            emit_rex(buffer, &offset, size, w,
                     inst.operands[1].kind == MASM_OPERAND_REGISTER && inst.operands[1].reg.id >= 8,
                     inst.operands[1].kind == MASM_OPERAND_MEMORY && inst.operands[1].mem.index.id >= 8,
                     inst.operands[1].kind == MASM_OPERAND_MEMORY ? inst.operands[1].mem.base.id >= 8 : dst >= 8);
            emit_byte(buffer, &offset, size, opcode);

            if (inst.operands[1].kind == MASM_OPERAND_REGISTER)
            {
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst), reg_low(inst.operands[1].reg.id)));
            }
            else if (inst.operands[1].kind == MASM_OPERAND_MEMORY)
            {
                emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, reg_low(dst));
            }

            if (opcode == 0x6B)
            {
                emit_byte(buffer, &offset, size, (int8_t)imm);
            }
            else
            {
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
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
            if (sz == 1)
            {
                // mov r8, imm8 -> B0+rb ib (needs REX for spl/bpl/sil/dil and r8b+)
                bool rex_b     = dst >= 8;
                bool rex_force = dst >= 4 && dst <= 7;
                emit_rex_force(buffer, &offset, size, rex_force, false, false, false, rex_b);
                emit_byte(buffer, &offset, size, (uint8_t)(0xB0 + reg_low(dst)));
                emit_byte(buffer, &offset, size, (int8_t)imm);
            }
            else
            {
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
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            bool     byte = inst.operands[0].reg.size == 1;
            bool     w    = inst.operands[0].reg.size == 8 && !byte;
            bool     rex_r     = src >= 8;
            bool     rex_b     = dst >= 8;
            bool     rex_force = byte && ((src >= 4 && src <= 7) || (dst >= 4 && dst <= 7));
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, false, rex_b);
            emit_byte(buffer, &offset, size, byte ? 0x88 : 0x89);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(src), reg_low(dst)));
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_MEMORY)
        {
            uint32_t dst    = inst.operands[0].reg.id;
            uint8_t  sz     = inst.operands[0].reg.size;
            bool     w      = sz == 8;
            uint8_t  opcode = (sz == 1) ? 0x8A : 0x8B;
            bool rex_r     = dst >= 8;
            bool rex_force = (sz == 1) && (dst >= 4 && dst <= 7);
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, inst.operands[1].mem.index.id >= 8, inst.operands[1].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, opcode);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, dst);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_MEMORY && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t src    = inst.operands[1].reg.id;
            uint8_t  sz     = inst.operands[1].reg.size;
            bool     w      = sz == 8;
            uint8_t  opcode = (sz == 1) ? 0x88 : 0x89;
            bool rex_r     = src >= 8;
            bool rex_force = (sz == 1) && (src >= 4 && src <= 7);
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, opcode);
            emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id, inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, src);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_MEMORY && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint8_t  sz   = inst.operands[0].mem.size;
            uint32_t base = inst.operands[0].mem.base.id;
            uint32_t index = inst.operands[0].mem.index.id;
            uint8_t  scale = inst.operands[0].mem.scale;
            int32_t  disp = (int32_t)inst.operands[0].mem.disp;
            int64_t  imm  = inst.operands[1].imm;
            bool     w    = sz == 8;

            if (sz == 8 || sz == 4)
            {
                emit_rex(buffer, &offset, size, w, false, index >= 8, base >= 8);
                emit_byte(buffer, &offset, size, 0xC7);
            }
            else
            {
                emit_rex(buffer, &offset, size, false, false, index >= 8, base >= 8);
                emit_byte(buffer, &offset, size, 0xC6);
            }

            // use the shared mem emitter to correctly handle:
            // - rsp/r12 base requiring a sib byte
            // - indexed addressing
            // - rbp/r13 base special-case for disp=0
            emit_mem(buffer, &offset, size, base, index, scale, disp, 0);

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
    case MASM_OP_OR:
    case MASM_OP_XOR:
    {
        // byte-sized variants use different opcodes
        uint8_t opcode_rm_reg_qw = 0;
        uint8_t opcode_reg_rm_qw = 0;
        uint8_t opcode_rm_reg_b  = 0;
        uint8_t opcode_reg_rm_b  = 0;

        // group-1 immediate subopcode
        uint8_t imm_subop = 0;

        switch (inst.opcode)
        {
        case MASM_OP_ADD:
            opcode_rm_reg_qw = 0x01;
            opcode_reg_rm_qw = 0x03;
            opcode_rm_reg_b  = 0x00;
            opcode_reg_rm_b  = 0x02;
            imm_subop        = 0;
            break;
        case MASM_OP_OR:
            opcode_rm_reg_qw = 0x09;
            opcode_reg_rm_qw = 0x0B;
            opcode_rm_reg_b  = 0x08;
            opcode_reg_rm_b  = 0x0A;
            imm_subop        = 1;
            break;
        case MASM_OP_AND:
            opcode_rm_reg_qw = 0x21;
            opcode_reg_rm_qw = 0x23;
            opcode_rm_reg_b  = 0x20;
            opcode_reg_rm_b  = 0x22;
            imm_subop        = 4;
            break;
        case MASM_OP_SUB:
            opcode_rm_reg_qw = 0x29;
            opcode_reg_rm_qw = 0x2B;
            opcode_rm_reg_b  = 0x28;
            opcode_reg_rm_b  = 0x2A;
            imm_subop        = 5;
            break;
        case MASM_OP_XOR:
            opcode_rm_reg_qw = 0x31;
            opcode_reg_rm_qw = 0x33;
            opcode_rm_reg_b  = 0x30;
            opcode_reg_rm_b  = 0x32;
            imm_subop        = 6;
            break;
        default:
            break;
        }

        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst  = inst.operands[0].reg.id;
            uint32_t src  = inst.operands[1].reg.id;
            bool     byte = inst.operands[0].reg.size == 1;
            bool     w    = inst.operands[0].reg.size == 8 && !byte;
            bool     rex_r     = src >= 8;
            bool     rex_b     = dst >= 8;
            bool     rex_force = byte && ((src >= 4 && src <= 7) || (dst >= 4 && dst <= 7));
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, false, rex_b);
            emit_byte(buffer, &offset, size, byte ? opcode_rm_reg_b : opcode_rm_reg_qw);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(src), reg_low(dst)));
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_MEMORY)
        {
            uint32_t dst  = inst.operands[0].reg.id;
            bool     byte = inst.operands[0].reg.size == 1 || inst.operands[1].mem.size == 1;
            bool     w    = inst.operands[0].reg.size == 8 && !byte;
            bool     rex_r     = dst >= 8;
            bool     rex_force = byte && (dst >= 4 && dst <= 7);
            bool     rex_x = inst.operands[1].mem.index.id >= 8;
            bool     rex_b = inst.operands[1].mem.base.id >= 8;
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, rex_x, rex_b);
            emit_byte(buffer, &offset, size, byte ? opcode_reg_rm_b : opcode_reg_rm_qw);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, dst);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_MEMORY && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t src  = inst.operands[1].reg.id;
            bool     byte = inst.operands[1].reg.size == 1 || inst.operands[0].mem.size == 1;
            bool     w    = inst.operands[1].reg.size == 8 && !byte;
            bool     rex_r     = src >= 8;
            bool     rex_force = byte && (src >= 4 && src <= 7);
            bool     rex_x = inst.operands[0].mem.index.id >= 8;
            bool     rex_b = inst.operands[0].mem.base.id >= 8;
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, rex_x, rex_b);
            emit_byte(buffer, &offset, size, byte ? opcode_rm_reg_b : opcode_rm_reg_qw);
            emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id, inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, src);
        }
        else if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t reg   = inst.operands[0].reg.id;
            int64_t  imm   = inst.operands[1].imm;
            bool     byte  = inst.operands[0].reg.size == 1;
            bool     w     = inst.operands[0].reg.size == 8 && !byte;
            uint8_t  subop = imm_subop;
            bool     rex_b     = reg >= 8;
            bool     rex_force = byte && (reg >= 4 && reg <= 7);
            emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rex_b);
            if (byte)
            {
                emit_byte(buffer, &offset, size, 0x80);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(reg)));
                emit_byte(buffer, &offset, size, (int8_t)imm);
            }
            else if (imm >= -128 && imm <= 127)
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
    }

    case MASM_OP_SHL:
    case MASM_OP_SHR:
    case MASM_OP_SAR:
    {
        // shl/shr/sar r/m{8,32,64}, imm8  -> C0/C1 /4|/5|/7 ib
        // shl/shr/sar r/m{8,32,64}, cl    -> D2/D3 /4|/5|/7
        if (inst.operand_count == 2)
        {
            uint8_t subop = (inst.opcode == MASM_OP_SHL) ? 4 : (inst.opcode == MASM_OP_SHR) ? 5 : 7;

            MasmOperand dst = inst.operands[0];
            MasmOperand cnt = inst.operands[1];

            uint8_t dst_size = 8;
            if (dst.kind == MASM_OPERAND_REGISTER)
                dst_size = dst.reg.size;
            else if (dst.kind == MASM_OPERAND_MEMORY)
                dst_size = dst.mem.size;

            bool byte = dst_size == 1;
            bool w    = (dst_size == 8) && !byte;

            if (cnt.kind == MASM_OPERAND_IMM)
            {
                uint8_t opcode = byte ? 0xC0 : 0xC1;

                if (dst.kind == MASM_OPERAND_REGISTER)
                {
                    uint32_t rm    = dst.reg.id;
                    bool     rex_b     = rm >= 8;
                    bool     rex_force = byte && (rm >= 4 && rm <= 7);
                    emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rex_b);
                    emit_byte(buffer, &offset, size, opcode);
                    emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(rm)));
                    emit_byte(buffer, &offset, size, (uint8_t)cnt.imm);
                }
                else if (dst.kind == MASM_OPERAND_MEMORY)
                {
                    bool rex_x = dst.mem.index.id >= 8;
                    bool rex_b = dst.mem.base.id >= 8;
                    emit_rex(buffer, &offset, size, w, false, rex_x, rex_b);
                    emit_byte(buffer, &offset, size, opcode);
                    emit_mem(buffer, &offset, size, dst.mem.base.id, dst.mem.index.id, dst.mem.scale, (int32_t)dst.mem.disp, subop);
                    emit_byte(buffer, &offset, size, (uint8_t)cnt.imm);
                }
            }
            else if (cnt.kind == MASM_OPERAND_REGISTER)
            {
                uint8_t opcode = byte ? 0xD2 : 0xD3;

                if (dst.kind == MASM_OPERAND_REGISTER)
                {
                    uint32_t rm    = dst.reg.id;
                    bool     rex_b     = rm >= 8;
                    bool     rex_force = byte && (rm >= 4 && rm <= 7);
                    emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rex_b);
                    emit_byte(buffer, &offset, size, opcode);
                    emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(rm)));
                }
                else if (dst.kind == MASM_OPERAND_MEMORY)
                {
                    bool rex_x = dst.mem.index.id >= 8;
                    bool rex_b = dst.mem.base.id >= 8;
                    emit_rex(buffer, &offset, size, w, false, rex_x, rex_b);
                    emit_byte(buffer, &offset, size, opcode);
                    emit_mem(buffer, &offset, size, dst.mem.base.id, dst.mem.index.id, dst.mem.scale, (int32_t)dst.mem.disp, subop);
                }
            }
        }
        break;
    }

    case MASM_OP_CMP:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER && inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t lhs = inst.operands[0].reg.id;
            uint32_t rhs = inst.operands[1].reg.id;
            // use 64-bit comparison only if operand size is 8, otherwise use 32-bit (or smaller)
            bool     w   = inst.operands[0].reg.size == 8;
            emit_rex(buffer, &offset, size, w, rhs >= 8, false, lhs >= 8);
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
    case MASM_OP_SETB:
    case MASM_OP_SETA:
    case MASM_OP_SETBE:
    case MASM_OP_SETAE:
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
            case MASM_OP_SETB:
                opcode = 0x92;
                break;
            case MASM_OP_SETA:
                opcode = 0x97;
                break;
            case MASM_OP_SETBE:
                opcode = 0x96;
                break;
            case MASM_OP_SETAE:
                opcode = 0x93;
                break;
            default:
                break;
            }
            bool rex_b     = reg >= 8;
            bool rex_force = reg >= 4 && reg <= 7;
            emit_rex_force(buffer, &offset, size, rex_force, false, false, false, rex_b);
            emit_byte(buffer, &offset, size, 0x0F);
            emit_byte(buffer, &offset, size, opcode);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, 0, reg_low(reg)));
        }
        break;

    case MASM_OP_X86_ADDSD:
    case MASM_OP_X86_SUBSD:
    case MASM_OP_X86_MULSD:
    case MASM_OP_X86_DIVSD:
        // SSE scalar double-precision arithmetic: F2 0F <op> /r
        // addsd: F2 0F 58, subsd: F2 0F 5C, mulsd: F2 0F 59, divsd: F2 0F 5E
        if (inst.operand_count == 2 && operand_is_xmm(inst.operands[0]))
        {
            MasmOperand a = inst.operands[0];
            MasmOperand b = inst.operands[1];

            uint8_t opbyte = 0x58; // default addsd
            switch (inst.opcode)
            {
            case MASM_OP_X86_ADDSD: opbyte = 0x58; break;
            case MASM_OP_X86_SUBSD: opbyte = 0x5C; break;
            case MASM_OP_X86_MULSD: opbyte = 0x59; break;
            case MASM_OP_X86_DIVSD: opbyte = 0x5E; break;
            default: break;
            }

            bool rex_r = a.reg.id >= 8;

            emit_byte(buffer, &offset, size, 0xF2);

            if (operand_is_xmm(b))
            {
                // xmm, xmm
                bool rex_b = b.reg.id >= 8;
                if (rex_r || rex_b)
                {
                    emit_rex(buffer, &offset, size, false, rex_r, false, rex_b);
                }
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, opbyte);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(a.reg.id), reg_low(b.reg.id)));
            }
            else if (b.kind == MASM_OPERAND_MEMORY)
            {
                // xmm, m64
                bool rex_x = b.mem.index.id >= 8;
                bool rex_b = b.mem.base.id >= 8;
                if (rex_r || rex_x || rex_b)
                {
                    emit_rex(buffer, &offset, size, false, rex_r, rex_x, rex_b);
                }
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, opbyte);
                emit_mem(buffer, &offset, size, b.mem.base.id, b.mem.index.id, b.mem.scale, (int32_t)b.mem.disp, reg_low(a.reg.id));
            }
        }
        break;

    case MASM_OP_MOVZX:
    case MASM_OP_MOVSX:
        if (inst.operand_count == 2 && inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            (inst.operands[1].kind == MASM_OPERAND_MEMORY || inst.operands[1].kind == MASM_OPERAND_REGISTER))
        {
            uint32_t dst     = inst.operands[0].reg.id;
            bool     dst64   = inst.operands[0].reg.size == 8;
            uint8_t  srcsize = (inst.operands[1].kind == MASM_OPERAND_MEMORY) ? inst.operands[1].mem.size : inst.operands[1].reg.size;
            bool     src16   = srcsize == 2;
            bool     src8    = srcsize == 1;
            if (!(src8 || src16))
                break; // unsupported width

            uint8_t opcode = 0x0F;
            uint8_t ext    = (inst.opcode == MASM_OP_MOVZX) ? (src16 ? 0xB7 : 0xB6) : (src16 ? 0xBF : 0xBE);

            if (inst.operands[1].kind == MASM_OPERAND_REGISTER)
            {
                uint32_t src   = inst.operands[1].reg.id;
                bool     rex_r     = dst >= 8; // reg field
                bool     rex_b     = (src >= 8);
                bool     rex_force = src8 && (src >= 4 && src <= 7);
                emit_rex_force(buffer, &offset, size, rex_force, dst64, rex_r, false, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, ext);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst), reg_low(src)));
            }
            else
            {
                bool rex_r = dst >= 8; // reg field
                bool rex_x = inst.operands[1].mem.index.id >= 8;
                bool rex_b = inst.operands[1].mem.base.id >= 8;
                emit_rex(buffer, &offset, size, dst64, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, ext);
                emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale,
                         (int32_t)inst.operands[1].mem.disp, dst);
            }
        }
        break;

    case MASM_OP_LABEL:
        break;

    default:
        break;
    }

    return (int)offset;
}

// ISA spec implementation for x86_64
static MasmOperand x86_reg_result(uint8_t size) { return masm_operand_register(MASM_X86_RAX, size); }
static MasmOperand x86_reg_tmp0(uint8_t size) { return masm_operand_register(MASM_X86_RCX, size); }
static MasmOperand x86_reg_tmp1(uint8_t size) { return masm_operand_register(MASM_X86_RDX, size); }
static MasmOperand x86_reg_div_hi(uint8_t size) { return masm_operand_register(MASM_X86_RDX, size); }
static MasmOperand x86_reg_div_lo(uint8_t size) { return masm_operand_register(MASM_X86_RAX, size); }
static MasmOperand x86_reg_arg(int index, uint8_t size)
{
    uint32_t reg = masm_sysv64_arg_reg(index);
    if (reg == (uint32_t)MASM_X86_REG_COUNT) return masm_operand_none();
    return masm_operand_register(reg, size);
}
static MasmOperand x86_reg_sp(uint8_t size) { return masm_operand_register(MASM_X86_RSP, size); }
static MasmOperand x86_reg_fp(uint8_t size) { return masm_operand_register(MASM_X86_RBP, size); }
static uint32_t x86_op_syscall() { return MASM_OP_X86_SYSCALL; }

static const uint32_t X86_SCRATCH[] = {
    MASM_X86_R10,
    MASM_X86_R11,
    MASM_X86_RAX,
    MASM_X86_RCX,
    MASM_X86_RDX,
    MASM_X86_RSI,
    MASM_X86_RDI,
    MASM_X86_R8,
    MASM_X86_R9,
};

static const uint32_t X86_RESERVED[] = {
    MASM_X86_RSP,
    MASM_X86_RBP,
    MASM_X86_RBX,
    MASM_X86_R12,
    MASM_X86_R13,
    MASM_X86_R14,
    MASM_X86_R15,
};

static MasmOperand x86_parse_reg(const char *name, uint8_t ptr_size)
{
    if (!name) return masm_operand_none();
    if (strcmp(name, "rax") == 0) return masm_operand_register(MASM_X86_RAX, ptr_size);
    if (strcmp(name, "eax") == 0) return masm_operand_register(MASM_X86_RAX, 4);
    if (strcmp(name, "ax") == 0) return masm_operand_register(MASM_X86_RAX, 2);
    if (strcmp(name, "al") == 0) return masm_operand_register(MASM_X86_RAX, 1);
    if (strcmp(name, "rbx") == 0) return masm_operand_register(MASM_X86_RBX, ptr_size);
    if (strcmp(name, "rcx") == 0) return masm_operand_register(MASM_X86_RCX, ptr_size);
    if (strcmp(name, "rdx") == 0) return masm_operand_register(MASM_X86_RDX, ptr_size);
    if (strcmp(name, "rsi") == 0) return masm_operand_register(MASM_X86_RSI, ptr_size);
    if (strcmp(name, "rdi") == 0) return masm_operand_register(MASM_X86_RDI, ptr_size);
    if (strcmp(name, "rbp") == 0) return masm_operand_register(MASM_X86_RBP, ptr_size);
    if (strcmp(name, "rsp") == 0) return masm_operand_register(MASM_X86_RSP, ptr_size);
    if (strcmp(name, "r8") == 0) return masm_operand_register(MASM_X86_R8, ptr_size);
    if (strcmp(name, "r9") == 0) return masm_operand_register(MASM_X86_R9, ptr_size);
    if (strcmp(name, "r10") == 0) return masm_operand_register(MASM_X86_R10, ptr_size);
    if (strcmp(name, "r11") == 0) return masm_operand_register(MASM_X86_R11, ptr_size);
    if (strcmp(name, "r12") == 0) return masm_operand_register(MASM_X86_R12, ptr_size);
    if (strcmp(name, "r13") == 0) return masm_operand_register(MASM_X86_R13, ptr_size);
    if (strcmp(name, "r14") == 0) return masm_operand_register(MASM_X86_R14, ptr_size);
    if (strcmp(name, "r15") == 0) return masm_operand_register(MASM_X86_R15, ptr_size);
    return masm_operand_none();
}

static const MasmISASpec X86_64_SPEC = {
    .reg_result = x86_reg_result,
    .reg_tmp0   = x86_reg_tmp0,
    .reg_tmp1   = x86_reg_tmp1,
    .reg_div_hi = x86_reg_div_hi,
    .reg_div_lo = x86_reg_div_lo,
    .reg_arg    = x86_reg_arg,
    .reg_sp     = x86_reg_sp,
    .reg_fp     = x86_reg_fp,
    .op_syscall = x86_op_syscall,
    .parse_reg  = x86_parse_reg,
    .reg_count      = MASM_X86_REG_COUNT,
    .scratch_regs   = X86_SCRATCH,
    .scratch_count  = sizeof(X86_SCRATCH) / sizeof(X86_SCRATCH[0]),
    .reserved_regs  = X86_RESERVED,
    .reserved_count = sizeof(X86_RESERVED) / sizeof(X86_RESERVED[0]),
};

const MasmISASpec *masm_isa_spec_select(MasmTarget target)
{
    switch (target.isa)
    {
    case MASM_ISA_X86_64:
        return &X86_64_SPEC;
    default:
        return NULL;
    }
}
