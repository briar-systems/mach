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
    ctx->reg_map.stack_slots = NULL;
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

    if (ctx->reg_map.stack_slots)
    {
        free(ctx->reg_map.stack_slots);
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

// map our register enum to the encoding used by x86
static uint8_t reg_to_x86_encoding(X86_64_Reg reg)
{
    // x86-64 register encoding: rax=0, rcx=1, rdx=2, rbx=3, rsp=4, rbp=5, rsi=6, rdi=7
    // our enum: rax=0, rbx=1, rcx=2, rdx=3, rsi=4, rdi=5, rbp=6, rsp=7
    static const uint8_t encoding_map[] = {
        0, // rax -> 0
        3, // rbx -> 3
        1, // rcx -> 1
        2, // rdx -> 2
        6, // rsi -> 6
        7, // rdi -> 7
        5, // rbp -> 5
        4, // rsp -> 4
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7 // r8-r15 (low 3 bits)
    };
    return encoding_map[reg];
}

// liveness information for a value
typedef struct
{
    uint32_t value_id;
    size_t   first_use; // instruction index where value is defined
    size_t   last_use;  // instruction index where value is last used
    bool     is_fp;     // whether value is floating point
} ValueLiveness;

static X86_64_Reg get_physical_reg(X86_64_CodegenContext *ctx, uint32_t virtual_reg, X86_64_Reg scratch)
{
    if (virtual_reg >= ctx->reg_map.count)
    {
        return X86_64_RAX; // fallback
    }
    
    X86_64_Reg reg = ctx->reg_map.map[virtual_reg];
    if (reg == X86_64_REG_COUNT) // stack slot
    {
        int32_t offset = ctx->reg_map.stack_slots[virtual_reg];
        if (offset != 0)
        {
            // emit load: mov scratch, [rbp + offset]
            uint8_t rex = 0x48;
            if (scratch >= X86_64_R8) rex |= 0x04;
            emit_byte(ctx, rex);
            emit_byte(ctx, 0x8B);
            
            // modrm: mod=10 (disp32), reg=scratch, rm=101 (rbp)
            uint8_t modrm = 0x80 | (reg_to_x86_encoding(scratch) << 3) | 0x05;
            emit_byte(ctx, modrm);
            emit_dword(ctx, (uint32_t)offset);
            
            return scratch;
        }
    }
    return reg;
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
                MIRType *type = inst->result->type;
                if (type && (type->kind == MIR_TYPE_F32 || type->kind == MIR_TYPE_F64))
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
    // abi-aware allocation with parameter mapping
    // system v amd64 abi: params in rdi, rsi, rdx, rcx, r8, r9
    // stack parameters arrive right to left at [rbp + 16], [rbp + 24], ...

    // allocate map
    if (func->next_value_id > ctx->reg_map.capacity)
    {
        size_t      new_capacity = func->next_value_id;
        X86_64_Reg *new_map      = realloc(ctx->reg_map.map, new_capacity * sizeof(X86_64_Reg));
        int32_t    *new_slots    = realloc(ctx->reg_map.stack_slots, new_capacity * sizeof(int32_t));
        
        if (!new_map || !new_slots)
        {
            return -1;
        }
        ctx->reg_map.map         = new_map;
        ctx->reg_map.stack_slots = new_slots;
        ctx->reg_map.capacity    = new_capacity;
    }

    ctx->reg_map.count = func->next_value_id;
    memset(ctx->reg_map.stack_slots, 0, ctx->reg_map.count * sizeof(int32_t));

    // abi parameter registers (system v amd64)
    static const X86_64_Reg gp_param_regs[] = {X86_64_RDI, X86_64_RSI, X86_64_RDX, X86_64_RCX, X86_64_R8, X86_64_R9};
    static const X86_64_Reg fp_param_regs[] = {X86_64_XMM0, X86_64_XMM1, X86_64_XMM2, X86_64_XMM3, X86_64_XMM4, X86_64_XMM5, X86_64_XMM6, X86_64_XMM7};

    // allocatable gp registers (callee-saved first for better calling convention)
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

    // allocatable fp registers (xmm8-xmm15 first as they're callee-saved in windows but not sysv)
    // for simplicity we keep the full xmm set available
    X86_64_Reg allocatable_fp_regs[16];
    int        num_allocatable_fp = 0;
    for (int i = 0; i < 16; i++)
    {
        allocatable_fp_regs[num_allocatable_fp++] = (X86_64_Reg)(X86_64_XMM0 + i);
    }

    // assign parameter registers and handle stack spilling
    int gp_param_idx = 0;
    int fp_param_idx = 0;

    for (size_t i = 0; i < func->param_count; i++)
    {
        if (func->params[i])
        {
            uint32_t param_id = func->params[i]->id;
            MIRType *type     = func->params[i]->type;

            if (type && (type->kind == MIR_TYPE_F32 || type->kind == MIR_TYPE_F64))
            {
                if (fp_param_idx < 8)
                {
                    // parameter in fp register
                    ctx->reg_map.map[param_id] = fp_param_regs[fp_param_idx++];
                }
                else
                {
                    // parameter fell to the stack so reserve a temporary register for now
                    // proper stack parameter support will load it in the prologue (xmm15 fallback)
                    ctx->reg_map.map[param_id] = X86_64_XMM15;
                }
            }
            else
            {
                if (gp_param_idx < 6)
                {
                    // parameter in gp register
                    ctx->reg_map.map[param_id] = gp_param_regs[gp_param_idx++];
                }
                else
                {
                    // parameter fell to the stack
                    // stack parameters arrive right to left at [rbp + 16], [rbp + 24], ...
                    int32_t offset = 16 + (gp_param_idx - 6) * 8;
                    ctx->reg_map.map[param_id] = X86_64_REG_COUNT; // mark as stack
                    ctx->reg_map.stack_slots[param_id] = offset;
                    gp_param_idx++;
                }
            }
        }
    }

    // add any unused param registers back into the allocatable pool
    for (int i = gp_param_idx; i < 6; i++)
    {
        allocatable_gp_regs[num_allocatable_gp++] = gp_param_regs[i];
    }

    // compute liveness information
    size_t         liveness_count = 0;
    ValueLiveness *liveness       = compute_liveness(func, &liveness_count);
    if (!liveness)
    {
        return -1;
    }

    // initialize non-parameter values
    for (uint32_t i = 0; i < func->next_value_id; i++)
    {
        // skip parameters that already own real registers
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
            ctx->reg_map.map[i] = X86_64_RAX; // placeholder until allocation runs
        }
    }

    // linear scan register allocation based on liveness
    // values already arrive roughly in first-use order, so no extra sort yet

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

    // track which physical registers are currently in use
    bool gp_in_use[16] = {false};
    bool fp_in_use[16] = {false};

    // process instructions in order and allocate registers on demand
    size_t    inst_idx = 0;
    MIRBlock *block    = func->first_block;
    while (block)
    {
        MIRInst *inst = block->first_inst;
        while (inst)
        {
            // free registers for values that are no longer live
            for (uint32_t i = 0; i < func->next_value_id; i++)
            {
                if (liveness[i].last_use < inst_idx && liveness[i].first_use != (size_t)-1 && value_live[i])
                {
                    // value is no longer live, so release its register
                    X86_64_Reg reg = ctx->reg_map.map[i];
                    if (reg >= X86_64_XMM0 && reg <= X86_64_XMM15)
                    {
                        int fp_idx        = reg - X86_64_XMM0;
                        fp_in_use[fp_idx] = false;
                    }
                    else if (reg >= X86_64_RAX && reg <= X86_64_R15)
                    {
                        // find the register in the allocatable list
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

            // allocate register for result if needed
            if (inst->result)
            {
                uint32_t id = inst->result->id;

                // skip parameters that already have physical registers
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
                    // choose register class based on value type
                    if (liveness[id].is_fp)
                    {
                        // find a free fp register
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
                            // spill to stack
                            ctx->reg_map.map[id] = X86_64_REG_COUNT;
                            func->frame_size += 8;
                            // raw offset from RBP (locals start at -40)
                            // lower.c frame_size is relative to locals start
                            int32_t offset = -40 - (int32_t)func->frame_size;
                            ctx->reg_map.stack_slots[id] = offset;
                            value_live[id] = true;
                        }
                    }
                    else
                    {
                        // find a free gp register
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
                            // spill to stack
                            ctx->reg_map.map[id] = X86_64_REG_COUNT;
                            func->frame_size += 8;
                            int32_t offset = -40 - (int32_t)func->frame_size;
                            ctx->reg_map.stack_slots[id] = offset;
                            value_live[id] = true;
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

// emit x86_64 instruction encodings
static void emit_mov_reg_imm64(X86_64_CodegenContext *ctx, X86_64_Reg dst, int64_t imm)
{
    // mov reg, imm64 -> rex.w + b8+r imm64
    uint8_t rex = 0x48; // rex.w prefix
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
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
    // add dst, src -> rex.w + 01 /r
    uint8_t rex = 0x48; // rex.w prefix
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x01);

    // modrm byte: mod=11 (register), reg=src, rm=dst
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_sub_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // sub dst, src -> rex.w + 29 /r
    uint8_t rex = 0x48; // rex.w prefix
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x29);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_imul_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // imul dst, src -> rex.w + 0f af /r (dst in reg field)
    uint8_t rex = 0x48; // rex.w prefix
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0xAF);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_imul_reg_imm(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t imm)
{
    // imul dst, dst, imm -> rex.w + 69 /r imm32 (or 6b /r imm8)
    uint8_t rex = 0x48; // rex.w prefix
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
        rex |= 0x01;
    }

    emit_byte(ctx, rex);

    // prefer the imm8 encoding when possible
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
    // and dst, src -> rex.w + 21 /r
    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x21);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_or_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // or dst, src -> rex.w + 09 /r
    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x09);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_xor_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // xor dst, src -> rex.w + 31 /r
    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x31);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_cmp_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // cmp dst, src -> rex.w + 39 /r
    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x39);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_setcc(X86_64_CodegenContext *ctx, int cond, X86_64_Reg dst)
{
    // setcc dst (low byte) using 0f 9x /0

    // rex is required for spl/bpl/sil/dil or any r8-r15 target
    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x41;
    }
    else if (dst == X86_64_RSP || dst == X86_64_RBP || dst == X86_64_RSI || dst == X86_64_RDI)
    {
        rex |= 0x40;
    }

    if (rex)
    {
        emit_byte(ctx, rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x90 | cond);

    uint8_t modrm = 0xC0 | (0 << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);

    // movzx dst, dst (byte to qword) to clear upper bytes using rex.w + 0f b6 /r
    rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x05;
    }

    emit_byte(ctx, rex);
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0xB6);

    modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_jmp(X86_64_CodegenContext *ctx, MIRBlock *target)
{
    emit_byte(ctx, 0xE9); // jmp rel32
    size_t disp_offset = ctx->code.size;
    emit_dword(ctx, 0); // placeholder
    add_pending_jump(ctx, target, disp_offset);
}

