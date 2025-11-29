#include "compiler/mir/codegen/x86_64.h"
#include "compiler/mir/isa/x86_64.h"
#include <stdlib.h>
#include <string.h>

X86_64_CodegenContext *x86_64_codegen_create()
{
    X86_64_CodegenContext *ctx = malloc(sizeof(X86_64_CodegenContext));
    if (!ctx)
    {
        return NULL;
    }

    ctx->code.data     = NULL;
    ctx->code.size     = 0;
    ctx->code.capacity = 0;

    ctx->reg_map.map      = NULL;
    ctx->reg_map.count    = 0;
    ctx->reg_map.capacity = 0;

    ctx->relocations = NULL;

    ctx->block_offsets.items    = NULL;
    ctx->block_offsets.count    = 0;
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
        size_t   new_capacity = ctx->code.capacity == 0 ? 256 : ctx->code.capacity * 2;
        uint8_t *new_data     = realloc(ctx->code.data, new_capacity);
        if (!new_data)
        {
            return;
        }
        ctx->code.data     = new_data;
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

static void add_relocation(X86_64_CodegenContext *ctx, uint64_t offset, const char *symbol_name, int type, int64_t addend)
{
    X86_64_Relocation *reloc = malloc(sizeof(X86_64_Relocation));
    if (!reloc)
    {
        return;
    }

    reloc->offset      = offset;
    reloc->symbol_name = strdup(symbol_name);
    reloc->type        = type;
    reloc->addend      = addend;
    reloc->next        = ctx->relocations;
    reloc->next        = ctx->relocations;
    ctx->relocations   = reloc;
}

static void register_block_offset(X86_64_CodegenContext *ctx, MIRBlock *block)
{
    if (ctx->block_offsets.count >= ctx->block_offsets.capacity)
    {
        size_t new_cap   = ctx->block_offsets.capacity == 0 ? 16 : ctx->block_offsets.capacity * 2;
        void  *new_items = realloc(ctx->block_offsets.items, new_cap * sizeof(BlockOffset));
        if (!new_items)
        {
            return;
        }
        ctx->block_offsets.items    = new_items;
        ctx->block_offsets.capacity = new_cap;
    }
    ctx->block_offsets.items[ctx->block_offsets.count].block  = block;
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
    PendingJump *jump  = malloc(sizeof(PendingJump));
    jump->target       = target;
    jump->disp_offset  = disp_offset;
    jump->next         = ctx->pending_jumps;
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
            ctx->code.data[jump->disp_offset]     = rel & 0xFF;
            ctx->code.data[jump->disp_offset + 1] = (rel >> 8) & 0xFF;
            ctx->code.data[jump->disp_offset + 2] = (rel >> 16) & 0xFF;
            ctx->code.data[jump->disp_offset + 3] = (rel >> 24) & 0xFF;
        }
        jump = jump->next;
    }
}

// liveness information for a value
typedef struct
{
    uint32_t value_id;
    size_t   first_use; // instruction index where value is defined
    size_t   last_use;  // instruction index where value is last used
    bool     is_fp;     // whether value is floating point
} ValueLiveness;

static X86_64_Reg get_physical_reg(X86_64_CodegenContext *ctx, uint32_t virtual_reg)
{
    if (virtual_reg >= ctx->reg_map.count)
    {
        return X86_64_RAX; // fallback
    }
    return ctx->reg_map.map[virtual_reg];
}

// compute liveness information for all values in function
static ValueLiveness *compute_liveness(MIRFunction *func, size_t *liveness_count)
{
    if (!func || !liveness_count)
    {
        return NULL;
    }

    // allocate liveness array
    ValueLiveness *liveness = calloc(func->next_value_id, sizeof(ValueLiveness));
    if (!liveness)
    {
        return NULL;
    }

    // initialize liveness info
    for (uint32_t i = 0; i < func->next_value_id; i++)
    {
        liveness[i].value_id  = i;
        liveness[i].first_use = (size_t)-1;
        liveness[i].last_use  = 0;
        liveness[i].is_fp     = false;
    }

    // traverse instructions and record uses
    size_t    inst_idx = 0;
    MIRBlock *block    = func->first_block;
    while (block)
    {
        MIRInst *inst = block->first_inst;
        while (inst)
        {
            // record definition
            if (inst->result)
            {
                uint32_t id = inst->result->id;
                if (liveness[id].first_use == (size_t)-1)
                {
                    liveness[id].first_use = inst_idx;
                }
                liveness[id].last_use = inst_idx;

                // track type
                Type *type = inst->result->type;
                if (type && (type->kind == TYPE_F32 || type->kind == TYPE_F64))
                {
                    liveness[id].is_fp = true;
                }
            }

            // record uses in operands
            for (size_t i = 0; i < inst->operand_count; i++)
            {
                if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    uint32_t id = inst->operands[i].value_id;
                    if (id < func->next_value_id)
                    {
                        liveness[id].last_use = inst_idx;
                    }
                }
            }

            inst = inst->next;
            inst_idx++;
        }
        block = block->next;
    }

    *liveness_count = func->next_value_id;
    return liveness;
}

