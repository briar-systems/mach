#include "backend/isa/x86_64.h"
#include "mir/mir.h"
#include "backend/codegen.h"
#include "backend/codebuf.h"
#include "backend/object/elf64.h"
#include "backend/reloc.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct X86FunctionFrame
{
    size_t stack_size;       // total stack frame size in bytes
    size_t locals_offset;    // offset from rbp for locals
    bool   needs_frame;      // whether to emit prologue/epilogue
} X86FunctionFrame;

typedef struct X86Emitter
{
    const Target   *target;
    MirModule             *module;
    BackendCodegenResult  *result;
    MirPhysicalReg        *vreg_map;
    size_t                 vreg_count;
    X86FunctionFrame       current_frame;
    bool                   in_function;
} X86Emitter;

static bool x86_lower(const Target *target, MirModule *module, BackendCodegenResult *result);

#define SCRATCH_REG MIR_PREG_R11
#define SHIFT_REG   MIR_PREG_RCX

static void emit_mov_reg_imm(X86Emitter *ctx, uint8_t dst_reg, int64_t imm);
static void emit_mov_reg_reg(X86Emitter *ctx, uint8_t dst_reg, uint8_t src_reg);
static void emit_binop_rr(X86Emitter *ctx, uint8_t opcode, uint8_t dst_reg, uint8_t src_reg);

// helper utilities
static uint8_t encode_preg(MirPhysicalReg preg)
{
    switch (preg)
    {
    case MIR_PREG_RAX: return 0;
    case MIR_PREG_RCX: return 1;
    case MIR_PREG_RDX: return 2;
    case MIR_PREG_RBX: return 3;
    case MIR_PREG_RSP: return 4;
    case MIR_PREG_RBP: return 5;
    case MIR_PREG_RSI: return 6;
    case MIR_PREG_RDI: return 7;
    case MIR_PREG_R8:  return 8;
    case MIR_PREG_R9:  return 9;
    case MIR_PREG_R10: return 10;
    case MIR_PREG_R11: return 11;
    case MIR_PREG_R12: return 12;
    case MIR_PREG_R13: return 13;
    case MIR_PREG_R14: return 14;
    case MIR_PREG_R15: return 15;
    default:           return 0;
    }
}

static void emit_rex_w(CodeBuffer *buf, uint8_t reg, uint8_t rm)
{
    uint8_t rex = 0x48;
    if (reg >= 8)
        rex |= 0x04; // REX.R
    if (rm >= 8)
        rex |= 0x01; // REX.B
    codebuf_emit_byte(buf, rex);
}

static void emit_push_reg(CodeBuffer *buf, uint8_t reg)
{
    if (reg >= 8)
        codebuf_emit_byte(buf, 0x40 | 0x01); // REX.B
    codebuf_emit_byte(buf, 0x50 + (reg & 7));
}

static void emit_pop_reg(CodeBuffer *buf, uint8_t reg)
{
    if (reg >= 8)
        codebuf_emit_byte(buf, 0x40 | 0x01); // REX.B
    codebuf_emit_byte(buf, 0x58 + (reg & 7));
}

static uint8_t modrm(uint8_t mod, uint8_t reg, uint8_t rm)
{
    return (mod << 6) | ((reg & 7) << 3) | (rm & 7);
}

static MirPhysicalReg default_virtual_reg(uint32_t idx)
{
    static const MirPhysicalReg order[] = {
        MIR_PREG_R10,
        MIR_PREG_R11,
        MIR_PREG_R12,
        MIR_PREG_R13,
        MIR_PREG_R14,
        MIR_PREG_R15,
    };
    size_t count = sizeof(order) / sizeof(order[0]);
    return order[idx % count];
}

static uint8_t scratch_reg_code(void)
{
    return encode_preg(SCRATCH_REG);
}

static uint8_t shift_reg_code(void)
{
    return encode_preg(SHIFT_REG);
}

