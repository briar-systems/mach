#include "compiler/masm/lower.h"
#include "compiler/masm/abi/spec.h"
#include "compiler/masm/instruction.h"
#include "compiler/masm/isa/spec.h"
#include "compiler/masm/isa/x86_64.h"
#include "compiler/masm/masm.h"
#include "compiler/masm/regalloc.h"
#include "compiler/symbol.h"
#include "compiler/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// local variable tracking
typedef struct LocalVar
{
    const char *name;
    int32_t     offset; // offset from RBP (negative for locals)
    uint8_t     size;
} LocalVar;

typedef struct LowerContext
{
    MasmTarget         target;
    const MasmISASpec *isa;
    const MasmABISpec *abi;
    uint8_t            ptr_size;
    uint8_t            stack_align;
    uint8_t            int_arg_count;
    uint8_t            float_arg_count;
    uint32_t           fp_reg;
    uint32_t           sp_reg;

    LocalVar *vars;
    int       var_count;
    int       var_capacity;
    int32_t   stack_offset;
    AstList  *local_vars; // List of LocalVar

    // deferred statements (fin)
    AstNode **deferred;
    int       deferred_count;
    int       deferred_capacity;

    // loop defer stack markers (index into deferred stack)
    int *loop_defer_stack;
    int  loop_defer_count;
    int  loop_defer_capacity;

    // Loop labels for break/continue
    char *loop_start_label;
    char *loop_end_label;

    // Register allocator
    RegAlloc regalloc;

    // current function return handling
    Type   *fn_ret_type;
    bool    fn_has_sret;
    int32_t sret_offset; // stack slot holding hidden sret pointer (rbp-relative)

    // variadic function lowering state
    bool    fn_is_variadic;
    int32_t va_reg_save_off;      // rbp-relative base offset for reg_save_area
    int     va_named_gp;          // named gp regs consumed (excluding sret)
    int     va_named_fp;          // named fp regs consumed
    int     va_named_stack_slots; // named params passed on stack
} LowerContext;

// Select ISA spec via shared selector
static const MasmISASpec *lower_select_isa(MasmTarget target)
{
    return masm_isa_spec_select(target);
}

static inline MasmOperand isa_result(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_result(size);
}
static inline MasmOperand isa_tmp(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_tmp0(size);
}
static inline __attribute__((unused)) MasmOperand isa_tmp2(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_tmp1(size);
}
static inline __attribute__((unused)) MasmOperand isa_div_hi(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_div_hi(size);
}
static inline __attribute__((unused)) MasmOperand isa_div_lo(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_div_lo(size);
}
static inline __attribute__((unused)) MasmOperand isa_arg0(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_arg(0, size);
}
static inline __attribute__((unused)) MasmOperand isa_arg1(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_arg(1, size);
}
static inline __attribute__((unused)) MasmOperand isa_sp(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_sp(size);
}
static inline __attribute__((unused)) MasmOperand isa_fp(LowerContext *ctx, uint8_t size)
{
    return ctx->isa->reg_fp(size);
}
static inline uint32_t isa_result_id(LowerContext *ctx)
{
    return isa_result(ctx, ctx->ptr_size).reg.id;
}
static inline __attribute__((unused)) uint32_t isa_tmp_id(LowerContext *ctx)
{
    return isa_tmp(ctx, ctx->ptr_size).reg.id;
}

static LowerContext *create_context(MasmTarget target)
{
    LowerContext *ctx = malloc(sizeof(LowerContext));
    ctx->isa          = lower_select_isa(target);
    if (!ctx->isa)
    {
        fprintf(stderr, "masm lower: unsupported isa for lowering (isa=%s)\n", masm_target_isa_name(target.isa));
        exit(1);
    }
    ctx->abi = masm_abi_spec_select(target);
    if (!ctx->abi)
    {
        fprintf(stderr, "masm lower: unsupported abi for lowering (abi=%s)\n", masm_target_abi_name(target.abi));
        exit(1);
    }
    ctx->target          = target;
    ctx->ptr_size        = ctx->abi->pointer_size;
    ctx->stack_align     = ctx->abi->stack_align;
    ctx->int_arg_count   = ctx->abi->int_arg_count;
    ctx->float_arg_count = ctx->abi->float_arg_count;
    ctx->fp_reg          = ctx->isa->reg_fp(ctx->ptr_size).reg.id;
    ctx->sp_reg          = ctx->isa->reg_sp(ctx->ptr_size).reg.id;
    if (ctx->fp_reg == UINT32_MAX || ctx->sp_reg == UINT32_MAX)
    {
        fprintf(stderr, "masm lower: unsupported fp/sp reg for isa %s\n", masm_target_isa_name(target.isa));
        exit(1);
    }
    ctx->vars         = malloc(sizeof(LocalVar) * 16);
    ctx->var_count    = 0;
    ctx->var_capacity = 16;
    ctx->stack_offset = 0;
    ctx->local_vars   = malloc(sizeof(AstList));
    ast_list_init(ctx->local_vars);

    ctx->deferred          = malloc(sizeof(AstNode *) * 8);
    ctx->deferred_count    = 0;
    ctx->deferred_capacity = 8;

    ctx->loop_defer_stack    = malloc(sizeof(int) * 8);
    ctx->loop_defer_count    = 0;
    ctx->loop_defer_capacity = 8;
    ctx->loop_start_label    = NULL;
    ctx->loop_end_label      = NULL;
    regalloc_init(&ctx->regalloc, ctx->isa, ctx->fp_reg, ctx->sp_reg, ctx->ptr_size);

    ctx->fn_ret_type          = NULL;
    ctx->fn_has_sret          = false;
    ctx->sret_offset          = 0;
    ctx->fn_is_variadic       = false;
    ctx->va_reg_save_off      = 0;
    ctx->va_named_gp          = 0;
    ctx->va_named_fp          = 0;
    ctx->va_named_stack_slots = 0;
    return ctx;
}

static void destroy_context(LowerContext *ctx)
{
    if (ctx)
    {
        free(ctx->vars);
        free(ctx->deferred);
        free(ctx->loop_defer_stack);
        free(ctx->local_vars);
        free(ctx->regalloc.in_use);
        free(ctx);
    }
}

static inline MasmOperand fp_reg_op(LowerContext *ctx)
{
    return masm_operand_register(ctx->fp_reg, ctx->ptr_size);
}

static inline MasmOperand sp_reg_op(LowerContext *ctx)
{
    return masm_operand_register(ctx->sp_reg, ctx->ptr_size);
}

static inline MasmOperand frame_mem(LowerContext *ctx, int32_t offset, uint8_t size)
{
    return masm_operand_memory_simple(ctx->fp_reg, offset, size);
}

// forward decl (used by helpers below)
static MasmOperand lower_expr(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx);

static inline bool type_is_large_aggregate(Type *t, uint8_t ptr_size)
{
    return t && (t->kind == TYPE_STRUCT || t->kind == TYPE_UNION || t->kind == TYPE_ARRAY) && t->size > ptr_size;
}

static inline bool type_is_fp_class(Type *t)
{
    return t && (t->kind == TYPE_F32 || t->kind == TYPE_F64);
}

static inline uint8_t type_fp_size(Type *t)
{
    if (!t)
    {
        return 8;
    }
    return (t->kind == TYPE_F32) ? 4 : 8;
}

static inline MasmOperand masm_xmm(uint32_t xmm_id)
{
    return masm_operand_register(xmm_id, 16);
}

static void materialize_bits_to_gpr(MasmSection *text, LowerContext *ctx, MasmOperand dst, MasmOperand src, uint8_t size)
{
    (void)ctx;

    if (dst.kind != MASM_OPERAND_REGISTER)
    {
        return;
    }

    dst.reg.size = size;

    if (src.kind == MASM_OPERAND_REGISTER)
    {
        MasmOperand s = src;
        s.reg.size    = size;
        if (s.reg.id != dst.reg.id || s.reg.size != dst.reg.size)
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, s));
        }
        return;
    }
    else if (src.kind == MASM_OPERAND_MEMORY)
    {
        MasmOperand m = src;
        m.mem.size    = size;
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, m));
        return;
    }
    else if (src.kind == MASM_OPERAND_IMM || src.kind == MASM_OPERAND_LABEL)
    {
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
        return;
    }

    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
}

static inline int32_t align_up_i32(int32_t v, int32_t align)
{
    if (align <= 1)
    {
        return v;
    }
    int32_t r = v % align;
    return r ? (v + (align - r)) : v;
}

// SysV x86_64 va_list layout:
//   +0  u32 gp_offset
//   +4  u32 fp_offset
//   +8  ptr overflow_arg_area
//   +16 ptr reg_save_area
static MasmOperand va_load_gp_slot(Masm *masm, MasmSection *text, LowerContext *ctx, MasmOperand ap, uint8_t load_size)
{
    static int label_counter = 0;
    int        id            = label_counter++;

    char overflow_label[48];
    char end_label[48];
    snprintf(overflow_label, sizeof(overflow_label), ".Lva_gp_overflow_%d", id);
    snprintf(end_label, sizeof(end_label), ".Lva_gp_end_%d", id);

    // gp_offset
    MasmOperand gp_off32 = isa_tmp(ctx, 4);
    MasmOperand gp_mem   = masm_operand_memory_simple(ap.reg.id, 0, 4);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, gp_off32, gp_mem));

    // if gp_offset >= 48 => overflow
    masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, gp_off32, masm_operand_imm(48)));
    masm_section_append_inst(text, masm_inst_1(MASM_OP_JGE, masm_operand_label(strdup(overflow_label))));

    // reg_save_area + gp_offset
    MasmOperand reg_save = isa_tmp2(ctx, ctx->ptr_size);
    MasmOperand rsa_mem  = masm_operand_memory_simple(ap.reg.id, 16, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, reg_save, rsa_mem));

    MasmOperand off64 = masm_operand_register(gp_off32.reg.id, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, reg_save, off64));

    MasmOperand out     = isa_result(ctx, ctx->ptr_size);
    MasmOperand out_ret = masm_operand_register(out.reg.id, load_size);
    MasmOperand src     = masm_operand_memory_simple(reg_save.reg.id, 0, load_size);
    if (load_size == 1 || load_size == 2)
    {
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, out, src));
    }
    else
    {
        // mov into a 32-bit reg zero-extends on x86_64; treat result as ptr-sized.
        MasmOperand out_n = masm_operand_register(out.reg.id, (load_size == 4) ? 4 : ctx->ptr_size);
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, out_n, src));
    }

    // gp_offset += 8
    masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, gp_off32, masm_operand_imm(8)));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, gp_mem, gp_off32));

    masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(end_label))));

    // overflow path
    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(overflow_label))));
    masm_add_symbol(masm, masm_symbol_create(overflow_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

    MasmOperand overflow_ptr = isa_tmp(ctx, ctx->ptr_size);
    MasmOperand ov_mem       = masm_operand_memory_simple(ap.reg.id, 8, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, overflow_ptr, ov_mem));

    MasmOperand ov_src = masm_operand_memory_simple(overflow_ptr.reg.id, 0, load_size);
    if (load_size == 1 || load_size == 2)
    {
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, out, ov_src));
    }
    else
    {
        MasmOperand out_n = masm_operand_register(out.reg.id, (load_size == 4) ? 4 : ctx->ptr_size);
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, out_n, ov_src));
    }

    // overflow_arg_area += 8
    masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, overflow_ptr, masm_operand_imm(8)));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, ov_mem, overflow_ptr));

    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));
    masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

    return out_ret;
}

static MasmOperand va_load_fp_slot(Masm *masm, MasmSection *text, LowerContext *ctx, MasmOperand ap, uint8_t load_size)
{
    static int label_counter = 0;
    int        id            = label_counter++;

    char overflow_label[48];
    char end_label[48];
    snprintf(overflow_label, sizeof(overflow_label), ".Lva_fp_overflow_%d", id);
    snprintf(end_label, sizeof(end_label), ".Lva_fp_end_%d", id);

    // fp_offset
    MasmOperand fp_off32 = isa_tmp(ctx, 4);
    MasmOperand fp_mem   = masm_operand_memory_simple(ap.reg.id, 4, 4);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, fp_off32, fp_mem));

    // if fp_offset >= 176 => overflow
    masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, fp_off32, masm_operand_imm(176)));
    masm_section_append_inst(text, masm_inst_1(MASM_OP_JGE, masm_operand_label(strdup(overflow_label))));

    // reg_save_area + fp_offset
    MasmOperand reg_save = isa_tmp2(ctx, ctx->ptr_size);
    MasmOperand rsa_mem  = masm_operand_memory_simple(ap.reg.id, 16, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, reg_save, rsa_mem));

    MasmOperand off64 = masm_operand_register(fp_off32.reg.id, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, reg_save, off64));

    MasmOperand out = isa_result(ctx, load_size);
    MasmOperand src = masm_operand_memory_simple(reg_save.reg.id, 0, load_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, out, src));

    // fp_offset += 16
    masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, fp_off32, masm_operand_imm(16)));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, fp_mem, fp_off32));

    masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(end_label))));

    // overflow path
    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(overflow_label))));
    masm_add_symbol(masm, masm_symbol_create(overflow_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

    MasmOperand overflow_ptr = isa_tmp(ctx, ctx->ptr_size);
    MasmOperand ov_mem       = masm_operand_memory_simple(ap.reg.id, 8, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, overflow_ptr, ov_mem));

    MasmOperand ov_src = masm_operand_memory_simple(overflow_ptr.reg.id, 0, load_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, out, ov_src));

    // overflow_arg_area += 8
    masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, overflow_ptr, masm_operand_imm(8)));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, ov_mem, overflow_ptr));

    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));
    masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

    return out;
}

static Type *infer_ret_type_from_stmt(AstNode *stmt)
{
    if (!stmt)
    {
        return NULL;
    }

    switch (stmt->kind)
    {
    case AST_STMT_RET:
        if (stmt->ret_stmt.expr && stmt->ret_stmt.expr->type)
        {
            return stmt->ret_stmt.expr->type;
        }
        return NULL;

    case AST_STMT_BLOCK:
        if (stmt->block_stmt.stmts)
        {
            for (int i = 0; i < stmt->block_stmt.stmts->count; i++)
            {
                Type *t = infer_ret_type_from_stmt(stmt->block_stmt.stmts->items[i]);
                if (t)
                {
                    return t;
                }
            }
        }
        return NULL;

    case AST_STMT_IF:
    case AST_STMT_OR:
    {
        Type *t = infer_ret_type_from_stmt(stmt->cond_stmt.body);
        if (t)
        {
            return t;
        }
        return infer_ret_type_from_stmt(stmt->cond_stmt.stmt_or);
    }

    case AST_STMT_FOR:
        return infer_ret_type_from_stmt(stmt->for_stmt.body);

    case AST_STMT_COMPTIME_IF:
    case AST_STMT_COMPTIME_OR:
        // sema selects the active branch in taken_branch.
        return infer_ret_type_from_stmt(stmt->comptime_if_stmt.taken_branch);

    default:
        return NULL;
    }
}

