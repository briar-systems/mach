#include "compiler/masm/isa/x86_64/x86_64.h"
#include "compiler/masm/isa/x86_64/asm.h"
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

    // handle IR pseudo-ops that survive isel
    if (inst.kind == MASM_OPCODE_IR)
    {
        switch (inst.opcode)
        {
        case MASM_IR_LABEL:
            // no bytes emitted for labels
            break;
        default:
            // unknown IR opcode in x86 encoder
            break;
        }
        return (int)offset;
    }

    // handle x86 opcodes
    switch (inst.opcode)
    {
    case MASM_OP_X86_SYSCALL:
        emit_byte(buffer, &offset, size, 0x0F);
        emit_byte(buffer, &offset, size, 0x05);
        break;

    case MASM_OP_X86_MOV_RR:
        if (inst.operand_count == 2)
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
        break;

    case MASM_OP_X86_MOV_RM:
        if (inst.operand_count == 2)
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
        break;

    case MASM_OP_X86_MOV_MR:
        if (inst.operand_count == 2)
        {
            uint32_t src    = inst.operands[1].reg.id;
            uint8_t  sz     = inst.operands[1].reg.size;
            bool     w      = sz == 8;
            uint8_t  opcode = (sz == 1) ? 0x88 : 0x89;
            bool rex_r     = src >= 8;
            bool rex_force = (sz == 1) && (src >= 4 && src <= 7);
            bool rex_x = inst.operands[0].mem.index.id >= 8;
            bool rex_b = inst.operands[0].mem.base.id >= 8;
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, rex_x, rex_b);
            emit_byte(buffer, &offset, size, opcode);
            emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id, inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, src);
        }
        break;

    case MASM_OP_X86_MOV_RI:
        if (inst.operand_count == 2)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint8_t  sz  = inst.operands[0].reg.size;

            if (inst.operands[1].kind == MASM_OPERAND_LABEL || inst.operands[1].kind == MASM_OPERAND_SYMBOL)
            {
                emit_rex(buffer, &offset, size, true, false, false, dst >= 8);
                emit_byte(buffer, &offset, size, (uint8_t)(0xB8 + reg_low(dst)));
                emit_imm64(buffer, &offset, size, 0);
            }
            else
            {
                int64_t  imm = inst.operands[1].imm;
                if (sz == 1)
                {
                    bool rex_b     = dst >= 8;
                    bool rex_force = dst >= 4 && dst <= 7;
                    emit_rex_force(buffer, &offset, size, rex_force, false, false, false, rex_b);
                    emit_byte(buffer, &offset, size, (uint8_t)(0xB0 + reg_low(dst)));
                    emit_byte(buffer, &offset, size, (int8_t)imm);
                }
                else
                {
                    if (sz == 2) emit_byte(buffer, &offset, size, 0x66);

                    if (sz == 8 && (uint64_t)imm <= 0xFFFFFFFF)
                    {
                        emit_rex(buffer, &offset, size, false, false, false, dst >= 8);
                        emit_byte(buffer, &offset, size, (uint8_t)(0xB8 + reg_low(dst)));
                        emit_imm32(buffer, &offset, size, (int32_t)imm);
                    }
                    else if (sz == 8 && (imm >= -2147483648LL && imm <= 2147483647LL))
                    {
                        emit_rex(buffer, &offset, size, true, false, false, dst >= 8);
                        emit_byte(buffer, &offset, size, 0xC7);
                        emit_byte(buffer, &offset, size, encode_modrm(0xC0, 0, reg_low(dst)));
                        emit_imm32(buffer, &offset, size, (int32_t)imm);
                    }
                    else
                    {
                        emit_rex(buffer, &offset, size, sz == 8, false, false, dst >= 8);
                        emit_byte(buffer, &offset, size, (uint8_t)(0xB8 + reg_low(dst)));
                        if (sz == 8)
                            emit_imm64(buffer, &offset, size, imm);
                        else if (sz == 2)
                        {
                            emit_byte(buffer, &offset, size, (uint8_t)imm);
                            emit_byte(buffer, &offset, size, (uint8_t)(imm >> 8));
                        }
                        else
                            emit_imm32(buffer, &offset, size, (int32_t)imm);
                    }
                }
            }
        }
        break;

    case MASM_OP_X86_MOV_MI:
        if (inst.operand_count == 2)
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

            emit_mem(buffer, &offset, size, base, index, scale, disp, 0);

            if (sz == 1)
                emit_byte(buffer, &offset, size, (int8_t)imm);
            else
                emit_imm32(buffer, &offset, size, (int32_t)imm);
        }
        break;

    case MASM_OP_X86_LEA:
        if (inst.operand_count == 2)
        {
            uint32_t dst = inst.operands[0].reg.id;
            bool     w   = inst.operands[0].reg.size == 8;
            emit_rex(buffer, &offset, size, w, dst >= 8, inst.operands[1].mem.index.id >= 8, inst.operands[1].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, 0x8D);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, dst);
        }
        break;

    case MASM_OP_X86_MOVZX_RR:
    case MASM_OP_X86_MOVSX_RR:
        if (inst.operand_count == 2)
        {
            uint32_t dst     = inst.operands[0].reg.id;
            bool     dst64   = inst.operands[0].reg.size == 8;
            uint8_t  srcsize = inst.operands[1].reg.size;
            bool     src16   = srcsize == 2;
            bool     src8    = srcsize == 1;

            bool is_zx = (inst.opcode == MASM_OP_X86_MOVZX_RR);
            uint8_t opcode = 0x0F;
            uint8_t ext    = is_zx ? (src16 ? 0xB7 : 0xB6) : (src16 ? 0xBF : 0xBE);

            uint32_t src   = inst.operands[1].reg.id;
            bool     rex_r     = dst >= 8;
            bool     rex_b     = (src >= 8);
            bool     rex_force = src8 && (src >= 4 && src <= 7);
            emit_rex_force(buffer, &offset, size, rex_force, dst64, rex_r, false, rex_b);
            emit_byte(buffer, &offset, size, opcode);
            emit_byte(buffer, &offset, size, ext);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst), reg_low(src)));
        }
        break;

    case MASM_OP_X86_MOVZX_RM:
    case MASM_OP_X86_MOVSX_RM:
        if (inst.operand_count == 2)
        {
            uint32_t dst     = inst.operands[0].reg.id;
            bool     dst64   = inst.operands[0].reg.size == 8;
            uint8_t  srcsize = inst.operands[1].mem.size;
            bool     src16   = srcsize == 2;

            bool is_zx = (inst.opcode == MASM_OP_X86_MOVZX_RM);
            uint8_t opcode = 0x0F;
            uint8_t ext    = is_zx ? (src16 ? 0xB7 : 0xB6) : (src16 ? 0xBF : 0xBE);

            bool rex_r = dst >= 8;
            bool rex_x = inst.operands[1].mem.index.id >= 8;
            bool rex_b = inst.operands[1].mem.base.id >= 8;
            emit_rex(buffer, &offset, size, dst64, rex_r, rex_x, rex_b);
            emit_byte(buffer, &offset, size, opcode);
            emit_byte(buffer, &offset, size, ext);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id, inst.operands[1].mem.scale,
                        (int32_t)inst.operands[1].mem.disp, dst);
        }
        break;

    // Arithmetic (RR, MR variants use the same opcode: op r/m, r)
    case MASM_OP_X86_ADD_RR: case MASM_OP_X86_ADD_MR:
    case MASM_OP_X86_OR_RR:  case MASM_OP_X86_OR_MR:
    case MASM_OP_X86_AND_RR: case MASM_OP_X86_AND_MR:
    case MASM_OP_X86_SUB_RR: case MASM_OP_X86_SUB_MR:
    case MASM_OP_X86_XOR_RR: case MASM_OP_X86_XOR_MR:
    case MASM_OP_X86_CMP_RR: case MASM_OP_X86_CMP_MR:
    case MASM_OP_X86_TEST_RR: case MASM_OP_X86_TEST_MR:
        if (inst.operand_count == 2)
        {
            uint32_t reg = inst.operands[1].reg.id;
            uint8_t  sz  = inst.operands[1].reg.size;
            bool     byte = sz == 1;
            bool     w    = sz == 8;
            uint8_t  opcode = 0;

            switch (inst.opcode) {
                case MASM_OP_X86_ADD_RR: case MASM_OP_X86_ADD_MR: opcode = byte ? 0x00 : 0x01; break;
                case MASM_OP_X86_OR_RR:  case MASM_OP_X86_OR_MR:  opcode = byte ? 0x08 : 0x09; break;
                case MASM_OP_X86_AND_RR: case MASM_OP_X86_AND_MR: opcode = byte ? 0x20 : 0x21; break;
                case MASM_OP_X86_SUB_RR: case MASM_OP_X86_SUB_MR: opcode = byte ? 0x28 : 0x29; break;
                case MASM_OP_X86_XOR_RR: case MASM_OP_X86_XOR_MR: opcode = byte ? 0x30 : 0x31; break;
                case MASM_OP_X86_CMP_RR: case MASM_OP_X86_CMP_MR: opcode = byte ? 0x38 : 0x39; break;
                case MASM_OP_X86_TEST_RR: case MASM_OP_X86_TEST_MR: opcode = byte ? 0x84 : 0x85; break;
            }

            // For RR: inst->operands[0] is r/m (dst), inst->operands[1] is reg (src)
            // For MR: inst->operands[0] is mem (dst), inst->operands[1] is reg (src)
            bool is_rr = (inst.opcode == MASM_OP_X86_ADD_RR || inst.opcode == MASM_OP_X86_OR_RR ||
                          inst.opcode == MASM_OP_X86_AND_RR || inst.opcode == MASM_OP_X86_SUB_RR ||
                          inst.opcode == MASM_OP_X86_XOR_RR || inst.opcode == MASM_OP_X86_CMP_RR ||
                          inst.opcode == MASM_OP_X86_TEST_RR);
            
            bool rex_r = reg >= 8;
            bool rex_x = false;
            bool rex_b = false;
            bool rex_force = false;

            if (is_rr) {
                uint32_t rm = inst.operands[0].reg.id;
                rex_b = rm >= 8;
                rex_force = byte && ((reg >= 4 && reg <= 7) || (rm >= 4 && rm <= 7));
                emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, false, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(reg), reg_low(rm)));
            } else {
                rex_b = inst.operands[0].mem.base.id >= 8;
                rex_x = inst.operands[0].mem.index.id >= 8;
                rex_force = byte && (reg >= 4 && reg <= 7);
                emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                         inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, reg);
            }
        }
        break;

    // Arithmetic (RM variants: op r, r/m)
    case MASM_OP_X86_ADD_RM:
    case MASM_OP_X86_OR_RM:
    case MASM_OP_X86_AND_RM:
    case MASM_OP_X86_SUB_RM:
    case MASM_OP_X86_XOR_RM:
    case MASM_OP_X86_CMP_RM:
    case MASM_OP_X86_TEST_RM:
        if (inst.operand_count == 2)
        {
            uint32_t reg = inst.operands[0].reg.id;
            uint8_t  sz  = inst.operands[0].reg.size;
            bool     byte = sz == 1;
            bool     w    = sz == 8;
            uint8_t  opcode = 0;

            switch (inst.opcode) {
                case MASM_OP_X86_ADD_RM: opcode = byte ? 0x02 : 0x03; break;
                case MASM_OP_X86_OR_RM:  opcode = byte ? 0x0A : 0x0B; break;
                case MASM_OP_X86_AND_RM: opcode = byte ? 0x22 : 0x23; break;
                case MASM_OP_X86_SUB_RM: opcode = byte ? 0x2A : 0x2B; break;
                case MASM_OP_X86_XOR_RM: opcode = byte ? 0x32 : 0x33; break;
                case MASM_OP_X86_CMP_RM: opcode = byte ? 0x3A : 0x3B; break;
                case MASM_OP_X86_TEST_RM: opcode = byte ? 0x84 : 0x85; break; // TEST is commutative, use same as MR
            }

            bool rex_r = reg >= 8;
            bool rex_x = inst.operands[1].mem.index.id >= 8;
            bool rex_b = inst.operands[1].mem.base.id >= 8;
            bool rex_force = byte && (reg >= 4 && reg <= 7);
            emit_rex_force(buffer, &offset, size, rex_force, w, rex_r, rex_x, rex_b);
            emit_byte(buffer, &offset, size, opcode);
            emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id,
                     inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, reg);
        }
        break;

    // Arithmetic Immediate (RI, MI)
    case MASM_OP_X86_ADD_RI: case MASM_OP_X86_ADD_MI:
    case MASM_OP_X86_OR_RI:  case MASM_OP_X86_OR_MI:
    case MASM_OP_X86_AND_RI: case MASM_OP_X86_AND_MI:
    case MASM_OP_X86_SUB_RI: case MASM_OP_X86_SUB_MI:
    case MASM_OP_X86_XOR_RI: case MASM_OP_X86_XOR_MI:
    case MASM_OP_X86_CMP_RI: case MASM_OP_X86_CMP_MI:
    case MASM_OP_X86_TEST_RI: case MASM_OP_X86_TEST_MI:
        if (inst.operand_count == 2)
        {
            // dst is op[0] (reg or mem), imm is op[1]
            uint8_t sz = (inst.opcode == MASM_OP_X86_ADD_RI || inst.opcode == MASM_OP_X86_OR_RI ||
                          inst.opcode == MASM_OP_X86_AND_RI || inst.opcode == MASM_OP_X86_SUB_RI ||
                          inst.opcode == MASM_OP_X86_XOR_RI || inst.opcode == MASM_OP_X86_CMP_RI ||
                          inst.opcode == MASM_OP_X86_TEST_RI) ? inst.operands[0].reg.size : inst.operands[0].mem.size;
            
            int64_t imm = inst.operands[1].imm;
            bool byte = sz == 1;
            bool w = sz == 8;
            uint8_t subop = 0;
            bool is_test = false;

            switch (inst.opcode) {
                case MASM_OP_X86_ADD_RI: case MASM_OP_X86_ADD_MI: subop = 0; break;
                case MASM_OP_X86_OR_RI:  case MASM_OP_X86_OR_MI:  subop = 1; break;
                case MASM_OP_X86_AND_RI: case MASM_OP_X86_AND_MI: subop = 4; break;
                case MASM_OP_X86_SUB_RI: case MASM_OP_X86_SUB_MI: subop = 5; break;
                case MASM_OP_X86_XOR_RI: case MASM_OP_X86_XOR_MI: subop = 6; break;
                case MASM_OP_X86_CMP_RI: case MASM_OP_X86_CMP_MI: subop = 7; break;
                case MASM_OP_X86_TEST_RI: case MASM_OP_X86_TEST_MI: subop = 0; is_test = true; break;
            }

            // Determine Group 1 (80/81/83) or Group 3 (F6/F7 for TEST)
            uint8_t opcode = 0;
            bool imm8_opt = (imm >= -128 && imm <= 127);

            if (is_test) {
                opcode = byte ? 0xF6 : 0xF7;
            } else {
                if (byte) opcode = 0x80;
                else if (imm8_opt) opcode = 0x83;
                else opcode = 0x81;
            }

            bool rex_b = false;
            bool rex_x = false;
            bool rex_force = false;
            uint32_t rm_val = 0;

            bool is_ri = (inst.operands[0].kind == MASM_OPERAND_REGISTER);
            if (is_ri) {
                rm_val = inst.operands[0].reg.id;
                rex_b = rm_val >= 8;
                rex_force = byte && (rm_val >= 4 && rm_val <= 7);
                emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(rm_val)));
            } else {
                rm_val = inst.operands[0].mem.base.id;
                rex_b = rm_val >= 8;
                rex_x = inst.operands[0].mem.index.id >= 8;
                emit_rex_force(buffer, &offset, size, false, w, false, rex_x, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_mem(buffer, &offset, size, rm_val, inst.operands[0].mem.index.id,
                         inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, subop);
            }

            if (byte || (imm8_opt && !is_test)) {
                emit_byte(buffer, &offset, size, (int8_t)imm);
            } else {
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        break;

    // IMUL
    case MASM_OP_X86_IMUL_RR: case MASM_OP_X86_IMUL_RM:
        if (inst.operand_count == 2)
        {
            uint32_t dst = inst.operands[0].reg.id;
            bool w = inst.operands[0].reg.size == 8;
            bool rex_r = dst >= 8;
            bool rex_b, rex_x = false;

            if (inst.opcode == MASM_OP_X86_IMUL_RR) {
                uint32_t src = inst.operands[1].reg.id;
                rex_b = src >= 8;
                emit_rex(buffer, &offset, size, w, rex_r, false, rex_b);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0xAF);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst), reg_low(src)));
            } else {
                rex_b = inst.operands[1].mem.base.id >= 8;
                rex_x = inst.operands[1].mem.index.id >= 8;
                emit_rex(buffer, &offset, size, w, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0xAF);
                emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id,
                         inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, reg_low(dst));
            }
        }
        break;

    case MASM_OP_X86_IMUL_RRI: case MASM_OP_X86_IMUL_RMI:
        if (inst.operand_count == 3)
        {
            uint32_t dst = inst.operands[0].reg.id;
            bool w = inst.operands[0].reg.size == 8;
            int64_t imm = inst.operands[2].imm;
            bool imm8 = (imm >= -128 && imm <= 127);
            uint8_t opcode = imm8 ? 0x6B : 0x69;
            bool rex_r = dst >= 8;
            bool rex_b, rex_x = false;

            if (inst.opcode == MASM_OP_X86_IMUL_RRI) {
                uint32_t src = inst.operands[1].reg.id;
                rex_b = src >= 8;
                emit_rex(buffer, &offset, size, w, rex_r, false, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst), reg_low(src)));
            } else {
                rex_b = inst.operands[1].mem.base.id >= 8;
                rex_x = inst.operands[1].mem.index.id >= 8;
                emit_rex(buffer, &offset, size, w, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, opcode);
                emit_mem(buffer, &offset, size, inst.operands[1].mem.base.id, inst.operands[1].mem.index.id,
                         inst.operands[1].mem.scale, (int32_t)inst.operands[1].mem.disp, reg_low(dst));
            }

            if (imm8) emit_byte(buffer, &offset, size, (int8_t)imm);
            else emit_imm32(buffer, &offset, size, (int32_t)imm);
        }
        break;

    case MASM_OP_X86_DIV: case MASM_OP_X86_IDIV:
        if (inst.operand_count == 1)
        {
            bool signed_div = (inst.opcode == MASM_OP_X86_IDIV);
            uint8_t subopcode = signed_div ? 7 : 6;
            bool is_reg = (inst.operands[0].kind == MASM_OPERAND_REGISTER);
            uint8_t sz = is_reg ? inst.operands[0].reg.size : inst.operands[0].mem.size;
            bool byte = sz == 1;
            bool w = sz == 8;
            bool word = sz == 2;

            if (word) emit_byte(buffer, &offset, size, 0x66);
            uint8_t opcode = byte ? 0xF6 : 0xF7;

            if (is_reg) {
                uint32_t rm = inst.operands[0].reg.id;
                bool rex_force = byte && (rm >= 4 && rm <= 7);
                emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rm >= 8);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subopcode, reg_low(rm)));
            } else {
                emit_rex(buffer, &offset, size, w, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
                emit_byte(buffer, &offset, size, opcode);
                emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                         inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, subopcode);
            }
        }
        break;

    case MASM_OP_X86_NEG_R: case MASM_OP_X86_NEG_M:
    case MASM_OP_X86_NOT_R: case MASM_OP_X86_NOT_M:
        if (inst.operand_count == 1)
        {
            uint8_t subopcode = 0;
            if (inst.opcode == MASM_OP_X86_NOT_R || inst.opcode == MASM_OP_X86_NOT_M) subopcode = 2;
            else subopcode = 3; // NEG

            bool is_reg = (inst.operands[0].kind == MASM_OPERAND_REGISTER);
            uint8_t sz = is_reg ? inst.operands[0].reg.size : inst.operands[0].mem.size;
            bool byte = sz == 1;
            bool w = sz == 8;
            bool word = sz == 2;

            if (word) emit_byte(buffer, &offset, size, 0x66);
            uint8_t opcode = byte ? 0xF6 : 0xF7;

            if (is_reg) {
                uint32_t rm = inst.operands[0].reg.id;
                bool rex_force = byte && (rm >= 4 && rm <= 7);
                emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rm >= 8);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subopcode, reg_low(rm)));
            } else {
                emit_rex(buffer, &offset, size, w, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
                emit_byte(buffer, &offset, size, opcode);
                emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                         inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, subopcode);
            }
        }
        break;

    case MASM_OP_X86_SHL_RI: case MASM_OP_X86_SHL_RC:
    case MASM_OP_X86_SHR_RI: case MASM_OP_X86_SHR_RC:
    case MASM_OP_X86_SAR_RI: case MASM_OP_X86_SAR_RC:
        if (inst.operand_count == 2)
        {
            uint8_t subop = 0;
            if (inst.opcode == MASM_OP_X86_SHL_RI || inst.opcode == MASM_OP_X86_SHL_RC) subop = 4;
            else if (inst.opcode == MASM_OP_X86_SHR_RI || inst.opcode == MASM_OP_X86_SHR_RC) subop = 5;
            else subop = 7;

            bool is_cl = (inst.opcode == MASM_OP_X86_SHL_RC || inst.opcode == MASM_OP_X86_SHR_RC || inst.opcode == MASM_OP_X86_SAR_RC);
            bool is_reg = (inst.operands[0].kind == MASM_OPERAND_REGISTER);
            uint8_t sz = is_reg ? inst.operands[0].reg.size : inst.operands[0].mem.size;
            bool byte = sz == 1;
            bool w = sz == 8;
            bool word = sz == 2;
            int64_t imm = 0;
            if (!is_cl) imm = inst.operands[1].imm;

            if (word) emit_byte(buffer, &offset, size, 0x66);
            
            uint8_t opcode;
            if (is_cl) opcode = byte ? 0xD2 : 0xD3;
            else if (imm == 1) opcode = byte ? 0xD0 : 0xD1;
            else opcode = byte ? 0xC0 : 0xC1;

            if (is_reg) {
                uint32_t rm = inst.operands[0].reg.id;
                bool rex_force = byte && (rm >= 4 && rm <= 7);
                emit_rex_force(buffer, &offset, size, rex_force, w, false, false, rm >= 8);
                emit_byte(buffer, &offset, size, opcode);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, subop, reg_low(rm)));
            } else {
                emit_rex(buffer, &offset, size, w, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
                emit_byte(buffer, &offset, size, opcode);
                emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                         inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, subop);
            }

            if (!is_cl && imm != 1) emit_byte(buffer, &offset, size, (int8_t)imm);
        }
        break;

    case MASM_OP_X86_SETE: case MASM_OP_X86_SETNE:
    case MASM_OP_X86_SETL: case MASM_OP_X86_SETG:
    case MASM_OP_X86_SETLE: case MASM_OP_X86_SETGE:
    case MASM_OP_X86_SETB: case MASM_OP_X86_SETA:
    case MASM_OP_X86_SETBE: case MASM_OP_X86_SETAE:
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
             uint8_t cc = 0x90;
             switch(inst.opcode) {
                 case MASM_OP_X86_SETE: cc = 0x94; break;
                 case MASM_OP_X86_SETNE: cc = 0x95; break;
                 case MASM_OP_X86_SETL: cc = 0x9C; break;
                 case MASM_OP_X86_SETLE: cc = 0x9E; break;
                 case MASM_OP_X86_SETG: cc = 0x9F; break;
                 case MASM_OP_X86_SETGE: cc = 0x9D; break;
                 case MASM_OP_X86_SETB: cc = 0x92; break;
                 case MASM_OP_X86_SETA: cc = 0x97; break;
                 case MASM_OP_X86_SETBE: cc = 0x96; break;
                 case MASM_OP_X86_SETAE: cc = 0x93; break;
             }
             uint32_t r = inst.operands[0].reg.id;
             bool rex = r >= 4;
             if (rex) emit_rex_force(buffer, &offset, size, true, false, false, false, r >= 8);
             emit_byte(buffer, &offset, size, 0x0F);
             emit_byte(buffer, &offset, size, cc);
             emit_byte(buffer, &offset, size, encode_modrm(0xC0, 0, reg_low(r)));
        }
        break;

    case MASM_OP_X86_PUSH_R:
        if (inst.operand_count == 1) {
            uint32_t r = inst.operands[0].reg.id;
            emit_rex(buffer, &offset, size, false, false, false, r >= 8);
            emit_byte(buffer, &offset, size, (uint8_t)(0x50 + reg_low(r)));
        }
        break;
    case MASM_OP_X86_POP_R:
        if (inst.operand_count == 1) {
            uint32_t r = inst.operands[0].reg.id;
            emit_rex(buffer, &offset, size, false, false, false, r >= 8);
            emit_byte(buffer, &offset, size, (uint8_t)(0x58 + reg_low(r)));
        }
        break;
    case MASM_OP_X86_PUSH_I:
        if (inst.operand_count == 1) {
            int64_t imm = inst.operands[0].imm;
            if (imm >= -128 && imm <= 127) {
                emit_byte(buffer, &offset, size, 0x6A);
                emit_byte(buffer, &offset, size, (int8_t)imm);
            } else {
                emit_byte(buffer, &offset, size, 0x68);
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        break;
    case MASM_OP_X86_PUSH_M:
        if (inst.operand_count == 1) {
            emit_rex(buffer, &offset, size, false, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, 0xFF);
            emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                     inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, 6);
        }
        break;
    case MASM_OP_X86_POP_M:
        if (inst.operand_count == 1) {
            emit_rex(buffer, &offset, size, false, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
            emit_byte(buffer, &offset, size, 0x8F);
            emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                     inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, 0);
        }
        break;

    case MASM_OP_X86_JMP_REL:
        emit_byte(buffer, &offset, size, 0xE9);
        if (inst.operands[0].kind == MASM_OPERAND_IMM)
            emit_imm32(buffer, &offset, size, (int32_t)inst.operands[0].imm);
        else
            emit_imm32(buffer, &offset, size, 0);
        break;
    case MASM_OP_X86_JMP_RM:
        emit_rex(buffer, &offset, size, false, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
        emit_byte(buffer, &offset, size, 0xFF);
        emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                 inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, 4);
        break;
    case MASM_OP_X86_CALL_REL:
        emit_byte(buffer, &offset, size, 0xE8);
        if (inst.operands[0].kind == MASM_OPERAND_IMM)
            emit_imm32(buffer, &offset, size, (int32_t)inst.operands[0].imm);
        else
            emit_imm32(buffer, &offset, size, 0);
        break;
    case MASM_OP_X86_CALL_RM:
        if (inst.operands[0].kind == MASM_OPERAND_REGISTER) {
            uint32_t rm = inst.operands[0].reg.id;
            emit_rex(buffer, &offset, size, false, false, false, rm >= 8);
            emit_byte(buffer, &offset, size, 0xFF);
            emit_byte(buffer, &offset, size, encode_modrm(0xC0, 2, reg_low(rm)));
        } else {
             emit_rex(buffer, &offset, size, false, false, inst.operands[0].mem.index.id >= 8, inst.operands[0].mem.base.id >= 8);
             emit_byte(buffer, &offset, size, 0xFF);
             emit_mem(buffer, &offset, size, inst.operands[0].mem.base.id, inst.operands[0].mem.index.id,
                      inst.operands[0].mem.scale, (int32_t)inst.operands[0].mem.disp, 2);
        }
        break;
    case MASM_OP_X86_RET:
        emit_byte(buffer, &offset, size, 0xC3);
        break;
    
    case MASM_OP_X86_JE: case MASM_OP_X86_JNE:
    case MASM_OP_X86_JL: case MASM_OP_X86_JG:
    case MASM_OP_X86_JLE: case MASM_OP_X86_JGE:
        {
            uint8_t cc = 0;
            switch(inst.opcode) {
                case MASM_OP_X86_JE: cc = 0x84; break;
                case MASM_OP_X86_JNE: cc = 0x85; break;
                case MASM_OP_X86_JL: cc = 0x8C; break;
                case MASM_OP_X86_JLE: cc = 0x8E; break;
                case MASM_OP_X86_JG: cc = 0x8F; break;
                case MASM_OP_X86_JGE: cc = 0x8D; break;
            }
            emit_byte(buffer, &offset, size, 0x0F);
            emit_byte(buffer, &offset, size, cc);
            emit_imm32(buffer, &offset, size, (int32_t)inst.operands[0].imm);
        }
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

    case MASM_OP_X86_CVTSI2SD:
    case MASM_OP_X86_CVTSI2SS:
        // cvtsi2sd xmm, r/m32|r/m64 (F2 0F 2A /r)
        // cvtsi2ss xmm, r/m32|r/m64 (F3 0F 2A /r)
        if (inst.operand_count == 2)
        {
            MasmOperand dst = inst.operands[0];
            MasmOperand src = inst.operands[1];
            bool ss = inst.opcode == MASM_OP_X86_CVTSI2SS;

            emit_byte(buffer, &offset, size, ss ? 0xF3 : 0xF2);

            uint32_t dst_id = dst.reg.id;
            bool rex_r = dst_id >= 8;

            if (src.kind == MASM_OPERAND_REGISTER)
            {
                bool w = src.reg.size == 8;
                uint32_t rm = src.reg.id;
                emit_rex(buffer, &offset, size, w, rex_r, false, rm >= 8);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x2A);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst_id), reg_low(rm)));
            }
            else if (src.kind == MASM_OPERAND_MEMORY)
            {
                bool w = src.mem.size == 8;
                bool rex_x = src.mem.index.id >= 8;
                bool rex_b = src.mem.base.id >= 8;
                emit_rex(buffer, &offset, size, w, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x2A);
                emit_mem(buffer, &offset, size, src.mem.base.id, src.mem.index.id, src.mem.scale, (int32_t)src.mem.disp, reg_low(dst_id));
            }
        }
        break;

    case MASM_OP_X86_CVTTSD2SI:
    case MASM_OP_X86_CVTTSS2SI:
        // cvttsd2si r32|r64, xmm/m64 (F2 0F 2C /r)
        // cvttss2si r32|r64, xmm/m32 (F3 0F 2C /r)
        if (inst.operand_count == 2)
        {
            MasmOperand dst = inst.operands[0];
            MasmOperand src = inst.operands[1];
            bool ss = inst.opcode == MASM_OP_X86_CVTTSS2SI;

            emit_byte(buffer, &offset, size, ss ? 0xF3 : 0xF2);

            uint32_t dst_id = dst.reg.id;
            bool w = dst.reg.size == 8;
            bool rex_r = dst_id >= 8;

            if (src.kind == MASM_OPERAND_REGISTER)
            {
                uint32_t rm = src.reg.id;
                emit_rex(buffer, &offset, size, w, rex_r, false, rm >= 8);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x2C);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst_id), reg_low(rm)));
            }
            else if (src.kind == MASM_OPERAND_MEMORY)
            {
                bool rex_x = src.mem.index.id >= 8;
                bool rex_b = src.mem.base.id >= 8;
                emit_rex(buffer, &offset, size, w, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x2C);
                emit_mem(buffer, &offset, size, src.mem.base.id, src.mem.index.id, src.mem.scale, (int32_t)src.mem.disp, reg_low(dst_id));
            }
        }
        break;

    case MASM_OP_X86_CVTSD2SS:
    case MASM_OP_X86_CVTSS2SD:
        // cvtsd2ss xmm1, xmm2/m64 (F2 0F 5A /r)
        // cvtss2sd xmm1, xmm2/m32 (F3 0F 5A /r)
        if (inst.operand_count == 2)
        {
            MasmOperand dst = inst.operands[0];
            MasmOperand src = inst.operands[1];
            bool sd2ss = inst.opcode == MASM_OP_X86_CVTSD2SS;

            emit_byte(buffer, &offset, size, sd2ss ? 0xF2 : 0xF3);

            uint32_t dst_id = dst.reg.id;
            bool rex_r = dst_id >= 8;

            if (src.kind == MASM_OPERAND_REGISTER)
            {
                uint32_t rm = src.reg.id;
                emit_rex(buffer, &offset, size, false, rex_r, false, rm >= 8);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x5A);
                emit_byte(buffer, &offset, size, encode_modrm(0xC0, reg_low(dst_id), reg_low(rm)));
            }
            else if (src.kind == MASM_OPERAND_MEMORY)
            {
                bool rex_x = src.mem.index.id >= 8;
                bool rex_b = src.mem.base.id >= 8;
                emit_rex(buffer, &offset, size, false, rex_r, rex_x, rex_b);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0x5A);
                emit_mem(buffer, &offset, size, src.mem.base.id, src.mem.index.id, src.mem.scale, (int32_t)src.mem.disp, reg_low(dst_id));
            }
        }
        break;

    case MASM_OP_X86_CBW:
        // AL -> AX
        emit_byte(buffer, &offset, size, 0x66);
        emit_byte(buffer, &offset, size, 0x98);
        break;

    case MASM_OP_X86_CWD:
        // AX -> DX:AX
        emit_byte(buffer, &offset, size, 0x66);
        emit_byte(buffer, &offset, size, 0x99);
        break;

    case MASM_OP_X86_CDQ:
        // EAX -> EDX:EAX
        emit_byte(buffer, &offset, size, 0x99);
        break;

    case MASM_OP_X86_CQO:
        // sign-extend RAX into RDX:RAX
        emit_rex(buffer, &offset, size, true, false, false, false);
        emit_byte(buffer, &offset, size, 0x99);
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
    if (reg == UINT32_MAX) return masm_operand_none();
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
    .reg_result     = x86_reg_result,
    .reg_tmp0       = x86_reg_tmp0,
    .reg_tmp1       = x86_reg_tmp1,
    .reg_div_hi     = x86_reg_div_hi,
    .reg_div_lo     = x86_reg_div_lo,
    .reg_arg        = x86_reg_arg,
    .reg_sp         = x86_reg_sp,
    .reg_fp         = x86_reg_fp,
    .op_syscall     = x86_op_syscall,
    .parse_reg        = x86_parse_reg,
    .parse_inline_asm = masm_x86_parse_inline_asm,
    .reg_count        = MASM_X86_REG_COUNT,
    .scratch_regs   = X86_SCRATCH,
    .scratch_count  = sizeof(X86_SCRATCH) / sizeof(X86_SCRATCH[0]),
    .reserved_regs  = X86_RESERVED,
    .reserved_count = sizeof(X86_RESERVED) / sizeof(X86_RESERVED[0]),
    .isel           = masm_x86_isel,
    .encode         = masm_x86_encode,
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
