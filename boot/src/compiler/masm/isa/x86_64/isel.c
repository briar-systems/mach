#include "compiler/masm/ir.h"
#include "compiler/masm/isa/x86_64/x86_64.h"
#include "compiler/masm/masm.h"
#include "compiler/masm/section.h"
#include "compiler/masm/symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// instruction selection: lowers portable MASM IR to x86_64-specific opcodes

#define VREG_START 1024

typedef struct CodeGenContext
{
    // Map vreg_id -> stack offset (rbp - offset)
    // index = vreg_id - VREG_START
    int32_t *vreg_offsets;
    size_t   vreg_capacity;
    size_t   max_vreg_index; // Track max vreg seen in function

    uint32_t base_stack_size;
    uint32_t total_stack_size;
} CodeGenContext;

static void ctx_init(CodeGenContext *ctx)
{
    ctx->vreg_offsets     = NULL;
    ctx->vreg_capacity    = 0;
    ctx->max_vreg_index   = 0;
    ctx->base_stack_size  = 0;
    ctx->total_stack_size = 0;
}

static void ctx_reset(CodeGenContext *ctx)
{
    if (ctx->vreg_offsets)
    {
        free(ctx->vreg_offsets);
    }
    ctx->vreg_offsets     = NULL;
    ctx->vreg_capacity    = 0;
    ctx->max_vreg_index   = 0;
    ctx->base_stack_size  = 0;
    ctx->total_stack_size = 0;
}

// Assign stack slot to vreg if not already assigned
static int32_t get_vreg_offset(CodeGenContext *ctx, uint32_t vreg_id)
{
    if (vreg_id < VREG_START)
    {
        return 0; // Should not happen for vreg lookup
    }

    size_t idx = vreg_id - VREG_START;
    if (idx >= ctx->vreg_capacity)
    {
        size_t new_cap = (idx + 1) * 2;
        if (new_cap < 64)
        {
            new_cap = 64;
        }
        ctx->vreg_offsets = realloc(ctx->vreg_offsets, new_cap * sizeof(int32_t));
        // Initialize new slots to 0 (unassigned)
        for (size_t i = ctx->vreg_capacity; i < new_cap; i++)
        {
            ctx->vreg_offsets[i] = 0;
        }
        ctx->vreg_capacity = new_cap;
    }

    if (ctx->vreg_offsets[idx] == 0)
    {
        // Allocate new slot
        // Slots start after base_stack_size.
        // We calculate them relative to RBP.
        // Stack grows down. Local vars are at [rbp - base_size ... rbp].
        // Vregs will be at [rbp - total ... rbp - base_size].
        // But we don't know total yet during scanning?
        // We just assign an index and finalize offsets later, or just assign dynamically.
        // Let's use a simple counter for now relative to the end of locals.

        // Note: We need to know this during the pre-scan phase.
        // This function is intended to be called during code gen, relying on pre-calculated info?
        // Or we do it on the fly if we reserved enough space?
        // Let's assume we scanned max_vreg_index and calculated offsets.
        // But simply: offset = base_stack_size + (idx + 1) * 8
        // So [rbp - offset]
        ctx->vreg_offsets[idx] = ctx->base_stack_size + (int32_t)((idx + 1) * 8);
    }

    return ctx->vreg_offsets[idx];
}

// Pre-scan function to find max vreg
static void scan_function(CodeGenContext *ctx, MasmInstruction *insts, size_t count)
{
    ctx->max_vreg_index = 0;
    bool found          = false;

    for (size_t i = 0; i < count; i++)
    {
        MasmInstruction *inst = &insts[i];
        if (inst->kind == MASM_OPCODE_IR && inst->opcode == MASM_IR_STACK_FRAME)
        {
            // Reset context for new function
            // Note: This naive scan assumes we process one function at a time.
            // But the section contains multiple. We'll handle that in the main loop.
            if (inst->operand_count > 0 && inst->operands[0].kind == MASM_OPERAND_IMM)
            {
                ctx->base_stack_size = (uint32_t)inst->operands[0].imm;
            }
        }

        for (int j = 0; j < inst->operand_count; j++)
        {
            MasmOperand *op = &inst->operands[j];
            if (op->kind == MASM_OPERAND_REGISTER && op->reg.id >= VREG_START)
            {
                size_t idx = op->reg.id - VREG_START;
                if (!found || idx > ctx->max_vreg_index)
                {
                    ctx->max_vreg_index = idx;
                    found               = true;
                }
            }
            if (op->kind == MASM_OPERAND_MEMORY && op->mem.base.id >= VREG_START)
            {
                size_t idx = op->mem.base.id - VREG_START;
                if (!found || idx > ctx->max_vreg_index)
                {
                    ctx->max_vreg_index = idx;
                    found               = true;
                }
            }
            if (op->kind == MASM_OPERAND_MEMORY && op->mem.index.id >= VREG_START)
            {
                size_t idx = op->mem.index.id - VREG_START;
                if (!found || idx > ctx->max_vreg_index)
                {
                    ctx->max_vreg_index = idx;
                    found               = true;
                }
            }
        }
    }

    // Calculate total stack size
    // base + (max_index + 1) * 8
    // align to 16
    uint32_t vreg_space   = found ? (uint32_t)((ctx->max_vreg_index + 1) * 8) : 0;
    ctx->total_stack_size = ctx->base_stack_size + vreg_space;
    if (ctx->total_stack_size % 16 != 0)
    {
        ctx->total_stack_size += (16 - (ctx->total_stack_size % 16));
    }
}

// -----------------------------------------------------------------------------
// Emitter Helpers
// -----------------------------------------------------------------------------

static void emit_inst(MasmSection *section, MasmInstruction inst)
{
    masm_section_append_inst(section, inst);
}