static bool x86_allocate_registers(X86Emitter *ctx)
{
    ctx->vreg_count = ctx->module->next_vreg_id;
    if (ctx->vreg_count == 0)
        return true;

    ctx->vreg_map = calloc(ctx->vreg_count, sizeof(MirPhysicalReg));
    if (!ctx->vreg_map)
        return false;

    for (size_t i = 0; i < ctx->vreg_count; i++)
    {
        ctx->vreg_map[i] = default_virtual_reg((uint32_t)i);
    }
    return true;
}

static uint8_t get_operand_reg(X86Emitter *ctx, const MirOperand *op)
{
    MirPhysicalReg preg = MIR_PREG_RAX;
    if (op->kind == MIR_OPERAND_PREG)
    {
        preg = op->preg;
    }
    else if (op->kind == MIR_OPERAND_VREG)
    {
        if (op->vreg_id < ctx->vreg_count)
            preg = ctx->vreg_map[op->vreg_id];
        else
            preg = default_virtual_reg(op->vreg_id);
    }
    return encode_preg(preg);
}

static bool materialize_operand_reg(X86Emitter *ctx, const MirOperand *op, uint8_t preferred_reg, uint8_t *out_reg)
{
    switch (op->kind)
    {
    case MIR_OPERAND_PREG:
    case MIR_OPERAND_VREG:
        *out_reg = get_operand_reg(ctx, op);
        return true;
    case MIR_OPERAND_IMM:
    {
        uint8_t tmp = preferred_reg;
        if (tmp == UINT8_MAX)
            tmp = scratch_reg_code();
        emit_mov_reg_imm(ctx, tmp, op->immediate);
        *out_reg = tmp;
        return true;
    }
    default:
        return false;
    }
}

static bool get_memory_operand(X86Emitter *ctx, const MirOperand *op, uint8_t *base_reg, int32_t *disp)
{
    if (!op || op->kind != MIR_OPERAND_MEMORY)
        return false;
    if (op->memory.base_vreg >= ctx->vreg_count)
        return false;
    *base_reg = encode_preg(ctx->vreg_map[op->memory.base_vreg]);
    *disp     = op->memory.offset;
    return true;
}

static void emit_modrm_disp32(CodeBuffer *buf, uint8_t reg, uint8_t base_reg, int32_t disp)
{
    bool needs_sib = ((base_reg & 7) == 4);
    codebuf_emit_byte(buf, modrm(2, reg, base_reg));
    if (needs_sib)
    {
        uint8_t sib = (0 << 6) | (4 << 3) | (base_reg & 7);
        codebuf_emit_byte(buf, sib);
    }
    codebuf_emit_u32(buf, (uint32_t)disp);
}

static bool add_text_label(X86Emitter *ctx, const char *name)
{
    size_t offset = codebuf_offset(&ctx->result->text.buffer);
    return backend_label_list_add(&ctx->result->labels, name, BACKEND_SECTION_TEXT, offset);
}

static bool add_rodata_label(X86Emitter *ctx, const char *name)
{
    size_t offset = codebuf_offset(&ctx->result->rodata.buffer);
    return backend_label_list_add(&ctx->result->labels, name, BACKEND_SECTION_RODATA, offset);
}

static bool emit_rel32_placeholder(X86Emitter *ctx, const char *label)
{
    size_t offset = codebuf_offset(&ctx->result->text.buffer);
    codebuf_emit_u32(&ctx->result->text.buffer, 0);
    return backend_reloc_list_add(&ctx->result->relocs,
                                  BACKEND_SECTION_TEXT,
                                  offset,
                                  BACKEND_RELOC_X86_64_PC32,
                                  label,
                                  -4);
}

