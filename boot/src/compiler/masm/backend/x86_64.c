#include "compiler/masm/backend.h"
#include "compiler/masm/ir.h"
#include "compiler/masm/isa/x86_64.h"
#include "compiler/masm/section.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Helper Macros & Structs
// -----------------------------------------------------------------------------

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
    ctx->vreg_offsets    = NULL;
    ctx->vreg_capacity   = 0;
    ctx->max_vreg_index  = 0;
    ctx->base_stack_size = 0;
    ctx->total_stack_size = 0;
}

static void ctx_reset(CodeGenContext *ctx)
{
    if (ctx->vreg_offsets) free(ctx->vreg_offsets);
    ctx->vreg_offsets    = NULL;
    ctx->vreg_capacity   = 0;
    ctx->max_vreg_index  = 0;
    ctx->base_stack_size = 0;
    ctx->total_stack_size = 0;
}

// Assign stack slot to vreg if not already assigned
static int32_t get_vreg_offset(CodeGenContext *ctx, uint32_t vreg_id)
{
    if (vreg_id < VREG_START) return 0; // Should not happen for vreg lookup
    
    size_t idx = vreg_id - VREG_START;
    if (idx >= ctx->vreg_capacity)
    {
        size_t new_cap = (idx + 1) * 2;
        if (new_cap < 64) new_cap = 64;
        ctx->vreg_offsets = realloc(ctx->vreg_offsets, new_cap * sizeof(int32_t));
        // Initialize new slots to 0 (unassigned)
        for (size_t i = ctx->vreg_capacity; i < new_cap; i++) ctx->vreg_offsets[i] = 0;
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
    bool found = false;
    
    for (size_t i = 0; i < count; i++)
    {
        MasmInstruction *inst = &insts[i];
        if (inst->opcode == MASM_IR_STACK_FRAME)
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
                    found = true;
                }
            }
            if (op->kind == MASM_OPERAND_MEMORY && op->mem.base.id >= VREG_START)
            {
                size_t idx = op->mem.base.id - VREG_START;
                if (!found || idx > ctx->max_vreg_index)
                {
                    ctx->max_vreg_index = idx;
                    found = true;
                }
            }
             if (op->kind == MASM_OPERAND_MEMORY && op->mem.index.id >= VREG_START)
            {
                size_t idx = op->mem.index.id - VREG_START;
                if (!found || idx > ctx->max_vreg_index)
                {
                    ctx->max_vreg_index = idx;
                    found = true;
                }
            }
        }
    }
    
    // Calculate total stack size
    // base + (max_index + 1) * 8
    // align to 16
    uint32_t vreg_space = found ? (uint32_t)((ctx->max_vreg_index + 1) * 8) : 0;
    ctx->total_stack_size = ctx->base_stack_size + vreg_space;
    if (ctx->total_stack_size % 16 != 0)
    {
        ctx->total_stack_size += (16 - (ctx->total_stack_size % 16));
    }
}

// -----------------------------------------------------------------------------
// Emitter Helpers
// -----------------------------------------------------------------------------

static void emit_bytes(MasmSection *section, void *data, size_t len)
{
    masm_section_append_data(section, data, len);
}