// Load operand into physical register
// If op is IMM -> MOV reg, imm
// If op is VREG -> MOV reg, [rbp - offset]
// If op is PHYS -> MOV reg, phys (if different)
// If op is MEM -> LEA reg, mem (if we want address) or MOV reg, mem (load)
// context: 0=load value, 1=load effective address
static void load_operand(MasmSection *sec, CodeGenContext *ctx, MasmOperand op, MasmX86Reg dest_reg, int mode)
{
    MasmOperand dest = masm_x86_reg(dest_reg, 8); // assume 64-bit scratch

    if (op.kind == MASM_OPERAND_REGISTER)
    {
        if (op.reg.id >= VREG_START)
        {
            int32_t     off = get_vreg_offset(ctx, op.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, op.reg.size);

            // Sign-extend if loading small value into 64-bit reg?
            // For now, use MOVZX for small, MOV for 32/64
            if (op.reg.size == 1 || op.reg.size == 2)
            {
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVZX_RM, dest, mem));
            }
            else
            {
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RM, dest, mem));
            }
        }
        else
        {
            // Physical register
            if (op.reg.id != (uint32_t)dest_reg)
            {
                MasmOperand src = masm_operand_register(op.reg.id, 8); // assume full width copy
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, dest, src));
            }
        }
    }
    else if (op.kind == MASM_OPERAND_IMM)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, dest, op));
    }
    else if (op.kind == MASM_OPERAND_MEMORY)
    {
        // Resolve memory operand (which might use vreg base)
        MasmOperand mem = op;
        if (mem.mem.base.id >= VREG_START)
        {
            // Load base vreg into R11 (address scratch)
            int32_t     off       = get_vreg_offset(ctx, mem.mem.base.id);
            MasmOperand base_slot = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RM, masm_x86_reg(MASM_X86_R11, 8), base_slot));
            mem.mem.base.id = MASM_X86_R11;
        }

        // If mode == 1 (LEA), we want the address
        if (mode == 1)
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_LEA, dest, mem));
        }
        else
        {
            // Load value
            if (mem.mem.size == 1 || mem.mem.size == 2)
            {
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVZX_RM, dest, mem));
            }
            else
            {
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RM, dest, mem));
            }
        }
    }
    else if (op.kind == MASM_OPERAND_LABEL || op.kind == MASM_OPERAND_SYMBOL)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, dest, op));
    }
}

// Store physical register to vreg slot
static void store_vreg(MasmSection *sec, CodeGenContext *ctx, uint32_t vreg_id, MasmX86Reg src_reg, uint8_t size)
{
    if (vreg_id < VREG_START)
    {
        if (vreg_id != (uint32_t)src_reg)
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, masm_operand_register(vreg_id, size), masm_x86_reg(src_reg, size)));
        }
        return;
    }

    int32_t     off  = get_vreg_offset(ctx, vreg_id);
    MasmOperand slot = masm_operand_memory_simple(MASM_X86_RBP, -off, size);
    MasmOperand src  = masm_x86_reg(src_reg, size); // store partial reg if size < 8

    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_MR, slot, src));
}

// -----------------------------------------------------------------------------
// Translation Handlers
// -----------------------------------------------------------------------------

static void emit_div_rem(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, bool is_signed, bool is_rem)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a   = inst->operands[1];
    MasmOperand b   = inst->operands[2];

    uint8_t size = 8;
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        size = dst.reg.size;
    }
    else if (a.kind == MASM_OPERAND_REGISTER)
    {
        size = a.reg.size;
    }

    // Load a -> RAX
    load_operand(sec, ctx, a, MASM_X86_RAX, 0);

    // Prepare dividend
    if (size == 1)
    {
        // 8-bit: Dividend is AX.
        if (is_signed)
        {
            emit_inst(sec, masm_x86_inst_0(MASM_OP_X86_CBW)); // AL -> AX
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_AND_RI, masm_x86_reg(MASM_X86_RAX, 8), masm_operand_imm(0xFF))); // Zero AH
        }
    }
    else if (size == 2)
    {
        // 16-bit: Dividend is DX:AX
        if (is_signed)
        {
            emit_inst(sec, masm_x86_inst_0(MASM_OP_X86_CWD)); // AX -> DX:AX
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_XOR_RR, masm_x86_reg(MASM_X86_RDX, 2), masm_x86_reg(MASM_X86_RDX, 2))); // Zero DX
        }
    }
    else if (size == 4)
    {
        // 32-bit: Dividend is EDX:EAX
        if (is_signed)
        {
            emit_inst(sec, masm_x86_inst_0(MASM_OP_X86_CDQ)); // EAX -> EDX:EAX
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_XOR_RR, masm_x86_reg(MASM_X86_RDX, 4), masm_x86_reg(MASM_X86_RDX, 4))); // Zero EDX
        }
    }
    else
    {
        // 64-bit: Dividend is RDX:RAX
        if (is_signed)
        {
            emit_inst(sec, masm_x86_inst_0(MASM_OP_X86_CQO)); // RAX -> RDX:RAX
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_XOR_RR, masm_x86_reg(MASM_X86_RDX, 8), masm_x86_reg(MASM_X86_RDX, 8))); // Zero RDX
        }
    }

    // Load b -> RCX
    load_operand(sec, ctx, b, MASM_X86_RCX, 0);

    // DIV/IDIV RCX
    emit_inst(sec, masm_x86_inst_1(is_signed ? MASM_OP_X86_IDIV : MASM_OP_X86_DIV, masm_x86_reg(MASM_X86_RCX, size)));

    // Result in RAX (quotient) or RDX (remainder)
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        if (size == 1)
        {
            if (is_rem)
            {
                // Remainder is in AH. Shift right to get it into AL.
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_SHR_RI, masm_x86_reg(MASM_X86_RAX, 2), masm_operand_imm(8)));
                store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, 1);
            }
            else
            {
                // Quotient in AL
                store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, 1);
            }
        }
        else
        {
            MasmX86Reg res_reg = is_rem ? MASM_X86_RDX : MASM_X86_RAX;
            store_vreg(sec, ctx, dst.reg.id, res_reg, dst.reg.size);
        }
    }
}
// Translation Handlers
// -----------------------------------------------------------------------------