int x86_64_allocate_registers(X86_64_CodegenContext *ctx, MIRFunction *func)
{
    if (!ctx || !func)
    {
        return -1;
    }
    // ABI-aware allocation with parameter mapping
    // System V AMD64 ABI: params in RDI, RSI, RDX, RCX, R8, R9
    // Stack parameters: passed in order from right to left at [rbp + 16], [rbp + 24], ...

    // allocate map
    if (func->next_value_id > ctx->reg_map.capacity)
    {
        size_t      new_capacity = func->next_value_id;
        X86_64_Reg *new_map      = realloc(ctx->reg_map.map, new_capacity * sizeof(X86_64_Reg));
        if (!new_map)
        {
            return -1;
        }
        ctx->reg_map.map      = new_map;
        ctx->reg_map.capacity = new_capacity;
    }

    ctx->reg_map.count = func->next_value_id;

    // ABI parameter registers (System V AMD64)
    static const X86_64_Reg gp_param_regs[] = {X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_RCX, X86_64_R8, X86_64_R9};
    static const X86_64_Reg fp_param_regs[] = {X86_64_XMM0, X86_64_XMM1, X86_64_XMM2, X86_64_XMM3, X86_64_XMM4, X86_64_XMM5, X86_64_XMM6, X86_64_XMM7};

    // Allocatable GP registers (callee-saved first for better calling convention)
    X86_64_Reg allocatable_gp_regs[16];
    int        num_allocatable_gp = 0;

    allocatable_gp_regs[num_allocatable_gp++] = X86_64_RBX;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_R12;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_R13;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_R14;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_R15;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_RAX;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_R10;
    allocatable_gp_regs[num_allocatable_gp++] = X86_64_R11;

    // Allocatable FP registers (XMM8-XMM15 first as they're callee-saved in Windows but not SysV)
    // For simplicity, we use all XMM registers
    X86_64_Reg allocatable_fp_regs[16];
    int        num_allocatable_fp = 0;
    for (int i = 0; i < 16; i++)
    {
        allocatable_fp_regs[num_allocatable_fp++] = (X86_64_Reg)(X86_64_XMM0 + i);
    }

    // Step 1: Assign parameter registers and handle stack spilling
    int gp_param_idx = 0;
    int fp_param_idx = 0;

    for (size_t i = 0; i < func->param_count; i++)
    {
        if (func->params[i])
        {
            uint32_t param_id = func->params[i]->id;
            Type    *type     = func->params[i]->type;

            if (type && (type->kind == TYPE_F32 || type->kind == TYPE_F64))
            {
                if (fp_param_idx < 8)
                {
                    // Parameter in FP register
                    ctx->reg_map.map[param_id] = fp_param_regs[fp_param_idx++];
                }
                else
                {
                    // Parameter on stack - we need to load it from stack
                    // For now, allocate a temporary register
                    // The load will happen in the function prologue when proper stack parameter
                    // support is added. Currently using XMM15 as fallback.
                    ctx->reg_map.map[param_id] = X86_64_XMM15;
                }
            }
            else
            {
                if (gp_param_idx < 6)
                {
                    // Parameter in GP register
                    ctx->reg_map.map[param_id] = gp_param_regs[gp_param_idx++];
                }
                else
                {
                    // Parameter on stack - we need to load it from stack
                    // For now, allocate a temporary register
                    // The load will happen in the function prologue when proper stack parameter
                    // support is added. Currently using R11 as fallback.
                    ctx->reg_map.map[param_id] = X86_64_R11;
                }
            }
        }
    }

    // Add unused param regs to allocatable list
    for (int i = gp_param_idx; i < 6; i++)
    {
        allocatable_gp_regs[num_allocatable_gp++] = gp_param_regs[i];
    }

    // Step 2: Compute liveness information
    size_t         liveness_count = 0;
    ValueLiveness *liveness       = compute_liveness(func, &liveness_count);
    if (!liveness)
    {
        return -1;
    }

    // Step 3: Initialize all non-param values
    for (uint32_t i = 0; i < func->next_value_id; i++)
    {
        // Check if this is a parameter
        bool is_param = false;
        for (size_t j = 0; j < func->param_count; j++)
        {
            if (func->params[j] && func->params[j]->id == i)
            {
                is_param = true;
                break;
            }
        }

        if (!is_param)
        {
            // Will be assigned during allocation pass
            ctx->reg_map.map[i] = X86_64_RAX; // Default placeholder
        }
    }

    // Step 4: Linear scan register allocation with liveness
    // Sort values by their first use (already in order by ID for simple cases)

    bool *value_live = calloc(func->next_value_id, sizeof(bool));
    if (!value_live)
    {
        free(liveness);
        return -1;
    }

    // mark parameters as live since they've been assigned registers already
    for (size_t j = 0; j < func->param_count; j++)
    {
        if (func->params[j])
        {
            value_live[func->params[j]->id] = true;
        }
    }

    // Track which physical registers are currently in use
    bool gp_in_use[16] = {false};
    bool fp_in_use[16] = {false};

    // Process all instructions in order and allocate registers
    size_t    inst_idx = 0;
    MIRBlock *block    = func->first_block;
    while (block)
    {
        MIRInst *inst = block->first_inst;
        while (inst)
        {
            // Free registers for values that are no longer live
            for (uint32_t i = 0; i < func->next_value_id; i++)
            {
                if (liveness[i].last_use < inst_idx && liveness[i].first_use != (size_t)-1 && value_live[i])
                {
                    // Value is no longer live, free its register
                    X86_64_Reg reg = ctx->reg_map.map[i];
                    if (reg >= X86_64_XMM0 && reg <= X86_64_XMM15)
                    {
                        int fp_idx        = reg - X86_64_XMM0;
                        fp_in_use[fp_idx] = false;
                    }
                    else if (reg >= X86_64_RAX && reg <= X86_64_R15)
                    {
                        // Find register in allocatable list
                        for (int j = 0; j < num_allocatable_gp; j++)
                        {
                            if (allocatable_gp_regs[j] == reg)
                            {
                                gp_in_use[j] = false;
                                break;
                            }
                        }
                    }

                    value_live[i]    = false;
                }
            }

            // Allocate register for result if needed
            if (inst->result)
            {
                uint32_t id = inst->result->id;

                // Skip if already assigned (parameter)
                bool is_param = false;
                for (size_t j = 0; j < func->param_count; j++)
                {
                    if (func->params[j] && func->params[j]->id == id)
                    {
                        is_param = true;
                        break;
                    }
                }

                if (!is_param && liveness[id].first_use == inst_idx)
                {
                    // Allocate register based on type
                    if (liveness[id].is_fp)
                    {
                        // Find free FP register
                        int free_fp = -1;
                        for (int j = 0; j < num_allocatable_fp; j++)
                        {
                            if (!fp_in_use[j])
                            {
                                free_fp = j;
                                break;
                            }
                        }

                        if (free_fp >= 0)
                        {
                            ctx->reg_map.map[id] = allocatable_fp_regs[free_fp];
                            fp_in_use[free_fp]   = true;
                            value_live[id]       = true;
                        }
                        else
                        {
                            // All FP registers in use, reuse round-robin
                            ctx->reg_map.map[id] = allocatable_fp_regs[id % num_allocatable_fp];
                        }
                    }
                    else
                    {
                        // Find free GP register
                        int free_gp = -1;
                        for (int j = 0; j < num_allocatable_gp; j++)
                        {
                            if (!gp_in_use[j])
                            {
                                free_gp = j;
                                break;
                            }
                        }

                        if (free_gp >= 0)
                        {
                            ctx->reg_map.map[id] = allocatable_gp_regs[free_gp];
                            gp_in_use[free_gp]   = true;
                            value_live[id]       = true;
                        }
                        else
                        {
                            // All GP registers in use, reuse round-robin
                            ctx->reg_map.map[id] = allocatable_gp_regs[id % num_allocatable_gp];
                            value_live[id]       = true;
                        }
                    }
                }
            }

            inst = inst->next;
            inst_idx++;
        }
        block = block->next;
    }

    free(value_live);
    free(liveness);
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
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7 // R8-R15 (low 3 bits)
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
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x29);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_imul_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // imul dst, src: REX.W + 0F AF /r
    // IMUL r64, r/m64: Reg=dst, RM=src
    uint8_t rex = 0x48; // REX.W
    if (src >= X86_64_R8)
    {
        rex |= 0x01; // REX.B (src is RM)
    }
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R (dst is Reg)
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0xAF);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_imul_reg_imm(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t imm)
{
    // imul dst, dst, imm: REX.W + 69 /r imm32 (or 6B /r imm8)
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B (src is same as dst)
    }

    emit_byte(ctx, rex);

    // Check if imm fits in 8 bits
    if (imm >= -128 && imm <= 127)
    {
        emit_byte(ctx, 0x6B);
        uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(dst);
        emit_byte(ctx, modrm);
        emit_byte(ctx, (uint8_t)imm);
    }
    else
    {
        emit_byte(ctx, 0x69);
        uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(dst);
        emit_byte(ctx, modrm);
        emit_dword(ctx, (uint32_t)imm);
    }
}