static void emit_jcc(X86_64_CodegenContext *ctx, int cond, MIRBlock *target)
{
    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x80 | cond); // jcc rel32
    size_t disp_offset = ctx->code.size;
    emit_dword(ctx, 0); // placeholder
    add_pending_jump(ctx, target, disp_offset);
}

static void emit_test_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // test dst, src -> rex.w + 85 /r
    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x85);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

// sse helpers (double precision)

static void emit_movsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // movsd dst, src -> f2 0f 10 /r
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
    emit_byte(ctx, 0x10);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_addsd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // addsd dst, src -> f2 0f 58 /r
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
    // subsd dst, src -> f2 0f 5c /r
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
    // mulsd dst, src -> f2 0f 59 /r
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
    // divsd dst, src -> f2 0f 5e /r
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
    // cvttsd2si dst, src -> f2 48 0f 2c /r (dst=r64, src=xmm)
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    emit_byte(ctx, rex);

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x2C);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_cvtsi2sd_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // cvtsi2sd dst, src -> f2 48 0f 2a /r (dst=xmm, src=r64)
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x01;
    }
    emit_byte(ctx, rex);

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x2A);

    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
    emit_byte(ctx, modrm);
}

static void emit_movsd_mem_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t offset)
{
    // movsd dst, [rbp + offset] -> f2 0f 10 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x10);

    // modrm: mod=10 (disp32), reg=dst, rm=101 (rbp)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
    emit_byte(ctx, modrm);
    emit_dword(ctx, (uint32_t)offset);
}