// internal ABI: pass large aggregates by pointer to a caller-created copy.
// returns an operand holding that pointer (in a register).
static MasmOperand lower_byval_arg_ptr(Masm *masm, MasmSection *text, AstNode *arg_expr, LowerContext *ctx)
{
    if (!arg_expr || !arg_expr->type || !type_is_large_aggregate(arg_expr->type, ctx->ptr_size))
    {
        return masm_operand_none();
    }

    size_t size = arg_expr->type->size;
    if (size == 0)
    {
        return masm_operand_none();
    }

    // reserve stack storage for the byval copy
    ctx->stack_offset -= (int32_t)size;
    int align = ctx->ptr_size ? ctx->ptr_size : 1;
    if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
    {
        ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
    }

    int32_t buf_offset = ctx->stack_offset;

    // normalize source value into an address in rax
    MasmOperand src_val = lower_expr(masm, text, arg_expr, ctx);
    MasmOperand src_ptr = isa_result(ctx, ctx->ptr_size);
    if (src_val.kind == MASM_OPERAND_REGISTER)
    {
        if (src_val.reg.id != src_ptr.reg.id)
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_val));
        }
    }
    else if (src_val.kind == MASM_OPERAND_MEMORY)
    {
        masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, src_ptr, src_val));
    }
    else if (src_val.kind == MASM_OPERAND_LABEL)
    {
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_val));
    }
    else if (src_val.kind == MASM_OPERAND_IMM)
    {
        // shouldn't happen for aggregates, but keep codegen safe.
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_val));
    }
    else
    {
        return masm_operand_none();
    }

    // compute destination address into tmp reg
    MasmOperand dst_ptr  = isa_tmp(ctx, ctx->ptr_size);
    MasmOperand dst_addr = frame_mem(ctx, buf_offset, ctx->ptr_size);
    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst_ptr, dst_addr));

    // copy the aggregate into the byval buffer
    for (int32_t off = 0; off < (int32_t)size;)
    {
        int32_t chunk = (int32_t)size - off;
        if (chunk > 8)
        {
            chunk = 8;
        }
        else if (chunk > 4)
        {
            chunk = 4;
        }
        else if (chunk > 2)
        {
            chunk = 2;
        }
        else
        {
            chunk = 1;
        }

        MasmOperand tmp = isa_tmp2(ctx, (uint32_t)chunk);
        MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
        MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
        off += chunk;
    }

    return dst_ptr;
}

static void        lower_stmt(Masm *masm, MasmSection *text, AstNode *stmt, LowerContext *ctx);
static MasmOperand lower_expr(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx);
static void        lower_inline_masm(Masm *masm, MasmSection *text, const char *content, LowerContext *ctx);
static MasmOperand parse_operand(const char *str, LowerContext *ctx);

static void push_deferred(LowerContext *ctx, AstNode *stmt)
{
    if (ctx->deferred_count >= ctx->deferred_capacity)
    {
        ctx->deferred_capacity *= 2;
        ctx->deferred = realloc(ctx->deferred, sizeof(AstNode *) * ctx->deferred_capacity);
    }
    ctx->deferred[ctx->deferred_count++] = stmt;
}

static void emit_deferred_from(Masm *masm, MasmSection *text, LowerContext *ctx, int start_index)
{
    for (int i = ctx->deferred_count - 1; i >= start_index; i--)
    {
        lower_stmt(masm, text, ctx->deferred[i], ctx);
    }
    ctx->deferred_count = start_index;
}

static void push_loop_defer_mark(LowerContext *ctx, int mark)
{
    if (ctx->loop_defer_count >= ctx->loop_defer_capacity)
    {
        ctx->loop_defer_capacity *= 2;
        ctx->loop_defer_stack = realloc(ctx->loop_defer_stack, sizeof(int) * ctx->loop_defer_capacity);
    }
    ctx->loop_defer_stack[ctx->loop_defer_count++] = mark;
}

static void pop_loop_defer_mark(LowerContext *ctx)
{
    if (ctx->loop_defer_count > 0)
    {
        ctx->loop_defer_count--;
    }
}

static void add_local_var(LowerContext *ctx, const char *name, int32_t offset, uint8_t size)
{
    if (ctx->var_count >= ctx->var_capacity)
    {
        ctx->var_capacity *= 2;
        ctx->vars = realloc(ctx->vars, sizeof(LocalVar) * ctx->var_capacity);
    }
    ctx->vars[ctx->var_count].name   = strdup(name);
    ctx->vars[ctx->var_count].offset = offset;
    ctx->vars[ctx->var_count].size   = size;
    ctx->var_count++;
}

static LocalVar *find_local_var(LowerContext *ctx, const char *name)
{
    for (int i = 0; i < ctx->var_count; i++)
    {
        if (strcmp(ctx->vars[i].name, name) == 0)
        {
            return &ctx->vars[i];
        }
    }
    return NULL;
}

