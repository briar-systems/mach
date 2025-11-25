#include "compiler/mir/codegen/x86_64.h"
#include "compiler/mir/opcode.h"
#include <stdlib.h>
#include <string.h>

X86_64_CodegenContext *x86_64_codegen_create()
{
    X86_64_CodegenContext *ctx = malloc(sizeof(X86_64_CodegenContext));
    if (!ctx)
    {
        return NULL;
    }

    ctx->code.data = NULL;
    ctx->code.size = 0;
    ctx->code.capacity = 0;

    ctx->reg_map.map = NULL;
    ctx->reg_map.count = 0;
    ctx->reg_map.capacity = 0;

    return ctx;
}

void x86_64_codegen_destroy(X86_64_CodegenContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->code.data)
    {
        free(ctx->code.data);
    }

    if (ctx->reg_map.map)
    {
        free(ctx->reg_map.map);
    }

    free(ctx);
}

static void emit_byte(X86_64_CodegenContext *ctx, uint8_t byte)
{
    if (ctx->code.size >= ctx->code.capacity)
    {
        size_t new_capacity = ctx->code.capacity == 0 ? 256 : ctx->code.capacity * 2;
        uint8_t *new_data = realloc(ctx->code.data, new_capacity);
        if (!new_data)
        {
            return;
        }
        ctx->code.data = new_data;
        ctx->code.capacity = new_capacity;
    }

    ctx->code.data[ctx->code.size++] = byte;
}

static void emit_bytes(X86_64_CodegenContext *ctx, const uint8_t *bytes, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        emit_byte(ctx, bytes[i]);
    }
}

static X86_64_Reg get_physical_reg(X86_64_CodegenContext *ctx, uint32_t virtual_reg)
{
    if (virtual_reg >= ctx->reg_map.count)
    {
        return X86_64_RAX; // fallback
    }
    return ctx->reg_map.map[virtual_reg];
}

int x86_64_allocate_registers(X86_64_CodegenContext *ctx, MIRFunction *func)
{
    // simple allocation: assign physical registers in order
    // this is a naive implementation - real allocator would do graph coloring or linear scan
    
    // allocate map
    if (func->next_value_id > ctx->reg_map.capacity)
    {
        size_t new_capacity = func->next_value_id;
        X86_64_Reg *new_map = realloc(ctx->reg_map.map, new_capacity * sizeof(X86_64_Reg));
        if (!new_map)
        {
            return -1;
        }
        ctx->reg_map.map = new_map;
        ctx->reg_map.capacity = new_capacity;
    }

    ctx->reg_map.count = func->next_value_id;

    // assign registers (simple strategy: use rax, rcx, rdx, rbx, rsi, rdi)
    static const X86_64_Reg available_regs[] = {
        X86_64_RAX, X86_64_RCX, X86_64_RDX, X86_64_RBX, X86_64_RSI, X86_64_RDI
    };
    size_t num_available = sizeof(available_regs) / sizeof(available_regs[0]);

    for (uint32_t i = 0; i < func->next_value_id; i++)
    {
        ctx->reg_map.map[i] = available_regs[i % num_available];
    }

    return 0;
}

// map our register enum to x86 encoding
static uint8_t reg_to_x86_encoding(X86_64_Reg reg)
{
    // x86-64 register encoding: RAX=0, RCX=1, RDX=2, RBX=3, RSP=4, RBP=5, RSI=6, RDI=7
    // our enum: RAX=0, RBX=1, RCX=2, RDX=3, RSI=4, RDI=5, RBP=6, RSP=7
    static const uint8_t encoding_map[] = {
        0, // RAX -> 0
        3, // RBX -> 3
        1, // RCX -> 1
        2, // RDX -> 2
        6, // RSI -> 6
        7, // RDI -> 7
        5, // RBP -> 5
        4, // RSP -> 4
        0, 1, 2, 3, 4, 5, 6, 7 // R8-R15 (low 3 bits)
    };
    return encoding_map[reg];
}