static void emit_function_prologue(X86Emitter *ctx)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    uint8_t rbp = encode_preg(MIR_PREG_RBP);
    uint8_t rsp = encode_preg(MIR_PREG_RSP);
    
    // push rbp
    emit_push_reg(buf, rbp);
    
    // mov rbp, rsp
    emit_rex_w(buf, rsp, rbp);
    codebuf_emit_byte(buf, 0x89);
    codebuf_emit_byte(buf, modrm(3, rsp, rbp));
    
    // sub rsp, frame_size (if needed)
    if (ctx->current_frame.stack_size > 0)
    {
        size_t frame_size = ctx->current_frame.stack_size;
        // align to 16 bytes
        frame_size = (frame_size + 15) & ~15;
        
        emit_rex_w(buf, 0, rsp);
        if (frame_size <= 127)
        {
            codebuf_emit_byte(buf, 0x83);
            codebuf_emit_byte(buf, modrm(3, 5, rsp));
            codebuf_emit_byte(buf, (uint8_t)frame_size);
        }
        else
        {
            codebuf_emit_byte(buf, 0x81);
            codebuf_emit_byte(buf, modrm(3, 5, rsp));
            codebuf_emit_u32(buf, (uint32_t)frame_size);
        }
    }
}

static void emit_function_epilogue(X86Emitter *ctx)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    
    // leave (mov rsp, rbp; pop rbp)
    codebuf_emit_byte(buf, 0xC9);
}

static void emit_mov_reg_imm(X86Emitter *ctx, uint8_t dst_reg, int64_t imm)
{
    emit_rex_w(&ctx->result->text.buffer, 0, dst_reg);
    codebuf_emit_byte(&ctx->result->text.buffer, 0xB8 + (dst_reg & 7));
    codebuf_emit_u64(&ctx->result->text.buffer, (uint64_t)imm);
}

static void emit_mov_reg_reg(X86Emitter *ctx, uint8_t dst_reg, uint8_t src_reg)
{
    emit_rex_w(&ctx->result->text.buffer, src_reg, dst_reg);
    codebuf_emit_byte(&ctx->result->text.buffer, 0x89);
    codebuf_emit_byte(&ctx->result->text.buffer, modrm(3, src_reg, dst_reg));
}

static void emit_binop_rr(X86Emitter *ctx, uint8_t opcode, uint8_t dst_reg, uint8_t src_reg)
{
    emit_rex_w(&ctx->result->text.buffer, src_reg, dst_reg);
    codebuf_emit_byte(&ctx->result->text.buffer, opcode);
    codebuf_emit_byte(&ctx->result->text.buffer, modrm(3, src_reg, dst_reg));
}

static void emit_binop_reg_imm(X86Emitter *ctx, uint8_t subopcode, uint8_t dst_reg, int32_t imm)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    emit_rex_w(buf, 0, dst_reg);
    if (imm >= -128 && imm <= 127)
    {
        codebuf_emit_byte(buf, 0x83);
        codebuf_emit_byte(buf, modrm(3, subopcode, dst_reg));
        codebuf_emit_byte(buf, (uint8_t)imm);
    }
    else
    {
        codebuf_emit_byte(buf, 0x81);
        codebuf_emit_byte(buf, modrm(3, subopcode, dst_reg));
        codebuf_emit_u32(buf, (uint32_t)imm);
    }
}

static bool move_operand_into_reg(X86Emitter *ctx, const MirOperand *src, uint8_t dst_reg)
{
    if (!src)
        return false;
    switch (src->kind)
    {
    case MIR_OPERAND_PREG:
    case MIR_OPERAND_VREG:
    {
        uint8_t reg = get_operand_reg(ctx, src);
        if (reg != dst_reg)
            emit_mov_reg_reg(ctx, dst_reg, reg);
        return true;
    }
    case MIR_OPERAND_IMM:
        emit_mov_reg_imm(ctx, dst_reg, src->immediate);
        return true;
    default:
        return false;
    }
}

static void emit_cmp_rr(X86Emitter *ctx, uint8_t lhs_reg, uint8_t rhs_reg)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    emit_rex_w(buf, rhs_reg, lhs_reg);
    codebuf_emit_byte(buf, 0x39);
    codebuf_emit_byte(buf, modrm(3, rhs_reg, lhs_reg));
}