static MasmOperand lower_expr(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx)
{
    (void)masm;
    if (!expr)
    {
        return masm_operand_none();
    }

    if (expr->kind == AST_COMPTIME)
    {
        // lowered comptime values become plain immediates/addresses in the emitted code.
        // sema is responsible for evaluating and storing the result in the node.
        if (expr->comptime.value_kind == COMPTIME_INT)
        {
            return masm_operand_imm((int64_t)expr->comptime.int_value);
        }
        else if (expr->comptime.value_kind == COMPTIME_STRING)
        {
            // treat as a string literal address
            if (!expr->comptime.string_value)
            {
                return masm_operand_none();
            }

            static int str_counter = 0;
            char       label[32];
            snprintf(label, sizeof(label), ".Lstr_%d", str_counter++);

            MasmSection *rodata = masm_get_or_create_section(masm, ".rodata", MASM_SECTION_RODATA);

            MasmSymbol *sym   = masm_symbol_create(label, MASM_SYMBOL_DATA, MASM_BIND_LOCAL);
            sym->section_name = strdup(".rodata");
            sym->offset       = rodata->data_size;
            size_t len        = strlen(expr->comptime.string_value);
            sym->size         = len + 1;
            masm_add_symbol(masm, sym);

            masm_section_append_data(rodata, expr->comptime.string_value, len);
            uint8_t zero = 0;
            masm_section_append_data(rodata, &zero, 1);

            MasmOperand res = isa_result(ctx, 8);
            MasmOperand src = masm_operand_label(strdup(label));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, res, src));
            return res;
        }

        // unevaluated comptime should not reach lowering.
        return masm_operand_none();
    }

    if (expr->kind == AST_EXPR_LIT)
    {
        if (expr->lit_expr.kind == TOKEN_LIT_INT)
        {
            return masm_operand_imm((int64_t)expr->lit_expr.int_val);
        }
        else if (expr->lit_expr.kind == TOKEN_LIT_CHAR)
        {
            return masm_operand_imm((int64_t)(uint8_t)expr->lit_expr.char_val);
        }
        else if (expr->lit_expr.kind == TOKEN_LIT_STRING)
        {
#ifdef MASM_DEBUG
            fprintf(stderr, "[lower] string literal '%s'\n", expr->lit_expr.string_val);
#endif
            // Create unique label
            static int str_counter = 0;
            char       label[32];
            snprintf(label, sizeof(label), ".Lstr_%d", str_counter++);

            // Add to .rodata
            MasmSection *rodata = masm_get_or_create_section(masm, ".rodata", MASM_SECTION_RODATA);

            // Add label symbol
            MasmSymbol *sym   = masm_symbol_create(label, MASM_SYMBOL_DATA, MASM_BIND_LOCAL);
            sym->section_name = strdup(".rodata");
            sym->offset       = rodata->data_size;
            size_t len        = strlen(expr->lit_expr.string_val);
            sym->size         = len + 1;
            masm_add_symbol(masm, sym);

            // append string data (null-terminated) to .rodata
            masm_section_append_data(rodata, expr->lit_expr.string_val, len);
            uint8_t zero = 0;
            masm_section_append_data(rodata, &zero, 1);

            // MOV result, label (absolute address)
            MasmOperand res = isa_result(ctx, 8);
            MasmOperand src = masm_operand_label(strdup(label));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, res, src));
            return res;
        }
    }
    else if (expr->kind == AST_EXPR_CAST)
    {
        MasmOperand inner = lower_expr(masm, text, expr->cast_expr.expr, ctx);
        uint8_t     size  = (expr->type && expr->type->size) ? expr->type->size : 8;

        if (inner.kind == MASM_OPERAND_REGISTER)
        {
            inner.reg.size = size;
            return inner;
        }
        else if (inner.kind == MASM_OPERAND_IMM)
        {
            return inner; // keep immediate; size handled by consumers
        }
        else if (inner.kind == MASM_OPERAND_MEMORY)
        {
            MasmOperand dst = isa_result(ctx, size > 8 ? 8 : size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, inner));
            return dst;
        }
        return inner;
    }
    else if (expr->kind == AST_EXPR_NULL)
    {
        // nil literal
        return masm_operand_imm(0);
    }
    else if (expr->kind == AST_EXPR_IDENT)
    {
        // variable access - load from stack
        LocalVar *var = find_local_var(ctx, expr->ident_expr.name);
        if (var)
        {
            // arrays/large aggregates: return address with LEA
            if (var->size > 8)
            {
                MasmOperand result = isa_result(ctx, 8);
                MasmOperand addr   = frame_mem(ctx, var->offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, result, addr));
                return result;
            }

            // scalar: load value
            MasmOperand var_mem = frame_mem(ctx, var->offset, var->size);
            if (var->size == 1 || var->size == 2)
            {
                MasmOperand res = isa_result(ctx, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, res, var_mem));
                return res;
            }

            MasmOperand result = isa_result(ctx, var->size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, var_mem));
            return result;
        }
        else
        {
            // Check global
            MasmSymbol *sym = masm_get_symbol(masm, expr->ident_expr.name);
            if (sym)
            {
                // For aggregates/arrays, return address in RAX
                MasmOperand addr     = isa_result(ctx, 8);
                MasmOperand label_op = masm_operand_label(strdup(sym->name));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, addr, label_op));

                if (sym->size > 8)
                {
                    return addr;
                }

                // scalar: load value from address
                MasmOperand mem    = masm_operand_memory_simple(isa_result(ctx, ctx->ptr_size).reg.id, 0, sym->size > 8 ? 8 : sym->size);
                MasmOperand result = isa_result(ctx, sym->size > 8 ? 8 : sym->size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, mem));

                return result;
            }

            // fallback: use the resolved symbol linkage name even if the symbol is defined
            // in a different lowered module (e.g. `true`/`false` from std.types.bool).
            if (expr->symbol)
            {
                const char *link_name = symbol_get_linkage_name(expr->symbol);
                if (!link_name)
                {
                    link_name = expr->symbol->name;
                }

                if (link_name)
                {
                    size_t sym_size = ctx->ptr_size;
                    if (expr->type && expr->type->size)
                    {
                        sym_size = expr->type->size;
                    }
                    else if (expr->symbol->type && expr->symbol->type->size)
                    {
                        sym_size = expr->symbol->type->size;
                    }

                    MasmOperand addr     = isa_result(ctx, ctx->ptr_size);
                    MasmOperand label_op = masm_operand_label(strdup(link_name));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, addr, label_op));

                    if (sym_size > 8)
                    {
                        return addr;
                    }

                    MasmOperand mem = masm_operand_memory_simple(addr.reg.id, 0, sym_size ? sym_size : 1);

                    if (sym_size == 1 || sym_size == 2)
                    {
                        MasmOperand res = isa_result(ctx, 8);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, res, mem));
                        return res;
                    }

                    MasmOperand res = isa_result(ctx, sym_size ? (uint8_t)sym_size : ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, res, mem));
                    return res;
                }
            }
        }
        // if not found, fall through to return none
    }
    else if (expr->kind == AST_EXPR_BINARY)
    {
        // Short-circuit operators
        if (expr->binary_expr.op == TOKEN_AMPERSAND_AMPERSAND)
        {
            static int label_counter = 0;
            char       false_label[32];
            char       end_label[32];
            snprintf(false_label, sizeof(false_label), ".Land_false_%d", label_counter);
            snprintf(end_label, sizeof(end_label), ".Land_end_%d", label_counter++);

            // evaluate left
            MasmOperand left   = lower_expr(masm, text, expr->binary_expr.left, ctx);
            MasmOperand result = isa_result(ctx, 8);

            if (left.kind != MASM_OPERAND_REGISTER || left.reg.id != isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }

            // check left
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, result, result));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(false_label))));

            // evaluate right
            MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);
            if (right.kind != MASM_OPERAND_REGISTER || right.reg.id != isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, right));
            }

            // check right
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, result, result));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(false_label))));

            // true
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, masm_operand_imm(1)));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(end_label))));

            // false
            masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(false_label))));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, masm_operand_imm(0)));

            masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));

            masm_add_symbol(masm, masm_symbol_create(false_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
            masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

            return result;
        }
        else if (expr->binary_expr.op == TOKEN_PIPE_PIPE)
        {
            static int label_counter = 0;
            char       true_label[32];
            char       end_label[32];
            snprintf(true_label, sizeof(true_label), ".Lor_true_%d", label_counter);
            snprintf(end_label, sizeof(end_label), ".Lor_end_%d", label_counter++);

            // evaluate left
            MasmOperand left   = lower_expr(masm, text, expr->binary_expr.left, ctx);
            MasmOperand result = isa_result(ctx, 8);

            if (left.kind != MASM_OPERAND_REGISTER || left.reg.id != isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }

            // check left
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, result, result));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JNE, masm_operand_label(strdup(true_label))));

            // evaluate right
            MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);
            if (right.kind != MASM_OPERAND_REGISTER || right.reg.id != isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, right));
            }

            // check right
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, result, result));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JNE, masm_operand_label(strdup(true_label))));

            // false
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, masm_operand_imm(0)));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(end_label))));

            // true
            masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(true_label))));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, masm_operand_imm(1)));

            masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));

            masm_add_symbol(masm, masm_symbol_create(true_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
            masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

            return result;
        }

        else if (expr->binary_expr.op == TOKEN_EQUAL)
        {
            // Assignment: left = right

            // Handle dereference assignment: @ptr = val
            if (expr->binary_expr.left->kind == AST_EXPR_UNARY)
            {
                if (expr->binary_expr.left->unary_expr.op == TOKEN_AT)
                {
                    // 1. Evaluate ptr -> result reg
                    MasmOperand ptr = lower_expr(masm, text, expr->binary_expr.left->unary_expr.expr, ctx);

                    if (ptr.kind != MASM_OPERAND_REGISTER)
                    {
                        MasmOperand r_res = isa_result(ctx, 8);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, r_res, ptr));
                        ptr = r_res;
                    }

                    // Push ptr
                    masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, ptr));

                    // 2. Evaluate RHS -> RAX
                    MasmOperand val = lower_expr(masm, text, expr->binary_expr.right, ctx);

                    int store_size = 8;
                    if (expr->binary_expr.left->type && expr->binary_expr.left->type->size > 0)
                    {
                        store_size = expr->binary_expr.left->type->size;
                        if (store_size > 8)
                        {
                            store_size = 8;
                        }
                    }

                    // Move val to tmp with correct width
                    MasmOperand val_reg = isa_tmp(ctx, store_size);
                    if (val.kind == MASM_OPERAND_REGISTER && val.reg.id == val_reg.reg.id && val.reg.size == store_size)
                    {
                        // already correct register/width
                    }
                    else
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                    }

                    // 3. Pop ptr -> result
                    MasmOperand ptr_reg = isa_result(ctx, 8);
                    masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, ptr_reg));

                    // 4. Store [ptr] = val
                    int         size = store_size;
                    MasmOperand dst  = masm_operand_memory_simple(ptr_reg.reg.id, 0, size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val_reg));

                    // Return val moved to result
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, ptr_reg, val_reg));
                    return ptr_reg;
                }
            }
            else if (expr->binary_expr.left->kind == AST_EXPR_INDEX || expr->binary_expr.left->kind == AST_EXPR_FIELD)
            {
                // Generic lvalue assignment (index, field)
                // 1. Evaluate LHS to memory operand
                MasmOperand left_mem = lower_expr(masm, text, expr->binary_expr.left, ctx);

                if (left_mem.kind != MASM_OPERAND_MEMORY)
                {
                    fprintf(stderr, "masm lower: unsupported lvalue in assignment\n");
                    exit(1);
                }

                // 2. Load effective address into result
                MasmOperand addr = isa_result(ctx, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, addr, left_mem));

                // 3. Push address
                masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, addr));

                // 4. Evaluate RHS
                MasmOperand val = lower_expr(masm, text, expr->binary_expr.right, ctx);

                // 5. Move val to tmp (to free result) using correct width
                int store_size = expr->binary_expr.left->type ? expr->binary_expr.left->type->size : 8;
                if (store_size == 0)
                {
                    store_size = 8;
                }
                if (store_size > 8)
                {
                    store_size = 8;
                }

                MasmOperand val_reg = isa_tmp(ctx, store_size);
                if (val.kind == MASM_OPERAND_REGISTER && val.reg.id == val_reg.reg.id && val.reg.size == store_size)
                {
                    // already correct
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                }

                // 6. Pop address to result
                masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, addr));

                // 7. Store [addr] = val
                int size = expr->binary_expr.left->type->size;
                if (size == 0)
                {
                    size = store_size;
                }
                MasmOperand dst = masm_operand_memory_simple(addr.reg.id, 0, size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val_reg));

                // 8. Return result
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, addr, val_reg));
                return addr;
            }

            MasmOperand right_val = lower_expr(masm, text, expr->binary_expr.right, ctx);

            if (expr->binary_expr.left->kind == AST_EXPR_IDENT)
            {
                LocalVar *var = find_local_var(ctx, expr->binary_expr.left->ident_expr.name);
                if (var)
                {
                    MasmOperand var_mem = frame_mem(ctx, var->offset, var->size);

                    // materialize RHS into RAX for the expression result, then store using the lvalue width.
                    MasmOperand rax = isa_result(ctx, ctx->ptr_size);
                    if (!(right_val.kind == MASM_OPERAND_REGISTER && right_val.reg.id == isa_result_id(ctx)))
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, right_val));
                    }

                    MasmOperand store_src = rax;
                    if (var->size < rax.reg.size)
                    {
                        store_src = masm_operand_register(rax.reg.id, (uint8_t)var->size);
                    }

                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, store_src));
                    return rax;
                }
            }
            return masm_operand_none();
        }

        MasmOperand left = lower_expr(masm, text, expr->binary_expr.left, ctx);

        // if the left operand is a memory reference, materialize it into a register
        // before evaluating the right operand. this avoids clobbering the memory base
        // register during right-side lowering (common for field/index expressions).
        if (left.kind == MASM_OPERAND_MEMORY)
        {
            uint8_t lsz = left.mem.size ? left.mem.size : ctx->ptr_size;
            if (lsz == 1 || lsz == 2)
            {
                MasmOperand r = isa_result(ctx, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, r, left));
                left = r;
            }
            else
            {
                MasmOperand r = isa_result(ctx, lsz > 8 ? 8 : lsz);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, r, left));
                left = r;
            }
        }

        // try to use register allocator to avoid PUSH/POP spilling
        bool     pushed        = false;
        uint32_t left_save_reg = UINT32_MAX;

        if (left.kind == MASM_OPERAND_REGISTER && left.reg.id == isa_result_id(ctx))
        {
            // try to allocate a register to save left value
            left_save_reg = regalloc_alloc(&ctx->regalloc);
            if (left_save_reg != UINT32_MAX)
            {
                // move left to allocated register
                MasmOperand save_reg = masm_operand_register(left_save_reg, left.reg.size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, save_reg, left));
                left = save_reg;
            }
            else
            {
                // fallback to PUSH/POP
                MasmOperand push_op = left;
                push_op.reg.size    = 8;
                masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, push_op));
                pushed = true;
            }
        }

        MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);

        MasmOperand result = isa_result(ctx, 8);
        if (left.kind == MASM_OPERAND_REGISTER)
        {
            result.reg.size = left.reg.size;
        }
        else if (left.kind == MASM_OPERAND_MEMORY)
        {
            result.reg.size = left.mem.size;
        }

        MasmOperand right_op = right;
        MasmOperand rcx      = isa_tmp(ctx, result.reg.size);

        if (pushed)
        {
            // right is in RAX (if register) or immediate.
            // Move right to RCX to free up RAX for left.
            if (right.kind == MASM_OPERAND_REGISTER && right.reg.id == isa_result_id(ctx))
            {
                // Ensure right operand size matches RCX size for MOV
                MasmOperand src = right;
                if (src.reg.size != rcx.reg.size)
                {
                    src.reg.size = rcx.reg.size; // Assume zero extension/truncation handled by MOV
                }

                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, src));
                right_op = rcx;
            }
            else if (right.kind == MASM_OPERAND_IMM)
            {
                // Load immediate into RCX to be safe and uniform
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, right));
                right_op = rcx;
            }

            // Pop left back into RAX (POP r32 is invalid)
            MasmOperand pop_op = result;
            pop_op.reg.size    = 8;
            masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, pop_op));
        }
        else if (left_save_reg != UINT32_MAX)
        {
            // left is in an allocated register, right may be in RAX
            // We want result in RAX for compatibility with existing code

            // If right is in RAX, move it to RCX
            if (right.kind == MASM_OPERAND_REGISTER && right.reg.id == isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, right));
                right_op = rcx;
            }

            // Move left from save register to RAX
            MasmOperand save_reg = masm_operand_register(left_save_reg, result.reg.size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, save_reg));

            // now that we've restored left, the save register can be reused.
            regalloc_free(&ctx->regalloc, left_save_reg);
        }
        else
        {
            // left was immediate (or other reg).
            // right is in RAX (if complex).

            // We want left in RAX.
            // If right is in RAX, move it to RCX first.
            if (right.kind == MASM_OPERAND_REGISTER && right.reg.id == isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, right));
                right_op = rcx;
            }

            // Load left into RAX
            if (left.kind == MASM_OPERAND_IMM)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }
            else if (left.kind == MASM_OPERAND_REGISTER && left.reg.id != isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }
            else if (left.kind == MASM_OPERAND_MEMORY)
            {
                if (left.mem.size == 1 || left.mem.size == 2)
                {
                    MasmOperand r_res64 = isa_result(ctx, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, r_res64, left));
                }
                else
                {
                    MasmOperand mov_dst = result;
                    if (left.mem.size == 4)
                    {
                        mov_dst.reg.size = 4; // mov r/m32 zero-extends to 64-bit automatically
                    }
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, mov_dst, left));
                }
            }
        }

        uint32_t opcode        = 0;
        uint32_t setcc_opcode  = 0;
        bool     is_comparison = false;

        if (expr->binary_expr.op == TOKEN_PLUS)
        {
            opcode = MASM_OP_ADD;
            // Pointer arithmetic: ptr + int
            if (expr->binary_expr.left->type && expr->binary_expr.left->type->kind == TYPE_POINTER)
            {
                // Scale right operand
                int size = expr->binary_expr.left->type->pointer.base->size;
                if (size > 1)
                {
                    // IMUL right_op, size
                    // right_op is in RCX or is immediate.
                    // If immediate, we can just multiply it.
                    // If register, we need to multiply it.

                    if (right_op.kind == MASM_OPERAND_IMM)
                    {
                        right_op.imm *= size;
                    }
                    else
                    {
                        // imul rcx, size
                        masm_section_append_inst(text, masm_inst_3(MASM_OP_IMUL, right_op, right_op, masm_operand_imm(size)));
                    }
                }
            }
            else if (expr->binary_expr.right->type && expr->binary_expr.right->type->kind == TYPE_POINTER)
            {
                // int + ptr -> ptr + int (commutative)
                // Scale left operand (which is in RAX)
                int size = expr->binary_expr.right->type->pointer.base->size;
                if (size > 1)
                {
                    masm_section_append_inst(text, masm_inst_3(MASM_OP_IMUL, result, result, masm_operand_imm(size)));
                }
            }
        }
        else if (expr->binary_expr.op == TOKEN_MINUS)
        {
            opcode = MASM_OP_SUB;
            // Pointer arithmetic: ptr - int
            if (expr->binary_expr.left->type && expr->binary_expr.left->type->kind == TYPE_POINTER)
            {
                // Scale right operand
                int size = expr->binary_expr.left->type->pointer.base->size;
                if (size > 1)
                {
                    if (right_op.kind == MASM_OPERAND_IMM)
                    {
                        right_op.imm *= size;
                    }
                    else
                    {
                        masm_section_append_inst(text, masm_inst_3(MASM_OP_IMUL, right_op, right_op, masm_operand_imm(size)));
                    }
                }
            }
        }
        else if (expr->binary_expr.op == TOKEN_STAR)
        {
            opcode = MASM_OP_IMUL;
        }
        else if (expr->binary_expr.op == TOKEN_SLASH)
        {
            opcode = MASM_OP_IDIV;
        }
        else if (expr->binary_expr.op == TOKEN_PERCENT)
        {
            opcode = MASM_OP_IDIV; // use idiv to get remainder in RDX
        }
        else if (expr->binary_expr.op == TOKEN_AMPERSAND)
        {
            opcode = MASM_OP_AND;
        }
        else if (expr->binary_expr.op == TOKEN_PIPE)
        {
            opcode = MASM_OP_OR;
        }
        else if (expr->binary_expr.op == TOKEN_CARET)
        {
            opcode = MASM_OP_XOR;
        }
        else if (expr->binary_expr.op == TOKEN_LESS_LESS)
        {
            opcode = MASM_OP_SHL;
        }
        else if (expr->binary_expr.op == TOKEN_GREATER_GREATER)
        {
            opcode = MASM_OP_SHR;
        }
        else if (expr->binary_expr.op == TOKEN_EQUAL_EQUAL)
        {
            opcode        = MASM_OP_CMP;
            setcc_opcode  = MASM_OP_SETE;
            is_comparison = true;
        }
        else if (expr->binary_expr.op == TOKEN_BANG_EQUAL)
        {
            opcode        = MASM_OP_CMP;
            setcc_opcode  = MASM_OP_SETNE;
            is_comparison = true;
        }
        else if (expr->binary_expr.op == TOKEN_LESS)
        {
            opcode        = MASM_OP_CMP;
            setcc_opcode  = MASM_OP_SETL;
            is_comparison = true;
        }
        else if (expr->binary_expr.op == TOKEN_GREATER)
        {
            opcode        = MASM_OP_CMP;
            setcc_opcode  = MASM_OP_SETG;
            is_comparison = true;
        }
        else if (expr->binary_expr.op == TOKEN_LESS_EQUAL)
        {
            opcode        = MASM_OP_CMP;
            setcc_opcode  = MASM_OP_SETLE;
            is_comparison = true;
        }
        else if (expr->binary_expr.op == TOKEN_GREATER_EQUAL)
        {
            opcode        = MASM_OP_CMP;
            setcc_opcode  = MASM_OP_SETGE;
            is_comparison = true;
        }
        else
        {
            return masm_operand_none();
        }

        // shifts on x86_64 take their variable count from CL.
        // ensure the count is either an imm8 or in RCX.
        if (opcode == MASM_OP_SHL || opcode == MASM_OP_SHR)
        {
            if (right_op.kind == MASM_OPERAND_REGISTER && right_op.reg.id != rcx.reg.id)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, right_op));
                right_op = rcx;
            }

            if (right_op.kind == MASM_OPERAND_REGISTER)
            {
                // encoded as CL; size here is informational.
                right_op.reg.size = 1;
            }
        }

        if (is_comparison)
        {
            // emit comparison: cmp left, right
            masm_section_append_inst(text, masm_inst_2(opcode, result, right_op));

            // set low byte based on condition
            MasmOperand al = isa_result(ctx, 1);
            masm_section_append_inst(text, masm_inst_1(setcc_opcode, al));

            // zero-extend AL to RAX using AND RAX, 1
            // This assumes boolean result is 0 or 1.
            // High bytes of RAX are preserved from 'left' operand, so we must clear them.
            // AND RAX, 1 keeps only the LSB (which is the result of SETcc).
            masm_section_append_inst(text, masm_inst_2(MASM_OP_AND, result, masm_operand_imm(1)));
        }
        else if (opcode == MASM_OP_IDIV)
        {
            // idiv requires divisor in r/m, not immediate. materialize immediates.
            MasmOperand div_op = right_op;
            if (right_op.kind == MASM_OPERAND_IMM)
            {
                MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, right_op));
                div_op = tmp;
            }

            // idiv uses RAX as dividend; ensure result is in RAX and save previous if needed
            if (result.kind != MASM_OPERAND_REGISTER || result.reg.id != isa_result_id(ctx))
            {
                MasmOperand res_reg = isa_result(ctx, result.reg.size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, res_reg, result));
                result = res_reg;
            }

            masm_section_append_inst(text, masm_inst_0(MASM_OP_CQO));
            masm_section_append_inst(text, masm_inst_1(opcode, div_op));

            if (expr->binary_expr.op == TOKEN_PERCENT)
            {
                // remainder in div_hi role (x86: RDX)
                MasmOperand rem = ctx->isa->reg_div_hi(ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, rem));
            }
        }
        else
        {
            // prefer 3-operand form for imul with immediates to ensure encoding support
            if (opcode == MASM_OP_IMUL && right_op.kind == MASM_OPERAND_IMM)
            {
                masm_section_append_inst(text, masm_inst_3(opcode, result, result, right_op));
            }
            else
            {
                masm_section_append_inst(text, masm_inst_2(opcode, result, right_op));
            }
        }

        return result;
    }
    else if (expr->kind == AST_EXPR_CALL)
    {
        // varargs builtins (no `$` prefix). these are handled as special calls and do not
        // require a declared symbol.
        if (ctx->fn_is_variadic && expr->call_expr.func && expr->call_expr.func->kind == AST_EXPR_IDENT)
        {
            const char *bname = expr->call_expr.func->ident_expr.name;
            AstList    *bargs = expr->call_expr.args;
            int         bargc = bargs ? bargs->count : 0;

            if (bname && (!strcmp(bname, "va_start") || !strcmp(bname, "va_end") || !strcmp(bname, "va_arg")))
            {
                // sema should enforce exactly one argument.
                if (bargc != 1)
                {
                    return masm_operand_none();
                }

                MasmOperand ap = lower_expr(masm, text, bargs->items[0], ctx);
                if (ap.kind != MASM_OPERAND_REGISTER)
                {
                    MasmOperand tmp = isa_result(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, ap));
                    ap = tmp;
                }

                if (!strcmp(bname, "va_end"))
                {
                    // SysV x86_64: va_end is a no-op.
                    return masm_operand_none();
                }

                if (!strcmp(bname, "va_start"))
                {
                    // initialize *va_list
                    // gp_offset
                    {
                        uint32_t    gp0   = (uint32_t)(ctx->va_named_gp * 8);
                        MasmOperand tmp32 = isa_tmp(ctx, 4);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp32, masm_operand_imm((int64_t)gp0)));
                        MasmOperand dst = masm_operand_memory_simple(ap.reg.id, 0, 4);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp32));
                    }

                    // fp_offset
                    {
                        uint32_t    fp0   = (uint32_t)(48 + ctx->va_named_fp * 16);
                        MasmOperand tmp32 = isa_tmp(ctx, 4);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp32, masm_operand_imm((int64_t)fp0)));
                        MasmOperand dst = masm_operand_memory_simple(ap.reg.id, 4, 4);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp32));
                    }

                    // overflow_arg_area = rbp + 16 + named_stack_slots*8
                    {
                        int32_t     off  = (int32_t)(2 * ctx->ptr_size + (ctx->va_named_stack_slots * ctx->ptr_size));
                        MasmOperand tmp  = isa_tmp(ctx, ctx->ptr_size);
                        MasmOperand addr = frame_mem(ctx, off, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, tmp, addr));
                        MasmOperand dst = masm_operand_memory_simple(ap.reg.id, 8, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                    }

                    // reg_save_area = rbp + va_reg_save_off
                    {
                        MasmOperand tmp  = isa_tmp(ctx, ctx->ptr_size);
                        MasmOperand addr = frame_mem(ctx, ctx->va_reg_save_off, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, tmp, addr));
                        MasmOperand dst = masm_operand_memory_simple(ap.reg.id, 16, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                    }

                    return masm_operand_none();
                }

                // va_arg[T](ap)
                Type *t = expr->type;
                if (type_is_fp_class(t))
                {
                    uint8_t sz = type_fp_size(t);
                    return va_load_fp_slot(masm, text, ctx, ap, sz);
                }

                bool is_agg = t && (t->kind == TYPE_STRUCT || t->kind == TYPE_UNION || t->kind == TYPE_ARRAY);
                if (is_agg)
                {
                    size_t tsize = t->size;
                    if (tsize == 0)
                    {
                        return masm_operand_none();
                    }

                    bool byptr = tsize > ctx->ptr_size;

                    // allocate stack storage for the extracted value
                    int32_t alloc = (int32_t)align_up_i32((int32_t)tsize, (int32_t)ctx->ptr_size);
                    if (alloc < (int32_t)ctx->ptr_size)
                    {
                        alloc = (int32_t)ctx->ptr_size;
                    }
                    ctx->stack_offset -= alloc;
                    int align = ctx->ptr_size ? ctx->ptr_size : 1;
                    if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
                    {
                        ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
                    }
                    int32_t tmp_off = ctx->stack_offset;

                    MasmOperand dst_ptr  = isa_tmp2(ctx, ctx->ptr_size);
                    MasmOperand dst_addr = frame_mem(ctx, tmp_off, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst_ptr, dst_addr));

                    if (byptr)
                    {
                        // vararg slot holds a pointer to the value bytes
                        MasmOperand src_ptr = va_load_gp_slot(masm, text, ctx, ap, ctx->ptr_size);

                        for (int32_t off = 0; off < (int32_t)tsize;)
                        {
                            int32_t chunk = (int32_t)tsize - off;
                            if (chunk > 8)
                            {
                                chunk = 8;
                            }
                            else if (chunk > 4)
                            {
                                chunk = 4;
                            }
                            else if (chunk > 2)
                            {
                                chunk = 2;
                            }
                            else
                            {
                                chunk = 1;
                            }

                            MasmOperand tmp = isa_tmp(ctx, (uint8_t)chunk);
                            MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
                            MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
                            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
                            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                            off += chunk;
                        }

                        // large aggregates are represented by address
                        return dst_ptr;
                    }
                    else
                    {
                        // small aggregates are represented in an 8-byte slot
                        MasmOperand bits = va_load_gp_slot(masm, text, ctx, ap, ctx->ptr_size);
                        MasmOperand dst  = frame_mem(ctx, tmp_off, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, bits));
                        return dst;
                    }
                }

                // integer/pointer-like types
                uint8_t load_size = ctx->ptr_size;
                if (t)
                {
                    if (t->kind == TYPE_U8 || t->kind == TYPE_I8)
                    {
                        load_size = 1;
                    }
                    else if (t->kind == TYPE_U16 || t->kind == TYPE_I16)
                    {
                        load_size = 2;
                    }
                    else if (t->kind == TYPE_U32 || t->kind == TYPE_I32)
                    {
                        load_size = 4;
                    }
                    else
                    {
                        load_size = ctx->ptr_size;
                    }
                }

                return va_load_gp_slot(masm, text, ctx, ap, load_size);
            }
        }

        AstList *args = expr->call_expr.args;

        int explicit_arg_count = args ? args->count : 0;

        // determine whether we can use the internal aggregate ABI (Mach functions).
        AstNode *func             = expr->call_expr.func;
        Symbol  *call_sym         = func ? func->symbol : NULL;
        bool     internal_agg_abi = call_sym && call_sym->decl && call_sym->decl->kind == AST_STMT_FUN;

        // internal ABI: aggregate returns (> ptr_size) are returned via an implicit sret pointer
        // passed in the first integer argument register.
        Type *ret_type = NULL;
        if (func && func->symbol && func->symbol->type && func->symbol->type->kind == TYPE_FUNCTION)
        {
            ret_type = func->symbol->type->function.return_type;
        }
        else if (func && func->type && func->type->kind == TYPE_FUNCTION)
        {
            ret_type = func->type->function.return_type;
        }

        // prefer the call expression's own type if the callee signature is missing/stale.
        if (!ret_type || ret_type->size == 0 || (!type_is_large_aggregate(ret_type, ctx->ptr_size) && type_is_large_aggregate(expr->type, ctx->ptr_size)))
        {
            ret_type = expr->type;
        }

        bool    needs_sret  = false;
        int32_t sret_offset = 0;
        if (type_is_large_aggregate(ret_type, ctx->ptr_size))
        {
            needs_sret = true;

            // reserve stack storage for the returned aggregate
            ctx->stack_offset -= (int32_t)ret_type->size;
            int align = ctx->ptr_size ? ctx->ptr_size : 1;
            if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
            {
                ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
            }
            sret_offset = ctx->stack_offset;
        }

        int arg_shift     = needs_sret ? 1 : 0;
        int abi_arg_count = explicit_arg_count + arg_shift;

        typedef enum CallArgKind
        {
            CALL_ARG_GP,
            CALL_ARG_FP,
            CALL_ARG_STACK,
        } CallArgKind;

        typedef struct CallArgLoc
        {
            CallArgKind kind;
            int         gp_index;
            int         fp_index;
            int         stack_slot;
            uint8_t     size;
            bool        is_fp;
        } CallArgLoc;

        CallArgLoc *locs = NULL;
        if (abi_arg_count > 0)
        {
            locs = calloc((size_t)abi_arg_count, sizeof(CallArgLoc));
        }

        // size per fp reg index (movd for f32, movq for f64)
        uint8_t fp_reg_sizes[8] = {0};

        int gp_i    = 0;
        int fp_i    = 0;
        int stack_i = 0;

        for (int i = 0; i < abi_arg_count; i++)
        {
            CallArgLoc *loc = &locs[i];
            loc->gp_index   = -1;
            loc->fp_index   = -1;
            loc->stack_slot = -1;
            loc->size       = ctx->ptr_size;
            loc->is_fp      = false;

            if (needs_sret && i == 0)
            {
                loc->kind     = CALL_ARG_GP;
                loc->gp_index = gp_i++;
                continue;
            }

            int      explicit_index = i - arg_shift;
            AstNode *arg_expr       = args ? args->items[explicit_index] : NULL;
            bool     byptr          = internal_agg_abi && arg_expr && type_is_large_aggregate(arg_expr->type, ctx->ptr_size);
            bool     is_fp          = !byptr && arg_expr && type_is_fp_class(arg_expr->type);
            uint8_t  fp_size        = is_fp ? type_fp_size(arg_expr->type) : ctx->ptr_size;

            loc->is_fp = is_fp;
            loc->size  = is_fp ? fp_size : ctx->ptr_size;

            if (is_fp)
            {
                if (fp_i < ctx->float_arg_count)
                {
                    loc->kind     = CALL_ARG_FP;
                    loc->fp_index = fp_i;
                    if (fp_i < 8)
                    {
                        fp_reg_sizes[fp_i] = fp_size;
                    }
                    fp_i++;
                }
                else
                {
                    loc->kind       = CALL_ARG_STACK;
                    loc->stack_slot = stack_i++;
                }
            }
            else
            {
                if (gp_i < ctx->int_arg_count)
                {
                    loc->kind     = CALL_ARG_GP;
                    loc->gp_index = gp_i++;
                }
                else
                {
                    loc->kind       = CALL_ARG_STACK;
                    loc->stack_slot = stack_i++;
                }
            }
        }

        int stack_args_count = stack_i;
        int padding          = (stack_args_count % 2 != 0) ? 8 : 0;

        int total_stack_space = (stack_args_count * 8) + padding;
        if (total_stack_space > 0)
        {
            MasmOperand rsp = sp_reg_op(ctx);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_SUB, rsp, masm_operand_imm(total_stack_space)));
        }

        // stack arguments (reverse order)
        for (int i = abi_arg_count - 1; i >= 0; i--)
        {
            if (!locs || locs[i].kind != CALL_ARG_STACK)
            {
                continue;
            }

            int      explicit_index = i - arg_shift;
            AstNode *arg_expr       = args ? args->items[explicit_index] : NULL;

            bool        byptr  = internal_agg_abi && arg_expr && type_is_large_aggregate(arg_expr->type, ctx->ptr_size);
            MasmOperand arg_op = byptr ? lower_byval_arg_ptr(masm, text, arg_expr, ctx) : lower_expr(masm, text, arg_expr, ctx);
            if (byptr && arg_op.kind == MASM_OPERAND_NONE)
            {
                arg_op = lower_expr(masm, text, arg_expr, ctx);
            }

            int32_t     disp = (int32_t)(locs[i].stack_slot * 8);
            MasmOperand dst  = masm_operand_memory_simple(ctx->sp_reg, disp, locs[i].size);

            if (locs[i].is_fp)
            {
                MasmOperand tmp = isa_result(ctx, locs[i].size);
                materialize_bits_to_gpr(text, ctx, tmp, arg_op, locs[i].size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
            }
            else
            {
                if (arg_op.kind == MASM_OPERAND_MEMORY)
                {
                    MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, arg_op));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, arg_op));
                }
            }
        }

        int gp_used = gp_i;
        int fp_used = fp_i;

        int32_t gp_scratch_base = 0;
        int32_t fp_scratch_base = 0;

        if (gp_used > 0)
        {
            ctx->stack_offset -= (int32_t)(gp_used * ctx->ptr_size);
            int align = ctx->ptr_size ? ctx->ptr_size : 1;
            if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
            {
                ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
            }
            gp_scratch_base = ctx->stack_offset;
        }

        if (fp_used > 0)
        {
            ctx->stack_offset -= (int32_t)(fp_used * ctx->ptr_size);
            int align = ctx->ptr_size ? ctx->ptr_size : 1;
            if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
            {
                ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
            }
            fp_scratch_base = ctx->stack_offset;
        }

        // register arguments: stage into scratch slots first to avoid clobbering
        // already-prepared argument registers during nested arg evaluation.
        for (int i = 0; i < abi_arg_count; i++)
        {
            if (!locs)
            {
                break;
            }

            if (locs[i].kind == CALL_ARG_GP)
            {
                MasmOperand slot  = frame_mem(ctx, gp_scratch_base + (locs[i].gp_index * (int32_t)ctx->ptr_size), ctx->ptr_size);
                MasmOperand tmp64 = isa_result(ctx, ctx->ptr_size);

                if (needs_sret && i == 0)
                {
                    MasmOperand addr = frame_mem(ctx, sret_offset, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, tmp64, addr));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, slot, tmp64));
                    continue;
                }

                int         explicit_index = i - arg_shift;
                AstNode    *arg_expr       = args ? args->items[explicit_index] : NULL;
                bool        byptr          = internal_agg_abi && arg_expr && type_is_large_aggregate(arg_expr->type, ctx->ptr_size);
                MasmOperand arg_op         = byptr ? lower_byval_arg_ptr(masm, text, arg_expr, ctx) : lower_expr(masm, text, arg_expr, ctx);
                if (byptr && arg_op.kind == MASM_OPERAND_NONE)
                {
                    arg_op = lower_expr(masm, text, arg_expr, ctx);
                }

                if (arg_op.kind == MASM_OPERAND_REGISTER)
                {
                    if (arg_op.reg.id != tmp64.reg.id)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp64, arg_op));
                    }
                }
                else if (arg_op.kind == MASM_OPERAND_IMM || arg_op.kind == MASM_OPERAND_LABEL)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp64, arg_op));
                }
                else if (arg_op.kind == MASM_OPERAND_MEMORY)
                {
                    // materialize memory into a full-width register (zero-extend small widths).
                    if (arg_op.mem.size == 1 || arg_op.mem.size == 2)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, tmp64, arg_op));
                    }
                    else if (arg_op.mem.size == 4)
                    {
                        MasmOperand tmp32 = isa_result(ctx, 4);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp32, arg_op));
                    }
                    else
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp64, arg_op));
                    }
                }
                else
                {
                    fprintf(stderr, "masm lower: unsupported operand kind in call arg (kind=%d", arg_op.kind);
                    if (arg_op.kind == MASM_OPERAND_SYMBOL && arg_op.symbol)
                    {
                        fprintf(stderr, " symbol=%s", arg_op.symbol);
                    }
                    if (arg_expr && arg_expr->token)
                    {
                        fprintf(stderr, " pos=%d len=%d", arg_expr->token->pos, arg_expr->token->len);
                    }
                    fprintf(stderr, ")\n");
                    exit(1);
                }

                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, slot, tmp64));
            }
            else if (locs[i].kind == CALL_ARG_FP)
            {
                int         explicit_index = i - arg_shift;
                AstNode    *arg_expr       = args ? args->items[explicit_index] : NULL;
                MasmOperand arg_op         = lower_expr(masm, text, arg_expr, ctx);

                MasmOperand slot = frame_mem(ctx, fp_scratch_base + (locs[i].fp_index * (int32_t)ctx->ptr_size), locs[i].size);
                MasmOperand tmp  = isa_result(ctx, locs[i].size);

                materialize_bits_to_gpr(text, ctx, tmp, arg_op, locs[i].size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, slot, tmp));
            }
        }

        // load staged gp args into the real ABI argument registers
        for (int i = 0; i < gp_used; i++)
        {
            uint32_t    reg = (i < ctx->abi->int_arg_count) ? ctx->abi->int_arg_regs[i] : UINT32_MAX;
            MasmOperand dst = masm_operand_register(reg, ctx->ptr_size);
            MasmOperand src = frame_mem(ctx, gp_scratch_base + (i * (int32_t)ctx->ptr_size), ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
        }

        // load staged fp args into xmm regs (via a non-arg scratch gpr)
        for (int i = 0; i < fp_used; i++)
        {
            uint32_t    xmm_id = (i < ctx->abi->float_arg_count) ? ctx->abi->float_arg_regs[i] : UINT32_MAX;
            uint8_t     sz     = fp_reg_sizes[i] ? fp_reg_sizes[i] : 8;
            MasmOperand xmm    = masm_xmm(xmm_id);
            MasmOperand src    = frame_mem(ctx, fp_scratch_base + (i * (int32_t)ctx->ptr_size), sz);
            MasmOperand tmp    = masm_operand_register(MASM_X86_R11, sz);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_X86_MOVQ, xmm, tmp));
        }

        // determine if callee is variadic (last param type sentinel NULL)
        bool is_variadic = false;
        {
            Type *ft = NULL;
            if (func && func->symbol && func->symbol->type && func->symbol->type->kind == TYPE_FUNCTION)
            {
                ft = func->symbol->type;
            }
            else if (func && func->type && func->type->kind == TYPE_FUNCTION)
            {
                ft = func->type;
            }

            if (ft && ft->function.param_count > 0 && ft->function.param_types[ft->function.param_count - 1] == NULL)
            {
                is_variadic = true;
            }
        }

        // emit call
        {
            const char *call_name = NULL;
            Symbol     *call_sym2 = func ? func->symbol : NULL;

            if (call_sym2)
            {
                call_name = symbol_get_linkage_name(call_sym2);
                if (!call_name)
                {
                    call_name = call_sym2->name;
                }
            }
            else if (func && func->kind == AST_EXPR_IDENT)
            {
                call_name = func->ident_expr.name;
            }

            if (call_name)
            {
                if (is_variadic)
                {
                    int xmm_used = fp_used;
                    if (xmm_used > 8)
                    {
                        xmm_used = 8;
                    }
                    MasmOperand al = isa_result(ctx, 1);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, al, masm_operand_imm(xmm_used & 0xFF)));
                }
                masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, masm_operand_label(strdup(call_name))));
            }
            else
            {
                // indirect call: evaluate function pointer expression
                MasmOperand func_ptr = lower_expr(masm, text, func, ctx);

                if (func_ptr.kind != MASM_OPERAND_REGISTER)
                {
                    MasmOperand r_res = isa_result(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, r_res, func_ptr));
                    func_ptr = r_res;
                }

                if (is_variadic)
                {
                    int xmm_used = fp_used;
                    if (xmm_used > 8)
                    {
                        xmm_used = 8;
                    }
                    MasmOperand al = isa_result(ctx, 1);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, al, masm_operand_imm(xmm_used & 0xFF)));
                }

                masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, func_ptr));
            }
        }

        // clean up stack arguments
        int total_cleanup = (stack_args_count * 8) + padding;
        if (total_cleanup > 0)
        {
            MasmOperand rsp = sp_reg_op(ctx);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, rsp, masm_operand_imm(total_cleanup)));
        }

        if (locs)
        {
            free(locs);
        }

        if (needs_sret)
        {
            return frame_mem(ctx, sret_offset, ctx->ptr_size);
        }

        // materialize fp returns (xmm0) back into a gpr holding the raw bits
        if (type_is_fp_class(ret_type) && ctx->abi->float_ret_count > 0)
        {
            uint8_t     rsz  = type_fp_size(ret_type);
            uint32_t    xmm0 = ctx->abi->float_ret_regs[0];
            MasmOperand dst  = isa_result(ctx, rsz);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_X86_MOVQ, dst, masm_xmm(xmm0)));
            return dst;
        }

        return isa_result(ctx, ctx->ptr_size);
    }
    else if (expr->kind == AST_EXPR_UNARY)
    {
        if (expr->unary_expr.op == TOKEN_QUESTION)
        {
            // Address-of: ?expr (produce pointer to lvalue)
            // IMPORTANT: `?` must return the address of an lvalue.
            // `lower_expr(AST_EXPR_IDENT)` loads the *value* into a register for scalars,
            // which would incorrectly treat `?x` as "x is already a pointer".
            // Handle identifiers explicitly by returning the address of their stack slot/global.
            AstNode *inner_expr = expr->unary_expr.expr;

            if (inner_expr && inner_expr->kind == AST_EXPR_IDENT)
            {
                LocalVar *var = find_local_var(ctx, inner_expr->ident_expr.name);
                if (var)
                {
                    MasmOperand dst  = isa_result(ctx, ctx->ptr_size);
                    MasmOperand addr = frame_mem(ctx, var->offset, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst, addr));
                    return dst;
                }

                MasmSymbol *sym = masm_get_symbol(masm, inner_expr->ident_expr.name);
                if (sym)
                {
                    MasmOperand dst      = isa_result(ctx, ctx->ptr_size);
                    MasmOperand label_op = masm_operand_label(strdup(sym->name));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, label_op));
                    return dst;
                }
            }

            MasmOperand val = lower_expr(masm, text, inner_expr, ctx);

            if (val.kind == MASM_OPERAND_MEMORY)
            {
                MasmOperand dst = isa_result(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst, val));
                return dst;
            }
            else if (val.kind == MASM_OPERAND_LABEL)
            {
                // absolute address of label/rodata
                MasmOperand dst = isa_result(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val));
                return dst;
            }
            else if (val.kind == MASM_OPERAND_REGISTER)
            {
                // for non-ident lvalues, some forms may already compute an address.
                return val;
            }

            return masm_operand_none();
        }
        else if (expr->unary_expr.op == TOKEN_AT)
        {
            // Dereference: @expr
            MasmOperand ptr = lower_expr(masm, text, expr->unary_expr.expr, ctx);

            if (ptr.kind != MASM_OPERAND_REGISTER)
            {
                MasmOperand r_res = isa_result(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, r_res, ptr));
                ptr = r_res;
            }

            int size = 8;
            if (expr->type)
            {
                size = expr->type->size;
            }

            return masm_operand_memory_simple(ptr.reg.id, 0, size);
        }

        MasmOperand operand = lower_expr(masm, text, expr->unary_expr.expr, ctx);
        MasmOperand result  = isa_result(ctx, ctx->ptr_size);

        if (operand.kind != MASM_OPERAND_REGISTER || operand.reg.id != isa_result_id(ctx))
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, operand));
        }

        if (expr->unary_expr.op == TOKEN_BANG)
        {
            // !expr -> cmp rax, 0; sete al; and rax, 1
            masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, result, masm_operand_imm(0)));

            MasmOperand al = isa_result(ctx, 1);
            masm_section_append_inst(text, masm_inst_1(MASM_OP_SETE, al));

            masm_section_append_inst(text, masm_inst_2(MASM_OP_AND, result, masm_operand_imm(1)));
        }
        else if (expr->unary_expr.op == TOKEN_MINUS)
        {
            // -expr -> sub 0, rax
            MasmOperand rcx = isa_tmp(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, result));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, masm_operand_imm(0)));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_SUB, result, rcx));
        }

        else if (expr->unary_expr.op == TOKEN_TILDE)
        {
            // ~expr -> bitwise not. constrain to the static type width when known.
            uint8_t sz = (expr->type && expr->type->size) ? (uint8_t)expr->type->size : (uint8_t)ctx->ptr_size;
            if (sz == 0)
            {
                sz = (uint8_t)ctx->ptr_size;
            }
            if (sz > 8)
            {
                sz = 8;
            }

            int64_t mask = -1;
            if (sz < 8)
            {
                mask = (int64_t)(((uint64_t)1ULL << (sz * 8)) - 1ULL);

                // clear high bits first so ~ behaves like an N-bit operation.
                masm_section_append_inst(text, masm_inst_2(MASM_OP_AND, result, masm_operand_imm(mask)));
            }

            masm_section_append_inst(text, masm_inst_2(MASM_OP_XOR, result, masm_operand_imm(mask)));
        }

        return result;
    }
    else if (expr->kind == AST_EXPR_FIELD)
    {
        // obj.field or ptr.field
        MasmOperand obj      = lower_expr(masm, text, expr->field_expr.object, ctx);
        Type       *obj_type = expr->field_expr.object->type;

        if (!obj_type)
        {
            return masm_operand_none();
        }

        Type *struct_type = obj_type;
        bool  is_pointer  = false;

        if (obj_type->kind == TYPE_POINTER)
        {
            struct_type = obj_type->pointer.base;
            is_pointer  = true;
        }

        if (struct_type->kind == TYPE_STRUCT || struct_type->kind == TYPE_UNION)
        {
            // Find field offset
            int32_t    offset = 0;
            TypeField *field  = NULL;

            TypeField *fields      = NULL;
            int        field_count = 0;
            if (struct_type->kind == TYPE_STRUCT)
            {
                fields      = struct_type->structure.fields;
                field_count = struct_type->structure.field_count;
            }
            else
            {
                fields      = struct_type->union_type.fields;
                field_count = struct_type->union_type.field_count;
            }

            for (int i = 0; i < field_count; i++)
            {
                if (strcmp(fields[i].name, expr->field_expr.field) == 0)
                {
                    field  = &fields[i];
                    offset = (int32_t)field->offset;
                    break;
                }
            }

            if (field)
            {
                // note: for large aggregates, lower_expr(AST_EXPR_IDENT) returns a register
                // containing the address of the object (LEA). treat that like pointer access.
                if (is_pointer || obj.kind == MASM_OPERAND_REGISTER)
                {
                    uint8_t size = field->type && field->type->size ? (uint8_t)field->type->size : ctx->ptr_size;
                    return masm_operand_memory_simple(obj.reg.id, offset, size);
                }
                else
                {
                    // obj is a memory operand (struct on stack)
                    if (obj.kind == MASM_OPERAND_MEMORY)
                    {
                        // Adjust displacement
                        uint8_t size = field->type && field->type->size ? (uint8_t)field->type->size : ctx->ptr_size;
                        return masm_operand_memory_simple(obj.mem.base.id, obj.mem.disp + offset, size);
                    }
                }
            }
        }
        return masm_operand_none();
    }
    else if (expr->kind == AST_EXPR_ARRAY)
    {
        Type *type = expr->type;
        if (!type || type->kind != TYPE_ARRAY)
        {
            return masm_operand_none();
        }

        // allocate stack space
        ctx->stack_offset -= type->size;
        // align to pointer size for now (could be tightened per-field later)
        int align = ctx->ptr_size ? ctx->ptr_size : 1;
        if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
        {
            ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
        }

        int32_t base_offset = ctx->stack_offset;
        Type   *elem_type   = type->array.elem_type;
        int     elem_size   = elem_type->size;

        AstList *elems = expr->array_expr.elems;
        if (elems)
        {
            for (int i = 0; i < elems->count; i++)
            {
                MasmOperand val         = lower_expr(masm, text, elems->items[i], ctx);
                int32_t     elem_offset = base_offset + (i * elem_size);
                MasmOperand dst         = frame_mem(ctx, elem_offset, elem_size);

                if (val.kind == MASM_OPERAND_REGISTER)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val));
                }
                else if (val.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val));
                }
                else if (val.kind == MASM_OPERAND_MEMORY)
                {
                    // mem to mem copy
                    MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, val));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                }
            }
        }

        return frame_mem(ctx, base_offset, type->size);
    }

    else if (expr->kind == AST_EXPR_INDEX)
    {
        // arr[i]
        // evaluate index first to keep base register live afterwards
        MasmOperand idx = lower_expr(masm, text, expr->index_expr.index, ctx);

        // stash index into a temp early so later evaluations (array) won't clobber it
        MasmOperand idx_reg = isa_tmp(ctx, ctx->ptr_size);
        if (idx.kind != MASM_OPERAND_REGISTER || idx.reg.id != idx_reg.reg.id)
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, idx_reg, idx));
        }

        MasmOperand arr = lower_expr(masm, text, expr->index_expr.array, ctx);

        Type *arr_type = expr->index_expr.array->type;
        if (!arr_type)
        {
            return masm_operand_none();
        }

        Type *elem_type = NULL;
        if (arr_type->kind == TYPE_ARRAY)
        {
            elem_type = arr_type->array.elem_type;
        }
        else if (arr_type->kind == TYPE_POINTER)
        {
            elem_type = arr_type->pointer.base;
        }
        else
        {
            return masm_operand_none();
        }

        int elem_size = elem_type->size;

        // Handle immediate index
        if (idx.kind == MASM_OPERAND_IMM)
        {
            int64_t offset = idx.imm * elem_size;

            MasmOperand mem;
            if (arr.kind == MASM_OPERAND_REGISTER)
            {
                // [reg + offset]
                mem = masm_operand_memory_simple(arr.reg.id, offset, elem_size);
            }
            else if (arr.kind == MASM_OPERAND_MEMORY)
            {
                // [base + disp + offset]
                mem = masm_operand_memory_simple(arr.mem.base.id, arr.mem.disp + offset, elem_size);
            }
            else
            {
                return masm_operand_none();
            }

            return mem;
        }
        else
        {
            // Register index
            // ensure base and index do not alias; if arr uses RCX, move idx to RDX
            if (arr.kind == MASM_OPERAND_REGISTER && arr.reg.id == idx_reg.reg.id)
            {
                MasmOperand idx_spill = isa_tmp2(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, idx_spill, idx_reg));
                idx_reg = idx_spill;
            }

            // Check if scale is valid (1, 2, 4, 8)
            bool valid_scale = (elem_size == 1 || elem_size == 2 || elem_size == 4 || elem_size == 8);

            if (!valid_scale)
            {
                // Manual scaling: imul idx_reg, elem_size
                // We need to move idx to a temp register if it's not already, to avoid clobbering if it's a variable?
                // Actually lower_expr returns a result which is usually a temp or a variable.
                // If it's a variable (memory), we already moved it to RAX above.
                // If it's a register, we can multiply it in place if it's a temp (RAX/RCX).
                // But if it's a variable in a register (not supported yet), we should be careful.
                // For now, assume we can modify the register if it's RAX.

                if (idx_reg.reg.id != isa_result_id(ctx) && idx_reg.reg.id != isa_tmp(ctx, ctx->ptr_size).reg.id)
                {
                    MasmOperand tmp_reg = isa_tmp(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp_reg, idx_reg));
                    idx_reg = tmp_reg;
                }

                masm_section_append_inst(text, masm_inst_3(MASM_OP_IMUL, idx_reg, idx_reg, masm_operand_imm(elem_size)));
                elem_size = 1; // scale is now 1
            }

            MasmOperand mem;
            if (arr.kind == MASM_OPERAND_REGISTER)
            {
                // [base + index * scale]
                MasmRegister base  = {arr.reg.id, 8};
                MasmRegister index = {idx_reg.reg.id, 8};
                mem                = masm_operand_memory(base, index, elem_size, 0, elem_type->size);
            }
            else if (arr.kind == MASM_OPERAND_MEMORY)
            {
                // [base + disp + index * scale]
                // arr.mem.base is usually RBP
                MasmRegister base  = arr.mem.base;
                MasmRegister index = {idx_reg.reg.id, 8};
                mem                = masm_operand_memory(base, index, elem_size, arr.mem.disp, elem_type->size);
            }
            else
            {
                return masm_operand_none();
            }

            return mem;
        }

        return masm_operand_none();
    }
    else if (expr->kind == AST_EXPR_STRUCT)
    {
        Type *type = expr->type;
        if (!type || (type->kind != TYPE_STRUCT && type->kind != TYPE_UNION))
        {
            return masm_operand_none();
        }

        // allocate stack space for struct/union
        ctx->stack_offset -= type->size;
        // align to pointer size
        int align = ctx->ptr_size ? ctx->ptr_size : 1;
        if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
        {
            ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
        }

        int32_t base_offset = ctx->stack_offset;

        // initialize fields (for unions the selected variant is initialized at offset 0)
        AstList *fields = expr->struct_expr.fields;
        if (fields)
        {
            for (int i = 0; i < fields->count; i++)
            {
                AstNode *field_node = fields->items[i];
                // field_node is AST_EXPR_FIELD
                char    *field_name = field_node->field_expr.field;
                AstNode *init_expr  = field_node->field_expr.object;

                // find field info
                size_t offset     = 0;
                Type  *field_type = NULL;

                if (type->kind == TYPE_STRUCT)
                {
                    for (int j = 0; j < type->structure.field_count; j++)
                    {
                        if (strcmp(type->structure.fields[j].name, field_name) == 0)
                        {
                            offset     = type->structure.fields[j].offset;
                            field_type = type->structure.fields[j].type;
                            break;
                        }
                    }
                }
                else // TYPE_UNION
                {
                    for (int j = 0; j < type->union_type.field_count; j++)
                    {
                        if (strcmp(type->union_type.fields[j].name, field_name) == 0)
                        {
                            field_type = type->union_type.fields[j].type;
                            break;
                        }
                    }
                    offset = 0; // unions start at offset 0
                }

                if (!field_type)
                {
                    continue;
                }

                // evaluate init expr
                MasmOperand init_op = lower_expr(masm, text, init_expr, ctx);

                int32_t dest_disp = base_offset + (int32_t)offset;

                // For union aggregates larger than ptr_size, copy bytes from the initializer's address
                if (type->kind == TYPE_UNION && field_type->size > ctx->ptr_size)
                {
                    size_t fsize = field_type->size;
                    if (fsize == 0)
                    {
                        continue;
                    }

                    // normalize source into an address in a register
                    MasmOperand src_ptr = isa_result(ctx, ctx->ptr_size);
                    if (init_op.kind == MASM_OPERAND_REGISTER)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, init_op));
                    }
                    else if (init_op.kind == MASM_OPERAND_MEMORY)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, src_ptr, init_op));
                    }
                    else if (init_op.kind == MASM_OPERAND_LABEL || init_op.kind == MASM_OPERAND_IMM)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, init_op));
                    }
                    else
                    {
                        // fallback: move to tmp reg then treat as address
                        MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, init_op));
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, tmp));
                    }

                    // dst address
                    MasmOperand dst_ptr  = isa_tmp2(ctx, ctx->ptr_size);
                    MasmOperand dst_addr = frame_mem(ctx, dest_disp, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst_ptr, dst_addr));

                    // copy in chunks (8/4/2/1)
                    for (int32_t off = 0; off < (int32_t)fsize;)
                    {
                        int32_t chunk = (int32_t)fsize - off;
                        if (chunk > 8)
                        {
                            chunk = 8;
                        }
                        else if (chunk > 4)
                        {
                            chunk = 4;
                        }
                        else if (chunk > 2)
                        {
                            chunk = 2;
                        }
                        else
                        {
                            chunk = 1;
                        }

                        MasmOperand tmp = isa_tmp(ctx, (uint32_t)chunk);
                        MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
                        MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                        off += chunk;
                    }
                }
                else
                {
                    // scalar or small aggregate: use existing scalar/mem logic
                    MasmOperand dest = frame_mem(ctx, dest_disp, field_type->size > ctx->ptr_size ? ctx->ptr_size : field_type->size);

                    if (init_op.kind == MASM_OPERAND_REGISTER)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dest, init_op));
                    }
                    else if (init_op.kind == MASM_OPERAND_IMM)
                    {
                        if (init_op.imm > 2147483647 || init_op.imm < -2147483648)
                        {
                            MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, init_op));
                            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dest, tmp));
                        }
                        else
                        {
                            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dest, init_op));
                        }
                    }
                    else if (init_op.kind == MASM_OPERAND_MEMORY)
                    {
                        // Mem to Mem -> use register
                        MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, init_op));
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dest, tmp));
                    }
                }
            }
        }

        // return pointer-to-value memory operand but mark small aggregates with their actual size
        uint8_t ret_size = (type->size && type->size < ctx->ptr_size) ? (uint8_t)type->size : ctx->ptr_size;
        if (ret_size == 0)
        {
            ret_size = ctx->ptr_size;
        }
        return frame_mem(ctx, base_offset, ret_size);
    }
    return masm_operand_none();
}