// emit x86_64 instruction encodings
static void emit_mov_reg_imm64(X86_64_CodegenContext *ctx, X86_64_Reg dst, int64_t imm)
{
    // mov reg, imm64: REX.W + B8+r imm64
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0xB8 + reg_to_x86_encoding(dst));
    
    // emit 64-bit immediate (little endian)
    for (int i = 0; i < 8; i++)
    {
        emit_byte(ctx, (imm >> (i * 8)) & 0xFF);
    }
}

static void emit_add_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // add dst, src: REX.W + 01 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x01);
    
    // ModR/M byte: mod=11 (register), reg=src, rm=dst
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_ret(X86_64_CodegenContext *ctx)
{
    // ret: C3
    emit_byte(ctx, 0xC3);
}

static void emit_syscall(X86_64_CodegenContext *ctx)
{
    // syscall: 0F 05
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x05);
}

static void emit_mov_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // mov dst, src: REX.W + 89 /r
    if (dst == src)
    {
        return; // no-op
    }
    
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x89);
    
    // ModR/M byte: mod=11 (register), reg=src, rm=dst
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_instruction(X86_64_CodegenContext *ctx, MIRInst *inst)
{
    switch (inst->op)
    {
    case MIR_OP_CONST:
        if (inst->result && inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_IMM_INT)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            emit_mov_reg_imm64(ctx, dst, inst->operands[0].imm_int);
        }
        break;

    case MIR_OP_ADD:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                // mov dst, src1 first if needed
                if (dst != src1)
                {
                    // simplified: assume already in place
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_add_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_RET:
        // move return value to rax if needed
        if (inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_VALUE)
        {
            X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
            emit_mov_reg_reg(ctx, X86_64_RAX, src);
        }
        emit_ret(ctx);
        break;

    case MIR_OP_SYSCALL:
        // linux x86_64 syscall convention:
        // syscall number in rax
        // args in rdi, rsi, rdx, r10, r8, r9
        // for our simple case: operand[0] = syscall num, operand[1..6] = args
        {
            static const X86_64_Reg syscall_arg_regs[] = {
                X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_R10, X86_64_R8, X86_64_R9
            };
            
            // move syscall number to rax
            if (inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                emit_mov_reg_reg(ctx, X86_64_RAX, src);
            }
            
            // move arguments to proper registers
            for (size_t i = 1; i < inst->operand_count && i <= 6; i++)
            {
                if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    X86_64_Reg src = get_physical_reg(ctx, inst->operands[i].value_id);
                    emit_mov_reg_reg(ctx, syscall_arg_regs[i - 1], src);
                }
            }
            
            // emit syscall instruction
            emit_syscall(ctx);
        }
        break;

    case MIR_OP_UNREACHABLE:
        // ud2: 0F 0B (undefined instruction)
        emit_byte(ctx, 0x0F);
        emit_byte(ctx, 0x0B);
        break;

    default:
        // unsupported instruction - emit nop
        emit_byte(ctx, 0x90);
        break;
    }
}

int x86_64_emit_function(X86_64_CodegenContext *ctx, MIRFunction *func)
{
    if (!ctx || !func)
    {
        return -1;
    }

    // allocate registers
    if (x86_64_allocate_registers(ctx, func) < 0)
    {
        return -1;
    }

    // emit prologue (simplified - no stack frame for now)
    // push rbp; mov rbp, rsp
    emit_byte(ctx, 0x55); // push rbp
    emit_bytes(ctx, (uint8_t[]){0x48, 0x89, 0xE5}, 3); // mov rbp, rsp

    // emit all blocks
    for (MIRBlock *block = func->first_block; block; block = block->next)
    {
        // emit all instructions in block
        for (MIRInst *inst = block->first_inst; inst; inst = inst->next)
        {
            emit_instruction(ctx, inst);
        }
    }

    // emit epilogue
    emit_byte(ctx, 0x5D); // pop rbp
    emit_ret(ctx);

    return 0;
}

uint8_t *x86_64_codegen_get_code(X86_64_CodegenContext *ctx, size_t *size)
{
    if (!ctx || !size)
    {
        return NULL;
    }

    *size = ctx->code.size;
    return ctx->code.data;
}