static void emit_binary_op_explicit(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, MasmIrOpcode op)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a   = inst->operands[1];
    MasmOperand b   = inst->operands[2];

    uint8_t size = 8;
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        size = dst.reg.size;
    }
    else if (a.kind == MASM_OPERAND_REGISTER)
    {
        size = a.reg.size;
    }

    // Load a -> RAX
    load_operand(sec, ctx, a, MASM_X86_RAX, 0);
    MasmOperand rax = masm_x86_reg(MASM_X86_RAX, size);

    if (b.kind == MASM_OPERAND_IMM)
    {
        uint32_t opcode = 0;
        switch (op)
        {
        case MASM_IR_ADD:
            opcode = MASM_OP_X86_ADD_RI;
            break;
        case MASM_IR_SUB:
            opcode = MASM_OP_X86_SUB_RI;
            break;
        case MASM_IR_MUL:
            opcode = MASM_OP_X86_IMUL_RRI;
            break; // IMUL RAX, RAX, IMM
        case MASM_IR_AND:
            opcode = MASM_OP_X86_AND_RI;
            break;
        case MASM_IR_OR:
            opcode = MASM_OP_X86_OR_RI;
            break;
        case MASM_IR_XOR:
            opcode = MASM_OP_X86_XOR_RI;
            break;
        default:
            break;
        }

        if (op == MASM_IR_MUL)
        {
            // IMUL 3-operand form: IMUL dst, src, imm
            // Here: IMUL RAX, RAX, imm
            emit_inst(sec, masm_x86_inst_3(opcode, rax, rax, b));
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(opcode, rax, b));
        }
    }
    else if (b.kind == MASM_OPERAND_REGISTER)
    {
        // If vreg, we might want to check if it's already in a physical reg?
        // load_operand puts it in a phys reg if needed.
        // But for explicit op selection, we can use ADD_RM if b is a vreg (memory).

        if (b.reg.id >= VREG_START)
        {
            // Memory operand (vreg stack slot)
            int32_t     off = get_vreg_offset(ctx, b.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, b.reg.size);

            uint32_t opcode = 0;
            switch (op)
            {
            case MASM_IR_ADD:
                opcode = MASM_OP_X86_ADD_RM;
                break;
            case MASM_IR_SUB:
                opcode = MASM_OP_X86_SUB_RM;
                break;
            case MASM_IR_MUL:
                opcode = MASM_OP_X86_IMUL_RM;
                break;
            case MASM_IR_AND:
                opcode = MASM_OP_X86_AND_RM;
                break;
            case MASM_IR_OR:
                opcode = MASM_OP_X86_OR_RM;
                break;
            case MASM_IR_XOR:
                opcode = MASM_OP_X86_XOR_RM;
                break;
            default:
                break;
            }
            emit_inst(sec, masm_x86_inst_2(opcode, rax, mem));
        }
        else
        {
            // Physical register
            uint32_t opcode = 0;
            switch (op)
            {
            case MASM_IR_ADD:
                opcode = MASM_OP_X86_ADD_RR;
                break;
            case MASM_IR_SUB:
                opcode = MASM_OP_X86_SUB_RR;
                break;
            case MASM_IR_MUL:
                opcode = MASM_OP_X86_IMUL_RR;
                break;
            case MASM_IR_AND:
                opcode = MASM_OP_X86_AND_RR;
                break;
            case MASM_IR_OR:
                opcode = MASM_OP_X86_OR_RR;
                break;
            case MASM_IR_XOR:
                opcode = MASM_OP_X86_XOR_RR;
                break;
            default:
                break;
            }
            MasmOperand src = masm_x86_reg(b.reg.id, b.reg.size);
            emit_inst(sec, masm_x86_inst_2(opcode, rax, src));
        }
    }
    else if (b.kind == MASM_OPERAND_MEMORY)
    {
        // Explicit memory operand
        // Need to resolve base if it's a vreg
        MasmOperand mem = b;
        if (mem.mem.base.id >= VREG_START)
        {
            load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_R11, 0);
            mem.mem.base.id = MASM_X86_R11;
        }

        uint32_t opcode = 0;
        switch (op)
        {
        case MASM_IR_ADD:
            opcode = MASM_OP_X86_ADD_RM;
            break;
        case MASM_IR_SUB:
            opcode = MASM_OP_X86_SUB_RM;
            break;
        case MASM_IR_MUL:
            opcode = MASM_OP_X86_IMUL_RM;
            break;
        case MASM_IR_AND:
            opcode = MASM_OP_X86_AND_RM;
            break;
        case MASM_IR_OR:
            opcode = MASM_OP_X86_OR_RM;
            break;
        case MASM_IR_XOR:
            opcode = MASM_OP_X86_XOR_RM;
            break;
        default:
            break;
        }
        emit_inst(sec, masm_x86_inst_2(opcode, rax, mem));
    }

    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_shift_explicit(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, MasmIrOpcode op)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a   = inst->operands[1];
    MasmOperand b   = inst->operands[2];

    uint8_t size = 8;
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        size = dst.reg.size;
    }
    else if (a.kind == MASM_OPERAND_REGISTER)
    {
        size = a.reg.size;
    }

    // Load a -> RAX
    load_operand(sec, ctx, a, MASM_X86_RAX, 0);

    // Prepare count in RCX or check for immediate
    bool use_imm = false;
    if (b.kind == MASM_OPERAND_IMM)
    {
        use_imm = true;
    }
    else
    {
        // Load count -> RCX
        load_operand(sec, ctx, b, MASM_X86_RCX, 0);
    }

    uint32_t opcode = 0;
    if (use_imm)
    {
        switch (op)
        {
        case MASM_IR_SHL:
            opcode = MASM_OP_X86_SHL_RI;
            break;
        case MASM_IR_SHR:
            opcode = MASM_OP_X86_SHR_RI;
            break;
        case MASM_IR_SAR:
            opcode = MASM_OP_X86_SAR_RI;
            break;
        default:
            break;
        }
        emit_inst(sec, masm_x86_inst_2(opcode, masm_x86_reg(MASM_X86_RAX, size), b));
    }
    else
    {
        switch (op)
        {
        case MASM_IR_SHL:
            opcode = MASM_OP_X86_SHL_RC;
            break;
        case MASM_IR_SHR:
            opcode = MASM_OP_X86_SHR_RC;
            break;
        case MASM_IR_SAR:
            opcode = MASM_OP_X86_SAR_RC;
            break;
        default:
            break;
        }
        emit_inst(sec, masm_x86_inst_2(opcode, masm_x86_reg(MASM_X86_RAX, size), masm_x86_reg(MASM_X86_RCX, 1)));
    }

    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_unary_op_explicit(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, MasmIrOpcode op)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand src = inst->operands[1];

    uint8_t size = 8;
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        size = dst.reg.size;
    }
    else if (src.kind == MASM_OPERAND_REGISTER)
    {
        size = src.reg.size;
    }

    // Load src -> RAX
    load_operand(sec, ctx, src, MASM_X86_RAX, 0);
    MasmOperand rax = masm_x86_reg(MASM_X86_RAX, size);

    uint32_t opcode = 0;
    if (op == MASM_IR_NEG)
    {
        opcode = MASM_OP_X86_NEG_R;
    }
    else if (op == MASM_IR_NOT)
    {
        opcode = MASM_OP_X86_NOT_R;
    }

    emit_inst(sec, masm_x86_inst_1(opcode, rax));

    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_float_op(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, uint32_t x86_opcode)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a   = inst->operands[1];
    MasmOperand b   = inst->operands[2];

    // Load a -> XMM0
    if (a.kind == MASM_OPERAND_REGISTER)
    {
        int32_t     off = get_vreg_offset(ctx, a.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), mem));
    }
    if (a.kind == MASM_OPERAND_IMM)
    {
        // Load imm -> XMM0 via stack/scratch
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), a));
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), masm_x86_reg(MASM_X86_RAX, 8)));
    }

    // Load b -> XMM1
    if (b.kind == MASM_OPERAND_REGISTER)
    {
        int32_t     off = get_vreg_offset(ctx, b.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(1, 16), mem));
    }
    if (b.kind == MASM_OPERAND_IMM)
    {
        // Load imm -> XMM1 via stack/scratch
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), b));
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(1, 16), masm_x86_reg(MASM_X86_RAX, 8)));
    }

    // Op XMM0, XMM1
    emit_inst(sec, masm_x86_inst_2(x86_opcode, masm_operand_register(0, 16), masm_operand_register(1, 16)));

    // Store XMM0 -> dst
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        int32_t     off = get_vreg_offset(ctx, dst.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, mem, masm_operand_register(0, 16)));
    }
}