static void lower_stmt(Masm *masm, MasmSection *text, AstNode *stmt, LowerContext *ctx)
{
    if (stmt->kind == AST_STMT_RET)
    {
        AstNode *expr = stmt->ret_stmt.expr;
        if (ctx->fn_has_sret && ctx->fn_ret_type && ctx->fn_ret_type->size > ctx->ptr_size)
        {
            if (expr)
            {
                // return aggregate: copy into sret buffer stored in frame
                MasmOperand src_val = lower_expr(masm, text, expr, ctx);

                // normalize src_val into an address in rax
                MasmOperand src_ptr = isa_result(ctx, ctx->ptr_size);
                if (src_val.kind == MASM_OPERAND_REGISTER)
                {
                    if (src_val.reg.id != src_ptr.reg.id)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_val));
                    }
                }
                else if (src_val.kind == MASM_OPERAND_MEMORY)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, src_ptr, src_val));
                }
                else if (src_val.kind == MASM_OPERAND_LABEL)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_val));
                }
                else if (src_val.kind == MASM_OPERAND_IMM)
                {
                    // shouldn't happen for aggregates, but keep codegen safe.
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_val));
                }

                // load dst sret pointer from frame slot into tmp
                MasmOperand dst_ptr  = isa_tmp(ctx, ctx->ptr_size);
                MasmOperand dst_slot = frame_mem(ctx, ctx->sret_offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst_ptr, dst_slot));

                int32_t size = (int32_t)ctx->fn_ret_type->size;
                for (int32_t off = 0; off < size;)
                {
                    int32_t chunk = size - off;
                    if (chunk > 8)
                    {
                        chunk = 8;
                    }
                    else if (chunk > 4)
                    {
                        chunk = 4;
                    }
                    else if (chunk > 2)
                    {
                        chunk = 2;
                    }
                    else
                    {
                        chunk = 1;
                    }

                    MasmOperand tmp = isa_tmp2(ctx, (uint32_t)chunk);
                    MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                    off += chunk;
                }

                // leave rax as the sret pointer (nice for debugging)
                if (dst_ptr.reg.id != src_ptr.reg.id)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, dst_ptr));
                }
            }
        }
        else if (expr)
        {
            if (ctx->fn_ret_type && type_is_fp_class(ctx->fn_ret_type) && ctx->abi->float_ret_count > 0)
            {
                uint8_t     rsz  = type_fp_size(ctx->fn_ret_type);
                MasmOperand val  = lower_expr(masm, text, expr, ctx);
                MasmOperand bits = isa_result(ctx, rsz);
                materialize_bits_to_gpr(text, ctx, bits, val, rsz);

                uint32_t xmm0 = ctx->abi->float_ret_regs[0];
                masm_section_append_inst(text, masm_inst_2(MASM_OP_X86_MOVQ, masm_xmm(xmm0), bits));
            }
            else
            {
                MasmOperand op = lower_expr(masm, text, expr, ctx);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, isa_result(ctx, ctx->ptr_size), op));
            }
        }

        // run all deferred statements before returning
        emit_deferred_from(masm, text, ctx, 0);

        // emit epilogue before return
        MasmOperand rbp = fp_reg_op(ctx);
        MasmOperand rsp = sp_reg_op(ctx);
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rsp, rbp));
        masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, rbp));

        masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
    }
    else if (stmt->kind == AST_STMT_COMPTIME_IF || stmt->kind == AST_STMT_COMPTIME_OR)
    {
        // sema selects the active branch in taken_branch
        if (stmt->comptime_if_stmt.taken_branch)
        {
            lower_stmt(masm, text, stmt->comptime_if_stmt.taken_branch, ctx);
        }
    }
    else if (stmt->kind == AST_STMT_BLOCK)
    {
        int start_deferred = ctx->deferred_count;

        // register deferred statements for this scope (LIFO execution on exit)
        if (stmt->block_stmt.deferred_stmts)
        {
            for (int i = 0; i < stmt->block_stmt.deferred_stmts->count; i++)
            {
                push_deferred(ctx, stmt->block_stmt.deferred_stmts->items[i]);
            }
        }

        AstList *stmts = stmt->block_stmt.stmts;
        for (int i = 0; i < stmts->count; i++)
        {
            lower_stmt(masm, text, stmts->items[i], ctx);
        }

        // run defers registered in this block on normal exit
        emit_deferred_from(masm, text, ctx, start_deferred);
    }
    else if (stmt->kind == AST_STMT_VAR || stmt->kind == AST_STMT_VAL)
    {
        if (stmt->kind == AST_STMT_VAL && stmt->var_stmt.init == NULL)
        {
            fprintf(stderr, "masm lower: val must have initializer\n");
            exit(1);
        }

        // determine logical size of the variable (do not inflate small types)
        size_t var_size = 8;
        if (stmt->type)
        {
            var_size = stmt->type->size;

            // compute array size explicitly if missing
            if (var_size == 0 && stmt->type->kind == TYPE_ARRAY && stmt->type->array.elem_type)
            {
                var_size = stmt->type->array.count * stmt->type->array.elem_type->size;
            }
        }
        else if (stmt->var_stmt.init && stmt->var_stmt.init->type)
        {
            var_size = stmt->var_stmt.init->type->size;
        }

        if (var_size == 0)
        {
            var_size = ctx->ptr_size; // fallback
        }

        // allocate aligned stack space but keep the declared size for loads/stores
        size_t alloc_size = var_size;
        size_t align      = ctx->ptr_size ? ctx->ptr_size : 1;
        if (align > 1 && (alloc_size % align) != 0)
        {
            alloc_size += (align - (alloc_size % align));
        }

        ctx->stack_offset -= alloc_size;
        int32_t offset = ctx->stack_offset;

        // add to symbol table with the logical size (so later loads use correct width)
        add_local_var(ctx, stmt->var_stmt.name, offset, (int)var_size);

        // if there's an initializer, evaluate and store it
        if (stmt->var_stmt.init)
        {
            MasmOperand value = lower_expr(masm, text, stmt->var_stmt.init, ctx);

            // store value to stack location [rbp + offset]
            MasmOperand var_mem = frame_mem(ctx, offset, var_size > ctx->ptr_size ? ctx->ptr_size : (int)var_size);

            // if value is in a register, match width (use subregister when narrower)
            if (value.kind == MASM_OPERAND_REGISTER)
            {
                MasmOperand src = value;
                if (var_mem.mem.size < value.reg.size)
                {
                    src = masm_operand_register(value.reg.id, var_mem.mem.size);
                }
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, src));
            }
            // if value is immediate, move to register first using correct width
            else if (value.kind == MASM_OPERAND_IMM)
            {
                int reg_size = var_mem.mem.size;
                if (reg_size < 1)
                {
                    reg_size = 8;
                }
                MasmOperand temp = isa_tmp(ctx, reg_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, value));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, temp));
            }
            else if (value.kind == MASM_OPERAND_MEMORY)
            {
                bool has_index = value.mem.index.size != 0 || value.mem.scale != 0;

                // if indexed or small, load once into tmp then store
                if (has_index || var_size <= 8)
                {
                    int reg_size = value.mem.size ? value.mem.size : var_mem.mem.size;
                    if (reg_size == 0)
                    {
                        reg_size = 8;
                    }
                    MasmOperand tmp = isa_tmp(ctx, reg_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, value));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, tmp));
                }
                else
                {
                    // Memory to Memory copy in 8-byte chunks
                    int32_t src_disp = value.mem.disp;

                    for (size_t i = 0; i < var_size; i += 8)
                    {
                        MasmOperand src_chunk = masm_operand_memory_simple(value.mem.base.id, src_disp + i, 8);
                        MasmOperand dst_chunk = frame_mem(ctx, offset + i, ctx->ptr_size);

                        MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src_chunk));
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst_chunk, tmp));
                    }
                }
            }
        }
    }
    else if (stmt->kind == AST_STMT_BRK)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_end_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(ctx->loop_end_label))));
        }
    }
    else if (stmt->kind == AST_STMT_CNT)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_start_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(ctx->loop_start_label))));
        }
    }
    else if (stmt->kind == AST_STMT_MASM)
    {
        if (stmt->masm_stmt.content)
        {
            // parse inline masm content and emit instructions
            lower_inline_masm(masm, text, stmt->masm_stmt.content, ctx);
        }
    }
    else if (stmt->kind == AST_STMT_IF)
    {
        static int label_counter = 0;
        char       else_label[32];
        char       end_label[32];
        snprintf(else_label, sizeof(else_label), ".Lif_or_%d", label_counter);
        snprintf(end_label, sizeof(end_label), ".Lif_end_%d", label_counter++);

        // evaluate condition
        MasmOperand cond = lower_expr(masm, text, stmt->cond_stmt.cond, ctx);

        // check condition
        if (cond.kind == MASM_OPERAND_REGISTER)
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, cond, cond));
        }
        else if (cond.kind == MASM_OPERAND_IMM)
        {
            MasmOperand temp = isa_tmp(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
        }
        else
        {
            // memory or other
            MasmOperand temp = isa_tmp(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
        }

        // jump to else/end if false (zero)
        masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(else_label))));

        // lower body
        lower_stmt(masm, text, stmt->cond_stmt.body, ctx);

        // jump to end (skip else)
        masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(end_label))));

        // else label
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(else_label))));
        masm_add_symbol(masm, masm_symbol_create(else_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

        // lower else block if exists
        if (stmt->cond_stmt.stmt_or)
        {
            lower_stmt(masm, text, stmt->cond_stmt.stmt_or, ctx);
        }

        // end label
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));
        masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
    }
    else if (stmt->kind == AST_STMT_FOR)
    {
        static int loop_counter = 0;
        char       start_label[32];
        char       end_label[32];
        snprintf(start_label, sizeof(start_label), ".Lloop_start_%d", loop_counter);
        snprintf(end_label, sizeof(end_label), ".Lloop_end_%d", loop_counter++);

        int defer_mark = ctx->deferred_count;
        push_loop_defer_mark(ctx, defer_mark);

        // Save previous loop labels
        char *prev_start = ctx->loop_start_label;
        char *prev_end   = ctx->loop_end_label;

        ctx->loop_start_label = strdup(start_label);
        ctx->loop_end_label   = strdup(end_label);

        masm_add_symbol(masm, masm_symbol_create(ctx->loop_start_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
        masm_add_symbol(masm, masm_symbol_create(ctx->loop_end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

        // Emit start label
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(ctx->loop_start_label))));

        // Condition
        if (stmt->for_stmt.cond)
        {
            MasmOperand cond   = lower_expr(masm, text, stmt->for_stmt.cond, ctx);
            MasmOperand result = isa_result(ctx, ctx->ptr_size);

            if (cond.kind != MASM_OPERAND_REGISTER || cond.reg.id != isa_result_id(ctx))
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, cond));
            }

            masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, result, masm_operand_imm(0)));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(ctx->loop_end_label))));
        }

        // Body
        lower_stmt(masm, text, stmt->for_stmt.body, ctx);

        // Jump back to start
        masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(ctx->loop_start_label))));

        // Emit end label
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(ctx->loop_end_label))));

        // Restore previous loop labels
        free(ctx->loop_start_label);
        free(ctx->loop_end_label);
        ctx->loop_start_label = prev_start;
        ctx->loop_end_label   = prev_end;

        emit_deferred_from(masm, text, ctx, defer_mark);
        pop_loop_defer_mark(ctx);
    }
    else if (stmt->kind == AST_STMT_OR)
    {
        if (stmt->cond_stmt.cond)
        {
            // Same as IF
            static int label_counter = 0;
            char       else_label[32];
            char       end_label[32];
            snprintf(else_label, sizeof(else_label), ".Lor_else_%d", label_counter);
            snprintf(end_label, sizeof(end_label), ".Lor_end_%d", label_counter++);

            // evaluate condition
            MasmOperand cond = lower_expr(masm, text, stmt->cond_stmt.cond, ctx);

            // check condition
            if (cond.kind == MASM_OPERAND_REGISTER)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, cond, cond));
            }
            else if (cond.kind == MASM_OPERAND_IMM)
            {
                MasmOperand temp = isa_tmp(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
            }
            else
            {
                MasmOperand temp = isa_tmp(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
            }

            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(else_label))));

            lower_stmt(masm, text, stmt->cond_stmt.body, ctx);

            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(end_label))));

            masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(else_label))));
            masm_add_symbol(masm, masm_symbol_create(else_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

            if (stmt->cond_stmt.stmt_or)
            {
                lower_stmt(masm, text, stmt->cond_stmt.stmt_or, ctx);
            }

            masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));
            masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
        }
        else
        {
            // Unconditional OR (else)
            lower_stmt(masm, text, stmt->cond_stmt.body, ctx);
        }
    }
    else if (stmt->kind == AST_STMT_FOR)
    {
        static int label_counter = 0;
        char       start_label[32];
        char       end_label[32];
        snprintf(start_label, sizeof(start_label), ".Lfor_start_%d", label_counter);
        snprintf(end_label, sizeof(end_label), ".Lfor_end_%d", label_counter++);

        int defer_mark = ctx->deferred_count;
        push_loop_defer_mark(ctx, defer_mark);

        // save previous loop labels
        char *prev_start = ctx->loop_start_label;
        char *prev_end   = ctx->loop_end_label;

        // set new loop labels (must be duplicated because they are stack allocated here)
        // actually, we can just use strdup once and free later, or rely on the fact that we use them immediately.
        // But lower_stmt is recursive, so we need them to persist during the recursion.
        // We can use the stack buffer if we are careful, but strdup is safer.
        ctx->loop_start_label = strdup(start_label);
        ctx->loop_end_label   = strdup(end_label);

        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(start_label))));
        masm_add_symbol(masm, masm_symbol_create(start_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

        // check condition if exists
        if (stmt->for_stmt.cond)
        {
            MasmOperand cond = lower_expr(masm, text, stmt->for_stmt.cond, ctx);

            if (cond.kind == MASM_OPERAND_REGISTER)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, cond, cond));
            }
            else if (cond.kind == MASM_OPERAND_IMM)
            {
                MasmOperand temp = isa_tmp(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
            }
            else
            {
                MasmOperand temp = isa_tmp(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
            }

            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(end_label))));
        }

        // lower body
        lower_stmt(masm, text, stmt->for_stmt.body, ctx);

        // jump back to start
        masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(start_label))));

        // end label
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(end_label))));
        masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

        // restore previous loop labels
        // free current ones (cast away const)
        free((void *)ctx->loop_start_label);
        free((void *)ctx->loop_end_label);
        ctx->loop_start_label = prev_start;
        ctx->loop_end_label   = prev_end;

        emit_deferred_from(masm, text, ctx, defer_mark);
        pop_loop_defer_mark(ctx);
    }
    else if (stmt->kind == AST_STMT_BRK)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_end_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(ctx->loop_end_label))));
        }
    }
    else if (stmt->kind == AST_STMT_CNT)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_start_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(strdup(ctx->loop_start_label))));
        }
    }
    else if (stmt->kind == AST_STMT_EXPR)
    {
        lower_expr(masm, text, stmt->expr_stmt.expr, ctx);
    }
}