static void emit_and_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // and dst, src: REX.W + 21 /r
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
    emit_byte(ctx, 0x21);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_or_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // or dst, src: REX.W + 09 /r
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
    emit_byte(ctx, 0x09);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_xor_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // xor dst, src: REX.W + 31 /r
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
    emit_byte(ctx, 0x31);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_cmp_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // cmp dst, src: REX.W + 39 /r
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
    emit_byte(ctx, 0x39);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_setcc(X86_64_CodegenContext *ctx, int cond, X86_64_Reg dst)
{
    // setcc dst (low byte)
    // 0F 9x /0

    // We need REX if dst is one of SPL, BPL, SIL, DIL (indices 4-7) or R8-R15
    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x41; // REX + B
    }
    else if (dst == X86_64_RSP || dst == X86_64_RBP || dst == X86_64_RSI || dst == X86_64_RDI)
    {
        rex |= 0x40; // REX
    }

    if (rex)
    {
        emit_byte(ctx, rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x90 | cond);

    uint8_t modrm = 0xC0 | (0 << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);

    // movzx dst, dst (byte to qword) to clear upper bytes
    // REX.W + 0F B6 /r
    rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x05; // REX.W + REX.R + REX.B (dst is both reg and rm)
    }

    emit_byte(ctx, rex);
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0xB6);

    modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(dst);
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
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x85);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

// SSE Instructions (Double Precision)

static void emit_movsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // movsd dst, src: F2 0F 10 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R (dst is Reg)
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01; // REX.B (src is RM)
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x10);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_addsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // addsd dst, src: F2 0F 58 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x58);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_subsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // subsd dst, src: F2 0F 5C /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x5C);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_mulsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // mulsd dst, src: F2 0F 59 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x59);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_divsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // divsd dst, src: F2 0F 5E /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x5E);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_cvttsd2si_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // cvttsd2si dst, src: F2 48 0F 2C /r
    // dst is r64, src is xmm
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R (dst is Reg)
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01; // REX.B (src is RM)
    }
    emit_byte(ctx, rex);

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x2C);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_cvtsi2sd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // cvtsi2sd dst, src: F2 48 0F 2A /r
    // dst is xmm, src is r64
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R (dst is Reg)
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01; // REX.B (src is RM)
    }
    emit_byte(ctx, rex);

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x2A);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_movsd_mem_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t offset)
{
    // movsd dst, [rbp + offset]: F2 0F 10 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x10);

    // ModRM: mod=10 (disp32), reg=dst, rm=101 (rbp)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
    emit_byte(ctx, modrm);
    emit_dword(ctx, (uint32_t)offset);
}

static void emit_movsd_reg_mem(X86_64_CodegenContext *ctx, int32_t offset, X86_64_Reg src)
{
    // movsd [rbp + offset], src: F2 0F 11 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (src >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x11);

    // ModRM: mod=10 (disp32), reg=src, rm=101 (rbp)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(src) << 3) | 0x05;
    emit_byte(ctx, modrm);
    emit_dword(ctx, (uint32_t)offset);
}

// Load from [rbp + offset] to register
// mov dst, [rbp + offset]
static void emit_mov_mem_to_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t offset)
{
    // Adjust offset for saved registers (5 * 8 = 40 bytes)
    offset -= 40;

    // REX.W + 8B /r [rbp + disp32]
    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }

    emit_byte(ctx, rex);
    emit_byte(ctx, 0x8B); // MOV r64, r/m64

    // ModR/M: mod=10 (disp32), reg=dst, rm=101 (RBP)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
    emit_byte(ctx, modrm);

    // 32-bit displacement (little-endian)
    emit_dword(ctx, (uint32_t)offset);
}