static void emit_float_cmp(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    MasmOperand dst  = inst->operands[0];
    MasmOperand a    = inst->operands[1];
    MasmOperand b    = inst->operands[2];
    int64_t     cond = inst->operands[3].imm;

    // Load a -> XMM0
    if (a.kind == MASM_OPERAND_REGISTER)
    {
        int32_t     off = get_vreg_offset(ctx, a.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), mem));
    }
    else if (a.kind == MASM_OPERAND_IMM)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), a));
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), masm_x86_reg(MASM_X86_RAX, 8)));
    }

    // Load b -> XMM1
    if (b.kind == MASM_OPERAND_REGISTER)
    {
        int32_t     off = get_vreg_offset(ctx, b.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(1, 16), mem));
    }
    else if (b.kind == MASM_OPERAND_IMM)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), b));
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(1, 16), masm_x86_reg(MASM_X86_RAX, 8)));
    }

    // UCOMISD XMM0, XMM1
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_UCOMISD, masm_operand_register(0, 16), masm_operand_register(1, 16)));

    MasmX86Opcode setcc_op;
    switch (cond)
    {
    case MASM_IR_FCMP_EQ:
        setcc_op = MASM_OP_X86_SETE;
        break;
    case MASM_IR_FCMP_NE:
        setcc_op = MASM_OP_X86_SETNE;
        break;
    case MASM_IR_FCMP_LT:
        setcc_op = MASM_OP_X86_SETB;
        break;
    case MASM_IR_FCMP_LE:
        setcc_op = MASM_OP_X86_SETBE;
        break;
    case MASM_IR_FCMP_GT:
        setcc_op = MASM_OP_X86_SETA;
        break;
    case MASM_IR_FCMP_GE:
        setcc_op = MASM_OP_X86_SETAE;
        break;
    default:
        setcc_op = MASM_OP_X86_SETE;
        break;
    }

    // SetCC -> AL
    emit_inst(sec, masm_x86_inst_1(setcc_op, masm_x86_reg(MASM_X86_RAX, 1)));

    // MOVZX RAX, AL
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVZX_RR, masm_x86_reg(MASM_X86_RAX, 8), masm_x86_reg(MASM_X86_RAX, 1)));

    // Store -> dst
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_mov(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand src = inst->operands[1];

    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        MasmX86Reg scratch = MASM_X86_R11;
        if (dst.reg.id < VREG_START)
        {
            scratch = (MasmX86Reg)dst.reg.id;
        }

        // Load src -> scratch
        load_operand(sec, ctx, src, scratch, 0);
        // Store scratch -> dst
        store_vreg(sec, ctx, dst.reg.id, scratch, dst.reg.size);
    }
    else if (dst.kind == MASM_OPERAND_MEMORY)
    {
        // Store to memory

        // Optimize: MOV [mem], imm (if fits in 32-bit)
        if (src.kind == MASM_OPERAND_IMM)
        {
            bool fits = true;
            if (dst.mem.size == 8 && (src.imm < -2147483648LL || src.imm > 2147483647LL))
            {
                fits = false;
            }

            if (fits)
            {
                MasmOperand mem = dst;
                if (mem.mem.base.id >= VREG_START)
                {
                    load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_RCX, 0);
                    mem.mem.base.id = MASM_X86_RCX;
                }
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_MI, mem, src));
                return;
            }
        }

        // Load src -> RAX
        load_operand(sec, ctx, src, MASM_X86_RAX, 0);

        // Resolve dst address -> RCX (if needed)
        MasmOperand mem = dst;
        if (mem.mem.base.id >= VREG_START)
        {
            load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_RCX, 0);
            mem.mem.base.id = MASM_X86_RCX;
        }

        // MOV [mem], RAX
        MasmOperand r = masm_x86_reg(MASM_X86_RAX, mem.mem.size);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_MR, mem, r));
    }
}

static void emit_load(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // LOAD dst, mem, type
    MasmOperand dst     = inst->operands[0];
    MasmOperand mem     = inst->operands[1];
    MasmOperand type_op = inst->operands[2]; // MASM_OPERAND_TYPE

    // Determine opcode and load size based on type
    uint32_t opcode    = MASM_OP_X86_MOV_RM;
    uint8_t  load_size = 8; // default to 64-bit
    if (type_op.type == MASM_TYPE_I8 || type_op.type == MASM_TYPE_I16)
    {
        opcode = MASM_OP_X86_MOVSX_RM;
    }
    else if (type_op.type == MASM_TYPE_U8 || type_op.type == MASM_TYPE_U16)
    {
        opcode = MASM_OP_X86_MOVZX_RM;
    }
    else if (type_op.type == MASM_TYPE_I32 || type_op.type == MASM_TYPE_U32)
    {
        load_size = 4; // 32-bit MOV zero-extends on x86_64
    }

    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        MasmX86Reg scratch = MASM_X86_R11;
        if (dst.reg.id < VREG_START)
        {
            scratch = (MasmX86Reg)dst.reg.id;
        }

        // Resolve mem base
        if (mem.mem.base.id >= VREG_START)
        {
            load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_RCX, 0);
            mem.mem.base.id = MASM_X86_RCX;
        }

        MasmOperand r = masm_x86_reg(scratch, load_size); // dest reg with proper size

        // MOV reg, [mem] - for 32-bit loads, this auto zero-extends to 64-bit on x86_64
        emit_inst(sec, masm_x86_inst_2(opcode, r, mem));

        // Store reg -> dst
        store_vreg(sec, ctx, dst.reg.id, scratch, dst.reg.size);
    }
}