static void lower_inline_masm(Masm *masm, MasmSection *text, const char *content, LowerContext *ctx)
{
    (void)masm;

    // simple parser for inline masm blocks
    // format: "opcode operand1, operand2"
    // for now, support basic syscall pattern
    char *line    = strdup(content);
    char *saveptr = NULL;
    char *token   = strtok_r(line, "\n;", &saveptr);

    while (token)
    {
        // trim leading whitespace
        while (*token == ' ' || *token == '\t')
        {
            token++;
        }

        // strip inline comments starting with '#'
        char *comment = strchr(token, '#');
        if (comment)
        {
            *comment = '\0';
        }

        // trim trailing whitespace now that comments are stripped
        size_t tlen = strlen(token);
        while (tlen > 0 && (token[tlen - 1] == ' ' || token[tlen - 1] == '\t' || token[tlen - 1] == '\r'))
        {
            token[--tlen] = '\0';
        }

        if (*token == '\0')
        {
            token = strtok_r(NULL, "\n;", &saveptr);
            continue;
        }

        if (strncmp(token, "syscall", 7) == 0)
        {
            uint32_t op = ctx->isa->op_syscall ? ctx->isa->op_syscall() : UINT32_MAX;
            if (op == UINT32_MAX)
            {
                fprintf(stderr, "masm inline: syscall unsupported for isa %s\n", masm_target_isa_name(ctx->target.isa));
                exit(1);
            }
            masm_section_append_inst(text, masm_inst_0(op));
        }
        else if (strncmp(token, "call ", 5) == 0)
        {
            char *label = token + 5;
            while (*label == ' ')
            {
                label++;
            }

            // trim trailing whitespace
            size_t len = strlen(label);
            while (len > 0 && (label[len - 1] == ' ' || label[len - 1] == '\t' || label[len - 1] == '\r'))
            {
                label[--len] = '\0';
            }

            MasmOperand target = masm_operand_label(strdup(label));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, target));
        }
        else if (strncmp(token, "ret", 3) == 0)
        {
            masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
        }
        else if (strncmp(token, "cmp ", 4) == 0)
        {
            // parse cmp instruction: "cmp rax, rcx"
            char *operands = token + 4;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, dst_op, src_op));
            }
        }
        else if (strncmp(token, "xor ", 4) == 0)
        {
            // parse xor instruction: "xor eax, eax"
            char *operands = token + 4;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_XOR, dst_op, src_op));
            }
        }
        else if (strncmp(token, "sete ", 5) == 0)
        {
            char *reg = token + 5;
            while (*reg == ' ')
            {
                reg++;
            }
            MasmOperand dst_op = parse_operand(reg, ctx);
            masm_section_append_inst(text, masm_inst_1(MASM_OP_SETE, dst_op));
        }
        else if (strncmp(token, "lea ", 4) == 0)
        {
            char *operands = token + 4;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst_op, src_op));
            }
        }
        else if (strncmp(token, "and ", 4) == 0)
        {
            char *operands = token + 4;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_AND, dst_op, src_op));
            }
        }
        else if (strncmp(token, "sub ", 4) == 0)
        {
            char *operands = token + 4;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_SUB, dst_op, src_op));
            }
        }
        else if (strncmp(token, "mov ", 4) == 0)
        {
            // parse mov instruction: "mov rax, 60"
            char *operands = token + 4;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                // trim whitespace
                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                // parse destination register
                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst_op, src_op));
            }
        }
        else if (strncmp(token, "movzx ", 6) == 0)
        {
            char *operands = token + 6;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVZX, dst_op, src_op));
            }
        }
        else if (strncmp(token, "movsx ", 6) == 0)
        {
            char *operands = token + 6;
            char *comma    = strchr(operands, ',');
            if (comma)
            {
                *comma     = '\0';
                char *dest = operands;
                char *src  = comma + 1;

                while (*dest == ' ')
                {
                    dest++;
                }
                while (*src == ' ')
                {
                    src++;
                }

                MasmOperand dst_op = parse_operand(dest, ctx);
                MasmOperand src_op = parse_operand(src, ctx);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOVSX, dst_op, src_op));
            }
        }

        token = strtok_r(NULL, "\n;", &saveptr);
    }

    free(line);
}