static void emit_cmp_reg_imm(X86Emitter *ctx, uint8_t lhs_reg, int32_t imm)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    emit_rex_w(buf, 0, lhs_reg);
    if (imm >= -128 && imm <= 127)
    {
        codebuf_emit_byte(buf, 0x83);
        codebuf_emit_byte(buf, modrm(3, 7, lhs_reg));
        codebuf_emit_byte(buf, (uint8_t)imm);
    }
    else
    {
        codebuf_emit_byte(buf, 0x81);
        codebuf_emit_byte(buf, modrm(3, 7, lhs_reg));
        codebuf_emit_u32(buf, (uint32_t)imm);
    }
}

static void emit_setcc_reg(X86Emitter *ctx, uint8_t dst_reg, uint8_t condition)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    uint8_t rex = 0x40;
    if ((dst_reg & 8) != 0)
        rex |= 0x01; // REX.B
    codebuf_emit_byte(buf, rex);
    codebuf_emit_byte(buf, 0x0F);
    codebuf_emit_byte(buf, 0x90 + condition);
    codebuf_emit_byte(buf, modrm(3, 0, dst_reg & 7));
}

static void emit_movzx_byte(X86Emitter *ctx, uint8_t dst_reg)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    emit_rex_w(buf, dst_reg, dst_reg);
    codebuf_emit_byte(buf, 0x0F);
    codebuf_emit_byte(buf, 0xB6);
    codebuf_emit_byte(buf, modrm(3, dst_reg, dst_reg));
}

static void emit_shift_reg_imm(X86Emitter *ctx, uint8_t opcode_ext, uint8_t dst_reg, uint8_t imm)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    emit_rex_w(buf, 0, dst_reg);
    codebuf_emit_byte(buf, 0xC1);
    codebuf_emit_byte(buf, modrm(3, opcode_ext, dst_reg));
    codebuf_emit_byte(buf, imm);
}

static void emit_shift_reg_cl(X86Emitter *ctx, uint8_t opcode_ext, uint8_t dst_reg)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    uint8_t cl = shift_reg_code();
    emit_rex_w(buf, cl, dst_reg);
    codebuf_emit_byte(buf, 0xD3);
    codebuf_emit_byte(buf, modrm(3, opcode_ext, dst_reg));
}

static bool emit_data_sections(X86Emitter *ctx)
{
    for (MirData *data = ctx->module->data; data; data = data->next)
    {
        if (!data->name)
            continue;
        if (data->kind == MIR_DATA_VAL && data->init_value.string_value)
        {
            if (!add_rodata_label(ctx, data->name))
                return false;

            const char *src = data->init_value.string_value;
            for (size_t i = 0; src[i] != '\0'; i++)
            {
                if (src[i] == '\\' && src[i + 1] != '\0')
                {
                    i++;
                    switch (src[i])
                    {
                    case 'n': codebuf_emit_byte(&ctx->result->rodata.buffer, '\n'); break;
                    case 't': codebuf_emit_byte(&ctx->result->rodata.buffer, '\t'); break;
                    case 'r': codebuf_emit_byte(&ctx->result->rodata.buffer, '\r'); break;
                    case '\\': codebuf_emit_byte(&ctx->result->rodata.buffer, '\\'); break;
                    case '"': codebuf_emit_byte(&ctx->result->rodata.buffer, '"'); break;
                    default:
                        codebuf_emit_byte(&ctx->result->rodata.buffer, src[i]);
                        break;
                    }
                }
                else
                {
                    codebuf_emit_byte(&ctx->result->rodata.buffer, (uint8_t)src[i]);
                }
            }
            codebuf_emit_byte(&ctx->result->rodata.buffer, 0);
        }
    }
    return true;
}