static void emit_zext(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // ZEXT dst, src - zero-extend src to dst size
    MasmOperand dst = inst->operands[0];
    MasmOperand src = inst->operands[1];

    uint8_t src_size = 8;
    if (src.kind == MASM_OPERAND_REGISTER)
    {
        src_size = src.reg.size;
    }

    // Load source into RAX with appropriate extension
    if (src.kind == MASM_OPERAND_REGISTER && src.reg.id >= VREG_START)
    {
        int32_t     off = get_vreg_offset(ctx, src.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, src_size);

        if (src_size == 1 || src_size == 2)
        {
            // Use MOVZX for 8/16-bit sources
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVZX_RM, masm_x86_reg(MASM_X86_RAX, 8), mem));
        }
        else if (src_size == 4)
        {
            // 32-bit MOV auto zero-extends to 64-bit on x86_64
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RM, masm_x86_reg(MASM_X86_RAX, 4), mem));
        }
        else
        {
            // 64-bit, just load
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RM, masm_x86_reg(MASM_X86_RAX, 8), mem));
        }
    }
    else if (src.kind == MASM_OPERAND_REGISTER)
    {
        // Physical register
        if (src_size == 1 || src_size == 2)
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVZX_RR, masm_x86_reg(MASM_X86_RAX, 8), masm_operand_register(src.reg.id, src_size)));
        }
        else if (src_size == 4)
        {
            // 32-bit MOV auto zero-extends
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, masm_x86_reg(MASM_X86_RAX, 4), masm_operand_register(src.reg.id, 4)));
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, masm_x86_reg(MASM_X86_RAX, 8), masm_operand_register(src.reg.id, 8)));
        }
    }
    else if (src.kind == MASM_OPERAND_IMM)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), src));
    }

    // Store to destination
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_sext(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // SEXT dst, src - sign-extend src to dst size
    MasmOperand dst = inst->operands[0];
    MasmOperand src = inst->operands[1];

    uint8_t src_size = 8;
    if (src.kind == MASM_OPERAND_REGISTER)
    {
        src_size = src.reg.size;
    }

    // Load source into RAX with sign extension
    if (src.kind == MASM_OPERAND_REGISTER && src.reg.id >= VREG_START)
    {
        int32_t     off = get_vreg_offset(ctx, src.reg.id);
        MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, src_size);

        if (src_size == 1 || src_size == 2)
        {
            // Use MOVSX for 8/16-bit sources
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVSX_RM, masm_x86_reg(MASM_X86_RAX, 8), mem));
        }
        else if (src_size == 4)
        {
            // MOVSXD for 32-bit to 64-bit sign extension
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVSXD, masm_x86_reg(MASM_X86_RAX, 8), mem));
        }
        else
        {
            // 64-bit, just load
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RM, masm_x86_reg(MASM_X86_RAX, 8), mem));
        }
    }
    else if (src.kind == MASM_OPERAND_REGISTER)
    {
        // Physical register
        if (src_size == 1 || src_size == 2)
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVSX_RR, masm_x86_reg(MASM_X86_RAX, 8), masm_operand_register(src.reg.id, src_size)));
        }
        else if (src_size == 4)
        {
            // MOVSXD for 32-bit to 64-bit - need to load to memory first for RR form
            // Actually use the RR variant of MOVSXD
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVSXD, masm_x86_reg(MASM_X86_RAX, 8), masm_operand_register(src.reg.id, 4)));
        }
        else
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, masm_x86_reg(MASM_X86_RAX, 8), masm_operand_register(src.reg.id, 8)));
        }
    }
    else if (src.kind == MASM_OPERAND_IMM)
    {
        // For immediates, sign extension happens at load time with MOV_RI
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), src));
    }

    // Store to destination
    if (dst.kind == MASM_OPERAND_REGISTER)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_store(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // STORE mem, src, size
    MasmOperand mem   = inst->operands[0];
    MasmOperand src   = inst->operands[1];
    MasmOperand sz_op = inst->operands[2];
    uint8_t     size  = (uint8_t)sz_op.imm;

    if (src.kind == MASM_OPERAND_REGISTER && src.reg.class == MASM_REG_CLASS_FLOAT)
    {
        MasmOperand xmm_src = masm_operand_register(0, 16); // Default to XMM0

        if (src.reg.id >= VREG_START)
        {
            // Load vreg -> XMM0
            int32_t     off     = get_vreg_offset(ctx, src.reg.id);
            MasmOperand src_mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, xmm_src, src_mem));
        }
        else
        {
            // Physical register
            xmm_src = masm_operand_register(src.reg.id, 16);
        }

        // Resolve mem base
        if (mem.mem.base.id >= VREG_START)
        {
            load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_RCX, 0);
            mem.mem.base.id = MASM_X86_RCX;
        }

        // Store XMM -> [mem]
        mem.mem.size = size;
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, mem, xmm_src));
        return;
    }

    // Load src -> RAX
    load_operand(sec, ctx, src, MASM_X86_RAX, 0);

    // Resolve mem base
    if (mem.mem.base.id >= VREG_START)
    {
        load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_RCX, 0);
        mem.mem.base.id = MASM_X86_RCX;
    }

    // MOV [mem], RAX
    MasmOperand r = masm_x86_reg(MASM_X86_RAX, size);
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_MR, mem, r));
}

static void emit_lea(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand src = inst->operands[1];

    // Load effective address of src -> RAX
    load_operand(sec, ctx, src, MASM_X86_RAX, 1); // mode=1 for LEA

    // Store RAX -> dst
    store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
}

static void emit_stack_frame(MasmSection *sec, CodeGenContext *ctx)
{
    // PROLOGUE
    // push rbp
    emit_inst(sec, masm_x86_inst_1(MASM_OP_X86_PUSH_R, masm_x86_reg(MASM_X86_RBP, 8)));
    // mov rbp, rsp
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, masm_x86_reg(MASM_X86_RBP, 8), masm_x86_reg(MASM_X86_RSP, 8)));

    // sub rsp, total_size
    if (ctx->total_stack_size > 0)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_SUB_RI, masm_x86_reg(MASM_X86_RSP, 8), masm_operand_imm(ctx->total_stack_size)));
    }
}

static void emit_ret(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // Optional: Load return value to RAX/XMM0 if present
    if (inst->operand_count > 0 && inst->operands[0].kind != MASM_OPERAND_NONE)
    {
        MasmOperand ret_op   = inst->operands[0];
        bool        is_float = (ret_op.kind == MASM_OPERAND_REGISTER) && (ret_op.reg.class == MASM_REG_CLASS_FLOAT);

        if (is_float)
        {
            // Load ret_op -> XMM0
            if (ret_op.kind == MASM_OPERAND_REGISTER)
            {
                int32_t     off = get_vreg_offset(ctx, ret_op.reg.id);
                MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
                emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), mem));
            }
        }
        else
        {
            load_operand(sec, ctx, ret_op, MASM_X86_RAX, 0);
        }
    }

    // EPILOGUE
    // mov rsp, rbp
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RR, masm_x86_reg(MASM_X86_RSP, 8), masm_x86_reg(MASM_X86_RBP, 8)));
    // pop rbp
    emit_inst(sec, masm_x86_inst_1(MASM_OP_X86_POP_R, masm_x86_reg(MASM_X86_RBP, 8)));
    // ret
    emit_inst(sec, masm_x86_inst_0(MASM_OP_X86_RET));
}