static MasmOperand parse_operand(const char *str, LowerContext *ctx)
{
    // ISA-provided register parse
    if (ctx && ctx->isa && ctx->isa->parse_reg)
    {
        MasmOperand reg = ctx->isa->parse_reg(str, ctx->ptr_size);
        if (reg.kind != MASM_OPERAND_NONE)
        {
            return reg;
        }
    }

    // parse simple memory operands: [reg] or [reg+imm]
    if (str[0] == '[')
    {
        size_t len = strlen(str);
        if (len >= 3 && str[len - 1] == ']')
        {
            char   inner[64];
            size_t copy_len = len - 2 < sizeof(inner) - 1 ? len - 2 : sizeof(inner) - 1;
            memcpy(inner, str + 1, copy_len);
            inner[copy_len] = '\0';

            // split on '+' if present
            char *plus    = strchr(inner, '+');
            char *reg_str = inner;
            char *off_str = NULL;
            if (plus)
            {
                *plus   = '\0';
                off_str = plus + 1;
            }

            MasmOperand base = parse_operand(reg_str, ctx);
            if (base.kind == MASM_OPERAND_REGISTER)
            {
                int64_t disp = 0;
                if (off_str)
                {
                    disp = strtoll(off_str, NULL, 0);
                }
                return masm_operand_memory_simple(base.reg.id, (int32_t)disp, 8);
            }
        }
    }

    // parse immediate (number)
    char *end;
    long  val = strtol(str, &end, 10);
    if (str != end && *end == '\0')
    {
        return masm_operand_imm(val);
    }

    // parse variable
    if (ctx)
    {
        LocalVar *var = find_local_var(ctx, str);
        if (var)
        {
            return frame_mem(ctx, var->offset, var->size);
        }
    }

    return masm_operand_none();
}