static bool emit_instruction(X86Emitter *ctx, MirInstruction *inst)
{
    CodeBuffer *buf = &ctx->result->text.buffer;
    switch (inst->opcode)
    {
    case MIR_OP_LOAD:
    {
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        uint8_t base_reg;
        int32_t disp;
        if (!get_memory_operand(ctx, &inst->operands[1], &base_reg, &disp))
            return false;
        emit_rex_w(buf, base_reg, dst_reg);
        codebuf_emit_byte(buf, 0x8B);
        emit_modrm_disp32(buf, dst_reg, base_reg, disp);
        break;
    }
    case MIR_OP_STORE:
    {
        uint8_t base_reg;
        int32_t disp;
        if (!get_memory_operand(ctx, &inst->operands[0], &base_reg, &disp))
            return false;
        const MirOperand *src = &inst->operands[1];
        uint8_t src_reg;
        if (src->kind == MIR_OPERAND_IMM)
        {
            src_reg = scratch_reg_code();
            emit_mov_reg_imm(ctx, src_reg, src->immediate);
        }
        else
        {
            src_reg = get_operand_reg(ctx, src);
        }
        emit_rex_w(buf, src_reg, base_reg);
        codebuf_emit_byte(buf, 0x89);
        emit_modrm_disp32(buf, src_reg, base_reg, disp);
        break;
    }
    case MIR_OP_MOV:
    {
        MirOperand *dst = &inst->operands[0];
        MirOperand *src = &inst->operands[1];
        uint8_t dst_reg = get_operand_reg(ctx, dst);
        if (src->kind == MIR_OPERAND_IMM)
        {
            emit_mov_reg_imm(ctx, dst_reg, src->immediate);
        }
        else
        {
            uint8_t src_reg = get_operand_reg(ctx, src);
            emit_mov_reg_reg(ctx, dst_reg, src_reg);
        }
        break;
    }
    case MIR_OP_ADD:
    case MIR_OP_SUB:
    case MIR_OP_OR:
    {
        MirOperand *dst = &inst->operands[0];
        MirOperand *lhs = &inst->operands[1];
        MirOperand *rhs = &inst->operands[2];
        uint8_t dst_reg = get_operand_reg(ctx, dst);
        if (!move_operand_into_reg(ctx, lhs, dst_reg))
            return false;

        uint8_t opcode = 0x01; // ADD
        uint8_t imm_subopcode = 0;
        if (inst->opcode == MIR_OP_SUB)
        {
            opcode = 0x29;
            imm_subopcode = 5;
        }
        else if (inst->opcode == MIR_OP_OR)
        {
            opcode = 0x09;
            imm_subopcode = 1;
        }

        if (rhs->kind == MIR_OPERAND_IMM)
        {
            emit_binop_reg_imm(ctx, imm_subopcode, dst_reg, (int32_t)rhs->immediate);
        }
        else
        {
            uint8_t src_reg = get_operand_reg(ctx, rhs);
            emit_binop_rr(ctx, opcode, dst_reg, src_reg);
        }
        break;
    }
    case MIR_OP_AND:
    case MIR_OP_XOR:
    {
        uint8_t dst_reg  = get_operand_reg(ctx, &inst->operands[0]);
        uint8_t src1_reg = get_operand_reg(ctx, &inst->operands[1]);
        uint8_t src2_reg;
        if (dst_reg != src1_reg)
        {
            emit_mov_reg_reg(ctx, dst_reg, src1_reg);
        }

        MirOperand *rhs = &inst->operands[2];
        if (rhs->kind == MIR_OPERAND_IMM)
        {
            uint8_t subopcode = (inst->opcode == MIR_OP_AND) ? 4 : 6;
            emit_binop_reg_imm(ctx, subopcode, dst_reg, (int32_t)rhs->immediate);
        }
        else
        {
            src2_reg = get_operand_reg(ctx, rhs);
            uint8_t opcode = (inst->opcode == MIR_OP_AND) ? 0x21 : 0x31;
            emit_binop_rr(ctx, opcode, dst_reg, src2_reg);
        }
        break;
    }
    case MIR_OP_MUL:
    {
        // imul dst, src (two-operand form)
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        if (!move_operand_into_reg(ctx, &inst->operands[1], dst_reg))
            return false;
        
        MirOperand *rhs = &inst->operands[2];
        if (rhs->kind == MIR_OPERAND_IMM)
        {
            // imul dst, imm32
            emit_rex_w(buf, dst_reg, dst_reg);
            codebuf_emit_byte(buf, 0x69);
            codebuf_emit_byte(buf, modrm(3, dst_reg, dst_reg));
            codebuf_emit_u32(buf, (uint32_t)rhs->immediate);
        }
        else
        {
            uint8_t src_reg = get_operand_reg(ctx, rhs);
            emit_rex_w(buf, dst_reg, src_reg);
            codebuf_emit_byte(buf, 0x0F);
            codebuf_emit_byte(buf, 0xAF);
            codebuf_emit_byte(buf, modrm(3, dst_reg, src_reg));
        }
        break;
    }
    case MIR_OP_DIV:
    case MIR_OP_MOD:
    {
        // div/mod: rax = dividend, operand = divisor, result in rax (quotient) or rdx (remainder)
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        uint8_t rax = encode_preg(MIR_PREG_RAX);
        uint8_t rdx = encode_preg(MIR_PREG_RDX);
        
        // move dividend into rax
        if (!move_operand_into_reg(ctx, &inst->operands[1], rax))
            return false;
        
        // sign-extend rax into rdx:rax (cqo)
        codebuf_emit_byte(buf, 0x48);
        codebuf_emit_byte(buf, 0x99);
        
        // get divisor into a register
        MirOperand *divisor = &inst->operands[2];
        uint8_t divisor_reg;
        if (!materialize_operand_reg(ctx, divisor, scratch_reg_code(), &divisor_reg))
            return false;
        
        // idiv divisor_reg
        emit_rex_w(buf, 0, divisor_reg);
        codebuf_emit_byte(buf, 0xF7);
        codebuf_emit_byte(buf, modrm(3, 7, divisor_reg));
        
        // move result to dst
        if (inst->opcode == MIR_OP_DIV)
        {
            if (dst_reg != rax)
                emit_mov_reg_reg(ctx, dst_reg, rax);
        }
        else // MOD
        {
            if (dst_reg != rdx)
                emit_mov_reg_reg(ctx, dst_reg, rdx);
        }
        break;
    }
    case MIR_OP_NOT:
    case MIR_OP_NEG:
    {
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        if (!move_operand_into_reg(ctx, &inst->operands[1], dst_reg))
            return false;
        uint8_t subcode = (inst->opcode == MIR_OP_NOT) ? 2 : 3;
        emit_rex_w(buf, 0, dst_reg);
        codebuf_emit_byte(buf, 0xF7);
        codebuf_emit_byte(buf, modrm(3, subcode, dst_reg));
        break;
    }
    case MIR_OP_ZEXT:
    case MIR_OP_SEXT:
    case MIR_OP_TRUNC:
    {
        // for now, simple register moves (proper size handling would check inst->type/type2)
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        if (!move_operand_into_reg(ctx, &inst->operands[1], dst_reg))
            return false;
        
        // ZEXT: movzx if needed (for now just ensure high bits clear)
        // SEXT: movsx if needed
        // TRUNC: just use lower bits
        // Proper implementation would check source/dest sizes from inst->type/type2
        break;
    }
    case MIR_OP_BITCAST:
    {
        // bitcast: reinterpret bits, just move register
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        if (!move_operand_into_reg(ctx, &inst->operands[1], dst_reg))
            return false;
        break;
    }
    case MIR_OP_SHL:
    case MIR_OP_SHR:
    case MIR_OP_SAR:
    {
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        if (!move_operand_into_reg(ctx, &inst->operands[1], dst_reg))
            return false;

        uint8_t ext = 4; // SHL
        if (inst->opcode == MIR_OP_SHR)
            ext = 5;
        else if (inst->opcode == MIR_OP_SAR)
            ext = 7;

        MirOperand *amount = &inst->operands[2];
        if (amount->kind == MIR_OPERAND_IMM)
        {
            uint8_t shift = (uint8_t)(amount->immediate & 0xFF);
            emit_shift_reg_imm(ctx, ext, dst_reg, shift);
        }
        else
        {
            uint8_t shift_reg = shift_reg_code();
            uint8_t src_reg = get_operand_reg(ctx, amount);
            if (src_reg != shift_reg)
                emit_mov_reg_reg(ctx, shift_reg, src_reg);
            emit_shift_reg_cl(ctx, ext, dst_reg);
        }
        break;
    }
    case MIR_OP_CMP_EQ:
    case MIR_OP_CMP_NE:
    case MIR_OP_CMP_LT:
    case MIR_OP_CMP_LE:
    case MIR_OP_CMP_GT:
    case MIR_OP_CMP_GE:
    {
        MirOperand *dst = &inst->operands[0];
        MirOperand *lhs = &inst->operands[1];
        MirOperand *rhs = &inst->operands[2];
        uint8_t dst_reg = get_operand_reg(ctx, dst);

        if (lhs->kind == MIR_OPERAND_IMM)
            return false;

        uint8_t lhs_reg = get_operand_reg(ctx, lhs);
        if (rhs->kind == MIR_OPERAND_IMM)
        {
            emit_cmp_reg_imm(ctx, lhs_reg, (int32_t)rhs->immediate);
        }
        else
        {
            uint8_t rhs_reg = get_operand_reg(ctx, rhs);
            emit_cmp_rr(ctx, lhs_reg, rhs_reg);
        }

        uint8_t condition = 0;
        switch (inst->opcode)
        {
        case MIR_OP_CMP_EQ: condition = 0x04; break;
        case MIR_OP_CMP_NE: condition = 0x05; break;
        case MIR_OP_CMP_LT: condition = 0x0C; break;
        case MIR_OP_CMP_LE: condition = 0x0E; break;
        case MIR_OP_CMP_GT: condition = 0x0F; break;
        case MIR_OP_CMP_GE: condition = 0x0D; break;
        default: break;
        }
        emit_setcc_reg(ctx, dst_reg, condition);
        emit_movzx_byte(ctx, dst_reg);
        break;
    }
    case MIR_OP_LEA:
    {
        uint8_t dst_reg = get_operand_reg(ctx, &inst->operands[0]);
        const char *label = inst->operands[1].label;
        emit_rex_w(buf, dst_reg, 5);
        codebuf_emit_byte(buf, 0x8D);
        codebuf_emit_byte(buf, modrm(0, dst_reg, 5));
        if (!emit_rel32_placeholder(ctx, label))
            return false;
        break;
    }
    case MIR_OP_JMP:
    {
        codebuf_emit_byte(buf, 0xE9);
        if (!emit_rel32_placeholder(ctx, inst->operands[0].label))
            return false;
        break;
    }
    case MIR_OP_BR:
    {
        MirOperand *cond = &inst->operands[0];
        uint8_t cond_reg;
        if (!materialize_operand_reg(ctx, cond, scratch_reg_code(), &cond_reg))
            return false;

        // compare cond != 0
        emit_cmp_reg_imm(ctx, cond_reg, 0);

        // jne true
        codebuf_emit_byte(buf, 0x0F);
        codebuf_emit_byte(buf, 0x85);
        if (!emit_rel32_placeholder(ctx, inst->operands[1].label))
            return false;

        // jmp false
        codebuf_emit_byte(buf, 0xE9);
        if (!emit_rel32_placeholder(ctx, inst->operands[2].label))
            return false;
        break;
    }
    case MIR_OP_ARCH_SYSCALL:
        codebuf_emit_byte(buf, 0x0F);
        codebuf_emit_byte(buf, 0x05);
        break;
    case MIR_OP_RET:
        // emit epilogue if in a function
        if (ctx->in_function && ctx->current_frame.needs_frame)
        {
            emit_function_epilogue(ctx);
        }
        codebuf_emit_byte(buf, 0xC3);
        break;
    case MIR_OP_UNREACHABLE:
        codebuf_emit_byte(buf, 0x0F);
        codebuf_emit_byte(buf, 0x0B);
        break;
    case MIR_OP_ALLOCA:
    {
        // alloca: allocate stack space and return address in dst
        // For now, just bump the frame size and return rbp-offset
        MirOperand *dst = &inst->operands[0];
        // operands[1] would be size, but for simple case assume compile-time known
        
        // allocate 16 bytes aligned
        size_t alloc_size = 16;
        ctx->current_frame.stack_size += alloc_size;
        size_t offset = ctx->current_frame.stack_size;
        
        uint8_t dst_reg = get_operand_reg(ctx, dst);
        uint8_t rbp = encode_preg(MIR_PREG_RBP);
        
        // lea dst, [rbp - offset]
        emit_rex_w(buf, dst_reg, rbp);
        codebuf_emit_byte(buf, 0x8D);
        codebuf_emit_byte(buf, modrm(2, dst_reg, rbp));
        codebuf_emit_u32(buf, (uint32_t)(-((int32_t)offset)));
        break;
    }
    default:
        return false;
    }
    return true;
}