static void emit_call(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // CALL dest, target, args...
    MasmOperand dst       = inst->operands[0];
    MasmOperand tgt       = inst->operands[1];
    int         arg_count = inst->operand_count - 2;

    MasmX86Reg int_regs[] = {MASM_X86_RDI, MASM_X86_RSI, MASM_X86_RDX, MASM_X86_RCX, MASM_X86_R8, MASM_X86_R9};
    int        int_count  = 6;
    int        xmm_count  = 8;

    struct ArgLoc
    {
        bool is_stack;
        bool is_float;
        int  reg_idx;
    };

    struct ArgLoc *locs        = malloc(sizeof(struct ArgLoc) * arg_count);
    int            int_used    = 0;
    int            xmm_used    = 0;
    int            stack_count = 0;

    // Classify args
    for (int i = 0; i < arg_count; i++)
    {
        MasmOperand op       = inst->operands[2 + i];
        bool        is_float = (op.kind == MASM_OPERAND_REGISTER) && (op.reg.class == MASM_REG_CLASS_FLOAT);
        locs[i].is_float     = is_float;

        if (is_float)
        {
            if (xmm_used < xmm_count)
            {
                locs[i].is_stack = false;
                locs[i].reg_idx  = xmm_used++;
            }
            else
            {
                locs[i].is_stack = true;
                stack_count++;
            }
        }
        else
        {
            if (int_used < int_count)
            {
                locs[i].is_stack = false;
                locs[i].reg_idx  = int_used++;
            }
            else
            {
                locs[i].is_stack = true;
                stack_count++;
            }
        }
    }

    // Align stack
    int pad = 0;
    if (stack_count > 0 && (stack_count % 2 != 0))
    {
        pad = 8;
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_SUB_RI, masm_x86_reg(MASM_X86_RSP, 8), masm_operand_imm(8)));
    }

    // Push stack args (reverse order)
    for (int i = arg_count - 1; i >= 0; i--)
    {
        if (locs[i].is_stack)
        {
            MasmOperand op = inst->operands[2 + i];
            load_operand(sec, ctx, op, MASM_X86_RAX, 0);
            emit_inst(sec, masm_x86_inst_1(MASM_OP_X86_PUSH_R, masm_x86_reg(MASM_X86_RAX, 8)));
        }
    }

    // Load register args
    for (int i = 0; i < arg_count; i++)
    {
        if (!locs[i].is_stack)
        {
            MasmOperand op = inst->operands[2 + i];

            if (locs[i].is_float)
            {
                MasmOperand xmm_dest = masm_operand_register(locs[i].reg_idx, 16);
                if (op.kind == MASM_OPERAND_REGISTER)
                {
                    int32_t     off = get_vreg_offset(ctx, op.reg.id);
                    MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
                    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, xmm_dest, mem));
                }
            }
            else
            {
                load_operand(sec, ctx, op, int_regs[locs[i].reg_idx], 0);
            }
        }
    }

    // Set AL = vector regs count (for varargs)
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 1), masm_operand_imm(xmm_used > 8 ? 8 : xmm_used)));

    // Call
    if (tgt.kind == MASM_OPERAND_REGISTER || tgt.kind == MASM_OPERAND_MEMORY)
    {
        load_operand(sec, ctx, tgt, MASM_X86_RAX, 0);
        emit_inst(sec, masm_x86_inst_1(MASM_OP_X86_CALL_RM, masm_x86_reg(MASM_X86_RAX, 8)));
    }
    else
    {
        emit_inst(sec, masm_x86_inst_1(MASM_OP_X86_CALL_REL, tgt));
    }

    // Restore stack
    int total_stack_bytes = (stack_count * 8) + pad;
    if (total_stack_bytes > 0)
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_ADD_RI, masm_x86_reg(MASM_X86_RSP, 8), masm_operand_imm(total_stack_bytes)));
    }

    // Store result RAX/XMM0 -> dst
    if (dst.kind != MASM_OPERAND_NONE)
    {
        bool is_float = (dst.kind == MASM_OPERAND_REGISTER) && (dst.reg.class == MASM_REG_CLASS_FLOAT);

        if (is_float)
        {
            // Store XMM0 -> dst
            int32_t     off = get_vreg_offset(ctx, dst.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, mem, masm_operand_register(0, 16)));
        }
        else
        {
            store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
        }
    }

    free(locs);
}

static void emit_syscall(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // SYSCALL dest, num, args...
    // Linux x86_64: RAX=sys_num, RDI, RSI, RDX, R10, R8, R9
    // inst operands: dest, target(unused?), args... ?
    // Wait, lower_call emits: dest, target(label "syscall"), args...
    // Actually lower_call for syscall emits: MASM_IR_SYSCALL dest, target, args...
    // But syscall number is usually an argument?
    // In `lower.c`, `lower_call` puts args in order.
    // If target is "syscall", it's a generic call.
    // The syscall number is usually the first argument in Mach/MASM convention for portable syscalls?
    // Or does `lower_call` handle it?
    // `lower.c` `lower_call` does NOT treat syscall specially regarding arguments, just opcode.
    // So operands[2] is first arg (syscall number?).
    // Let's assume: syscall(num, arg1, arg2...)
    // So arg[0] -> RAX, arg[1] -> RDI, etc.

    MasmOperand dst       = (inst->operand_count > 0) ? inst->operands[0] : masm_operand_none();
    int         arg_count = (inst->operand_count >= 2) ? inst->operand_count - 2 : 0;

    // Map args to Linux syscall ABI
    // Arg 0 (Syscall Num) -> RAX
    // Arg 1 -> RDI
    // Arg 2 -> RSI
    // Arg 3 -> RDX
    // Arg 4 -> R10 (NOT RCX)
    // Arg 5 -> R8
    // Arg 6 -> R9

    MasmX86Reg sys_regs[] = {MASM_X86_RAX, MASM_X86_RDI, MASM_X86_RSI, MASM_X86_RDX, MASM_X86_R10, MASM_X86_R8, MASM_X86_R9};

    for (int i = 0; i < arg_count; i++)
    {
        if (i < 7)
        {
            load_operand(sec, ctx, inst->operands[2 + i], sys_regs[i], 0);
        }
    }

    emit_inst(sec, masm_x86_inst_0(MASM_OP_X86_SYSCALL));

    if (dst.kind != MASM_OPERAND_NONE)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_fconv(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    MasmOperand dst  = inst->operands[0];
    MasmOperand src  = inst->operands[1];
    int64_t     mode = inst->operands[2].imm;

    if (mode == 0) // int -> float
    {
        // Load integer src -> RAX
        load_operand(sec, ctx, src, MASM_X86_RAX, 0);

        uint32_t opcode   = (dst.reg.size == 4) ? MASM_OP_X86_CVTSI2SS : MASM_OP_X86_CVTSI2SD;
        uint8_t  src_size = (src.kind == MASM_OPERAND_REGISTER) ? src.reg.size : 8;

        // cvtsi2sx xmm0, rax
        emit_inst(sec, masm_x86_inst_2(opcode, masm_operand_register(0, 16), masm_x86_reg(MASM_X86_RAX, src_size)));

        // Store float result -> dst
        if (dst.kind == MASM_OPERAND_REGISTER)
        {
            int32_t     off = get_vreg_offset(ctx, dst.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, mem, masm_operand_register(0, 16)));
        }
    }
    else if (mode == 1) // float -> int
    {
        // Load float src -> XMM0
        if (src.kind == MASM_OPERAND_REGISTER)
        {
            int32_t     off = get_vreg_offset(ctx, src.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), mem));
        }
        else if (src.kind == MASM_OPERAND_IMM)
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), src));
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), masm_x86_reg(MASM_X86_RAX, 8)));
        }

        uint8_t  src_size = (src.kind == MASM_OPERAND_REGISTER) ? src.reg.size : 8;
        uint32_t opcode   = (src_size == 4) ? MASM_OP_X86_CVTTSS2SI : MASM_OP_X86_CVTTSD2SI;

        // cvttsx2si rax, xmm0
        emit_inst(sec, masm_x86_inst_2(opcode, masm_x86_reg(MASM_X86_RAX, dst.reg.size), masm_operand_register(0, 16)));

        // Store integer result -> dst
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
    else // float -> float
    {
        // Load float src -> XMM0
        if (src.kind == MASM_OPERAND_REGISTER)
        {
            int32_t     off = get_vreg_offset(ctx, src.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), mem));
        }
        else if (src.kind == MASM_OPERAND_IMM)
        {
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOV_RI, masm_x86_reg(MASM_X86_RAX, 8), src));
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, masm_operand_register(0, 16), masm_x86_reg(MASM_X86_RAX, 8)));
        }

        uint8_t  src_size = (src.kind == MASM_OPERAND_REGISTER) ? src.reg.size : 8;
        uint32_t opcode   = 0;
        if (src_size == 4 && dst.reg.size == 8)
        {
            opcode = MASM_OP_X86_CVTSS2SD;
        }
        else if (src_size == 8 && dst.reg.size == 4)
        {
            opcode = MASM_OP_X86_CVTSD2SS;
        }

        if (opcode != 0)
        {
            emit_inst(sec, masm_x86_inst_2(opcode, masm_operand_register(0, 16), masm_operand_register(0, 16)));
        }

        // Store float result -> dst
        if (dst.kind == MASM_OPERAND_REGISTER)
        {
            int32_t     off = get_vreg_offset(ctx, dst.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVQ, mem, masm_operand_register(0, 16)));
        }
    }
}