static void emit_movsd_reg_mem(X86_64_CodegenContext *ctx, int32_t offset, X86_64_Reg src)
{
    // movsd [rbp + offset], src -> f2 0f 11 /r
    emit_byte(ctx, 0xF2);

    uint8_t rex = 0;
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    if (rex)
    {
        emit_byte(ctx, 0x40 | rex);
    }

    emit_byte(ctx, 0x0F);
    emit_byte(ctx, 0x11);

    // modrm: mod=10 (disp32), reg=src, rm=101 (rbp)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(src) << 3) | 0x05;
    emit_byte(ctx, modrm);
    emit_dword(ctx, (uint32_t)offset);
}

// load from [rbp + offset] into a register
static void emit_mov_mem_to_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t offset)
{
    // adjust offset for the callee-saved spill area (5 * 8 = 40 bytes)
    offset -= 40;

    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x04;
    }

    emit_byte(ctx, rex);
    emit_byte(ctx, 0x8B);

    // modrm: mod=10 (disp32), reg=dst, rm=101 (rbp)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
    emit_byte(ctx, modrm);

    // 32-bit displacement (little-endian)
    emit_dword(ctx, (uint32_t)offset);
}

// store a register into [rbp + offset]
static void emit_mov_reg_to_mem(X86_64_CodegenContext *ctx, int32_t offset, X86_64_Reg src)
{
    // adjust offset for the callee-saved spill area (5 * 8 = 40 bytes)
    offset -= 40;

    uint8_t rex = 0x48;
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }

    emit_byte(ctx, rex);
    emit_byte(ctx, 0x89);

    // modrm: mod=10 (disp32), reg=src, rm=101 (rbp)
    uint8_t modrm = 0x80 | (reg_to_x86_encoding(src) << 3) | 0x05;
    emit_byte(ctx, modrm);

    // 32-bit displacement (little-endian)
    emit_dword(ctx, (uint32_t)offset);
}
static void emit_ret(X86_64_CodegenContext *ctx)
{
    // ret
    emit_byte(ctx, 0xC3);
}