static bool emit_block(X86Emitter *ctx, MirBasicBlock *block)
{
    // detect function entry: exported block starts a new function
    if (block->is_exported)
    {
        // finish previous function if any
        if (ctx->in_function)
        {
            ctx->in_function = false;
        }
        
        // start new function
        ctx->in_function = true;
        ctx->current_frame.stack_size = 0;
        ctx->current_frame.locals_offset = 0;
        ctx->current_frame.needs_frame = true;
        
        if (block->label)
        {
            if (!add_text_label(ctx, block->label))
                return false;
        }
        
        // emit prologue
        if (ctx->current_frame.needs_frame)
        {
            emit_function_prologue(ctx);
        }
    }
    else
    {
        // regular block label
        if (block->label)
        {
            if (!add_text_label(ctx, block->label))
                return false;
        }
    }
    
    for (MirInstruction *inst = block->instructions; inst; inst = inst->next)
    {
        if (!emit_instruction(ctx, inst))
            return false;
    }
    return true;
}

static bool x86_lower(const Target *target, MirModule *module, BackendCodegenResult *result)
{
    X86Emitter ctx = {
        .target = target,
        .module = module,
        .result = result,
        .vreg_map = NULL,
        .vreg_count = 0,
        .current_frame = {0},
        .in_function = false,
    };

    if (!x86_allocate_registers(&ctx))
        goto fail;
    if (!emit_data_sections(&ctx))
        goto fail;

    for (MirBasicBlock *block = module->blocks; block; block = block->next)
    {
        if (!emit_block(&ctx, block))
            goto fail;
    }

    free(ctx.vreg_map);
    return true;

fail:
    free(ctx.vreg_map);
    return false;
}

bool backend_register_target_x86_64_linux(void)
{
    static TargetISA isa = { .lower = x86_lower };
    static TargetABI abi = { .name = "sysv64" };
    static RuntimeShim runtime = { .entry_label = "_start" };
    static Target target;
    static bool registered = false;

    if (registered)
        return true;

    target.desc   = target_desc(TARGET_ARCH_X86_64, TARGET_OS_LINUX);
    target.format = TARGET_OBJ_ELF;
    target.isa    = &isa;
    target.abi    = &abi;
    target.writer = backend_object_writer_elf64();
    target.runtime = &runtime;

    registered = target_register(&target);
    return registered;
}