static void emit_inst(MasmSection *section, MasmInstruction inst)
{
    uint8_t buf[15];
    int len = masm_x86_encode(inst, buf, sizeof(buf));
    if (len > 0)
    {
        emit_bytes(section, buf, len);
    }
    masm_inst_destroy(inst);
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
            int32_t off = get_vreg_offset(ctx, op.reg.id);
            MasmOperand mem = masm_operand_memory_simple(MASM_X86_RBP, -off, op.reg.size);
            
            // Sign-extend if loading small value into 64-bit reg? 
            // For now, use MOVZX for small, MOV for 32/64
            uint32_t opcode = MASM_OP_MOV;
            if (op.reg.size == 1 || op.reg.size == 2) opcode = MASM_OP_MOVZX;
            
            // Adjust dest size to match source?
            // If we are loading into a 64-bit scratch, we usually want zero extension for small values.
            emit_inst(sec, masm_inst_2(opcode, dest, mem));
        }
        else
        {
            // Physical register
            if (op.reg.id != (uint32_t)dest_reg)
            {
                MasmOperand src = masm_operand_register(op.reg.id, 8); // assume full width copy
                emit_inst(sec, masm_inst_2(MASM_OP_MOV, dest, src));
            }
        }
    }
    else if (op.kind == MASM_OPERAND_IMM)
    {
        emit_inst(sec, masm_inst_2(MASM_OP_MOV, dest, op));
    }
    else if (op.kind == MASM_OPERAND_MEMORY)
    {
        // Resolve memory operand (which might use vreg base)
        MasmOperand mem = op;
        if (mem.mem.base.id >= VREG_START)
        {
            // Load base vreg into RCX (scratch)
            // Wait, we can't clobber RCX if dest_reg is RCX. 
            // Caller should provide safe dest.
            // But here we need a temp reg for address calculation.
            // Let's use R11 as address scratch.
            int32_t off = get_vreg_offset(ctx, mem.mem.base.id);
            MasmOperand base_slot = masm_operand_memory_simple(MASM_X86_RBP, -off, 8);
            emit_inst(sec, masm_inst_2(MASM_OP_MOV, masm_x86_reg(MASM_X86_R11, 8), base_slot));
            mem.mem.base.id = MASM_X86_R11;
        }
        
        // If mode == 1 (LEA), we want the address
        if (mode == 1)
        {
            emit_inst(sec, masm_inst_2(MASM_OP_LEA, dest, mem));
        }
        else
        {
            // Load value
            uint32_t opcode = MASM_OP_MOV;
            if (mem.mem.size == 1 || mem.mem.size == 2) opcode = MASM_OP_MOVZX;
            emit_inst(sec, masm_inst_2(opcode, dest, mem));
        }
    }
    else if (op.kind == MASM_OPERAND_LABEL || op.kind == MASM_OPERAND_SYMBOL)
    {
         // Load address of label
         // In x86-64, this is usually LEA reg, [rip + label] or MOV reg, imm64
         // masm_x86_encode handles label as immediate or disp.
         // Let's use MOV reg, label (immediate)
         // Or LEA reg, [label]
         // masm encoder usually treats MOV reg, label as loading the address.
         emit_inst(sec, masm_inst_2(MASM_OP_MOV, dest, op));
    }
}

// Store physical register to vreg slot
static void store_vreg(MasmSection *sec, CodeGenContext *ctx, uint32_t vreg_id, MasmX86Reg src_reg, uint8_t size)
{
    if (vreg_id < VREG_START) return; // Physical destination, assumed already there
    
    int32_t off = get_vreg_offset(ctx, vreg_id);
    MasmOperand slot = masm_operand_memory_simple(MASM_X86_RBP, -off, size);
    MasmOperand src = masm_x86_reg(src_reg, size); // store partial reg if size < 8
    
    emit_inst(sec, masm_inst_2(MASM_OP_MOV, slot, src));
}

// -----------------------------------------------------------------------------
// Translation Handlers
// -----------------------------------------------------------------------------

static void emit_binary_op(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, uint32_t x86_opcode)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a   = inst->operands[1];
    MasmOperand b   = inst->operands[2];
    
    // Load a -> RAX
    load_operand(sec, ctx, a, MASM_X86_RAX, 0);
    // Load b -> RCX
    load_operand(sec, ctx, b, MASM_X86_RCX, 0);
    
    // OP RAX, RCX
    emit_inst(sec, masm_inst_2(x86_opcode, masm_x86_reg(MASM_X86_RAX, 8), masm_x86_reg(MASM_X86_RCX, 8)));
    
    // Store RAX -> dst
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
        // Load src -> RAX
        load_operand(sec, ctx, src, MASM_X86_RAX, 0);
        // Store RAX -> dst
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
    else if (dst.kind == MASM_OPERAND_MEMORY)
    {
        // Store to memory
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
        emit_inst(sec, masm_inst_2(MASM_OP_MOV, mem, r));
    }
}