static void emit_mov_reg_reg(X86_64_CodegenContext *ctx, X86_64_Reg dst, X86_64_Reg src)
{
    // mov dst, src -> rex.w + 89 /r
    if (dst == src)
    {
        return; // no-op
    }

    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
    }
    if (src >= X86_64_R8)
    {
        rex |= 0x04;
    }
    emit_byte(ctx, rex);
    emit_byte(ctx, 0x89);

    // modrm byte: mod=11 (register), reg=src, rm=dst
    uint8_t modrm = 0xC0 | (reg_to_x86_encoding(src) << 3) | reg_to_x86_encoding(dst);
    emit_byte(ctx, modrm);
}

static void emit_add_reg_imm(X86_64_CodegenContext *ctx, X86_64_Reg dst, int32_t imm)
{
    // add dst, imm (83/81 encodings depending on literal width)

    uint8_t rex = 0x48;
    if (dst >= X86_64_R8)
    {
        rex |= 0x01;
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
    // restore stack pointer to point to saved registers
    // lea rsp, [rbp - 40] (5 regs * 8 bytes)
    // this works regardless of frame size and ensures we point to the saved regs
    emit_byte(ctx, 0x48);
    emit_byte(ctx, 0x8D);
    emit_byte(ctx, 0x65);
    emit_byte(ctx, 0xD8); // -40

    // restore callee-saved gp registers
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5F}, 2);
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5E}, 2);
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5D}, 2);
    emit_bytes(ctx, (uint8_t[]){0x41, 0x5C}, 2);
    emit_byte(ctx, 0x5B);

    emit_byte(ctx, 0xC9); // leave (mov rsp, rbp; pop rbp)
    emit_ret(ctx);
}

static void emit_spill_store(X86_64_CodegenContext *ctx, MIRValue *val, X86_64_Reg src)
{
    if (!val) return;
    if (val->id >= ctx->reg_map.count) return;
    
    if (ctx->reg_map.map[val->id] == X86_64_REG_COUNT)
    {
        int32_t offset = ctx->reg_map.stack_slots[val->id];
        if (offset != 0)
        {
            // store src to [rbp + offset]
            uint8_t rex = 0x48;
            if (src >= X86_64_R8) rex |= 0x04;
            emit_byte(ctx, rex);
            emit_byte(ctx, 0x89);
            
            uint8_t modrm = 0x80 | (reg_to_x86_encoding(src) << 3) | 0x05;
            emit_byte(ctx, modrm);
            emit_dword(ctx, (uint32_t)offset);
        }
    }
}