// Store from register to [rbp + offset]
// mov [rbp + offset], src
static void emit_mov_reg_to_mem(X86_64_CodegenContext *ctx, int32_t offset, X86_64_Reg src)
{
    // Adjust offset for saved registers (5 * 8 = 40 bytes)
    offset -= 40;

    // REX.W + 89 /r [rbp + disp32]
    uint8_t rex = 0x48; // REX.W
    if (src >= X86_64_R8)
    {
        rex |= 0x04; // REX.R
    }

    emit_byte(ctx, rex);
    emit_byte(ctx, 0x89); // MOV r/m64, r64

    // ModR/M: mod=10 (disp32), reg=src, rm=101 (RBP)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(src) << 3) | 0x05;
    emit_byte(ctx, modrm);

    // 32-bit displacement (little-endian)
    emit_dword(ctx, (uint32_t)offset);
}
static void emit_ret(X86_64_CodegenContext *ctx)
{
    // ret: C3
    emit_byte(ctx, 0xC3);
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

static void emit_add_reg_imm(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t imm)
{
    // add dst, imm
    // if imm fits in 8 bits: REX.W + 83 /0 ib
    // else: REX.W + 81 /0 id

    uint8_t rex = 0x48; // REX.W
    if (dst >= X86_64_R8)
    {
        rex |= 0x01; // REX.B
    }
    emit_byte(ctx, rex);

    if (imm >= -128 && imm <= 127)
    {
        emit_byte(ctx, 0x83);
        uint8_t modrm = 0xC0 | (0 << 3) | reg_to_x86_encoding(dst); // /0
        emit_byte(ctx, modrm);
        emit_byte(ctx, (uint8_t)imm);
    }
    else
    {
        emit_byte(ctx, 0x81);
        uint8_t modrm = 0xC0 | (0 << 3) | reg_to_x86_encoding(dst); // /0
        emit_byte(ctx, modrm);
        emit_dword(ctx, (uint32_t)imm);
    }
}

static void emit_epilogue(X86_64_CodegenContext *ctx)
{
    MIRFunction *func = ctx->current_function;

    if (func->frame_size > 0)
    {
        size_t aligned_size = (func->frame_size + 15) & ~15;
        // add rsp, aligned_size
        emit_byte(ctx, 0x48);
        emit_byte(ctx, 0x81);
        emit_byte(ctx, 0xC4); // ModRM: 11 000 100 (rsp)
        emit_dword(ctx, (uint32_t)aligned_size);
    }

    // pop r15
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5F}, 2);
    // pop r14
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5E}, 2);
    // pop r13
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5D}, 2);
    // pop r12
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5C}, 2);
    // pop rbx
    emit_byte(ctx, 0x5B);

    // leave; ret
    emit_byte(ctx, 0xC9); // leave (mov rsp, rbp; pop rbp)
    emit_ret(ctx);
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
                if (x86_64_reg_is_fp(dst) && x86_64_reg_is_fp(src))
                {
                    emit_movsd_reg_reg(ctx, dst, src);
                }
                else
                {
                    emit_mov_reg_reg(ctx, dst, src);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_GLOBAL)
            {
                // LEA dst, [rip + symbol]
                uint8_t rex = 0x48; // REX.W
                if (dst >= X86_64_R8)
                {
                    rex |= 0x04; // REX.R
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x8D); // LEA
                // ModR/M: mod=00, reg=dst, rm=101 (RIP-relative)
                emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                uint64_t reloc_offset = ctx->code.size;
                emit_dword(ctx, 0);                                                      // displacement filled by relocation
                add_relocation(ctx, reloc_offset, inst->operands[0].global_name, 2, -4); // R_X86_64_PC32 with addend -4
            }
            else if (inst->operands[0].kind == MIR_OPERAND_IMM_INT)
            {
                emit_mov_reg_imm64(ctx, dst, inst->operands[0].imm_int);
            }
        }
        break;

    case MIR_OP_LOAD:
        // Load from memory
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);

            if (inst->operands[0].kind == MIR_OPERAND_IMM_INT)
            {
                // Load from stack slot: [rbp + offset] -> register
                int32_t offset = (int32_t)inst->operands[0].imm_int;
                if (x86_64_reg_is_fp(dst))
                {
                    emit_movsd_mem_reg(ctx, dst, offset - 40); // Adjust offset manually as emit_movsd_mem_reg doesn't do it
                }
                else
                {
                    emit_mov_mem_to_reg(ctx, dst, offset);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                // Load from address in register: [src] -> dst
                // mov dst, [src]
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);

                if (x86_64_reg_is_fp(dst))
                {
                    // movsd xmm, [gp]
                    // F2 0F 10 /r
                    emit_byte(ctx, 0xF2);

                    uint8_t rex = 0;
                    if (dst >= X86_64_R8)
                    {
                        rex |= 0x04; // REX.R
                    }
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x01; // REX.B
                    }
                    if (rex)
                    {
                        emit_byte(ctx, 0x40 | rex);
                    }

                    emit_byte(ctx, 0x0F);
                    emit_byte(ctx, 0x10);

                    uint8_t modrm = 0x00 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
                    emit_byte(ctx, modrm);
                }
                else
                {
                    Type *type    = inst->result->type;
                    bool  is_byte = (type && (type->kind == TYPE_U8 || type->kind == TYPE_I8));

                    // REX.W + 8B /r (MOV) or REX.W + 0F B6 /r (MOVZX)
                    uint8_t rex = 0x48; // REX.W
                    if (dst >= X86_64_R8)
                    {
                        rex |= 0x04; // REX.R
                    }
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x01; // REX.B
                    }

                    emit_byte(ctx, rex);

                    if (is_byte)
                    {
                        emit_byte(ctx, 0x0F);
                        emit_byte(ctx, 0xB6);
                    }
                    else
                    {
                        emit_byte(ctx, 0x8B);
                    }

                    uint8_t rm  = reg_to_x86_encoding(src);
                    uint8_t reg = reg_to_x86_encoding(dst);

                    if ((rm & 7) == 4) // RSP or R12 (encoding 4)
                    {
                        // SIB required
                        // Mod=00, Reg=dst, RM=100 (SIB)
                        emit_byte(ctx, (reg << 3) | 0x04);
                        // SIB: Scale=0, Index=4 (none), Base=rm
                        emit_byte(ctx, 0x24);
                    }
                    else if ((rm & 7) == 5) // RBP or R13 (encoding 5)
                    {
                        // Mod=01 (disp8), Reg=dst, RM=101
                        emit_byte(ctx, 0x40 | (reg << 3) | 0x05);
                        emit_byte(ctx, 0x00); // disp8 = 0
                    }
                    else
                    {
                        // Mod=00, Reg=dst, RM=rm
                        emit_byte(ctx, (reg << 3) | rm);
                    }
                }
            }
        }
        break;

    case MIR_OP_STORE:
        // Store to memory
        if (inst->operand_count >= 2)
        {
            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                // Store to stack slot: register -> [rbp + offset]
                X86_64_Reg src    = get_physical_reg(ctx, inst->operands[0].value_id);
                int32_t    offset = (int32_t)inst->operands[1].imm_int;

                if (x86_64_reg_is_fp(src))
                {
                    emit_movsd_reg_mem(ctx, offset - 40, src); // Adjust offset manually
                }
                else
                {
                    emit_mov_reg_to_mem(ctx, offset, src);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                // Store to address in register: src -> [dst_addr]
                // mov [dst_addr], src
                X86_64_Reg src      = get_physical_reg(ctx, inst->operands[0].value_id);
                X86_64_Reg dst_addr = get_physical_reg(ctx, inst->operands[1].value_id);

                if (x86_64_reg_is_fp(src))
                {
                    // movsd [gp], xmm
                    // F2 0F 11 /r
                    emit_byte(ctx, 0xF2);

                    uint8_t rex = 0;
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x04; // REX.R (src is Reg)
                    }
                    if (dst_addr >= X86_64_R8)
                    {
                        rex |= 0x01; // REX.B (dst is RM)
                    }
                    if (rex)
                    {
                        emit_byte(ctx, 0x40 | rex);
                    }

                    emit_byte(ctx, 0x0F);
                    emit_byte(ctx, 0x11);

                    uint8_t modrm = 0x00 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst_addr);
                    emit_byte(ctx, modrm);
                }
                else
                {
                    // REX.W + 89 /r
                    uint8_t rex = 0x48; // REX.W
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x04; // REX.R
                    }
                    if (dst_addr >= X86_64_R8)
                    {
                        rex |= 0x01; // REX.B
                    }

                    emit_byte(ctx, rex);
                    emit_byte(ctx, 0x89);

                    uint8_t rm  = reg_to_x86_encoding(dst_addr);
                    uint8_t reg = reg_to_x86_encoding(src);

                    if ((rm & 7) == 4) // RSP or R12 (encoding 4)
                    {
                        // SIB required
                        // Mod=00, Reg=src, RM=100 (SIB)
                        emit_byte(ctx, (reg << 3) | 0x04);
                        // SIB: Scale=0, Index=4 (none), Base=rm
                        emit_byte(ctx, 0x24);
                    }
                    else if ((rm & 7) == 5) // RBP or R13 (encoding 5)
                    {
                        // Mod=01 (disp8), Reg=src, RM=101
                        emit_byte(ctx, 0x40 | (reg << 3) | 0x05);
                        emit_byte(ctx, 0x00); // disp8 = 0
                    }
                    else
                    {
                        // Mod=00, Reg=src, RM=rm
                        emit_byte(ctx, (reg << 3) | rm);
                    }
                }
            }
        }
        break;

    case MIR_OP_ADDR:
        // Get address of stack slot: lea dst, [rbp + offset]
        if (inst->result && inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_IMM_INT)
        {
            X86_64_Reg dst    = get_physical_reg(ctx, inst->result->id);
            int32_t    offset = (int32_t)inst->operands[0].imm_int;
            // Adjust offset for saved registers (5 * 8 = 40 bytes)
            offset -= 40;

            // REX.W + 8D /r
            uint8_t rex = 0x48; // REX.W
            if (dst >= X86_64_R8)
            {
                rex |= 0x04; // REX.R
            }

            emit_byte(ctx, rex);
            emit_byte(ctx, 0x8D); // LEA

            // ModR/M: mod=10 (disp32), reg=dst, rm=101 (RBP)
            uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
            emit_byte(ctx, modrm);

            emit_dword(ctx, (uint32_t)offset);
        }
        break;

        // ... existing code ...

    case MIR_OP_ADD:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id);

                if (inst->type && (inst->type->kind == TYPE_F32 || inst->type->kind == TYPE_F64))
                {
                    if (dst != lhs)
                    {
                        emit_movsd_reg_reg(ctx, dst, lhs);
                    }
                    emit_addsd_reg_reg(ctx, dst, rhs);
                }
                else
                {
                    if (dst != lhs)
                    {
                        emit_mov_reg_reg(ctx, dst, lhs);
                    }
                    emit_add_reg_reg(ctx, dst, rhs);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != lhs)
                {
                    emit_mov_reg_reg(ctx, dst, lhs);
                }
                emit_add_reg_imm(ctx, dst, (int32_t)inst->operands[1].imm_int);
            }
        }
        break;
        break;

    case MIR_OP_SUB:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id);

                if (inst->type && (inst->type->kind == TYPE_F32 || inst->type->kind == TYPE_F64))
                {
                    if (dst != lhs)
                    {
                        emit_movsd_reg_reg(ctx, dst, lhs);
                    }
                    emit_subsd_reg_reg(ctx, dst, rhs);
                }
                else
                {
                    if (dst != lhs)
                    {
                        emit_mov_reg_reg(ctx, dst, lhs);
                    }
                    emit_sub_reg_reg(ctx, dst, rhs);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                int32_t    imm = (int32_t)inst->operands[1].imm_int;

                if (dst != lhs)
                {
                    emit_mov_reg_reg(ctx, dst, lhs);
                }

                // sub dst, imm
                // REX.W + 81 /5 imm32
                uint8_t rex = 0x48;
                if (dst >= X86_64_R8)
                {
                    rex |= 0x01; // REX.B
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x81);
                emit_byte(ctx, 0xE8 | reg_to_x86_encoding(dst)); // ModRM: 11 101 reg -> E8 | reg
                emit_dword(ctx, imm);
            }
        }
        break;

    case MIR_OP_MUL:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id);

                if (inst->type && (inst->type->kind == TYPE_F32 || inst->type->kind == TYPE_F64))
                {
                    if (dst != lhs)
                    {
                        emit_mulsd_reg_reg(ctx, dst, lhs);
                    }
                    emit_mulsd_reg_reg(ctx, dst, rhs);
                }
                else
                {
                    if (dst != lhs)
                    {
                        emit_mov_reg_reg(ctx, dst, lhs);
                    }
                    emit_imul_reg_reg(ctx, dst, rhs);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != lhs)
                {
                    emit_mov_reg_reg(ctx, dst, lhs);
                }
                emit_imul_reg_imm(ctx, dst, (int32_t)inst->operands[1].imm_int);
            }
        }
        break;

    case MIR_OP_DIV:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[1].value_id);

                if (inst->type && (inst->type->kind == TYPE_F32 || inst->type->kind == TYPE_F64))
                {
                    emit_divsd_reg_reg(ctx, dst, src);
                }
                else
                {
                    // Integer division
                    // Uses IDIV: RDX:RAX / src -> RAX=quotient, RDX=remainder

                    X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                    X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id);

                    // mov rax, lhs
                    emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                    // cqo
                    emit_byte(ctx, 0x48);
                    emit_byte(ctx, 0x99);

                    // idiv rhs
                    uint8_t rex = 0x48;
                    if (rhs >= X86_64_R8)
                    {
                        rex |= 0x01; // REX.B
                    }
                    emit_byte(ctx, rex);
                    emit_byte(ctx, 0xF7);
                    emit_byte(ctx, 0xF8 | reg_to_x86_encoding(rhs));

                    // mov dst, rax (quotient)
                    emit_mov_reg_reg(ctx, dst, X86_64_RAX);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                // Integer division with immediate
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                int64_t    imm = inst->operands[1].imm_int;

                // mov rax, lhs
                emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                // cqo
                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0x99);

                // mov rcx, imm
                emit_mov_reg_imm64(ctx, X86_64_RCX, imm);

                // idiv rcx
                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0xF7);
                emit_byte(ctx, 0xF9);

                // mov dst, rax (quotient)
                emit_mov_reg_reg(ctx, dst, X86_64_RAX);
            }
        }
        break;

    case MIR_OP_MOD:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                // Integer modulo
                // Uses IDIV: RDX:RAX / src -> RAX=quotient, RDX=remainder

                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id);

                // mov rax, lhs
                emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                // cqo (sign extend RAX to RDX:RAX)
                // REX.W + 99
                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0x99);

                // idiv rhs
                // REX.W + F7 /7
                uint8_t rex = 0x48;
                if (rhs >= X86_64_R8)
                {
                    rex |= 0x01; // REX.B
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0xF7);
                // ModRM: 11 111 reg -> 0xF8 | reg
                emit_byte(ctx, 0xF8 | reg_to_x86_encoding(rhs));

                // mov dst, rdx (remainder)
                emit_mov_reg_reg(ctx, dst, X86_64_RDX);
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                // Integer modulo with immediate
                // IDIV requires register/memory, cannot take immediate
                // Move immediate to RCX (scratch)

                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id);
                int64_t    imm = inst->operands[1].imm_int;

                // mov rax, lhs
                emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                // cqo
                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0x99);

                // mov rcx, imm
                emit_mov_reg_imm64(ctx, X86_64_RCX, imm);

                // idiv rcx
                // REX.W + F7 /7 (RCX is 1)
                // ModRM: 11 111 001 = F9
                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0xF7);
                emit_byte(ctx, 0xF9);

                // mov dst, rdx
                emit_mov_reg_reg(ctx, dst, X86_64_RDX);
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

    case MIR_OP_TRUNC:
        // Truncate is just a move for us since we use 64-bit registers mostly
        // But if we had smaller registers, we'd use MOV with smaller size
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src)
                {
                    emit_mov_reg_reg(ctx, dst, src);
                }
            }
        }
        break;

    case MIR_OP_FPTOSI:
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                emit_cvttsd2si_reg_reg(ctx, dst, src);
            }
        }
        break;

    case MIR_OP_SITOFP:
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                emit_cvtsi2sd_reg_reg(ctx, dst, src);
            }
        }
        break;

    case MIR_OP_CAST:
        // For now, CAST is a no-op if source and destination are both integer or both float
        // If it's int to float or float to int, it should be handled by FPTOSI/SITOFP
        // If it's widening/narrowing, it's often a move or sign/zero extension
        // For 64-bit, a simple move is often sufficient for widening/narrowing
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id);
                if (dst != src)
                {
                    emit_mov_reg_reg(ctx, dst, src);
                }
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
                switch (inst->op)
                {
                case MIR_OP_EQ:
                    cond = 0x4;
                    break; // Z
                case MIR_OP_NE:
                    cond = 0x5;
                    break; // NZ
                case MIR_OP_LT:
                    cond = 0xC;
                    break; // L
                case MIR_OP_LE:
                    cond = 0xE;
                    break; // LE
                case MIR_OP_GT:
                    cond = 0xF;
                    break; // G
                case MIR_OP_GE:
                    cond = 0xD;
                    break; // GE
                case MIR_OP_ULT:
                    cond = 0x2;
                    break; // B
                case MIR_OP_ULE:
                    cond = 0x6;
                    break; // BE
                case MIR_OP_UGT:
                    cond = 0x7;
                    break; // A
                case MIR_OP_UGE:
                    cond = 0x3;
                    break; // AE
                default:
                    break;
                }

                emit_setcc(ctx, cond, dst);
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id);
                int32_t    imm  = (int32_t)inst->operands[1].imm_int;

                // cmp src1, imm
                // REX.W + 81 /7 imm32
                uint8_t rex = 0x48;
                if (src1 >= X86_64_R8)
                {
                    rex |= 0x01; // REX.B
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x81);
                emit_byte(ctx, 0xF8 | reg_to_x86_encoding(src1)); // ModRM: 11 111 reg -> F8 | reg
                emit_dword(ctx, imm);

                int cond = 0;
                switch (inst->op)
                {
                case MIR_OP_EQ:
                    cond = 0x4;
                    break; // Z
                case MIR_OP_NE:
                    cond = 0x5;
                    break; // NZ
                case MIR_OP_LT:
                    cond = 0xC;
                    break; // L
                case MIR_OP_LE:
                    cond = 0xE;
                    break; // LE
                case MIR_OP_GT:
                    cond = 0xF;
                    break; // G
                case MIR_OP_GE:
                    cond = 0xD;
                    break; // GE
                case MIR_OP_ULT:
                    cond = 0x2;
                    break; // B
                case MIR_OP_ULE:
                    cond = 0x6;
                    break; // BE
                case MIR_OP_UGT:
                    cond = 0x7;
                    break; // A
                case MIR_OP_UGE:
                    cond = 0x3;
                    break; // AE
                default:
                    break;
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
            if (target)
            {
                emit_jmp(ctx, target);
            }
        }
        break;

    case MIR_OP_BRCOND:
        if (inst->operand_count >= 3 && inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_BLOCK && inst->operands[2].kind == MIR_OPERAND_BLOCK)
        {
            X86_64_Reg cond_reg = get_physical_reg(ctx, inst->operands[0].value_id);

            // test cond, cond
            emit_test_reg_reg(ctx, cond_reg, cond_reg);

            // find true and false blocks
            MIRBlock *true_block  = NULL;
            MIRBlock *false_block = NULL;
            for (MIRBlock *b = ctx->current_function->first_block; b; b = b->next)
            {
                if (b->id == inst->operands[1].block_id)
                {
                    true_block = b;
                }
                if (b->id == inst->operands[2].block_id)
                {
                    false_block = b;
                }
            }

            // jnz true_block
            if (true_block)
            {
                emit_jcc(ctx, 0x5, true_block); // JNZ (NE)
            }

            // jmp false_block
            if (false_block)
            {
                emit_jmp(ctx, false_block);
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
        emit_epilogue(ctx);
        break;

    case MIR_OP_SYSCALL:
        // linux x86_64 syscall convention:
        // syscall number in rax, args in rdi, rsi, rdx, r10, r8, r9
        {
            static const X86_64_Reg syscall_arg_regs[] = {X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_R10, X86_64_R8, X86_64_R9};

            // load arguments to proper registers FIRST to avoid clobbering if they are in RAX
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

                    // lea dst, [rip + symbol]
                    uint8_t rex = 0x48; // REX.W
                    if (dst >= X86_64_R8)
                    {
                        rex |= 0x04; // REX.R
                    }
                    emit_byte(ctx, rex);
                    emit_byte(ctx, 0x8D); // LEA
                    emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                    uint64_t reloc_offset = ctx->code.size;
                    emit_dword(ctx, 0);
                    add_relocation(ctx, reloc_offset, inst->operands[i].global_name, 2, -4); // R_X86_64_PC32 with addend -4 for LEA
                }
            }

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
                    emit_byte(ctx, 0x48);                                                    // REX.W
                    emit_byte(ctx, 0x8D);                                                    // LEA
                    emit_byte(ctx, 0x05);                                                    // ModR/M: [rip+disp32] -> rax
                    emit_dword(ctx, 0);                                                      // displacement filled by relocation
                    add_relocation(ctx, reloc_offset, inst->operands[0].global_name, 2, -4); // R_X86_64_PC32 with addend -4 for LEA
                }
            }

            // emit call instruction with PC-relative relocation
            // (syscall instruction is 0F 05, not CALL)
            emit_byte(ctx, 0x0F);
            emit_byte(ctx, 0x05);

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

    case MIR_OP_CALL:
        // function call - System V AMD64 ABI
        // args in rdi, rsi, rdx, rcx, r8, r9 (then stack)
        // return value in rax
        {
            static const X86_64_Reg call_arg_regs[] = {X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_RCX, X86_64_R8, X86_64_R9};

            // first operand is the function name (global)
            const char *func_name = NULL;
            size_t      first_arg = 0;

            if (inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_GLOBAL)
            {
                func_name = inst->operands[0].global_name;
                first_arg = 1;
            }

            // load arguments into registers
            // To handle register conflicts, we do this in two passes:
            // Pass 1: Save all VALUE sources that have conflicts to the stack
            // Pass 2: Load all arguments from their (possibly saved) locations

            X86_64_Reg sources[6];
            int32_t    saved_offsets[6] = {0}; // Stack offsets for saved values

            // First, collect all source registers
            for (size_t i = first_arg; i < inst->operand_count && (i - first_arg) < 6; i++)
            {
                int arg_idx = i - first_arg;

                if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    sources[arg_idx] = get_physical_reg(ctx, inst->operands[i].value_id);
                }
                else
                {
                    sources[arg_idx] = X86_64_RAX; // Not used for non-VALUE
                }
            }

            // Check for any conflicts and save to stack if needed
            for (size_t i = first_arg; i < inst->operand_count && (i - first_arg) < 6; i++)
            {
                int arg_idx = i - first_arg;

                if (inst->operands[i].kind != MIR_OPERAND_VALUE)
                {
                    continue;
                }

                // Check if this source will be clobbered by an earlier destination
                for (size_t j = first_arg; j < i; j++)
                {
                    int        earlier_idx = j - first_arg;
                    X86_64_Reg earlier_dst = call_arg_regs[earlier_idx];

                    if (sources[arg_idx] == earlier_dst)
                    {
                        // Save this value to stack at a fixed offset
                        saved_offsets[arg_idx] = (arg_idx + 1) * 8;
                        emit_byte(ctx, 0x50 + reg_to_x86_encoding(sources[arg_idx]));
                        break;
                    }
                }
            }

            // Track how many values were saved for stack cleanup
            int num_saved = 0;
            for (int i = 0; i < 6; i++)
            {
                if (saved_offsets[i] != 0)
                {
                    num_saved++;
                }
            }

            // Now load all arguments
            for (size_t i = first_arg; i < inst->operand_count && (i - first_arg) < 6; i++)
            {
                int        arg_idx = i - first_arg;
                X86_64_Reg dst     = call_arg_regs[arg_idx];

                if (inst->operands[i].kind == MIR_OPERAND_IMM_INT)
                {
                    emit_mov_reg_imm64(ctx, dst, inst->operands[i].imm_int);
                }
                else if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    X86_64_Reg src = sources[arg_idx];

                    if (saved_offsets[arg_idx] != 0)
                    {
                        // Load from stack at specific offset
                        // Calculate offset based on LIFO stack ordering
                        int saved_position = 0;
                        for (int j = 0; j < arg_idx; j++)
                        {
                            if (saved_offsets[j] != 0)
                            {
                                saved_position++;
                            }
                        }
                        int stack_offset = (num_saved - 1 - saved_position) * 8;

                        // mov dst, [rsp + offset]
                        uint8_t rex = 0x48; // REX.W
                        if (dst >= X86_64_R8)
                        {
                            rex |= 0x04; // REX.R
                        }
                        emit_byte(ctx, rex);
                        emit_byte(ctx, 0x8B);                                   // MOV r64, r/m64
                        uint8_t modrm = 0x44 | (reg_to_x86_encoding(dst) << 3); // Mod=01 (disp8), Reg=dst, RM=100 (SIB)
                        emit_byte(ctx, modrm);
                        emit_byte(ctx, 0x24);                  // SIB: scale=0, index=100(none), base=100(rsp)
                        emit_byte(ctx, (uint8_t)stack_offset); // disp8
                    }
                    else
                    {
                        if (src != dst)
                        {
                            emit_mov_reg_reg(ctx, dst, src);
                        }
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
                    add_relocation(ctx, reloc_offset, inst->operands[i].global_name, 2, -4); // R_X86_64_PC32 with addend -4 for LEA
                }
            }

            // emit call instruction with PC-relative relocation
            if (func_name)
            {
                emit_byte(ctx, 0xE8); // CALL rel32
                uint64_t reloc_offset = ctx->code.size;
                emit_dword(ctx, 0);                                  // displacement filled by linker
                add_relocation(ctx, reloc_offset, func_name, 4, -4); // R_X86_64_PLT32 with addend -4
            }

            // Clean up stack: add rsp, num_saved * 8
            if (num_saved > 0)
            {
                // add rsp, imm8/imm32
                uint8_t cleanup_bytes = num_saved * 8;
                if (cleanup_bytes <= 127)
                {
                    // REX.W + 83 /0 imm8
                    emit_byte(ctx, 0x48); // REX.W
                    emit_byte(ctx, 0x83); // ADD r/m64, imm8
                    emit_byte(ctx, 0xC4); // ModRM: Mod=11, Reg=0 (/0 for ADD), RM=100 (RSP)
                    emit_byte(ctx, cleanup_bytes);
                }
                else
                {
                    // REX.W + 81 /0 imm32
                    emit_byte(ctx, 0x48); // REX.W
                    emit_byte(ctx, 0x81); // ADD r/m64, imm32
                    emit_byte(ctx, 0xC4); // ModRM: Mod=11, Reg=0, RM=100 (RSP)
                    emit_dword(ctx, cleanup_bytes);
                }
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

    // emit prologue
    // push rbp; mov rbp, rsp
    emit_byte(ctx, 0x55);                              // push rbp
    emit_bytes(ctx, (uint8_t[]){0x48, 0x89, 0xE5}, 3); // mov rbp, rsp

    // save callee-saved registers: RBX, R12, R13, R14, R15
    // push rbx
    emit_byte(ctx, 0x53);
    // push r12
    emit_bytes(ctx, (uint8_t[]){0x41, 0x54}, 2);
    // push r13
    emit_bytes(ctx, (uint8_t[]){0x41, 0x55}, 2);
    // push r14
    emit_bytes(ctx, (uint8_t[]){0x41, 0x56}, 2);
    // push r15
    emit_bytes(ctx, (uint8_t[]){0x41, 0x57}, 2);

    // allocate stack frame if needed
    if (func->frame_size > 0)
    {
        // align frame size to 16 bytes (ABI requirement)
        size_t aligned_size = (func->frame_size + 15) & ~15;

        // sub rsp, aligned_size
        // REX.W + 81 /5 id
        emit_byte(ctx, 0x48);
        emit_byte(ctx, 0x81);
        emit_byte(ctx, 0xEC); // ModRM: 11 101 100 (rsp)
        emit_dword(ctx, (uint32_t)aligned_size);
    }

    // emit all blocks
    for (MIRBlock *block = func->first_block; block; block = block->next)
    {
        register_block_offset(ctx, block);

        // emit all instructions in block
        for (MIRInst *inst = block->first_inst; inst; inst = inst->next)
        {
            // emit instruction
            emit_instruction(ctx, inst);
        }
    }

    // emit epilogue
    emit_epilogue(ctx);

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
