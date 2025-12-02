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
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            emit_byte(buffer, &offset, size, 0xE8);
            emit_imm32(buffer, &offset, size, 0);
        }
    }
    else if (inst.opcode == MASM_OP_MOV)
    {
        if (inst.operand_count == 2 && 
            inst.operands[0].kind == MASM_OPERAND_REGISTER && 
            inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t reg = inst.operands[0].reg.id;
            int64_t  imm = inst.operands[1].imm;
            
            if (inst.operands[0].reg.size == 8)
            {
                uint8_t rex = 0x48;
                if (reg >= 8) rex |= 0x01; // REX.B
                emit_byte(buffer, &offset, size, rex);
                emit_byte(buffer, &offset, size, 0xB8 + (reg & 7));
                emit_imm64(buffer, &offset, size, imm);
            }
            else if (inst.operands[0].reg.size == 4)
            {
                uint8_t rex = 0;
                if (reg >= 8) rex |= 0x41; // REX.B (no W bit for 32-bit, but need REX if reg >= 8)
                if (rex) emit_byte(buffer, &offset, size, rex);
                emit_byte(buffer, &offset, size, 0xB8 + (reg & 7));
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
             uint32_t dst = inst.operands[0].reg.id;
             uint32_t src = inst.operands[1].reg.id;
             
             if (inst.operands[0].reg.size == 8 && inst.operands[1].reg.size == 8)
             {
                 uint8_t rex = 0x48;
                 if (src >= 8) rex |= 0x04; // REX.R
                 if (dst >= 8) rex |= 0x01; // REX.B
                 emit_byte(buffer, &offset, size, rex);
                 emit_byte(buffer, &offset, size, 0x89);
                 uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                 emit_byte(buffer, &offset, size, modrm);
             }
        }
        // MOV reg, [mem] - load from memory
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_MEMORY)
        {
            uint32_t dst_reg = inst.operands[0].reg.id;
            uint32_t base_reg = inst.operands[1].mem.base.id;
            int32_t disp = (int32_t)inst.operands[1].mem.disp;
            
            if (inst.operands[0].reg.size == 8)
            {
                uint8_t rex = 0x48;
                if (dst_reg >= 8) rex |= 0x04; // REX.R (dst is reg field)
                if (base_reg >= 8) rex |= 0x01; // REX.B (base is r/m field)
                emit_byte(buffer, &offset, size, rex);
                emit_byte(buffer, &offset, size, 0x8B);  // MOV r64, r/m64
                
                // ModR/M byte: mod=01 (disp8) or mod=10 (disp32), reg=dst, r/m=base
                uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
                uint8_t modrm = mod | ((dst_reg & 7) << 3) | (base_reg & 7);
                emit_byte(buffer, &offset, size, modrm);
                
                // Displacement
                if (mod == 0x40)
                    emit_byte(buffer, &offset, size, (int8_t)disp);
                else
                    emit_imm32(buffer, &offset, size, disp);
            }
            else if (inst.operands[0].reg.size == 4)
            {
                uint8_t rex = 0;
                if (dst_reg >= 8) rex |= 0x04; // REX.R
                if (base_reg >= 8) rex |= 0x01; // REX.B
                if (rex) emit_byte(buffer, &offset, size, rex);
                
                emit_byte(buffer, &offset, size, 0x8B);  // MOV r32, r/m32
                
                // ModR/M byte
                uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
                uint8_t modrm = mod | ((dst_reg & 7) << 3) | (base_reg & 7);
                emit_byte(buffer, &offset, size, modrm);
                
                // Displacement
                if (mod == 0x40)
                    emit_byte(buffer, &offset, size, (int8_t)disp);
                else
                    emit_imm32(buffer, &offset, size, disp);
            }
        }
        // MOV [mem], reg - store to memory
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_MEMORY &&
                 inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t base_reg = inst.operands[0].mem.base.id;
            uint32_t src_reg = inst.operands[1].reg.id;
            int32_t disp = (int32_t)inst.operands[0].mem.disp;
            
            if (inst.operands[1].reg.size == 8)
            {
                uint8_t rex = 0x48;
                if (src_reg >= 8) rex |= 0x04; // REX.R (src is reg field)
                if (base_reg >= 8) rex |= 0x01; // REX.B (base is r/m field)
                emit_byte(buffer, &offset, size, rex);
                emit_byte(buffer, &offset, size, 0x89);  // MOV r/m64, r64
                
                // ModR/M byte
                uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
                uint8_t modrm = mod | ((src_reg & 7) << 3) | (base_reg & 7);
                emit_byte(buffer, &offset, size, modrm);
                
                // Displacement
                if (mod == 0x40)
                    emit_byte(buffer, &offset, size, (int8_t)disp);
                else
                    emit_imm32(buffer, &offset, size, disp);
            }
            else if (inst.operands[1].reg.size == 4)
            {
                uint8_t rex = 0;
                if (src_reg >= 8) rex |= 0x04; // REX.R
                if (base_reg >= 8) rex |= 0x01; // REX.B
                if (rex) emit_byte(buffer, &offset, size, rex);
                
                emit_byte(buffer, &offset, size, 0x89);  // MOV r/m32, r32
                
                // ModR/M byte
                uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
                uint8_t modrm = mod | ((src_reg & 7) << 3) | (base_reg & 7);
                emit_byte(buffer, &offset, size, modrm);
                
                // Displacement
                if (mod == 0x40)
                    emit_byte(buffer, &offset, size, (int8_t)disp);
                else
                    emit_imm32(buffer, &offset, size, disp);
            }
        }
        // MOV [mem], imm
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_MEMORY &&
                 inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t base_reg = inst.operands[0].mem.base.id;
            int32_t disp = (int32_t)inst.operands[0].mem.disp;
            int64_t imm = inst.operands[1].imm;
            uint8_t mem_size = inst.operands[0].mem.size;
            
            if (mem_size == 8)
            {
                // MOV r/m64, imm32 (sign extended)
                uint8_t rex = 0x48;
                if (base_reg >= 8) rex |= 0x01; // REX.B
                emit_byte(buffer, &offset, size, rex);
                emit_byte(buffer, &offset, size, 0xC7);
                
                uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
                uint8_t modrm = mod | (0 << 3) | (base_reg & 7); // /0
                emit_byte(buffer, &offset, size, modrm);
                
                if (mod == 0x40)
                    emit_byte(buffer, &offset, size, (int8_t)disp);
                else
                    emit_imm32(buffer, &offset, size, disp);
                    
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
            else if (mem_size == 4)
            {
                // MOV r/m32, imm32
                uint8_t rex = 0;
                if (base_reg >= 8) rex |= 0x01; // REX.B
                if (rex) emit_byte(buffer, &offset, size, rex);
                
                emit_byte(buffer, &offset, size, 0xC7);
                
                uint8_t mod = (disp >= -128 && disp <= 127) ? 0x40 : 0x80;
                uint8_t modrm = mod | (0 << 3) | (base_reg & 7); // /0
                emit_byte(buffer, &offset, size, modrm);
                
                if (mod == 0x40)
                    emit_byte(buffer, &offset, size, (int8_t)disp);
                else
                    emit_imm32(buffer, &offset, size, disp);
                    
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
    }
    else if (inst.opcode == MASM_OP_ADD)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0x01);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t dst = inst.operands[0].reg.id;
            int64_t imm = inst.operands[1].imm;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                
                // 81 /0 id: ADD r/m64, imm32
                // 83 /0 ib: ADD r/m64, imm8
                if (imm >= -128 && imm <= 127)
                {
                    emit_byte(buffer, &offset, size, 0x83);
                    uint8_t modrm = 0xC0 | (0 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_byte(buffer, &offset, size, (int8_t)imm);
                }
                else
                {
                    emit_byte(buffer, &offset, size, 0x81);
                    uint8_t modrm = 0xC0 | (0 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_imm32(buffer, &offset, size, (int32_t)imm);
                }
            }
        }
    }
    else if (inst.opcode == MASM_OP_AND)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0x21);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t dst = inst.operands[0].reg.id;
            int64_t imm = inst.operands[1].imm;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                
                // 81 /4 id: AND r/m64, imm32
                // 83 /4 ib: AND r/m64, imm8
                if (imm >= -128 && imm <= 127)
                {
                    emit_byte(buffer, &offset, size, 0x83);
                    uint8_t modrm = 0xC0 | (4 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_byte(buffer, &offset, size, (int8_t)imm);
                }
                else
                {
                    emit_byte(buffer, &offset, size, 0x81);
                    uint8_t modrm = 0xC0 | (4 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_imm32(buffer, &offset, size, (int32_t)imm);
                }
            }
        }
    }
    else if (inst.opcode == MASM_OP_SUB)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0x29);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t dst = inst.operands[0].reg.id;
            int64_t imm = inst.operands[1].imm;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                
                // 81 /5 id: SUB r/m64, imm32
                // 83 /5 ib: SUB r/m64, imm8
                if (imm >= -128 && imm <= 127)
                {
                    emit_byte(buffer, &offset, size, 0x83);
                    uint8_t modrm = 0xC0 | (5 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_byte(buffer, &offset, size, (int8_t)imm);
                }
                else
                {
                    emit_byte(buffer, &offset, size, 0x81);
                    uint8_t modrm = 0xC0 | (5 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_imm32(buffer, &offset, size, (int32_t)imm);
                }
            }
        }
    }
    else if (inst.opcode == MASM_OP_IMUL)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0x0F);
                emit_byte(buffer, &offset, size, 0xAF);
                uint8_t modrm = 0xC0 | ((dst & 7) << 3) | (src & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
    }
    else if (inst.opcode == MASM_OP_IDIV)
    {
        if (inst.operand_count == 1 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t divisor = inst.operands[0].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0xF7);
                uint8_t modrm = 0xF8 | (divisor & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
    }
    else if (inst.opcode == MASM_OP_CQO)
    {
        emit_byte(buffer, &offset, size, 0x48);
        emit_byte(buffer, &offset, size, 0x99);
    }
    else if (inst.opcode == MASM_OP_CMP)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0x39);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
        else if (inst.operand_count == 2 &&
                 inst.operands[0].kind == MASM_OPERAND_REGISTER &&
                 inst.operands[1].kind == MASM_OPERAND_IMM)
        {
            uint32_t dst = inst.operands[0].reg.id;
            int64_t imm = inst.operands[1].imm;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                
                // 81 /7 id: CMP r/m64, imm32
                // 83 /7 ib: CMP r/m64, imm8
                if (imm >= -128 && imm <= 127)
                {
                    emit_byte(buffer, &offset, size, 0x83);
                    uint8_t modrm = 0xC0 | (7 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_byte(buffer, &offset, size, (int8_t)imm);
                }
                else
                {
                    emit_byte(buffer, &offset, size, 0x81);
                    uint8_t modrm = 0xC0 | (7 << 3) | (dst & 7);
                    emit_byte(buffer, &offset, size, modrm);
                    emit_imm32(buffer, &offset, size, (int32_t)imm);
                }
            }
        }
    }
    else if (inst.opcode == MASM_OP_TEST)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            if (inst.operands[0].reg.size == 8)
            {
                emit_byte(buffer, &offset, size, 0x48);
                emit_byte(buffer, &offset, size, 0x85);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
    }
    else if (inst.opcode >= MASM_OP_SETE && inst.opcode <= MASM_OP_SETGE)
    {
        // SETcc instructions
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t reg = inst.operands[0].reg.id;
            
            emit_byte(buffer, &offset, size, 0x0F);
            
            // opcode map for SETcc
            uint8_t setcc_opcode = 0x94;
            switch (inst.opcode)
            {
                case MASM_OP_SETE:   setcc_opcode = 0x94; break;
                case MASM_OP_SETNE:  setcc_opcode = 0x95; break;
                case MASM_OP_SETL:   setcc_opcode = 0x9C; break;
                case MASM_OP_SETG:   setcc_opcode = 0x9F; break;
                case MASM_OP_SETLE:  setcc_opcode = 0x9E; break;
                case MASM_OP_SETGE:  setcc_opcode = 0x9D; break;
            }
            
            emit_byte(buffer, &offset, size, setcc_opcode);
            uint8_t modrm = 0xC0 | (reg & 7);
            emit_byte(buffer, &offset, size, modrm);
        }
    }
    else if (inst.opcode == MASM_OP_PUSH)
    {
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t reg = inst.operands[0].reg.id;
            if (inst.operands[0].reg.size == 8)
            {
                // 50+rd: PUSH r64
                // No REX.W needed for 64-bit push, default is 64-bit
                // But if we want to be explicit or if reg > 7, we might need REX.B
                if (reg > 7)
                {
                    emit_byte(buffer, &offset, size, 0x41); // REX.B
                }
                emit_byte(buffer, &offset, size, 0x50 + (reg & 7));
            }
        }
        else if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_IMM)
        {
            int64_t imm = inst.operands[0].imm;
            if (imm >= -128 && imm <= 127)
            {
                // 6A ib: PUSH imm8
                emit_byte(buffer, &offset, size, 0x6A);
                emit_byte(buffer, &offset, size, (int8_t)imm);
            }
            else
            {
                // 68 id: PUSH imm32
                emit_byte(buffer, &offset, size, 0x68);
                emit_imm32(buffer, &offset, size, (int32_t)imm);
            }
        }
    }
    else if (inst.opcode == MASM_OP_POP)
    {
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t reg = inst.operands[0].reg.id;
            if (inst.operands[0].reg.size == 8)
            {
                // 58+rd: POP r64
                if (reg > 7)
                {
                    emit_byte(buffer, &offset, size, 0x41); // REX.B
                }
                emit_byte(buffer, &offset, size, 0x58 + (reg & 7));
            }
        }
    }
    else if (inst.opcode == MASM_OP_XOR)
    {
        if (inst.operand_count == 2 &&
            inst.operands[0].kind == MASM_OPERAND_REGISTER &&
            inst.operands[1].kind == MASM_OPERAND_REGISTER)
        {
            uint32_t dst = inst.operands[0].reg.id;
            uint32_t src = inst.operands[1].reg.id;
            
            // XOR r32, r32 (also clears upper 32 bits, effectively r64, r64 zeroing)
            if (inst.operands[0].reg.size == 4)
            {
                // 31 /r: XOR r/m32, r32
                // REX prefix not needed for 32-bit unless regs > 7
                if (dst > 7 || src > 7)
                {
                     uint8_t rex = 0x40 | ((src >> 3) << 2) | (dst >> 3);
                     emit_byte(buffer, &offset, size, rex);
                }
                emit_byte(buffer, &offset, size, 0x31);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
            else if (inst.operands[0].reg.size == 8)
            {
                // REX.W + 31 /r: XOR r/m64, r64
                uint8_t rex = 0x48 | ((src >> 3) << 2) | (dst >> 3);
                emit_byte(buffer, &offset, size, rex);
                emit_byte(buffer, &offset, size, 0x31);
                uint8_t modrm = 0xC0 | ((src & 7) << 3) | (dst & 7);
                emit_byte(buffer, &offset, size, modrm);
            }
        }
    }
    else if (inst.opcode == MASM_OP_JMP)
    {
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            emit_byte(buffer, &offset, size, 0xE9);
            emit_imm32(buffer, &offset, size, 0); // placeholder
        }
    }
    else if (inst.opcode >= MASM_OP_JE && inst.opcode <= MASM_OP_JLE)
    {
        if (inst.operand_count == 1 && inst.operands[0].kind == MASM_OPERAND_LABEL)
        {
            emit_byte(buffer, &offset, size, 0x0F);
            
            uint8_t opcode = 0x84; // JE
            switch (inst.opcode)
            {
                case MASM_OP_JE:  opcode = 0x84; break;
                case MASM_OP_JNE: opcode = 0x85; break;
                case MASM_OP_JL:  opcode = 0x8C; break;
                case MASM_OP_JG:  opcode = 0x8F; break;
                case MASM_OP_JLE: opcode = 0x8E; break;
                case MASM_OP_JGE: opcode = 0x8D; break;
            }
            
            emit_byte(buffer, &offset, size, opcode);
            emit_imm32(buffer, &offset, size, 0); // placeholder
        }
    }
    else if (inst.opcode == MASM_OP_LABEL)
    {
    }
    
    return offset;
}
