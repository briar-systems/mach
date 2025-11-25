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

    ctx->relocations = NULL;

    ctx->block_offsets.items = NULL;
    ctx->block_offsets.count = 0;
    ctx->block_offsets.capacity = 0;

    ctx->pending_jumps = NULL;

    ctx->current_function = NULL;

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

    // free relocations
    X86_64_Relocation *reloc = ctx->relocations;
    while (reloc)
    {
        X86_64_Relocation *next = reloc->next;
        free(reloc->symbol_name);
        free(reloc);
        reloc = next;
    }

    if (ctx->block_offsets.items)
    {
        free(ctx->block_offsets.items);
    }

    PendingJump *jump = ctx->pending_jumps;
    while (jump)
    {
        PendingJump *next = jump->next;
        free(jump);
        jump = next;
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

static void emit_dword(X86_64_CodegenContext *ctx, uint32_t value)
{
    emit_byte(ctx, value & 0xFF);
    emit_byte(ctx, (value >> 8) & 0xFF);
    emit_byte(ctx, (value >> 16) & 0xFF);
    emit_byte(ctx, (value >> 24) & 0xFF);
}

static void add_relocation(X86_64_CodegenContext *ctx, uint64_t offset, const char *symbol_name, int type)
{
    X86_64_Relocation *reloc = malloc(sizeof(X86_64_Relocation));
    if (!reloc)
    {
        return;
    }

    reloc->offset = offset;
    reloc->symbol_name = strdup(symbol_name);
    reloc->type = type;
    reloc->next = ctx->relocations;
    reloc->next = ctx->relocations;
    ctx->relocations = reloc;
}

static void register_block_offset(X86_64_CodegenContext *ctx, MIRBlock *block)
{
    if (ctx->block_offsets.count >= ctx->block_offsets.capacity)
    {
        size_t new_cap = ctx->block_offsets.capacity == 0 ? 16 : ctx->block_offsets.capacity * 2;
        void *new_items = realloc(ctx->block_offsets.items, new_cap * sizeof(BlockOffset));
        if (!new_items) return;
        ctx->block_offsets.items = new_items;
        ctx->block_offsets.capacity = new_cap;
    }
    ctx->block_offsets.items[ctx->block_offsets.count].block = block;
    ctx->block_offsets.items[ctx->block_offsets.count].offset = ctx->code.size;
    ctx->block_offsets.count++;
}

static size_t get_block_offset(X86_64_CodegenContext *ctx, MIRBlock *block)
{
    for (size_t i = 0; i < ctx->block_offsets.count; i++)
    {
        if (ctx->block_offsets.items[i].block == block)
        {
            return ctx->block_offsets.items[i].offset;
        }
    }
    return (size_t)-1;
}

static void add_pending_jump(X86_64_CodegenContext *ctx, MIRBlock *target, size_t disp_offset)
{
    PendingJump *jump = malloc(sizeof(PendingJump));
    jump->target = target;
    jump->disp_offset = disp_offset;
    jump->next = ctx->pending_jumps;
    ctx->pending_jumps = jump;
}

static void resolve_jumps(X86_64_CodegenContext *ctx)
{
    PendingJump *jump = ctx->pending_jumps;
    while (jump)
    {
        size_t target_offset = get_block_offset(ctx, jump->target);
        if (target_offset != (size_t)-1)
        {
            // calculate relative offset
            // jump instruction is usually opcode + 4 bytes displacement
            // relative offset = target - (disp_offset + 4)
            int32_t rel = (int32_t)(target_offset - (jump->disp_offset + 4));
            
            // patch code
            ctx->code.data[jump->disp_offset] = rel & 0xFF;
            ctx->code.data[jump->disp_offset + 1] = (rel >> 8) & 0xFF;
            ctx->code.data[jump->disp_offset + 2] = (rel >> 16) & 0xFF;
            ctx->code.data[jump->disp_offset + 3] = (rel >> 24) & 0xFF;
        }
        jump = jump->next;
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

static void emit_sub_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // sub dst, src: REX.W + 29 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x29);
    
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_imul_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // imul dst, src: REX.W + 0F AF /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0xAF);
    
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_and_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // and dst, src: REX.W + 21 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x21);
    
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_or_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // or dst, src: REX.W + 09 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x09);
    
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_xor_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // xor dst, src: REX.W + 31 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x31);
    
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_cmp_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // cmp dst, src: REX.W + 39 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x39);
    
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_setcc(X86_64_CodegenContext *ctx, int cond, X86_64_Reg dst)
{
    // xor dst, dst (clear register)
    emit_xor_reg_reg(ctx, dst, dst);
    
    // setcc dst (low byte)
    // 0F 9x /0
    
    // We need REX if dst is one of SPL, BPL, SIL, DIL (indices 4-7) or R8-R15
    uint8_t rex = 0;
    if (dst >= X86_64_R8) rex |= 0x41; // REX + B
    else if (dst == X86_64_RSP || dst == X86_64_RBP || dst == X86_64_RSI || dst == X86_64_RDI) rex |= 0x40; // REX
    
    if (rex) emit_byte(ctx, rex);
    
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x90 | cond);
    
    uint8_t modrm = 0xC0 | (0 << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_jmp(X86_64_CodegenContext *ctx, MIRBlock *target)
{
    emit_byte(ctx, 0xE9); // JMP rel32
    size_t disp_offset = ctx->code.size;
    emit_dword(ctx, 0); // placeholder
    add_pending_jump(ctx, target, disp_offset);
}

static void emit_jcc(X86_64_CodegenContext *ctx, int cond, MIRBlock *target)
{
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x80 | cond); // Jcc rel32
    size_t disp_offset = ctx->code.size;
    emit_dword(ctx, 0); // placeholder
    add_pending_jump(ctx, target, disp_offset);
}

static void emit_test_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // test dst, src: REX.W + 85 /r
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8) rex |= 0x01; // REX.B
    if (src >= X86_64_R8) rex |= 0x04; // REX.R
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x85);
    
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

    case MIR_OP_MOV:
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                emit_mov_reg_reg(ctx, dst, src);
            }
            else if (inst->operands[0].kind == MIR_OPERAND_GLOBAL)
            {
                // LEA dst, [rip + symbol]
                uint8_t rex = 0x48; // REX.W
                if (dst >= X86_64_R8) rex |= 0x04; // REX.R
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x8D); // LEA
                // ModR/M: mod=00, reg=dst, rm=101 (RIP-relative)
                emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                uint64_t reloc_offset = ctx->code.size;
                emit_dword(ctx, 0);   // displacement filled by relocation
                add_relocation(ctx, reloc_offset, inst->operands[0].global_name, 2); // R_X86_64_PC32
            }
            else if (inst->operands[0].kind == MIR_OPERAND_IMM_INT)
            {
                emit_mov_reg_imm64(ctx, dst, inst->operands[0].imm_int);
            }
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
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_add_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_SUB:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_sub_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_MUL:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_imul_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_AND:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_and_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_OR:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_or_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_XOR:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                emit_xor_reg_reg(ctx, dst, src2);
            }
        }
        break;

    case MIR_OP_EQ:
    case MIR_OP_NE:
    case MIR_OP_LT:
    case MIR_OP_LE:
    case MIR_OP_GT:
    case MIR_OP_GE:
    case MIR_OP_ULT:
    case MIR_OP_ULE:
    case MIR_OP_UGT:
    case MIR_OP_UGE:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id);
                
                emit_cmp_reg_reg(ctx, src1, src2); // cmp src1, src2
                
                int cond = 0;
                switch (inst->op) {
                    case MIR_OP_EQ: cond = 0x4; break; // Z
                    case MIR_OP_NE: cond = 0x5; break; // NZ
                    case MIR_OP_LT: cond = 0xC; break; // L
                    case MIR_OP_LE: cond = 0xE; break; // LE
                    case MIR_OP_GT: cond = 0xF; break; // G
                    case MIR_OP_GE: cond = 0xD; break; // GE
                    case MIR_OP_ULT: cond = 0x2; break; // B
                    case MIR_OP_ULE: cond = 0x6; break; // BE
                    case MIR_OP_UGT: cond = 0x7; break; // A
                    case MIR_OP_UGE: cond = 0x3; break; // AE
                    default: break;
                }
                
                emit_setcc(ctx, cond, dst);
            }
        }
        break;

    case MIR_OP_BR:
        if (inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_BLOCK)
        {
            // operand contains block_id (uint32_t)
            MIRBlock *target = NULL;
            // find block by ID
            for (MIRBlock *b = ctx->current_function->first_block; b; b = b->next)
            {
                if (b->id == inst->operands[0].block_id)
                {
                    target = b;
                    break;
                }
            }
            if (target) emit_jmp(ctx, target);
        }
        break;

    case MIR_OP_BRCOND:
        if (inst->operand_count >= 3 && 
            inst->operands[0].kind == MIR_OPERAND_VALUE &&
            inst->operands[1].kind == MIR_OPERAND_BLOCK &&
            inst->operands[2].kind == MIR_OPERAND_BLOCK)
        {
            X86_64_Reg cond_reg = get_physical_reg(ctx, inst->operands[0].value_id);
            
            // test cond, cond
            emit_test_reg_reg(ctx, cond_reg, cond_reg);
            
            // find true and false blocks
            MIRBlock *true_block = NULL;
            MIRBlock *false_block = NULL;
            for (MIRBlock *b = ctx->current_function->first_block; b; b = b->next)
            {
                if (b->id == inst->operands[1].block_id) true_block = b;
                if (b->id == inst->operands[2].block_id) false_block = b;
            }
            
            // jnz true_block
            if (true_block) emit_jcc(ctx, 0x5, true_block); // JNZ (NE)
            
            // jmp false_block
            if (false_block) emit_jmp(ctx, false_block);
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
        // syscall number in rax, args in rdi, rsi, rdx, r10, r8, r9
        {
            static const X86_64_Reg syscall_arg_regs[] = {
                X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_R10, X86_64_R8, X86_64_R9
            };
            
            // load syscall number to rax
            if (inst->operand_count > 0)
            {
                if (inst->operands[0].kind == MIR_OPERAND_IMM_INT)
                {
                    emit_mov_reg_imm64(ctx, X86_64_RAX, inst->operands[0].imm_int);
                }
                else if (inst->operands[0].kind == MIR_OPERAND_VALUE)
                {
                    X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                    emit_mov_reg_reg(ctx, X86_64_RAX, src);
                }
                else if (inst->operands[0].kind == MIR_OPERAND_GLOBAL)
                {
                    // lea rax, [rip + symbol] with relocation
                    uint64_t reloc_offset = ctx->code.size + 3;
                    emit_byte(ctx, 0x48); // REX.W
                    emit_byte(ctx, 0x8D); // LEA
                    emit_byte(ctx, 0x05); // ModR/M: [rip+disp32] -> rax
                    emit_dword(ctx, 0);   // displacement filled by relocation
                    add_relocation(ctx, reloc_offset, inst->operands[0].global_name, 2); // R_X86_64_PC32
                }
            }
            
            // load arguments to proper registers
            for (size_t i = 1; i < inst->operand_count && i <= 6; i++)
            {
                if (inst->operands[i].kind == MIR_OPERAND_IMM_INT)
                {
                    emit_mov_reg_imm64(ctx, syscall_arg_regs[i - 1], inst->operands[i].imm_int);
                }
                else if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    X86_64_Reg src = get_physical_reg(ctx, inst->operands[i].value_id);
                    emit_mov_reg_reg(ctx, syscall_arg_regs[i - 1], src);
                }
                else if (inst->operands[i].kind == MIR_OPERAND_GLOBAL)
                {
                    // lea reg, [rip + symbol] with PC-relative relocation
                    X86_64_Reg dst = syscall_arg_regs[i - 1];
                    emit_byte(ctx, 0x48); // REX.W
                    emit_byte(ctx, 0x8D); // LEA
                    emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3)); // ModR/M: [rip + disp32]
                    uint64_t reloc_offset = ctx->code.size; // relocation at displacement field
                    emit_dword(ctx, 0); // displacement filled by linker
                    add_relocation(ctx, reloc_offset, inst->operands[i].global_name, 2); // R_X86_64_PC32
                }
            }
            
            emit_syscall(ctx);
        }
        break;

    case MIR_OP_CALL:
        // function call - System V AMD64 ABI
        // args in rdi, rsi, rdx, rcx, r8, r9 (then stack)
        // return value in rax
        {
            static const X86_64_Reg call_arg_regs[] = {
                X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_RCX, X86_64_R8, X86_64_R9
            };
            
            // first operand is the function name (global)
            const char *func_name = NULL;
            size_t first_arg = 0;
            
            if (inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_GLOBAL)
            {
                func_name = inst->operands[0].global_name;
                first_arg = 1;
            }
            
            // load arguments into registers
            for (size_t i = first_arg; i < inst->operand_count && (i - first_arg) < 6; i++)
            {
                X86_64_Reg dst = call_arg_regs[i - first_arg];
                
                if (inst->operands[i].kind == MIR_OPERAND_IMM_INT)
                {
                    emit_mov_reg_imm64(ctx, dst, inst->operands[i].imm_int);
                }
                else if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    X86_64_Reg src = get_physical_reg(ctx, inst->operands[i].value_id);
                    if (src != dst)
                    {
                        emit_mov_reg_reg(ctx, dst, src);
                    }
                }
                else if (inst->operands[i].kind == MIR_OPERAND_GLOBAL)
                {
                    // lea dst, [rip + symbol]
                    emit_byte(ctx, 0x48); // REX.W
                    emit_byte(ctx, 0x8D); // LEA
                    emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                    uint64_t reloc_offset = ctx->code.size;
                    emit_dword(ctx, 0);
                    add_relocation(ctx, reloc_offset, inst->operands[i].global_name, 2);
                }
            }
            
            // emit call instruction with PC-relative relocation
            if (func_name)
            {
                emit_byte(ctx, 0xE8); // CALL rel32
                uint64_t reloc_offset = ctx->code.size;
                emit_dword(ctx, 0); // displacement filled by linker
                add_relocation(ctx, reloc_offset, func_name, 4); // R_X86_64_PLT32
            }
            
            // result is in rax - if instruction has result, move to allocated register
            if (inst->result)
            {
                X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
                if (dst != X86_64_RAX)
                {
                    emit_mov_reg_reg(ctx, dst, X86_64_RAX);
                }
            }
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

    ctx->current_function = func;

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
        register_block_offset(ctx, block);
        
        // emit all instructions in block
        for (MIRInst *inst = block->first_inst; inst; inst = inst->next)
        {
            emit_instruction(ctx, inst);
        }
    }

    // emit epilogue
    emit_byte(ctx, 0x5D); // pop rbp
    emit_ret(ctx);
    
    resolve_jumps(ctx);

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

X86_64_Relocation *x86_64_codegen_get_relocations(X86_64_CodegenContext *ctx)
{
    if (!ctx)
    {
        return NULL;
    }

    return ctx->relocations;
}