static void emit_cmp_branch(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, uint32_t jcc_op)
{
    MasmOperand a     = inst->operands[0];
    MasmOperand b     = inst->operands[1];
    MasmOperand label = inst->operands[2];

    uint8_t size = 8;
    if (a.kind == MASM_OPERAND_REGISTER)
    {
        size = a.reg.size;
    }
    else if (b.kind == MASM_OPERAND_REGISTER)
    {
        size = b.reg.size;
    }

    load_operand(sec, ctx, a, MASM_X86_RAX, 0);

    if (b.kind == MASM_OPERAND_IMM && (b.imm >= -2147483648LL && b.imm <= 2147483647LL))
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_CMP_RI, masm_x86_reg(MASM_X86_RAX, size), b));
    }
    else
    {
        load_operand(sec, ctx, b, MASM_X86_RCX, 0);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_CMP_RR, masm_x86_reg(MASM_X86_RAX, size), masm_x86_reg(MASM_X86_RCX, size)));
    }
    emit_inst(sec, masm_x86_inst_1(jcc_op, label));
}

static void emit_setcc(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, uint32_t setcc_op)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a   = inst->operands[1];
    MasmOperand b   = inst->operands[2];

    uint8_t size = 8;
    if (a.kind == MASM_OPERAND_REGISTER)
    {
        size = a.reg.size;
    }
    else if (b.kind == MASM_OPERAND_REGISTER)
    {
        size = b.reg.size;
    }

    load_operand(sec, ctx, a, MASM_X86_RAX, 0);

    if (b.kind == MASM_OPERAND_IMM && (b.imm >= -2147483648LL && b.imm <= 2147483647LL))
    {
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_CMP_RI, masm_x86_reg(MASM_X86_RAX, size), b));
    }
    else
    {
        load_operand(sec, ctx, b, MASM_X86_RCX, 0);
        emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_CMP_RR, masm_x86_reg(MASM_X86_RAX, size), masm_x86_reg(MASM_X86_RCX, size)));
    }

    MasmOperand al = masm_x86_reg(MASM_X86_RAX, 1);
    emit_inst(sec, masm_x86_inst_1(setcc_op, al));
    emit_inst(sec, masm_x86_inst_2(MASM_OP_X86_MOVZX_RR, masm_x86_reg(MASM_X86_RAX, 8), al)); // zero extend byte to 64

    store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
}

// -----------------------------------------------------------------------------
// Main Codegen Loop
// -----------------------------------------------------------------------------