static void emit_instruction(X86_64_CodegenContext *ctx, MIRInst *inst)
{
    switch (inst->op)
    {
    case MIR_OP_CONST:
        if (inst->result && inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_IMM_INT)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            emit_mov_reg_imm64(ctx, dst, inst->operands[0].imm_int);
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_MOV:
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
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
                // lea dst, [rip + symbol]
                uint8_t rex = 0x48;
                if (dst >= X86_64_R8)
                {
                    rex |= 0x04;
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x8D);
                // modrm: mod=00, reg=dst, rm=101 (rip-relative)
                emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                uint64_t reloc_offset = ctx->code.size;
                emit_dword(ctx, 0);
                add_relocation(ctx, reloc_offset, inst->operands[0].global_name, 2, -4);
            }
            else if (inst->operands[0].kind == MIR_OPERAND_IMM_INT)
            {
                emit_mov_reg_imm64(ctx, dst, inst->operands[0].imm_int);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_LOAD:
        // load from memory
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_IMM_INT)
            {
                // load from stack slot: [rbp + offset] -> register
                int32_t offset = (int32_t)inst->operands[0].imm_int;
                if (x86_64_reg_is_fp(dst))
                {
                    emit_movsd_mem_reg(ctx, dst, offset - 40); // adjust offset manually as emit_movsd_mem_reg doesn't do it
                }
                else
                {
                    // check if loading a byte value - use movzx for u8/i8
                    MIRType *type = inst->result->type;
                    bool  is_byte = (type && (type->kind == MIR_TYPE_U8 || type->kind == MIR_TYPE_I8));
                    
                    if (is_byte)
                    {
                        // movzx dst, byte [rbp + offset]
                        offset -= 40; // adjust like emit_mov_mem_to_reg does
                        
                        uint8_t rex = 0x48;
                        if (dst >= X86_64_R8)
                        {
                            rex |= 0x04;
                        }
                        emit_byte(ctx, rex);
                        emit_byte(ctx, 0x0F);
                        emit_byte(ctx, 0xB6);
                        
                        uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
                        emit_byte(ctx, modrm);
                        emit_dword(ctx, (uint32_t)offset);
                    }
                    else
                    {
                        emit_mov_mem_to_reg(ctx, dst, offset);
                    }
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                // load from address in register: [src] -> dst
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);

                if (x86_64_reg_is_fp(dst))
                {
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
                    emit_byte(ctx, 0x10);

                    uint8_t modrm = 0x00 | (reg_to_x86_encoding(dst) << 3) | reg_to_x86_encoding(src);
                    emit_byte(ctx, modrm);
                }
                else
                {
                    MIRType *type = inst->result->type;
                    bool  is_byte = (type && (type->kind == MIR_TYPE_U8 || type->kind == MIR_TYPE_I8));

                    uint8_t rex = 0x48;
                    if (dst >= X86_64_R8)
                    {
                        rex |= 0x04;
                    }
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x01;
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

                    if ((rm & 7) == 4) // rsp or r12
                    {
                        // emit sib form with no index
                        emit_byte(ctx, (reg << 3) | 0x04);
                        emit_byte(ctx, 0x24);
                    }
                    else if ((rm & 7) == 5) // rbp or r13
                    {
                        // force a disp8 form with zero displacement
                        emit_byte(ctx, 0x40 | (reg << 3) | 0x05);
                        emit_byte(ctx, 0x00);
                    }
                    else
                    {
                        // plain rm encoding
                        emit_byte(ctx, (reg << 3) | rm);
                    }
                }
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_STORE:
        // store to memory
        if (inst->operand_count >= 2)
        {
            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                // store to stack slot: register -> [rbp + offset]
                X86_64_Reg src    = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                int32_t    offset = (int32_t)inst->operands[1].imm_int;

                if (x86_64_reg_is_fp(src))
                {
                    emit_movsd_reg_mem(ctx, offset - 40, src); // adjust offset manually
                }
                else
                {
                    emit_mov_reg_to_mem(ctx, offset, src);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                // store to address in register: src -> [dst_addr]
                X86_64_Reg src      = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                X86_64_Reg dst_addr = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                if (x86_64_reg_is_fp(src))
                {
                    emit_byte(ctx, 0xF2);

                    uint8_t rex = 0;
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x04;
                    }
                    if (dst_addr >= X86_64_R8)
                    {
                        rex |= 0x01;
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
                    // encode via rex.w + 89 /r
                    uint8_t rex = 0x48;
                    if (src >= X86_64_R8)
                    {
                        rex |= 0x04;
                    }
                    if (dst_addr >= X86_64_R8)
                    {
                        rex |= 0x01;
                    }

                    emit_byte(ctx, rex);
                    emit_byte(ctx, 0x89);

                    uint8_t rm  = reg_to_x86_encoding(dst_addr);
                    uint8_t reg = reg_to_x86_encoding(src);

                    if ((rm & 7) == 4) // rsp or r12
                    {
                        // emit sib form with no index
                        emit_byte(ctx, (reg << 3) | 0x04);
                        emit_byte(ctx, 0x24);
                    }
                    else if ((rm & 7) == 5) // rbp or r13
                    {
                        // force disp8 form with zero displacement
                        emit_byte(ctx, 0x40 | (reg << 3) | 0x05);
                        emit_byte(ctx, 0x00);
                    }
                    else
                    {
                        // plain rm encoding
                        emit_byte(ctx, (reg << 3) | rm);
                    }
                }
            }
        }
        break;

    case MIR_OP_ADDR:
        // get the address of a stack slot via lea
        if (inst->result && inst->operand_count > 0 && inst->operands[0].kind == MIR_OPERAND_IMM_INT)
        {
            X86_64_Reg dst    = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            int32_t    offset = (int32_t)inst->operands[0].imm_int;
            // adjust offset for saved registers (5 * 8 = 40 bytes)
            offset -= 40;

            uint8_t rex = 0x48;
            if (dst >= X86_64_R8)
            {
                rex |= 0x04;
            }

            emit_byte(ctx, rex);
            emit_byte(ctx, 0x8D);

            // modrm: mod=10 (disp32), reg=dst, rm=101 (rbp)
            uint8_t modrm = 0x80 | (reg_to_x86_encoding(dst) << 3) | 0x05;
            emit_byte(ctx, modrm);

            emit_dword(ctx, (uint32_t)offset);
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_ADD:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                if (inst->type && (inst->type->kind == MIR_TYPE_F32 || inst->type->kind == MIR_TYPE_F64))
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
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != lhs)
                {
                    emit_mov_reg_reg(ctx, dst, lhs);
                }
                emit_add_reg_imm(ctx, dst, (int32_t)inst->operands[1].imm_int);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;
        break;

    case MIR_OP_SUB:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                if (inst->type && (inst->type->kind == MIR_TYPE_F32 || inst->type->kind == MIR_TYPE_F64))
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
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                int32_t    imm = (int32_t)inst->operands[1].imm_int;

                if (dst != lhs)
                {
                    emit_mov_reg_reg(ctx, dst, lhs);
                }

                // encode sub dst, imm via rex.w + 81 /5 imm32
                uint8_t rex = 0x48;
                if (dst >= X86_64_R8)
                {
                    rex |= 0x01;
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x81);
                emit_byte(ctx, 0xE8 | reg_to_x86_encoding(dst));
                emit_dword(ctx, imm);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_MUL:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                if (inst->type && (inst->type->kind == MIR_TYPE_F32 || inst->type->kind == MIR_TYPE_F64))
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
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != lhs)
                {
                    emit_mov_reg_reg(ctx, dst, lhs);
                }
                emit_imul_reg_imm(ctx, dst, (int32_t)inst->operands[1].imm_int);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_DIV:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                if (inst->type && (inst->type->kind == MIR_TYPE_F32 || inst->type->kind == MIR_TYPE_F64))
                {
                    emit_divsd_reg_reg(ctx, dst, src);
                }
                else
                {
                    // integer division via idiv (rdx:rax / src)

                    X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                    X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                    emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                    emit_byte(ctx, 0x48);
                    emit_byte(ctx, 0x99);

                    uint8_t rex = 0x48;
                    if (rhs >= X86_64_R8)
                    {
                        rex |= 0x01;
                    }
                    emit_byte(ctx, rex);
                    emit_byte(ctx, 0xF7);
                    emit_byte(ctx, 0xF8 | reg_to_x86_encoding(rhs));

                    emit_mov_reg_reg(ctx, dst, X86_64_RAX);
                }
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                // integer division with an immediate, routing through rcx
                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                int64_t    imm = inst->operands[1].imm_int;

                emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0x99);

                emit_mov_reg_imm64(ctx, X86_64_RCX, imm);

                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0xF7);
                emit_byte(ctx, 0xF9);

                emit_mov_reg_reg(ctx, dst, X86_64_RAX);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_MOD:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);

            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                // integer modulo via idiv (rdx captures the remainder)

                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                X86_64_Reg rhs = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0x99);

                uint8_t rex = 0x48;
                if (rhs >= X86_64_R8)
                {
                    rex |= 0x01;
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0xF7);
                emit_byte(ctx, 0xF8 | reg_to_x86_encoding(rhs));

                emit_mov_reg_reg(ctx, dst, X86_64_RDX);
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                // integer modulo with immediate: move literal into rcx first

                X86_64_Reg lhs = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                int64_t    imm = inst->operands[1].imm_int;

                emit_mov_reg_reg(ctx, X86_64_RAX, lhs);

                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0x99);

                emit_mov_reg_imm64(ctx, X86_64_RCX, imm);

                emit_byte(ctx, 0x48);
                emit_byte(ctx, 0xF7);
                emit_byte(ctx, 0xF9);

                emit_mov_reg_reg(ctx, dst, X86_64_RDX);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_AND:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);
                emit_and_reg_reg(ctx, dst, src2);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_OR:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);
                emit_or_reg_reg(ctx, dst, src2);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_XOR:
        if (inst->result && inst->operand_count >= 2)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != src1)
                {
                    emit_mov_reg_reg(ctx, dst, src1);
                }
            }
            if (inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);
                emit_xor_reg_reg(ctx, dst, src2);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_TRUNC:
        // truncation is effectively a move since we mostly operate on 64-bit regs
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != src)
                {
                    emit_mov_reg_reg(ctx, dst, src);
                }
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_FPTOSI:
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                emit_cvttsd2si_reg_reg(ctx, dst, src);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_SITOFP:
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                emit_cvtsi2sd_reg_reg(ctx, dst, src);
            }
            emit_spill_store(ctx, inst->result, dst);
        }
        break;

    case MIR_OP_CAST:
        // casts within the same category (int-int or float-float) boil down to moves for now
        // other conversions are delegated to the dedicated mir opcodes during lowering
        if (inst->result && inst->operand_count > 0)
        {
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                if (dst != src)
                {
                    emit_mov_reg_reg(ctx, dst, src);
                }
            }
            emit_spill_store(ctx, inst->result, dst);
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
            X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
            if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_VALUE)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                X86_64_Reg src2 = get_physical_reg(ctx, inst->operands[1].value_id, X86_64_R11);

                emit_cmp_reg_reg(ctx, src1, src2);

                int cond = 0;
                switch (inst->op)
                {
                case MIR_OP_EQ:
                    cond = 0x4;
                    break; // zero
                case MIR_OP_NE:
                    cond = 0x5;
                    break; // non-zero
                case MIR_OP_LT:
                    cond = 0xC;
                    break; // less-than
                case MIR_OP_LE:
                    cond = 0xE;
                    break; // less-or-equal
                case MIR_OP_GT:
                    cond = 0xF;
                    break; // greater-than
                case MIR_OP_GE:
                    cond = 0xD;
                    break; // greater-or-equal
                case MIR_OP_ULT:
                    cond = 0x2;
                    break; // below
                case MIR_OP_ULE:
                    cond = 0x6;
                    break; // below-or-equal
                case MIR_OP_UGT:
                    cond = 0x7;
                    break; // above
                case MIR_OP_UGE:
                    cond = 0x3;
                    break; // above-or-equal
                default:
                    break;
                }

                emit_setcc(ctx, cond, dst);
            }
            else if (inst->operands[0].kind == MIR_OPERAND_VALUE && inst->operands[1].kind == MIR_OPERAND_IMM_INT)
            {
                X86_64_Reg src1 = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                int32_t    imm  = (int32_t)inst->operands[1].imm_int;

                uint8_t rex = 0x48;
                if (src1 >= X86_64_R8)
                {
                    rex |= 0x01;
                }
                emit_byte(ctx, rex);
                emit_byte(ctx, 0x81);
                emit_byte(ctx, 0xF8 | reg_to_x86_encoding(src1));
                emit_dword(ctx, imm);

                int cond = 0;
                switch (inst->op)
                {
                case MIR_OP_EQ:
                    cond = 0x4;
                    break; // zero
                case MIR_OP_NE:
                    cond = 0x5;
                    break; // non-zero
                case MIR_OP_LT:
                    cond = 0xC;
                    break; // less-than
                case MIR_OP_LE:
                    cond = 0xE;
                    break; // less-or-equal
                case MIR_OP_GT:
                    cond = 0xF;
                    break; // greater-than
                case MIR_OP_GE:
                    cond = 0xD;
                    break; // greater-or-equal
                case MIR_OP_ULT:
                    cond = 0x2;
                    break; // below
                case MIR_OP_ULE:
                    cond = 0x6;
                    break; // below-or-equal
                case MIR_OP_UGT:
                    cond = 0x7;
                    break; // above
                case MIR_OP_UGE:
                    cond = 0x3;
                    break; // above-or-equal
                default:
                    break;
                }

                emit_setcc(ctx, cond, dst);
            }
            emit_spill_store(ctx, inst->result, dst);
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
            X86_64_Reg cond_reg = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);

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

            if (true_block)
            {
                emit_jcc(ctx, 0x5, true_block); // jnz (ne)
            }

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
            X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
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
                    X86_64_Reg src = get_physical_reg(ctx, inst->operands[i].value_id, X86_64_R10);
                    emit_mov_reg_reg(ctx, syscall_arg_regs[i - 1], src);
                }
                else if (inst->operands[i].kind == MIR_OPERAND_GLOBAL)
                {
                    // lea reg, [rip + symbol] with pc-relative relocation
                    X86_64_Reg dst = syscall_arg_regs[i - 1];

                    uint8_t rex = 0x48;
                    if (dst >= X86_64_R8)
                    {
                        rex |= 0x04;
                    }
                    emit_byte(ctx, rex);
                    emit_byte(ctx, 0x8D);
                    emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                    uint64_t reloc_offset = ctx->code.size;
                    emit_dword(ctx, 0);
                    add_relocation(ctx, reloc_offset, inst->operands[i].global_name, 2, -4);
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
                    X86_64_Reg src = get_physical_reg(ctx, inst->operands[0].value_id, X86_64_R10);
                    emit_mov_reg_reg(ctx, X86_64_RAX, src);
                }
                else if (inst->operands[0].kind == MIR_OPERAND_GLOBAL)
                {
                    // lea rax, [rip + symbol] with relocation
                    uint64_t reloc_offset = ctx->code.size + 3;
                    emit_byte(ctx, 0x48);
                    emit_byte(ctx, 0x8D);
                    emit_byte(ctx, 0x05);
                    emit_dword(ctx, 0);
                    add_relocation(ctx, reloc_offset, inst->operands[0].global_name, 2, -4);
                }
            }

            // emit the syscall instruction (0f 05)
            emit_byte(ctx, 0x0F);
            emit_byte(ctx, 0x05);

            // result is in rax - if instruction has result, move to allocated register
            if (inst->result)
            {
                X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
                if (dst != X86_64_RAX)
                {
                    emit_mov_reg_reg(ctx, dst, X86_64_RAX);
                }
                emit_spill_store(ctx, inst->result, dst);
            }
        }
        break;

    case MIR_OP_CALL:
        // function call (system v amd64 abi): args -> rdi,rsi,rdx,rcx,r8,r9 then stack, return in rax
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

            // push stack arguments (right to left)
            size_t total_args = inst->operand_count - first_arg;
            size_t num_stack_args = 0;
            if (total_args > 6)
            {
                num_stack_args = total_args - 6;
                for (size_t i = total_args; i > 6; i--)
                {
                    size_t arg_idx = first_arg + i - 1;
                    if (inst->operands[arg_idx].kind == MIR_OPERAND_VALUE)
                    {
                        X86_64_Reg src = get_physical_reg(ctx, inst->operands[arg_idx].value_id, X86_64_R10);
                        // push src
                        if (src >= X86_64_R8) {
                            emit_byte(ctx, 0x41);
                            emit_byte(ctx, 0x50 + (src - X86_64_R8));
                        } else {
                            emit_byte(ctx, 0x50 + src);
                        }
                    }
                    else if (inst->operands[arg_idx].kind == MIR_OPERAND_IMM_INT)
                    {
                        emit_mov_reg_imm64(ctx, X86_64_R10, inst->operands[arg_idx].imm_int);
                        emit_byte(ctx, 0x41);
                        emit_byte(ctx, 0x50 + (X86_64_R10 - X86_64_R8));
                    }
                }
            }

            // load arguments into registers in two passes:
            //   1) save conflicting values to the stack
            //   2) reload arguments from their safe locations

            X86_64_Reg sources[6];
            int32_t    saved_offsets[6] = {0};

            // collect all source registers
            for (size_t i = first_arg; i < inst->operand_count && (i - first_arg) < 6; i++)
            {
                int arg_idx = i - first_arg;

                if (inst->operands[i].kind == MIR_OPERAND_VALUE)
                {
                    sources[arg_idx] = get_physical_reg(ctx, inst->operands[i].value_id, X86_64_R10);
                }
                else
                {
                    sources[arg_idx] = X86_64_RAX; // placeholder for non-values
                }
            }

            // detect conflicts and spill temporaries as needed
            for (size_t i = first_arg; i < inst->operand_count && (i - first_arg) < 6; i++)
            {
                int arg_idx = i - first_arg;

                if (inst->operands[i].kind != MIR_OPERAND_VALUE)
                {
                    continue;
                }

                // check if this source will be clobbered by an earlier destination
                for (size_t j = first_arg; j < i; j++)
                {
                    int        earlier_idx = j - first_arg;
                    X86_64_Reg earlier_dst = call_arg_regs[earlier_idx];

                    if (sources[arg_idx] == earlier_dst)
                    {
                        // save this value to the stack at a fixed offset
                        saved_offsets[arg_idx] = (arg_idx + 1) * 8;
                        emit_byte(ctx, 0x50 + reg_to_x86_encoding(sources[arg_idx]));
                        break;
                    }
                }
            }

            // count spills so we can rewind the stack afterward
            int num_saved = 0;
            for (int i = 0; i < 6; i++)
            {
                if (saved_offsets[i] != 0)
                {
                    num_saved++;
                }
            }

            // reload arguments for the call
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
                        // load from stack at the correct lifo offset
                        int saved_position = 0;
                        for (int j = 0; j < arg_idx; j++)
                        {
                            if (saved_offsets[j] != 0)
                            {
                                saved_position++;
                            }
                        }
                        int stack_offset = (num_saved - 1 - saved_position) * 8;

                        uint8_t rex = 0x48;
                        if (dst >= X86_64_R8)
                        {
                            rex |= 0x04;
                        }
                        emit_byte(ctx, rex);
                        emit_byte(ctx, 0x8B);
                        uint8_t modrm = 0x44 | (reg_to_x86_encoding(dst) << 3);
                        emit_byte(ctx, modrm);
                        emit_byte(ctx, 0x24);
                        emit_byte(ctx, (uint8_t)stack_offset);
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
                    emit_byte(ctx, 0x48);
                    emit_byte(ctx, 0x8D);
                    emit_byte(ctx, 0x05 | (reg_to_x86_encoding(dst) << 3));
                    uint64_t reloc_offset = ctx->code.size;
                    emit_dword(ctx, 0);
                    add_relocation(ctx, reloc_offset, inst->operands[i].global_name, 2, -4);
                }
            }

            if (func_name)
            {
                emit_byte(ctx, 0xE8);
                uint64_t reloc_offset = ctx->code.size;
                emit_dword(ctx, 0);
                add_relocation(ctx, reloc_offset, func_name, 4, -4);
            }

            // clean up stack: add rsp, (num_saved + num_stack_args) * 8
            // NOTE: stack args are pushed BEFORE saved regs, so they are deeper in the stack
            // but we pop everything at once
            size_t total_cleanup = (num_saved + num_stack_args) * 8;
            if (total_cleanup > 0)
            {
                if (total_cleanup <= 127)
                {
                    emit_byte(ctx, 0x48);
                    emit_byte(ctx, 0x83);
                    emit_byte(ctx, 0xC4);
                    emit_byte(ctx, (uint8_t)total_cleanup);
                }
                else
                {
                    emit_byte(ctx, 0x48);
                    emit_byte(ctx, 0x81);
                    emit_byte(ctx, 0xC4);
                    emit_dword(ctx, (uint32_t)total_cleanup);
                }
            }

            // result is in rax - if instruction has result, move to allocated register
            if (inst->result)
            {
                X86_64_Reg dst = get_physical_reg(ctx, inst->result->id, X86_64_R11);
                if (dst != X86_64_RAX)
                {
                    emit_mov_reg_reg(ctx, dst, X86_64_RAX);
                }
                emit_spill_store(ctx, inst->result, dst);
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

    // allocate stack frame
    // ensure 16-byte alignment relative to return address (ABI requirement)
    // current rsp is 8-byte aligned (after 6 pushes: rbp + 5 callee-saved regs)
    // we need (alloc_size % 16) == 8 to restore 16-byte alignment
    size_t aligned_size = ((func->frame_size + 8 + 15) & ~15) - 8;

    if (aligned_size > 0)
    {
        // sub rsp, aligned_size using rex.w + 81 /5
        emit_byte(ctx, 0x48);
        emit_byte(ctx, 0x81);
        emit_byte(ctx, 0xEC); // modrm: 11 101 100 (rsp)
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