static void lower_function(Masm *masm, AstNode *func_node, SymbolTable *symbols)
{
    (void)symbols;

    // guard: this lowering path is currently x86_64-only. fail fast on other ISAs
    if (masm->target.isa != MASM_ISA_X86_64)
    {
        fprintf(stderr, "masm lower: unsupported isa for lowering (isa=%s)\n", masm_target_isa_name(masm->target.isa));
        exit(1);
    }

    // determine linkage name (mangled unless overridden) and entry flag
    const char *ast_name  = func_node->fun_stmt.name;
    const char *func_name = ast_name;

    if (func_node->symbol)
    {
        const char *link_name = symbol_get_linkage_name(func_node->symbol);
        if (link_name)
        {
            func_name = link_name;
        }
    }

    bool is_entry = func_name && strcmp(func_name, "_start") == 0;

#ifdef MASM_DEBUG
    fprintf(stderr, "[lower] func %s (entry=%d)\n", func_name, is_entry);
#endif

    char       *func_name_copy = strdup(func_name);
    MasmSymbol *sym            = masm_symbol_create(func_name_copy, MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
    masm_add_symbol(masm, sym);

    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);

    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(func_name))));

    // create context for this function
    LowerContext *ctx = create_context(masm->target);

    // determine whether this function returns an aggregate via an implicit sret pointer.
    // sema stores the function type on the symbol (node->type is not reliably populated for fun stmts).
    Type *fn_type = NULL;
    if (func_node->symbol && func_node->symbol->type && func_node->symbol->type->kind == TYPE_FUNCTION)
    {
        fn_type = func_node->symbol->type;
    }
    else if (func_node->type && func_node->type->kind == TYPE_FUNCTION)
    {
        fn_type = func_node->type;
    }

    if (fn_type)
    {
        ctx->fn_ret_type = fn_type->function.return_type;
    }

    // fallback: infer return type from the body. this helps when the symbol's function type
    // isn't reliably populated for instantiated generics.
    Type *inferred_ret = infer_ret_type_from_stmt(func_node->fun_stmt.body);
    if ((!ctx->fn_ret_type || ctx->fn_ret_type->size == 0 || !type_is_large_aggregate(ctx->fn_ret_type, ctx->ptr_size)) && inferred_ret && type_is_large_aggregate(inferred_ret, ctx->ptr_size))
    {
        ctx->fn_ret_type = inferred_ret;
    }

    if (type_is_large_aggregate(ctx->fn_ret_type, ctx->ptr_size))
    {
        ctx->fn_has_sret = true;
    }

    MasmOperand rbp = fp_reg_op(ctx);
    MasmOperand rsp = sp_reg_op(ctx);

    size_t sub_rsp_idx = 0;
    if (!is_entry)
    {
        // emit prologue: push rbp; mov rbp, rsp
        masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, rbp));
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rbp, rsp));

        // reserve space for locals (placeholder, will be patched)
        sub_rsp_idx = text->inst_count;
        masm_section_append_inst(text, masm_inst_2(MASM_OP_SUB, rsp, masm_operand_imm(0)));
    }

    // hidden sret pointer is passed in arg0 for aggregate returns.
    // stash it in the frame so return statements can copy into it.
    int arg_shift = 0;
    if (!is_entry && ctx->fn_has_sret)
    {
        ctx->stack_offset -= (int32_t)ctx->ptr_size;
        ctx->sret_offset = ctx->stack_offset;
        arg_shift        = 1;

        MasmOperand src = masm_operand_register(ctx->abi->int_arg_regs[0], ctx->ptr_size);
        MasmOperand dst = frame_mem(ctx, ctx->sret_offset, ctx->ptr_size);
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
    }

    // detect and reserve register save area for variadic functions (non-entry)
    if (!is_entry)
    {
        bool fn_is_variadic = false;
        if (func_node->fun_stmt.is_variadic)
        {
            fn_is_variadic = true;
        }
        else if (fn_type && fn_type->kind == TYPE_FUNCTION && fn_type->function.param_count > 0 && fn_type->function.param_types[fn_type->function.param_count - 1] == NULL)
        {
            fn_is_variadic = true;
        }

        if (fn_is_variadic)
        {
            ctx->fn_is_variadic = true;
            // allocate 176 bytes: 6 GP regs (48) + 8 XMM slots (128)
            ctx->stack_offset -= (int32_t)176;
            int align = ctx->stack_align ? ctx->stack_align : 16;
            if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
            {
                ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
            }
            ctx->va_reg_save_off = ctx->stack_offset;
        }
    }

    // handle parameters (skip for _start)
    if (!is_entry && func_node->fun_stmt.params)
    {
        int gp_i    = arg_shift;
        int fp_i    = 0;
        int stack_i = 0;

        AstList *params = func_node->fun_stmt.params;
        for (int i = 0; i < params->count; i++)
        {
            AstNode *param      = params->items[i];
            Type    *ptype      = param ? param->type : NULL;
            size_t   param_size = ctx->ptr_size;
            if (ptype && ptype->size)
            {
                param_size = ptype->size;
            }
            else if (ptype && ptype->kind == TYPE_ARRAY && ptype->array.elem_type)
            {
                param_size = ptype->array.count * ptype->array.elem_type->size;
            }
            if (param_size == 0)
            {
                param_size = ctx->ptr_size;
            }

            bool byval_ptr = type_is_large_aggregate(ptype, ctx->ptr_size);
            bool is_fp     = !byval_ptr && type_is_fp_class(ptype);

            // allocate aligned stack space for the local parameter copy
            size_t alloc_size = param_size;
            size_t align      = ctx->ptr_size ? ctx->ptr_size : 1;
            if (align > 1 && (alloc_size % align) != 0)
            {
                alloc_size += (align - (alloc_size % align));
            }

            ctx->stack_offset -= (int32_t)alloc_size;
            int32_t offset = ctx->stack_offset;

            // add to symbol table with its logical size
            add_local_var(ctx, param->param_stmt.name, offset, (int)param_size);

            // load from ABI arg location
            if (!byval_ptr && is_fp)
            {
                uint8_t     store_size = type_fp_size(ptype);
                MasmOperand dst        = frame_mem(ctx, offset, store_size);

                if (fp_i < ctx->float_arg_count)
                {
                    uint32_t xmm_id = ctx->abi->float_arg_regs[fp_i++];
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_X86_MOVQ, dst, masm_xmm(xmm_id)));
                }
                else
                {
                    int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (stack_i++ * ctx->ptr_size));
                    MasmOperand src_mem            = frame_mem(ctx, stack_param_offset, store_size);
                    MasmOperand tmp                = isa_result(ctx, store_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src_mem));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                }
            }
            else if (!byval_ptr)
            {
                uint8_t     store_size = (uint8_t)(param_size > ctx->ptr_size ? ctx->ptr_size : param_size);
                MasmOperand dst        = frame_mem(ctx, offset, store_size);

                if (gp_i < ctx->int_arg_count)
                {
                    uint32_t    reg = ctx->abi->int_arg_regs[gp_i++];
                    MasmOperand src = masm_operand_register(reg, store_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
                }
                else
                {
                    int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (stack_i++ * ctx->ptr_size));
                    MasmOperand src_mem            = frame_mem(ctx, stack_param_offset, ctx->ptr_size);
                    MasmOperand tmp                = isa_tmp(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src_mem));
                    MasmOperand tmp_n = masm_operand_register(tmp.reg.id, store_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp_n));
                }
            }
            else
            {
                // large aggregate passed by pointer: copy into our local value slot
                MasmOperand src_ptr = isa_result(ctx, ctx->ptr_size);

                if (gp_i < ctx->int_arg_count)
                {
                    uint32_t    reg = ctx->abi->int_arg_regs[gp_i++];
                    MasmOperand src = masm_operand_register(reg, ctx->ptr_size);
                    if (src.reg.id != src_ptr.reg.id)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src));
                    }
                }
                else
                {
                    int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (stack_i++ * ctx->ptr_size));
                    MasmOperand src_mem            = frame_mem(ctx, stack_param_offset, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, src_ptr, src_mem));
                }

                MasmOperand dst_ptr  = isa_tmp(ctx, ctx->ptr_size);
                MasmOperand dst_addr = frame_mem(ctx, offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst_ptr, dst_addr));

                for (int32_t off = 0; off < (int32_t)param_size;)
                {
                    int32_t chunk = (int32_t)param_size - off;
                    if (chunk > 8)
                    {
                        chunk = 8;
                    }
                    else if (chunk > 4)
                    {
                        chunk = 4;
                    }
                    else if (chunk > 2)
                    {
                        chunk = 2;
                    }
                    else
                    {
                        chunk = 1;
                    }

                    MasmOperand tmp = isa_tmp2(ctx, (uint32_t)chunk);
                    MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                    off += chunk;
                }
            }
        }

        // if variadic, record named counts and save register-save area
        if (ctx->fn_is_variadic)
        {
            ctx->va_named_gp          = gp_i - arg_shift;
            ctx->va_named_fp          = fp_i;
            ctx->va_named_stack_slots = stack_i;

            // save GP regs (RDI, RSI, RDX, RCX, R8, R9) into reg_save_area offsets 0,8,16,24,32,40
            for (int i = 0; i < 6; i++)
            {
                int32_t     off = ctx->va_reg_save_off + (i * ctx->ptr_size);
                MasmOperand dst = frame_mem(ctx, off, ctx->ptr_size);
                MasmOperand src = masm_operand_register(ctx->abi->int_arg_regs[i], ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
            }

            // save low 8 bytes of XMM0..XMM7 into offsets 48 + 16*i
            for (int i = 0; i < 8; i++)
            {
                int32_t     off   = ctx->va_reg_save_off + (48 + i * 16);
                MasmOperand dst   = frame_mem(ctx, off, 8);
                uint32_t    xmmid = ctx->abi->float_arg_regs[i];
                masm_section_append_inst(text, masm_inst_2(MASM_OP_X86_MOVQ, dst, masm_xmm(xmmid)));
            }
        }
    }

    if (func_node->fun_stmt.body)
    {
        lower_stmt(masm, text, func_node->fun_stmt.body, ctx);
    }

    if (!is_entry)
    {
        // patch stack allocation
        int32_t frame_size = -ctx->stack_offset;
        uint8_t align      = ctx->stack_align ? ctx->stack_align : 16;
        if (frame_size % align != 0)
        {
            frame_size = (frame_size + (align - 1)) & ~(align - 1);
        }

        if (frame_size > 0)
        {
            text->instructions[sub_rsp_idx].operands[1].imm = frame_size;
        }
    }

    // emit remaining deferred statements for fallthrough paths
    emit_deferred_from(masm, text, ctx, 0);

    if (!is_entry)
    {
        // implicit epilogue for functions without explicit return
        MasmOperand rbp = fp_reg_op(ctx);
        MasmOperand rsp = sp_reg_op(ctx);
        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rsp, rbp));
        masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, rbp));
        masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
    }

    destroy_context(ctx);
}