static void emit_load(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // LOAD dst, mem, type
    MasmOperand dst = inst->operands[0];
    MasmOperand mem = inst->operands[1];
    MasmOperand type_op = inst->operands[2]; // MASM_OPERAND_TYPE
    
    // Determine opcode based on type
    uint32_t opcode = MASM_OP_MOV;
    if (type_op.type == MASM_TYPE_I8 || type_op.type == MASM_TYPE_I16) opcode = MASM_OP_MOVSX;
    else if (type_op.type == MASM_TYPE_U8 || type_op.type == MASM_TYPE_U16) opcode = MASM_OP_MOVZX;
    
    // Resolve mem base
    if (mem.mem.base.id >= VREG_START)
    {
        load_operand(sec, ctx, masm_operand_register(mem.mem.base.id, 8), MASM_X86_RCX, 0);
        mem.mem.base.id = MASM_X86_RCX;
    }
    
    MasmOperand r = masm_x86_reg(MASM_X86_RAX, 8); // dest reg
    
    // MOV RAX, [mem]
    emit_inst(sec, masm_inst_2(opcode, r, mem));
    
    // Store RAX -> dst
    store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
}

static void emit_store(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // STORE mem, src, size
    MasmOperand mem = inst->operands[0];
    MasmOperand src = inst->operands[1];
    MasmOperand sz_op = inst->operands[2];
    uint8_t size = (uint8_t)sz_op.imm;
    
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
    emit_inst(sec, masm_inst_2(MASM_OP_MOV, mem, r));
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
    emit_inst(sec, masm_inst_1(MASM_OP_PUSH, masm_x86_reg(MASM_X86_RBP, 8)));
    // mov rbp, rsp
    emit_inst(sec, masm_inst_2(MASM_OP_MOV, masm_x86_reg(MASM_X86_RBP, 8), masm_x86_reg(MASM_X86_RSP, 8)));
    
    // sub rsp, total_size
    if (ctx->total_stack_size > 0)
    {
        emit_inst(sec, masm_inst_2(MASM_OP_SUB, masm_x86_reg(MASM_X86_RSP, 8), masm_operand_imm(ctx->total_stack_size)));
    }
}

static void emit_ret(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // Optional: Load return value to RAX if present
    if (inst->operand_count > 0 && inst->operands[0].kind != MASM_OPERAND_NONE)
    {
        load_operand(sec, ctx, inst->operands[0], MASM_X86_RAX, 0);
    }

    // EPILOGUE
    // mov rsp, rbp
    emit_inst(sec, masm_inst_2(MASM_OP_MOV, masm_x86_reg(MASM_X86_RSP, 8), masm_x86_reg(MASM_X86_RBP, 8)));
    // pop rbp
    emit_inst(sec, masm_inst_1(MASM_OP_POP, masm_x86_reg(MASM_X86_RBP, 8)));
    // ret
    emit_inst(sec, masm_inst_0(MASM_OP_RET));
}

static void emit_call(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst)
{
    // CALL dest, target, args...
    MasmOperand dst = inst->operands[0];
    MasmOperand tgt = inst->operands[1];
    int arg_count = inst->operand_count - 2;
    
    // SysV ABI Regs: RDI, RSI, RDX, RCX, R8, R9
    MasmX86Reg arg_regs[] = {MASM_X86_RDI, MASM_X86_RSI, MASM_X86_RDX, MASM_X86_RCX, MASM_X86_R8, MASM_X86_R9};
    
    // Stack alignment for call
    // RSP must be 16-byte aligned before CALL (which pushes return address 8 bytes).
    // Current RSP is aligned to 16 bytes (assuming stack frame setup correctly).
    // Pushing args changes alignment.
    // If we have stack args, we need to ensure (stack_arg_count * 8) % 16 == 0?
    // No, we need (RSP_after_pushes) % 16 == 0.
    // Since RSP_start % 16 == 0, if stack_arg_count is odd, we are misaligned.
    // So if stack_arg_count % 2 != 0, we push padding.
    
    int stack_args = arg_count - 6;
    if (stack_args > 0 && (stack_args % 2 != 0))
    {
        emit_inst(sec, masm_inst_2(MASM_OP_SUB, masm_x86_reg(MASM_X86_RSP, 8), masm_operand_imm(8)));
    }

    // Load args
    for (int i = 0; i < arg_count; i++)
    {
        if (i < 6)
        {
            load_operand(sec, ctx, inst->operands[2 + i], arg_regs[i], 0);
        }
    }
    
    // Push stack args in reverse order
    for (int i = arg_count - 1; i >= 6; i--)
    {
        load_operand(sec, ctx, inst->operands[2 + i], MASM_X86_RAX, 0);
        emit_inst(sec, masm_inst_1(MASM_OP_PUSH, masm_x86_reg(MASM_X86_RAX, 8)));
    }
    
    // Load target -> RAX if not label
    if (tgt.kind == MASM_OPERAND_REGISTER || tgt.kind == MASM_OPERAND_MEMORY)
    {
        load_operand(sec, ctx, tgt, MASM_X86_RAX, 0);
        emit_inst(sec, masm_inst_1(MASM_OP_CALL, masm_x86_reg(MASM_X86_RAX, 8)));
    }
    else
    {
        emit_inst(sec, masm_inst_1(MASM_OP_CALL, tgt));
    }
    
    // Store result RAX -> dst
    if (dst.kind != MASM_OPERAND_NONE)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
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
    
    MasmOperand dst = (inst->operand_count > 0) ? inst->operands[0] : masm_operand_none();
    int arg_count = (inst->operand_count >= 2) ? inst->operand_count - 2 : 0;
    
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
    
    emit_inst(sec, masm_inst_0(MASM_OP_X86_SYSCALL));
    
    if (dst.kind != MASM_OPERAND_NONE)
    {
        store_vreg(sec, ctx, dst.reg.id, MASM_X86_RAX, dst.reg.size);
    }
}