static void x86_64_codegen(Masm *masm)
{
    CodeGenContext ctx;
    ctx_init(&ctx);

    for (size_t i = 0; i < masm->section_count; ++i)
    {
        MasmSection *section = masm->sections[i];
        if (section->kind != MASM_SECTION_TEXT)
        {
            continue;
        }

#ifdef MASM_DEBUG
        fprintf(stderr, "[codegen] processing section %s (%zu insts)\n", section->name, section->inst_count);
#endif

        MasmSection *out = masm_section_create(MASM_SECTION_TEXT, section->name);

        for (size_t j = 0; j < section->inst_count; j++)
        {
            MasmInstruction *inst = &section->instructions[j];

#ifdef MASM_DEBUG
            fprintf(stderr, "[codegen] inst %zu: opcode=%d, ops=%d\n", j, inst->opcode, inst->operand_count);
#endif

            if (inst->kind == MASM_OPCODE_IR && inst->opcode == MASM_IR_STACK_FRAME)
            {
                ctx_reset(&ctx);

                // Scan function body until next STACK_FRAME or end of section
                size_t end = j + 1;
                while (end < section->inst_count && !(section->instructions[end].kind == MASM_OPCODE_IR && section->instructions[end].opcode == MASM_IR_STACK_FRAME))
                {
                    end++;
                }
                scan_function(&ctx, &section->instructions[j], end - j);

                emit_stack_frame(out, &ctx);
            }
            else if (inst->kind == MASM_OPCODE_IR && inst->opcode == MASM_IR_LABEL)
            {
                // Labels must be preserved
                masm_section_append_inst(out, masm_inst_create(MASM_OPCODE_IR, MASM_IR_LABEL, inst->operands, inst->operand_count));
            }
            else if (inst->kind == MASM_OPCODE_IR)
            {
                switch (inst->opcode)
                {
                case MASM_IR_MOV:
                    emit_mov(out, &ctx, inst);
                    break;
                case MASM_IR_LOAD:
                    emit_load(out, &ctx, inst);
                    break;
                case MASM_IR_STORE:
                    emit_store(out, &ctx, inst);
                    break;
                case MASM_IR_LEA:
                    emit_lea(out, &ctx, inst);
                    break;

                case MASM_IR_ADD:
                    emit_binary_op_explicit(out, &ctx, inst, MASM_IR_ADD);
                    break;
                case MASM_IR_SUB:
                    emit_binary_op_explicit(out, &ctx, inst, MASM_IR_SUB);
                    break;
                case MASM_IR_MUL:
                    emit_binary_op_explicit(out, &ctx, inst, MASM_IR_MUL);
                    break;
                case MASM_IR_AND:
                    emit_binary_op_explicit(out, &ctx, inst, MASM_IR_AND);
                    break;
                case MASM_IR_OR:
                    emit_binary_op_explicit(out, &ctx, inst, MASM_IR_OR);
                    break;
                case MASM_IR_XOR:
                    emit_binary_op_explicit(out, &ctx, inst, MASM_IR_XOR);
                    break;

                case MASM_IR_DIV:
                    emit_div_rem(out, &ctx, inst, true, false);
                    break;
                case MASM_IR_DIVU:
                    emit_div_rem(out, &ctx, inst, false, false);
                    break;
                case MASM_IR_REM:
                    emit_div_rem(out, &ctx, inst, true, true);
                    break;
                case MASM_IR_REMU:
                    emit_div_rem(out, &ctx, inst, false, true);
                    break;

                case MASM_IR_NEG:
                    emit_unary_op_explicit(out, &ctx, inst, MASM_IR_NEG);
                    break;
                case MASM_IR_NOT:
                    emit_unary_op_explicit(out, &ctx, inst, MASM_IR_NOT);
                    break;

                case MASM_IR_ZEXT:
                    emit_zext(out, &ctx, inst);
                    break;
                case MASM_IR_SEXT:
                    emit_sext(out, &ctx, inst);
                    break;

                case MASM_IR_SHL:
                    emit_shift_explicit(out, &ctx, inst, MASM_IR_SHL);
                    break;
                case MASM_IR_SHR:
                    emit_shift_explicit(out, &ctx, inst, MASM_IR_SHR);
                    break;
                case MASM_IR_SAR:
                    emit_shift_explicit(out, &ctx, inst, MASM_IR_SAR);
                    break;

                case MASM_IR_FADD:
                    emit_float_op(out, &ctx, inst, MASM_OP_X86_ADDSD);
                    break;
                case MASM_IR_FSUB:
                    emit_float_op(out, &ctx, inst, MASM_OP_X86_SUBSD);
                    break;
                case MASM_IR_FMUL:
                    emit_float_op(out, &ctx, inst, MASM_OP_X86_MULSD);
                    break;
                case MASM_IR_FDIV:
                    emit_float_op(out, &ctx, inst, MASM_OP_X86_DIVSD);
                    break;
                case MASM_IR_FCMP:
                    emit_float_cmp(out, &ctx, inst);
                    break;
                case MASM_IR_FCONV:
                    emit_fconv(out, &ctx, inst);
                    break;

                case MASM_IR_SEQ:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETE);
                    break;
                case MASM_IR_SNE:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETNE);
                    break;
                case MASM_IR_SLT:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETL);
                    break;
                case MASM_IR_SGT:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETG);
                    break;
                case MASM_IR_SLE:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETLE);
                    break;
                case MASM_IR_SGE:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETGE);
                    break;
                case MASM_IR_SLTU:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETB);
                    break;
                case MASM_IR_SGTU:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETA);
                    break;
                case MASM_IR_SLEU:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETBE);
                    break;
                case MASM_IR_SGEU:
                    emit_setcc(out, &ctx, inst, MASM_OP_X86_SETAE);
                    break;

                case MASM_IR_BEQ:
                    emit_cmp_branch(out, &ctx, inst, MASM_OP_X86_JE);
                    break;
                case MASM_IR_BNE:
                    emit_cmp_branch(out, &ctx, inst, MASM_OP_X86_JNE);
                    break;
                case MASM_IR_BLT:
                    emit_cmp_branch(out, &ctx, inst, MASM_OP_X86_JL);
                    break;
                case MASM_IR_BGE:
                    emit_cmp_branch(out, &ctx, inst, MASM_OP_X86_JGE);
                    break;

                case MASM_IR_JMP:
                    emit_inst(out, masm_x86_inst_1(MASM_OP_X86_JMP_REL, inst->operands[0]));
                    break;
                case MASM_IR_RET:
                    emit_ret(out, &ctx, inst);
                    break;
                case MASM_IR_CALL:
                    emit_call(out, &ctx, inst);
                    break;
                case MASM_IR_SYSCALL:
                    emit_syscall(out, &ctx, inst);
                    break;

                default:
                    // Unknown IR opcode - preserve as-is
                    masm_section_append_inst(out, masm_inst_create(inst->kind, inst->opcode, inst->operands, inst->operand_count));
                    break;
                }
            }
            else
            {
                // Non-IR instruction (e.g. from inline assembly) - preserve as-is
                masm_section_append_inst(out, masm_inst_create(inst->kind, inst->opcode, inst->operands, inst->operand_count));
            }
        }

        // Replace section instructions
        for (size_t k = 0; k < section->inst_count; ++k)
        {
            masm_inst_destroy(section->instructions[k]);
        }
        free(section->instructions);
        section->instructions  = out->instructions;
        section->inst_count    = out->inst_count;
        section->inst_capacity = out->inst_capacity;

        // Prevent double free
        out->instructions = NULL;
        out->inst_count   = 0;
        masm_section_destroy(out);

        // Update symbol offsets
        size_t offset = 0;
        for (size_t k = 0; k < section->inst_count; ++k)
        {
            MasmInstruction *inst = &section->instructions[k];
            if (inst->kind == MASM_OPCODE_IR && inst->opcode == MASM_IR_LABEL)
            {
                if (inst->operand_count > 0 && inst->operands[0].kind == MASM_OPERAND_LABEL)
                {
                    MasmSymbol *sym = masm_get_symbol(masm, inst->operands[0].label);
                    if (sym)
                    {
                        sym->offset = offset;
                    }
                }
            }
            else
            {
                offset += masm_x86_encode(*inst, NULL, 0);
            }
        }
        section->data_size = offset;
    }

    ctx_reset(&ctx);
}

void masm_x86_isel(Masm *masm)
{
    x86_64_codegen(masm);
}