static void lower_global_var(Masm *masm, AstNode *stmt);

static bool lowered_name_has(char **names, int count, const char *name)
{
    if (!names || !name)
    {
        return false;
    }
    for (int i = 0; i < count; i++)
    {
        if (names[i] && strcmp(names[i], name) == 0)
        {
            return true;
        }
    }
    return false;
}

static void lowered_name_add(char ***names, int *count, int *cap, const char *name)
{
    if (!names || !count || !cap || !name)
    {
        return;
    }
    if (lowered_name_has(*names, *count, name))
    {
        return;
    }
    if (*count >= *cap)
    {
        *cap   = (*cap) ? (*cap) * 2 : 16;
        *names = realloc(*names, sizeof(char *) * (*cap));
    }
    (*names)[(*count)++] = strdup(name);
}

Masm *masm_lower_module(AstNode *ast, SymbolTable *symbols)
{
    MasmTarget target = masm_target_native();
    Masm      *masm   = masm_create(target);

    // track lowered function names to avoid duplicate emission when also lowering
    // instantiated generic functions that are not present in the original AST.
    char **lowered_names = NULL;
    int    lowered_count = 0;
    int    lowered_cap   = 0;

    if (ast->kind == AST_PROGRAM)
    {
        AstList *stmts = ast->program.stmts;

        // pass 1: lower global vars/vals first so their symbols exist when lowering functions.
        // functions may reference globals (e.g. `true`/`false`) and need `masm_get_symbol()` to succeed.
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *decl = stmts->items[i];
            if (decl->kind == AST_STMT_VAR || decl->kind == AST_STMT_VAL)
            {
                lower_global_var(masm, decl);
            }
        }

        // pass 2: lower functions.
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *decl = stmts->items[i];
            if (decl->kind == AST_STMT_FUN)
            {
                // skip generic templates; only instantiated functions are lowered.
                if (decl->fun_stmt.generics && decl->fun_stmt.generics->count > 0)
                {
                    continue;
                }
                if (decl->symbol && decl->symbol->is_generic)
                {
                    continue;
                }

                lower_function(masm, decl, symbols);
                lowered_name_add(&lowered_names, &lowered_count, &lowered_cap, decl->fun_stmt.name);
            }
        }

        // lower instantiated generic functions that are not present in the original AST.
        if (symbols)
        {
            for (Symbol *sym = symbols->symbols; sym; sym = sym->next)
            {
                if (sym->kind != SYMBOL_FUNCTION || sym->is_generic)
                {
                    continue;
                }
                if (!sym->decl || sym->decl->kind != AST_STMT_FUN)
                {
                    continue;
                }
                AstNode *fn = sym->decl;
                if (fn->fun_stmt.generics && fn->fun_stmt.generics->count > 0)
                {
                    continue;
                }
                if (lowered_name_has(lowered_names, lowered_count, fn->fun_stmt.name))
                {
                    continue;
                }

                lower_function(masm, fn, symbols);
                lowered_name_add(&lowered_names, &lowered_count, &lowered_cap, fn->fun_stmt.name);
            }
        }
    }

    for (int i = 0; i < lowered_count; i++)
    {
        free(lowered_names[i]);
    }
    free(lowered_names);

    return masm;
}

static void lower_global_var(Masm *masm, AstNode *stmt)
{
    const char *name = stmt->var_stmt.name;
    if (stmt->symbol)
    {
        const char *link_name = symbol_get_linkage_name(stmt->symbol);
        if (link_name)
        {
            name = link_name;
        }
    }

    // Determine size
    size_t size = 8;
    if (stmt->type)
    {
        size = stmt->type->size;
    }
    else if (stmt->var_stmt.init && stmt->var_stmt.init->type)
    {
        size = stmt->var_stmt.init->type->size;
    }

    if (size == 0)
    {
        size = 8;
    }

    // Determine section and initial value
    bool     is_bss   = true;
    uint64_t init_val = 0;

    if (stmt->var_stmt.init)
    {
        // printf("Global var %s init kind: %d\n", name, stmt->var_stmt.init->kind);
        if (stmt->var_stmt.init->kind == AST_EXPR_LIT)
        {
            // printf("Lit kind: %d\n", stmt->var_stmt.init->lit_expr.kind);
            if (stmt->var_stmt.init->lit_expr.kind == TOKEN_LIT_INT)
            {
                is_bss   = false;
                init_val = stmt->var_stmt.init->lit_expr.int_val;
                // printf("Int val: %ld\n", init_val);
            }
        }
    }

    MasmSection *section;
    if (is_bss)
    {
        section = masm_get_or_create_section(masm, ".bss", MASM_SECTION_BSS);
    }
    else
    {
        section = masm_get_or_create_section(masm, ".data", MASM_SECTION_DATA);
    }

    // Create symbol
    MasmSymbol *sym   = masm_symbol_create(name, MASM_SYMBOL_DATA, MASM_BIND_GLOBAL);
    sym->section_name = strdup(section->name);
    sym->offset       = section->data_size;
    sym->size         = size;

    masm_add_symbol(masm, sym);

    // Append data
    if (is_bss)
    {
        masm_section_append_zero(section, size);
    }
    else
    {
        // Write value (little endian)
        masm_section_append_data(section, &init_val, size > 8 ? 8 : size);
        if (size > 8)
        {
            masm_section_append_zero(section, size - 8);
        }
    }
}