static void emit_cmp_branch(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, uint32_t jcc_op)
{
    MasmOperand a = inst->operands[0];
    MasmOperand b = inst->operands[1];
    MasmOperand label = inst->operands[2];
    
    load_operand(sec, ctx, a, MASM_X86_RAX, 0);
    load_operand(sec, ctx, b, MASM_X86_RCX, 0);
    
    emit_inst(sec, masm_inst_2(MASM_OP_CMP, masm_x86_reg(MASM_X86_RAX, 8), masm_x86_reg(MASM_X86_RCX, 8)));
    emit_inst(sec, masm_inst_1(jcc_op, label));
}

static void emit_setcc(MasmSection *sec, CodeGenContext *ctx, MasmInstruction *inst, uint32_t setcc_op)
{
    MasmOperand dst = inst->operands[0];
    MasmOperand a = inst->operands[1];
    MasmOperand b = inst->operands[2];
    
    load_operand(sec, ctx, a, MASM_X86_RAX, 0);
    load_operand(sec, ctx, b, MASM_X86_RCX, 0);
    
    emit_inst(sec, masm_inst_2(MASM_OP_CMP, masm_x86_reg(MASM_X86_RAX, 8), masm_x86_reg(MASM_X86_RCX, 8)));
    
    MasmOperand al = masm_x86_reg(MASM_X86_RAX, 1);
    emit_inst(sec, masm_inst_1(setcc_op, al));
    emit_inst(sec, masm_inst_2(MASM_OP_MOVZX, masm_x86_reg(MASM_X86_RAX, 8), al)); // zero extend byte to 64
    
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
        if (section->kind != MASM_SECTION_TEXT) continue;

        // Naive function boundary detection:
        // We assume functions start with LABEL and have STACK_FRAME near start.
        // We scan linearly. If we see STACK_FRAME, we reset context and scan ahead.
        // Or we can just process instructions one by one, but we need to know STACK_FRAME size before emitting.
        // Actually, scan_function logic relies on seeing the whole function.
        // Let's identify ranges.
        

        
        for (size_t j = 0; j < section->inst_count; j++)
        {
            MasmInstruction *inst = &section->instructions[j];
            
            if (inst->opcode == MASM_IR_STACK_FRAME)
            {
                // Found a function body start. Scan from func_start (label) or here?
                // Scan from here until next label or end.
                size_t end = j + 1;
                while (end < section->inst_count && section->instructions[end].opcode != MASM_IR_LABEL)
                {
                    end++;
                }
                
                // Scan this range
                scan_function(&ctx, &section->instructions[j], end - j);
                
                // Now emit for this range (starting from current inst, which is STACK_FRAME)
                // Wait, we need to emit the LABEL that came before STACK_FRAME too?
                // The loop iterates j. The label was emitted in previous iterations.
                // But the context was not ready then?
                // Labels don't use vregs/stack, so they are fine to emit without context.
                
                emit_stack_frame(section, &ctx);
            }
            else if (inst->opcode == MASM_IR_LABEL)
            {
                // Emit label
                // Labels are pseudo-ops in encode? x86_encode doesn't handle LABEL op, 
                // but the Section logic handles label binding if we add symbols.
                // Actually `masm_x86_encode` returns 0 for LABEL.
                // We just need to ensure the offset is recorded.
                // `masm_section_append_inst` does not encode immediately.
                // `masm_x86_encode` writes bytes.
                // Labels are usually handled by `emit_object` resolving symbols to current data offset.
                // We need to record that this label is at current section->data_size.
                
                // Find symbol for this label name and update offset
                if (inst->operand_count > 0 && inst->operands[0].kind == MASM_OPERAND_LABEL)
                {
                    MasmSymbol *sym = masm_get_symbol(masm, inst->operands[0].label);
                    if (sym)
                    {
                        sym->offset = section->data_size;
                        // If it's a function symbol, we might want to align?
                    }
                }
                
                // Update func_start
            }
            else
            {
                // Emit instruction
                switch (inst->opcode)
                {
                    case MASM_IR_MOV: emit_mov(section, &ctx, inst); break;
                    case MASM_IR_LOAD: emit_load(section, &ctx, inst); break;
                    case MASM_IR_STORE: emit_store(section, &ctx, inst); break;
                    case MASM_IR_LEA: emit_lea(section, &ctx, inst); break;
                    
                    case MASM_IR_ADD: emit_binary_op(section, &ctx, inst, MASM_OP_ADD); break;
                    case MASM_IR_SUB: emit_binary_op(section, &ctx, inst, MASM_OP_SUB); break;
                    case MASM_IR_MUL: emit_binary_op(section, &ctx, inst, MASM_OP_IMUL); break; // signed mul
                    case MASM_IR_AND: emit_binary_op(section, &ctx, inst, MASM_OP_AND); break;
                    case MASM_IR_OR:  emit_binary_op(section, &ctx, inst, MASM_OP_OR); break;
                    case MASM_IR_XOR: emit_binary_op(section, &ctx, inst, MASM_OP_XOR); break;
                    
                    case MASM_IR_SEQ: emit_setcc(section, &ctx, inst, MASM_OP_SETE); break;
                    case MASM_IR_SNE: emit_setcc(section, &ctx, inst, MASM_OP_SETNE); break;
                    case MASM_IR_SLT: emit_setcc(section, &ctx, inst, MASM_OP_SETL); break;
                    case MASM_IR_SGT: emit_setcc(section, &ctx, inst, MASM_OP_SETG); break;
                    case MASM_IR_SLE: emit_setcc(section, &ctx, inst, MASM_OP_SETLE); break;
                    case MASM_IR_SGE: emit_setcc(section, &ctx, inst, MASM_OP_SETGE); break;
                    
                    case MASM_IR_BEQ: emit_cmp_branch(section, &ctx, inst, MASM_OP_JE); break;
                    case MASM_IR_BNE: emit_cmp_branch(section, &ctx, inst, MASM_OP_JNE); break;
                    case MASM_IR_BLT: emit_cmp_branch(section, &ctx, inst, MASM_OP_JL); break;
                    case MASM_IR_BGE: emit_cmp_branch(section, &ctx, inst, MASM_OP_JGE); break;
                    
                    case MASM_IR_JMP: emit_inst(section, masm_inst_1(MASM_OP_JMP, inst->operands[0])); break;
                    case MASM_IR_RET: emit_ret(section, &ctx, inst); break;
                    case MASM_IR_CALL: emit_call(section, &ctx, inst); break;
                    case MASM_IR_SYSCALL: emit_syscall(section, &ctx, inst); break;
                    
                    // Unhandled
                    default: break;
                }
            }
        }
    }
    
    ctx_reset(&ctx);
}

const MasmBackend masm_backend_x86_64 = {
    .name = "x86_64",
    .codegen = x86_64_codegen
};
