#include "compiler/masm/lower.h"
#include "compiler/masm/abi/spec.h"
#include "compiler/masm/ir.h"
#include "compiler/masm/isa/spec.h"
#include "compiler/masm/masm.h"
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

    // Virtual register counter
    uint32_t vreg_next;

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

    // label generation counters (module-wide, persist across functions)
    int str_counter;
    int label_counter;
    int loop_counter;
} LowerContext;

// Select ISA spec via shared selector
static const MasmISASpec *lower_select_isa(MasmTarget target)
{
    return masm_isa_spec_select(target);
}

static MasmOperand alloc_vreg(LowerContext *ctx, uint8_t size)
{
    return masm_operand_register(ctx->vreg_next++, size);
}

static MasmOperand alloc_vreg_fp(LowerContext *ctx, uint8_t size)
{
    return masm_operand_register_fp(ctx->vreg_next++, size);
}

static inline MasmOperand isa_result(LowerContext *ctx, uint8_t size)
{
    return alloc_vreg(ctx, size);
}
static inline MasmOperand isa_tmp(LowerContext *ctx, uint8_t size)
{
    return alloc_vreg(ctx, size);
}
static inline __attribute__((unused)) MasmOperand isa_tmp2(LowerContext *ctx, uint8_t size)
{
    return alloc_vreg(ctx, size);
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

static inline __attribute__((unused)) uint32_t isa_tmp_id(LowerContext *ctx)
{
    (void)ctx;
    return UINT32_MAX;
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
    ctx->vreg_next           = 1024;

    ctx->fn_ret_type          = NULL;
    ctx->fn_has_sret          = false;
    ctx->sret_offset          = 0;
    ctx->fn_is_variadic       = false;
    ctx->va_reg_save_off      = 0;
    ctx->va_named_gp          = 0;
    ctx->va_named_fp          = 0;
    ctx->va_named_stack_slots = 0;

    // label generation counters
    ctx->str_counter   = 0;
    ctx->label_counter = 0;
    ctx->loop_counter  = 0;
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
    }
    free(ctx);
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

static inline bool type_is_aggregate(Type *t)
{
    return t && (t->kind == TYPE_STRUCT || t->kind == TYPE_UNION || t->kind == TYPE_ARRAY);
}

static inline void emit_aggregate_copy(MasmSection *text, LowerContext *ctx, MasmOperand dst_ptr, MasmOperand src_ptr, size_t size)
{
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
        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, tmp, src));
        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst, tmp));
        off += chunk;
    }
}

static inline bool type_is_fp_class(Type *t)
{
    return t && (t->kind == TYPE_F32 || t->kind == TYPE_F64);
}

static inline bool type_is_signed(Type *t)
{
    return t && (t->kind == TYPE_I8 || t->kind == TYPE_I16 || t->kind == TYPE_I32 || t->kind == TYPE_I64);
}



static inline uint8_t type_fp_size(Type *t)
{
    if (!t)
    {
        return 8;
    }
    return (t->kind == TYPE_F32) ? 4 : 8;
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

static MasmOperand lower_expr(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx);

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

static MasmOperand ensure_in_reg(MasmSection *text, MasmOperand op, Type *type, LowerContext *ctx)
{
    if (op.kind != MASM_OPERAND_MEMORY)
    {
        return op;
    }

    uint8_t size = op.mem.size;
    if (size == 0 && type) size = type->size;
    if (size == 0) size = 8;

    MasmOperand reg = alloc_vreg(ctx, size);

    MasmTypeKind tk = MASM_TYPE_I64;
    bool is_signed = type ? type_is_signed(type) : true;

    if (type && type_is_fp_class(type))
    {
        if (size == 4) tk = MASM_TYPE_F32;
        else tk = MASM_TYPE_F64;
    }
    else
    {
        switch (size)
        {
        case 1: tk = is_signed ? MASM_TYPE_I8 : MASM_TYPE_U8; break;
        case 2: tk = is_signed ? MASM_TYPE_I16 : MASM_TYPE_U16; break;
        case 4: tk = is_signed ? MASM_TYPE_I32 : MASM_TYPE_U32; break;
        default: tk = is_signed ? MASM_TYPE_I64 : MASM_TYPE_U64; break;
        }
    }

    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, reg, op, masm_operand_type(tk)));
    return reg;
}

static MasmOperand lower_binary_op(MasmSection *text, TokenKind op, MasmOperand left, MasmOperand right, Type *result_type, Type *operand_type, LowerContext *ctx)
{
    left  = ensure_in_reg(text, left, operand_type, ctx);
    right = ensure_in_reg(text, right, operand_type, ctx);

    bool is_signed = operand_type ? type_is_signed(operand_type) : true;
    bool is_float  = operand_type ? type_is_float(operand_type) : false;
    int  size      = result_type ? result_type->size : 8;
    if (size == 0)
        size = 8;

    MasmIrOpcode opcode;

    if (is_float)
    {
        switch (op)
        {
        case TOKEN_PLUS:  opcode = MASM_IR_FADD; break;
        case TOKEN_MINUS: opcode = MASM_IR_FSUB; break;
        case TOKEN_STAR:  opcode = MASM_IR_FMUL; break;
        case TOKEN_SLASH: opcode = MASM_IR_FDIV; break;
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_BANG_EQUAL:
        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL: {
            MasmIrFcmpCond cond;
            switch (op)
            {
            case TOKEN_EQUAL_EQUAL:   cond = MASM_IR_FCMP_EQ; break;
            case TOKEN_BANG_EQUAL:    cond = MASM_IR_FCMP_NE; break;
            case TOKEN_LESS:          cond = MASM_IR_FCMP_LT; break;
            case TOKEN_LESS_EQUAL:    cond = MASM_IR_FCMP_LE; break;
            case TOKEN_GREATER:       cond = MASM_IR_FCMP_GT; break;
            case TOKEN_GREATER_EQUAL: cond = MASM_IR_FCMP_GE; break;
            default:
                fprintf(stderr, "masm lower: unhandled float comparison %d\n", op);
                exit(1);
            }
            MasmOperand res = isa_result(ctx, size);
            masm_section_append_inst(text, masm_inst_4(MASM_IR_FCMP, res, left, right, masm_operand_imm(cond)));
            return res;
        }
        default:
            fprintf(stderr, "masm lower: unhandled float binary op %d\n", op);
            exit(1);
        }
    }
    else
    {
        switch (op)
        {
        case TOKEN_PLUS:          opcode = MASM_IR_ADD; break;
        case TOKEN_MINUS:         opcode = MASM_IR_SUB; break;
        case TOKEN_STAR:          opcode = MASM_IR_MUL; break;
        case TOKEN_SLASH:         opcode = is_signed ? MASM_IR_DIV : MASM_IR_DIVU; break;
        case TOKEN_PERCENT:       opcode = is_signed ? MASM_IR_REM : MASM_IR_REMU; break;
        case TOKEN_AMPERSAND:     opcode = MASM_IR_AND; break;
        case TOKEN_PIPE:          opcode = MASM_IR_OR; break;
        case TOKEN_CARET:         opcode = MASM_IR_XOR; break;
        case TOKEN_LESS_LESS:     opcode = MASM_IR_SHL; break;
        case TOKEN_GREATER_GREATER: opcode = is_signed ? MASM_IR_SAR : MASM_IR_SHR; break;
        case TOKEN_EQUAL_EQUAL:   opcode = MASM_IR_SEQ; break;
        case TOKEN_BANG_EQUAL:    opcode = MASM_IR_SNE; break;
        case TOKEN_LESS:          opcode = is_signed ? MASM_IR_SLT : MASM_IR_SLTU; break;
        case TOKEN_LESS_EQUAL:    opcode = is_signed ? MASM_IR_SLE : MASM_IR_SLEU; break;
        case TOKEN_GREATER:       opcode = is_signed ? MASM_IR_SGT : MASM_IR_SGTU; break;
        case TOKEN_GREATER_EQUAL: opcode = is_signed ? MASM_IR_SGE : MASM_IR_SGEU; break;
        default:
            fprintf(stderr, "masm lower: unhandled binary op %d\n", op);
            exit(1);
        }
    }

    MasmOperand res = isa_result(ctx, size);
    masm_section_append_inst(text, masm_inst_3(opcode, res, left, right));
    return res;
}

static MasmOperand lower_short_circuit(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx)
{
    char label1[32];
    char label_end[32];
    snprintf(label1, sizeof(label1), ".Lsc_%d", ctx->label_counter);
    snprintf(label_end, sizeof(label_end), ".Lsc_end_%d", ctx->label_counter++);

    bool is_and = (expr->binary_expr.op == TOKEN_AMPERSAND_AMPERSAND);

    // Result register
    MasmOperand res = isa_result(ctx, 8);

    // Evaluate Left
    MasmOperand left = lower_expr(masm, text, expr->binary_expr.left, ctx);

    // Move left to result
    masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, res, left));

    if (is_and)
    {
        // AND: if res == 0, jump to end (result is 0)
        masm_section_append_inst(text, masm_inst_3(MASM_IR_BEQ, res, masm_operand_imm(0), masm_operand_label(label_end)));
    }
    else
    {
        // OR: if res != 0, jump to label1 (set result to 1)
        masm_section_append_inst(text, masm_inst_3(MASM_IR_BNE, res, masm_operand_imm(0), masm_operand_label(label1)));
    }

    // Evaluate Right
    MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);

    // Result = (right != 0)
    masm_section_append_inst(text, masm_inst_3(MASM_IR_SNE, res, right, masm_operand_imm(0)));
    masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(label_end)));

    if (!is_and)
    {
        // Label True (for OR short-circuit)
        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(label1)));
        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, res, masm_operand_imm(1)));
        masm_add_symbol(masm, masm_symbol_create(label1, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
    }

    masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(label_end)));
    masm_add_symbol(masm, masm_symbol_create(label_end, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

    return res;
}

static MasmOperand lower_assign(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx)
{
    AstNode *lhs = expr->binary_expr.left;
    AstNode *rhs = expr->binary_expr.right;

    // Handle dereference assignment: @ptr = val
    if (lhs->kind == AST_EXPR_UNARY && lhs->unary_expr.op == TOKEN_AT)
    {
        MasmOperand ptr = lower_expr(masm, text, lhs->unary_expr.expr, ctx);
        
        // Ensure ptr is a register
        if (ptr.kind != MASM_OPERAND_REGISTER)
        {
            MasmOperand new_ptr = isa_result(ctx, 8);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, new_ptr, ptr));
            ptr = new_ptr;
        }

        MasmOperand val  = lower_expr(masm, text, rhs, ctx);
        int         size = lhs->type ? lhs->type->size : 8;
        if (size == 0) size = 8;

        MasmOperand mem = masm_operand_memory_simple(ptr.reg.id, 0, size);
        masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, mem, val, masm_operand_imm(size)));
        return val;
    }
    // Handle generic lvalue (index, field)
    else if (lhs->kind == AST_EXPR_INDEX || lhs->kind == AST_EXPR_FIELD)
    {
        MasmOperand left_mem = lower_expr(masm, text, lhs, ctx);
        
        MasmOperand addr = isa_result(ctx, 8);
        masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, addr, left_mem));

        MasmOperand val = lower_expr(masm, text, rhs, ctx);
        
        int         size = lhs->type ? lhs->type->size : 8;
        if (size == 0) size = 8;
        
        MasmOperand mem = masm_operand_memory_simple(addr.reg.id, 0, size);
        masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, mem, val, masm_operand_imm(size)));
        return val;
    }
    // Handle identifier assignment
    else if (lhs->kind == AST_EXPR_IDENT)
    {
        LocalVar *var = find_local_var(ctx, lhs->ident_expr.name);
        if (var)
        {
            MasmOperand val = lower_expr(masm, text, rhs, ctx);
            MasmOperand dst = frame_mem(ctx, var->offset, var->size);
            masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, val, masm_operand_imm(var->size)));
            return val;
        }
        else
        {
            // Global
            MasmOperand sym_op = masm_operand_symbol(lhs->ident_expr.name);
            if (lhs->symbol)
            {
                const char *link_name = symbol_get_linkage_name(lhs->symbol);
                if (link_name) sym_op = masm_operand_symbol(link_name);
            }
            
            MasmOperand addr = isa_result(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, addr, sym_op));
            
            MasmOperand val = lower_expr(masm, text, rhs, ctx);
            
            int size = lhs->type ? lhs->type->size : ctx->ptr_size;
            MasmOperand mem = masm_operand_memory_simple(addr.reg.id, 0, size);
            masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, mem, val, masm_operand_imm(size)));
            return val;
        }
    }

    return masm_operand_none();
}

static MasmOperand lower_unary_op(MasmSection *text, TokenKind op, MasmOperand operand, Type *type, LowerContext *ctx)
{
    operand = ensure_in_reg(text, operand, type, ctx);
    int size = type ? type->size : 8;
    if (size == 0) size = 8;

    MasmOperand res = isa_result(ctx, size);

    if (op == TOKEN_BANG)
    {
        // !x -> x == 0
        masm_section_append_inst(text, masm_inst_3(MASM_IR_SEQ, res, operand, masm_operand_imm(0)));
    }
    else if (op == TOKEN_MINUS)
    {
        // -x -> 0 - x (neg)
        masm_section_append_inst(text, masm_inst_2(MASM_IR_NEG, res, operand));
    }
    else if (op == TOKEN_TILDE)
    {
        // ~x
        masm_section_append_inst(text, masm_inst_2(MASM_IR_NOT, res, operand));
    }
    else
    {
        fprintf(stderr, "masm lower: unhandled unary op %d\n", op);
        exit(1);
    }
    return res;
}

static MasmOperand lower_call(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx)
{
    // Handle varargs builtins (stub for now, defer to backend or implement later)
    if (ctx->fn_is_variadic && expr->call_expr.func && expr->call_expr.func->kind == AST_EXPR_IDENT)
    {
        const char *bname = expr->call_expr.func->ident_expr.name;
        if (bname && (!strcmp(bname, "va_start") || !strcmp(bname, "va_end") || !strcmp(bname, "va_arg")))
        {
             // TODO: Implement portable varargs lowering
             return masm_operand_none();
        }
    }

    AstNode *func = expr->call_expr.func;
    AstList *args = expr->call_expr.args;
    int arg_count = args ? args->count : 0;

    // Resolve target
    MasmOperand target;
    if (func->kind == AST_EXPR_IDENT)
    {
        LocalVar *var = find_local_var(ctx, func->ident_expr.name);
        if (var)
        {
            // Local function pointer
            target = lower_expr(masm, text, func, ctx);
            target = ensure_in_reg(text, target, func->type, ctx);
        }
        else
        {
            // Symbol
            target = masm_operand_label(func->ident_expr.name);
            if (func->symbol)
            {
                const char *link = symbol_get_linkage_name(func->symbol);
                if (link) target = masm_operand_label(link);
            }
        }
    }
    else
    {
        target = lower_expr(masm, text, func, ctx);
        target = ensure_in_reg(text, target, func->type, ctx);
    }

    // Evaluate arguments
    MasmOperand *op_args = NULL;
    if (arg_count > 0)
    {
        op_args = malloc(sizeof(MasmOperand) * arg_count);
        for (int i = 0; i < arg_count; ++i)
        {
            AstNode *arg = args->items[i];
            MasmOperand op = lower_expr(masm, text, arg, ctx);
            if (arg->type && type_is_float(arg->type))
            {
                if (op.kind != MASM_OPERAND_REGISTER)
                {
                    MasmOperand r = alloc_vreg_fp(ctx, arg->type->size ? arg->type->size : 8);
                    if (op.kind == MASM_OPERAND_MEMORY)
                    {
                         masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, r, op, masm_operand_type(arg->type->size == 4 ? MASM_TYPE_F32 : MASM_TYPE_F64)));
                    }
                    else
                    {
                         masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, r, op));
                    }
                    op = r;
                }
                op.reg.class = MASM_REG_CLASS_FLOAT;
            }
            else
            {
                op = ensure_in_reg(text, op, arg->type, ctx);
            }
            op_args[i] = op;
        }
    }

    // Result
    int res_size = 0;
    if (expr->type && expr->type->size > 0)
    {
        res_size = expr->type->size;
    }

    MasmOperand res = masm_operand_none();
    MasmOperand res_in_inst = res;

    if (res_size > 0)
    {
        res = isa_result(ctx, res_size);
        res_in_inst = res;
        if (expr->type && type_is_float(expr->type))
        {
            res_in_inst.reg.class = MASM_REG_CLASS_FLOAT;
        }
    }

    // Emit MASM_IR_CALL dest, target, args...
    int total_ops = 2 + arg_count;
    MasmOperand *ops = malloc(sizeof(MasmOperand) * total_ops);
    ops[0] = res_in_inst;
    ops[1] = target;
    for (int i = 0; i < arg_count; ++i)
    {
        ops[2 + i] = op_args[i];
    }

    bool is_syscall = false;
    if (target.kind == MASM_OPERAND_LABEL && strcmp(target.label, "syscall") == 0)
    {
        is_syscall = true;
    }

    MasmInstruction inst;
    if (is_syscall)
    {
         inst = masm_inst_create(MASM_OPCODE_IR, MASM_IR_SYSCALL, ops, total_ops);
    }
    else
    {
         inst = masm_inst_create(MASM_OPCODE_IR, MASM_IR_CALL, ops, total_ops);
    }

    masm_section_append_inst(text, inst);

    free(ops);
    if (op_args) free(op_args);

    return res;
}

static MasmOperand lower_cast(Masm *masm, MasmSection *text, AstNode *expr, LowerContext *ctx)
{
    Type *dst_type = expr->type;
    Type *src_type = expr->cast_expr.expr->type;
    int dst_size = (dst_type && dst_type->size) ? dst_type->size : ctx->ptr_size;
    if (dst_size == 0) dst_size = ctx->ptr_size;

    int src_size = (src_type && src_type->size) ? src_type->size : ctx->ptr_size;
    if (src_size == 0) src_size = ctx->ptr_size;

    MasmOperand src = lower_expr(masm, text, expr->cast_expr.expr, ctx);

    bool from_agg = src_type && (src_type->kind == TYPE_STRUCT || src_type->kind == TYPE_UNION || src_type->kind == TYPE_ARRAY);

    if (from_agg)
    {
        // Aggregate cast: load prefix (up to register size)
        MasmOperand ptr;
        if (src.kind == MASM_OPERAND_MEMORY)
        {
            ptr = alloc_vreg(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, ptr, src));
        }
        else
        {
            ptr = src;
        }

        int copy_size = (src_size < dst_size) ? src_size : dst_size;
        if (copy_size > 8) copy_size = 8; // limit to register size

        MasmOperand dst = isa_result(ctx, dst_size);
        MasmOperand mem = masm_operand_memory_simple(ptr.reg.id, 0, copy_size);

        // Treat as unsigned load (raw bits) unless we have better info
        MasmTypeKind tk = MASM_TYPE_U64;
        switch (copy_size)
        {
            case 1: tk = MASM_TYPE_U8; break;
            case 2: tk = MASM_TYPE_U16; break;
            case 4: tk = MASM_TYPE_U32; break;
            default: tk = MASM_TYPE_U64; break;
        }

        masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, dst, mem, masm_operand_type(tk)));
        return dst;
    }

    // Scalar cast
    src = ensure_in_reg(text, src, src_type, ctx);
    MasmOperand dst = isa_result(ctx, dst_size);

    bool dst_fp = dst_type && type_is_fp_class(dst_type);
    bool src_fp = src_type && type_is_fp_class(src_type);

    if (dst_fp || src_fp)
    {
        // FCONV mode: 0=i2f, 1=f2i, 2=f2f
        int mode = 0;
        if (src_fp && dst_fp) mode = 2;
        else if (src_fp) mode = 1;
        else mode = 0;

        masm_section_append_inst(text, masm_inst_3(MASM_IR_FCONV, dst, src, masm_operand_imm(mode)));
    }
    else
    {
        // Integer cast
        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst, src));
    }

    return dst;
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

            char label[32];
            snprintf(label, sizeof(label), ".Lstr_%d", ctx->str_counter++);

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
            MasmOperand src = masm_operand_label(label);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, res, src));
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
        else if (expr->lit_expr.kind == TOKEN_LIT_FLOAT)
        {
            // convert float to its bit representation as i64
            // this allows f64 values to be passed around in GPRs
            double   fval = expr->lit_expr.float_val;
            uint64_t bits;
            memcpy(&bits, &fval, sizeof(bits));
            return masm_operand_imm((int64_t)bits);
        }
        else if (expr->lit_expr.kind == TOKEN_LIT_STRING)
        {
#ifdef MASM_DEBUG
            fprintf(stderr, "[lower_expr] string literal '%s', text=%p, inst_count before=%zu\n", expr->lit_expr.string_val, (void *)text, text->inst_count);
#endif
            // Create unique label
            char label[32];
            snprintf(label, sizeof(label), ".Lstr_%d", ctx->str_counter++);

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
            MasmOperand src = masm_operand_label(label);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, res, src));
#ifdef MASM_DEBUG
            fprintf(stderr, "[lower_expr] string literal: emitted MOV, text section inst_count after=%zu\n", text->inst_count);
#endif
            return res;
        }
    }
    else if (expr->kind == AST_EXPR_CAST)
    {
        return lower_cast(masm, text, expr, ctx);
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
            // aggregates: return address with LEA
            // arrays and structs need LEA regardless of size because
            // indexing/field access expects a pointer base
            bool is_aggregate = expr->type && (expr->type->kind == TYPE_ARRAY || expr->type->kind == TYPE_STRUCT || expr->type->kind == TYPE_UNION);
            if (var->size > 8 || is_aggregate)
            {
                MasmOperand result = isa_result(ctx, 8);
                MasmOperand addr   = frame_mem(ctx, var->offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, result, addr));
                return result;
            }

            // scalar: load value
            MasmOperand var_mem = frame_mem(ctx, var->offset, var->size);

            MasmTypeKind tk = MASM_TYPE_I64;
            bool is_signed = expr->type ? type_is_signed(expr->type) : true;
            if (expr->type && type_is_fp_class(expr->type))
            {
                tk = (var->size == 4) ? MASM_TYPE_F32 : MASM_TYPE_F64;
            }
            else
            {
                switch (var->size) {
                    case 1: tk = is_signed ? MASM_TYPE_I8 : MASM_TYPE_U8; break;
                    case 2: tk = is_signed ? MASM_TYPE_I16 : MASM_TYPE_U16; break;
                    case 4: tk = is_signed ? MASM_TYPE_I32 : MASM_TYPE_U32; break;
                    default: tk = is_signed ? MASM_TYPE_I64 : MASM_TYPE_U64; break;
                }
            }

            MasmOperand result = isa_result(ctx, var->size);
            masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, result, var_mem, masm_operand_type(tk)));
            return result;
        }
        else
        {
            // Check global
            MasmSymbol *sym = masm_get_symbol(masm, expr->ident_expr.name);
            if (sym)
            {
                // For aggregates/arrays, return address in Register
                MasmOperand addr     = isa_result(ctx, 8);
                MasmOperand label_op = masm_operand_label(sym->name);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, addr, label_op));

                if (sym->size > 8)
                {
                    return addr;
                }

                // scalar: load value from address
                MasmOperand mem    = masm_operand_memory_simple(addr.reg.id, 0, sym->size > 8 ? 8 : sym->size);
                MasmOperand result = isa_result(ctx, sym->size > 8 ? 8 : sym->size);

                MasmTypeKind tk = MASM_TYPE_I64;
                bool is_signed = expr->type ? type_is_signed(expr->type) : true;
                if (expr->type && type_is_fp_class(expr->type))
                {
                    tk = (sym->size == 4) ? MASM_TYPE_F32 : MASM_TYPE_F64;
                }
                else
                {
                    switch (sym->size) {
                        case 1: tk = is_signed ? MASM_TYPE_I8 : MASM_TYPE_U8; break;
                        case 2: tk = is_signed ? MASM_TYPE_I16 : MASM_TYPE_U16; break;
                        case 4: tk = is_signed ? MASM_TYPE_I32 : MASM_TYPE_U32; break;
                        default: tk = is_signed ? MASM_TYPE_I64 : MASM_TYPE_U64; break;
                    }
                }

                masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, result, mem, masm_operand_type(tk)));
                return result;
            }
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
                    MasmOperand label_op = masm_operand_label(link_name);
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, addr, label_op));

                    if (sym_size > 8)
                    {
                        return addr;
                    }

                    MasmOperand mem = masm_operand_memory_simple(addr.reg.id, 0, sym_size ? sym_size : 1);

                    if (sym_size == 1 || sym_size == 2)
                    {
                        MasmOperand res = isa_result(ctx, 8);
                        masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, res, mem, masm_operand_type(MASM_TYPE_U8)));
                        return res;
                    }

                    MasmOperand res = isa_result(ctx, sym_size ? (uint8_t)sym_size : ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, res, mem, masm_operand_type(MASM_TYPE_U64)));
                    return res;
                }
            }
        }
        // if not found, fall through to return none
    else if (expr->kind == AST_EXPR_BINARY)
    {
        if (expr->binary_expr.op == TOKEN_AMPERSAND_AMPERSAND || expr->binary_expr.op == TOKEN_PIPE_PIPE)
        {
            return lower_short_circuit(masm, text, expr, ctx);
        }

        if (expr->binary_expr.op == TOKEN_EQUAL)
        {
            return lower_assign(masm, text, expr, ctx);
        }

        MasmOperand left = lower_expr(masm, text, expr->binary_expr.left, ctx);
        MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);
        return lower_binary_op(text, expr->binary_expr.op, left, right, expr->type, expr->binary_expr.left->type, ctx);
    }
    else if (expr->kind == AST_EXPR_CALL)
    {
        return lower_call(masm, text, expr, ctx);
    }
    else if (expr->kind == AST_EXPR_UNARY)
    {
        if (expr->unary_expr.op == TOKEN_QUESTION)
        {
            // Address-of: ?expr (produce pointer to lvalue)
            AstNode *inner_expr = expr->unary_expr.expr;

            if (inner_expr && inner_expr->kind == AST_EXPR_IDENT)
            {
                LocalVar *var = find_local_var(ctx, inner_expr->ident_expr.name);
                if (var)
                {
                    MasmOperand dst  = isa_result(ctx, ctx->ptr_size);
                    MasmOperand addr = frame_mem(ctx, var->offset, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst, addr));
                    return dst;
                }

                MasmSymbol *sym = masm_get_symbol(masm, inner_expr->ident_expr.name);
                if (sym)
                {
                    MasmOperand dst      = isa_result(ctx, ctx->ptr_size);
                    MasmOperand label_op = masm_operand_label(sym->name);
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst, label_op));
                    return dst;
                }
            }

            MasmOperand val = lower_expr(masm, text, inner_expr, ctx);

            if (val.kind == MASM_OPERAND_MEMORY)
            {
                MasmOperand dst = isa_result(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst, val));
                return dst;
            }
            else if (val.kind == MASM_OPERAND_LABEL)
            {
                MasmOperand dst = isa_result(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst, val));
                return dst;
            }

            return val;
        }
        else if (expr->unary_expr.op == TOKEN_AT)
        {
            // Dereference: @expr
            MasmOperand ptr = lower_expr(masm, text, expr->unary_expr.expr, ctx);

            if (ptr.kind != MASM_OPERAND_REGISTER)
            {
                MasmOperand r_res = isa_result(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, r_res, ptr));
                ptr = r_res;
            }

            int size = 8;
            if (expr->type) size = expr->type->size;
            if (size == 0) size = 8;

            return masm_operand_memory_simple(ptr.reg.id, 0, size);
        }

        MasmOperand operand = lower_expr(masm, text, expr->unary_expr.expr, ctx);
        return lower_unary_op(text, expr->unary_expr.op, operand, expr->type, ctx);
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

                bool elem_is_agg = type_is_aggregate(elem_type);
                if (elem_is_agg)
                {
                    MasmOperand src_ptr = isa_result(ctx, ctx->ptr_size);
                    if (val.kind == MASM_OPERAND_REGISTER)
                    {
                        if (val.reg.id != src_ptr.reg.id)
                        {
                            masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, src_ptr, val));
                        }
                    }
                    else if (val.kind == MASM_OPERAND_MEMORY)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, src_ptr, val));
                    }
                    else if (val.kind == MASM_OPERAND_LABEL || val.kind == MASM_OPERAND_IMM)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, src_ptr, val));
                    }
                    else
                    {
                        MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, tmp, val));
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, src_ptr, tmp));
                    }

                    MasmOperand dst_ptr  = isa_tmp(ctx, ctx->ptr_size);
                    MasmOperand dst_addr = frame_mem(ctx, elem_offset, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst_ptr, dst_addr));

                    emit_aggregate_copy(text, ctx, dst_ptr, src_ptr, (size_t)elem_size);
                }
                else
                {
                    if (val.kind == MASM_OPERAND_REGISTER)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst, val));
                    }
                    else if (val.kind == MASM_OPERAND_IMM)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst, val));
                    }
                    else if (val.kind == MASM_OPERAND_MEMORY)
                    {
                        // mem to mem copy
                        MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, tmp, val));
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst, tmp));
                    }
                }
            }
        }

        return frame_mem(ctx, base_offset, type->size);
    }
    else if (expr->kind == AST_EXPR_INDEX)
    {
        // arr[i]
        MasmOperand idx = lower_expr(masm, text, expr->index_expr.index, ctx);
        idx = ensure_in_reg(text, idx, expr->index_expr.index->type, ctx);

        MasmOperand arr = lower_expr(masm, text, expr->index_expr.array, ctx);
        if (arr.kind == MASM_OPERAND_MEMORY)
        {
            MasmOperand arr_reg = alloc_vreg(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, arr_reg, arr));
            arr = arr_reg;
        }
        else
        {
            arr = ensure_in_reg(text, arr, expr->index_expr.array->type, ctx);
        }

        Type *arr_type = expr->index_expr.array->type;
        if (!arr_type) return masm_operand_none();

        Type *elem_type = NULL;
        if (arr_type->kind == TYPE_ARRAY) elem_type = arr_type->array.elem_type;
        else if (arr_type->kind == TYPE_POINTER) elem_type = arr_type->pointer.base;
        else return masm_operand_none();

        int elem_size = elem_type->size;

        if (idx.kind == MASM_OPERAND_IMM)
        {
            int64_t offset = idx.imm * elem_size;
            return masm_operand_memory_simple(arr.reg.id, offset, elem_size);
        }
        else
        {
            MasmOperand scaled_idx = idx;
            if (elem_size > 1)
            {
                scaled_idx = alloc_vreg(ctx, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_3(MASM_IR_MUL, scaled_idx, idx, masm_operand_imm(elem_size)));
            }

            MasmOperand ptr = alloc_vreg(ctx, ctx->ptr_size);
            masm_section_append_inst(text, masm_inst_3(MASM_IR_ADD, ptr, arr, scaled_idx));
            
            return masm_operand_memory_simple(ptr.reg.id, 0, elem_size);
        }
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

                bool is_agg_field = field_type && (field_type->kind == TYPE_STRUCT || field_type->kind == TYPE_UNION || field_type->kind == TYPE_ARRAY);
                if (is_agg_field || field_type->size > 8)
                {
                    size_t fsize = field_type->size;
                    if (fsize == 0) continue;

                    MasmOperand src_ptr;
                    if (init_op.kind == MASM_OPERAND_MEMORY)
                    {
                        src_ptr = alloc_vreg(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, src_ptr, init_op));
                    }
                    else
                    {
                        src_ptr = init_op;
                    }
                    src_ptr = ensure_in_reg(text, src_ptr, NULL, ctx);

                    MasmOperand dst_ptr = alloc_vreg(ctx, ctx->ptr_size);
                    MasmOperand dst_addr = frame_mem(ctx, dest_disp, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst_ptr, dst_addr));
                    
                    int chunk = 8;
                    for (size_t k = 0; k < fsize; k+=chunk)
                    {
                         if (fsize - k < 8) chunk = 1;
                         MasmOperand val = alloc_vreg(ctx, chunk);
                         MasmOperand s = masm_operand_memory_simple(src_ptr.reg.id, k, chunk);
                         MasmOperand d = masm_operand_memory_simple(dst_ptr.reg.id, k, chunk);
                         masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, val, s, masm_operand_type(MASM_TYPE_I64)));
                         masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, d, val, masm_operand_imm(chunk)));
                    }
                }
                else
                {
                    MasmOperand dest = frame_mem(ctx, dest_disp, field_type->size);
                    init_op = ensure_in_reg(text, init_op, field_type, ctx);
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dest, init_op, masm_operand_imm(field_type->size)));
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
#ifdef MASM_DEBUG
        fprintf(stderr, "[lower_stmt] RET: expr=%p, fn_has_sret=%d, fn_ret_type=%p (size=%zu)\n", (void *)expr, ctx->fn_has_sret, (void *)ctx->fn_ret_type, ctx->fn_ret_type ? ctx->fn_ret_type->size : 0);
#endif
        MasmOperand ret_val = masm_operand_none();

        if (ctx->fn_has_sret && ctx->fn_ret_type && ctx->fn_ret_type->size > ctx->ptr_size)
        {
            if (expr)
            {
                // return aggregate: copy into sret buffer stored in frame
                MasmOperand src_val = lower_expr(masm, text, expr, ctx);
                MasmOperand src_ptr;

                // normalize src_val into an address
                if (src_val.kind == MASM_OPERAND_MEMORY)
                {
                    src_ptr = alloc_vreg(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, src_ptr, src_val));
                }
                else
                {
                    src_ptr = src_val;
                }
                src_ptr = ensure_in_reg(text, src_ptr, NULL, ctx);

                // load dst sret pointer from frame slot
                MasmOperand dst_ptr = alloc_vreg(ctx, ctx->ptr_size);
                MasmOperand dst_slot = frame_mem(ctx, ctx->sret_offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, dst_ptr, dst_slot, masm_operand_type(MASM_TYPE_U64)));

                // Copy
                int32_t size = (int32_t)ctx->fn_ret_type->size;
                int chunk = 8;
                for (int32_t off = 0; off < size; off += chunk)
                {
                    if (size - off < 8) chunk = 1; // Simplify copy loop
                    
                    MasmOperand tmp = alloc_vreg(ctx, chunk);
                    MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, tmp, src, masm_operand_type(MASM_TYPE_U64)));
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, tmp, masm_operand_imm(chunk)));
                }

                // return sret pointer
                ret_val = dst_ptr;
            }
        }
        else if (expr)
        {
            ret_val = lower_expr(masm, text, expr, ctx);

            if (ctx->fn_ret_type && type_is_float(ctx->fn_ret_type))
            {
                if (ret_val.kind != MASM_OPERAND_REGISTER)
                {
                    MasmOperand r = alloc_vreg_fp(ctx, ctx->fn_ret_type->size);
                    if (ret_val.kind == MASM_OPERAND_MEMORY)
                    {
                         masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, r, ret_val, masm_operand_type(ctx->fn_ret_type->size == 4 ? MASM_TYPE_F32 : MASM_TYPE_F64)));
                    }
                    else
                    {
                         masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, r, ret_val));
                    }
                    ret_val = r;
                }
                ret_val.reg.class = MASM_REG_CLASS_FLOAT;
            }
            else
            {
                ret_val = ensure_in_reg(text, ret_val, expr->type, ctx);
            }
        }

        // run all deferred statements before returning
        emit_deferred_from(masm, text, ctx, 0);

        if (ret_val.kind != MASM_OPERAND_NONE)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_IR_RET, ret_val));
        }
        else
        {
            masm_section_append_inst(text, masm_inst_0(MASM_IR_RET));
        }
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

            bool is_aggregate = var_size > 8;
            if (stmt->type && type_is_aggregate(stmt->type))
            {
                is_aggregate = true;
            }
            else if (stmt->var_stmt.init->type && type_is_aggregate(stmt->var_stmt.init->type))
            {
                is_aggregate = true;
            }

            if (is_aggregate)
            {
                // Aggregate copy: normalize source to address, get dst address, copy bytes
                MasmOperand src_ptr = isa_result(ctx, ctx->ptr_size);
                if (value.kind == MASM_OPERAND_REGISTER)
                {
                    // For aggregates, lower_expr returns the address in a register
                    if (value.reg.id != src_ptr.reg.id)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, src_ptr, value));
                    }
                }
                else if (value.kind == MASM_OPERAND_MEMORY)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, src_ptr, value));
                }
                else if (value.kind == MASM_OPERAND_LABEL)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, src_ptr, value));
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, src_ptr, value));
                }

                MasmOperand dst_ptr  = isa_tmp(ctx, ctx->ptr_size);
                MasmOperand dst_addr = frame_mem(ctx, offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst_ptr, dst_addr));

                emit_aggregate_copy(text, ctx, dst_ptr, src_ptr, var_size);
            }
            else
            {
                // store value to stack location [rbp + offset]
                MasmOperand var_mem = frame_mem(ctx, offset, var_size);

                value = ensure_in_reg(text, value, NULL, ctx);
                masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, var_mem, value, masm_operand_imm(var_size)));
            }
        }
    }
    else if (stmt->kind == AST_STMT_BRK)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_end_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(ctx->loop_end_label)));
        }
    }
    else if (stmt->kind == AST_STMT_CNT)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_start_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(ctx->loop_start_label)));
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
        char else_label[32];
        char end_label[32];
        snprintf(else_label, sizeof(else_label), ".Lif_or_%d", ctx->label_counter);
        snprintf(end_label, sizeof(end_label), ".Lif_end_%d", ctx->label_counter++);

        // evaluate condition
        MasmOperand cond = lower_expr(masm, text, stmt->cond_stmt.cond, ctx);
        cond = ensure_in_reg(text, cond, stmt->cond_stmt.cond->type, ctx);

        // jump to else/end if false (zero)
        masm_section_append_inst(text, masm_inst_3(MASM_IR_BEQ, cond, masm_operand_imm(0), masm_operand_label(else_label)));

        // lower body
        lower_stmt(masm, text, stmt->cond_stmt.body, ctx);

        // jump to end (skip else)
        masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(end_label)));

        // else label
        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(else_label)));
        masm_add_symbol(masm, masm_symbol_create(else_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

        // lower else block if exists
        if (stmt->cond_stmt.stmt_or)
        {
            lower_stmt(masm, text, stmt->cond_stmt.stmt_or, ctx);
        }

        // end label
        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(end_label)));
        masm_add_symbol(masm, masm_symbol_create(end_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));
    }
    else if (stmt->kind == AST_STMT_FOR)
    {
        char start_label[32];
        char end_label[32];
        snprintf(start_label, sizeof(start_label), ".Lloop_start_%d", ctx->loop_counter);
        snprintf(end_label, sizeof(end_label), ".Lloop_end_%d", ctx->loop_counter++);

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
        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(ctx->loop_start_label)));

        // Condition
        if (stmt->for_stmt.cond)
        {
            MasmOperand cond = lower_expr(masm, text, stmt->for_stmt.cond, ctx);
            cond = ensure_in_reg(text, cond, stmt->for_stmt.cond->type, ctx);
            masm_section_append_inst(text, masm_inst_3(MASM_IR_BEQ, cond, masm_operand_imm(0), masm_operand_label(ctx->loop_end_label)));
        }

        // Body
        lower_stmt(masm, text, stmt->for_stmt.body, ctx);

        // Jump back to start
        masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(ctx->loop_start_label)));

        // Emit end label
        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(ctx->loop_end_label)));

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
            char else_label[32];
            char end_label[32];
            snprintf(else_label, sizeof(else_label), ".Lor_stmt_else_%d", ctx->label_counter);
            snprintf(end_label, sizeof(end_label), ".Lor_stmt_end_%d", ctx->label_counter++);

            // evaluate condition
            MasmOperand cond = lower_expr(masm, text, stmt->cond_stmt.cond, ctx);
            cond = ensure_in_reg(text, cond, stmt->cond_stmt.cond->type, ctx);

            masm_section_append_inst(text, masm_inst_3(MASM_IR_BEQ, cond, masm_operand_imm(0), masm_operand_label(else_label)));

            lower_stmt(masm, text, stmt->cond_stmt.body, ctx);

            masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(end_label)));

            masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(else_label)));
            masm_add_symbol(masm, masm_symbol_create(else_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

            if (stmt->cond_stmt.stmt_or)
            {
                lower_stmt(masm, text, stmt->cond_stmt.stmt_or, ctx);
            }

            masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(end_label)));
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
        char start_label[32];
        char end_label[32];
        snprintf(start_label, sizeof(start_label), ".Lfor_start_%d", ctx->label_counter);
        snprintf(end_label, sizeof(end_label), ".Lfor_end_%d", ctx->label_counter++);

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

        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(start_label)));
        masm_add_symbol(masm, masm_symbol_create(start_label, MASM_SYMBOL_LABEL, MASM_BIND_LOCAL));

        // check condition if exists
        if (stmt->for_stmt.cond)
        {
            MasmOperand cond = lower_expr(masm, text, stmt->for_stmt.cond, ctx);
            cond = ensure_in_reg(text, cond, stmt->for_stmt.cond->type, ctx);
            masm_section_append_inst(text, masm_inst_3(MASM_IR_BEQ, cond, masm_operand_imm(0), masm_operand_label(end_label)));
        }

        // lower body
        lower_stmt(masm, text, stmt->for_stmt.body, ctx);

        // jump back to start
        masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(start_label)));

        // end label
        masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(end_label)));
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
            masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(ctx->loop_end_label)));
        }
    }
    else if (stmt->kind == AST_STMT_CNT)
    {
        int defer_mark = ctx->loop_defer_count > 0 ? ctx->loop_defer_stack[ctx->loop_defer_count - 1] : ctx->deferred_count;
        emit_deferred_from(masm, text, ctx, defer_mark);
        if (ctx->loop_start_label)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_IR_JMP, masm_operand_label(ctx->loop_start_label)));
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
            // SYSCALL dest(implicit), target(none)
            masm_section_append_inst(text, masm_inst_1(MASM_IR_SYSCALL, masm_operand_none()));
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

            MasmOperand target = masm_operand_label(label);
            masm_section_append_inst(text, masm_inst_2(MASM_IR_CALL, masm_operand_none(), target));
        }
        else if (strncmp(token, "ret", 3) == 0)
        {
            masm_section_append_inst(text, masm_inst_0(MASM_IR_RET));
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

                masm_section_append_inst(text, masm_inst_2(MASM_IR_CMP, dst_op, src_op));
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

                masm_section_append_inst(text, masm_inst_3(MASM_IR_XOR, dst_op, dst_op, src_op));
            }
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

                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst_op, src_op));
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
                masm_section_append_inst(text, masm_inst_3(MASM_IR_AND, dst_op, dst_op, src_op));
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
                masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst_op, src_op));
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

                masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, dst_op, src_op));
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
                masm_section_append_inst(text, masm_inst_2(MASM_IR_ZEXT, dst_op, src_op));
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
                masm_section_append_inst(text, masm_inst_2(MASM_IR_SEXT, dst_op, src_op));
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

    masm_section_append_inst(text, masm_inst_1(MASM_IR_LABEL, masm_operand_label(func_name)));
#ifdef MASM_DEBUG
    fprintf(stderr, "[lower_function] starting %s, text=%p, inst_count=%zu\n", func_name, (void *)text, text->inst_count);
#endif

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

    size_t frame_idx = 0;
    if (!is_entry)
    {
        // reserve space for locals (placeholder, will be patched)
        frame_idx = text->inst_count;
        masm_section_append_inst(text, masm_inst_1(MASM_IR_STACK_FRAME, masm_operand_imm(0)));
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
        masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, src, masm_operand_imm(ctx->ptr_size)));
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
                    MasmOperand src = masm_operand_register_fp(xmm_id, store_size);
                    // store directly to stack
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, src, masm_operand_imm(store_size)));
                }
                else
                {
                    int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (stack_i++ * ctx->ptr_size));
                    MasmOperand src_mem            = frame_mem(ctx, stack_param_offset, store_size);
                    MasmOperand tmp                = alloc_vreg_fp(ctx, store_size);
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, tmp, src_mem, masm_operand_type(MASM_TYPE_F64)));
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, tmp, masm_operand_imm(store_size)));
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
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, src, masm_operand_imm(store_size)));
                }
                else
                {
                    int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (stack_i++ * ctx->ptr_size));
                    MasmOperand src_mem            = frame_mem(ctx, stack_param_offset, ctx->ptr_size);
                    MasmOperand tmp                = alloc_vreg(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, tmp, src_mem, masm_operand_type(MASM_TYPE_U64)));
                    
                    // We might need to truncate if store_size < ptr_size, IR STORE handles size
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, tmp, masm_operand_imm(store_size)));
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
                    masm_section_append_inst(text, masm_inst_2(MASM_IR_MOV, src_ptr, src));
                }
                else
                {
                    int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (stack_i++ * ctx->ptr_size));
                    MasmOperand src_mem            = frame_mem(ctx, stack_param_offset, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, src_ptr, src_mem, masm_operand_type(MASM_TYPE_U64)));
                }

                MasmOperand dst_ptr  = alloc_vreg(ctx, ctx->ptr_size);
                MasmOperand dst_addr = frame_mem(ctx, offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_IR_LEA, dst_ptr, dst_addr));

                int32_t chunk = 8;
                for (int32_t off = 0; off < (int32_t)param_size; off += chunk)
                {
                    if ((int32_t)param_size - off < 8) chunk = 1;
                    MasmOperand tmp = alloc_vreg(ctx, chunk);
                    MasmOperand src = masm_operand_memory_simple(src_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    MasmOperand dst = masm_operand_memory_simple(dst_ptr.reg.id, (int64_t)off, (size_t)chunk);
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_LOAD, tmp, src, masm_operand_type(MASM_TYPE_U64)));
                    masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, tmp, masm_operand_imm(chunk)));
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
                masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, src, masm_operand_imm(ctx->ptr_size)));
            }

            // save low 8 bytes of XMM0..XMM7 into offsets 48 + 16*i
            for (int i = 0; i < 8; i++)
            {
                int32_t     off   = ctx->va_reg_save_off + (48 + i * 16);
                MasmOperand dst   = frame_mem(ctx, off, 8);
                uint32_t    xmmid = ctx->abi->float_arg_regs[i];
                MasmOperand src   = masm_operand_register(xmmid, 8);
                masm_section_append_inst(text, masm_inst_3(MASM_IR_STORE, dst, src, masm_operand_imm(8)));
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
            text->instructions[frame_idx].operands[0].imm = frame_size;
        }
    }

    // emit remaining deferred statements for fallthrough paths
    emit_deferred_from(masm, text, ctx, 0);

    if (!is_entry)
    {
        // implicit return
        masm_section_append_inst(text, masm_inst_0(MASM_IR_RET));
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

static void emit_global_data(MasmSection *section, AstNode *expr, size_t size)
{
    if (!expr)
    {
        masm_section_append_zero(section, size);
        return;
    }

    if (expr->kind == AST_EXPR_LIT)
    {
        if (expr->lit_expr.kind == TOKEN_LIT_INT)
        {
            uint64_t val        = (uint64_t)expr->lit_expr.int_val;
            size_t   write_size = size > 8 ? 8 : size;
            masm_section_append_data(section, &val, write_size);
            if (size > 8)
            {
                masm_section_append_zero(section, size - 8);
            }
        }
        else if (expr->lit_expr.kind == TOKEN_LIT_CHAR)
        {
            uint64_t val        = (uint64_t)(uint8_t)expr->lit_expr.char_val;
            size_t   write_size = size > 8 ? 8 : size;
            masm_section_append_data(section, &val, write_size);
            if (size > 8)
            {
                masm_section_append_zero(section, size - 8);
            }
        }
        else if (expr->lit_expr.kind == TOKEN_LIT_FLOAT)
        {
            // write float as raw bits
            double   fval = expr->lit_expr.float_val;
            uint64_t bits;
            memcpy(&bits, &fval, sizeof(bits));
            size_t write_size = size > 8 ? 8 : size;
            masm_section_append_data(section, &bits, write_size);
            if (size > 8)
            {
                masm_section_append_zero(section, size - 8);
            }
        }
        else
        {
            masm_section_append_zero(section, size);
        }
    }
    else if (expr->kind == AST_EXPR_STRUCT)
    {
        Type *type = expr->type;
        if (!type)
        {
            masm_section_append_zero(section, size);
            return;
        }

        AstList *fields         = expr->struct_expr.fields;
        size_t   current_offset = 0;

        if (type->kind == TYPE_STRUCT)
        {
            for (int i = 0; i < type->structure.field_count; i++)
            {
                // Handle padding
                size_t field_offset = type->structure.fields[i].offset;
                if (field_offset > current_offset)
                {
                    masm_section_append_zero(section, field_offset - current_offset);
                    current_offset = field_offset;
                }

                // Find initializer
                AstNode *init_expr = NULL;
                if (fields)
                {
                    for (int j = 0; j < fields->count; j++)
                    {
                        AstNode *fnode = fields->items[j];
                        if (strcmp(fnode->field_expr.field, type->structure.fields[i].name) == 0)
                        {
                            init_expr = fnode->field_expr.object;
                            break;
                        }
                    }
                }

                size_t field_size = type->structure.fields[i].type->size;
                emit_global_data(section, init_expr, field_size);
                current_offset += field_size;
            }
        }
        else if (type->kind == TYPE_UNION)
        {
            AstNode *init_expr  = NULL;
            size_t   field_size = 0;
            if (fields && fields->count > 0)
            {
                AstNode *fnode = fields->items[0];
                init_expr      = fnode->field_expr.object;

                for (int i = 0; i < type->union_type.field_count; i++)
                {
                    if (strcmp(type->union_type.fields[i].name, fnode->field_expr.field) == 0)
                    {
                        field_size = type->union_type.fields[i].type->size;
                        break;
                    }
                }
            }
            emit_global_data(section, init_expr, field_size);
            if (field_size < size)
            {
                masm_section_append_zero(section, size - field_size);
            }
            return;
        }

        if (current_offset < size)
        {
            masm_section_append_zero(section, size - current_offset);
        }
    }
    else if (expr->kind == AST_EXPR_ARRAY)
    {
        Type    *type           = expr->type;
        Type    *elem_type      = type->array.elem_type;
        size_t   elem_size      = elem_type->size;
        AstList *elems          = expr->array_expr.elems;
        size_t   count          = elems ? elems->count : 0;
        size_t   expected_count = type->array.count;

        for (size_t i = 0; i < expected_count; i++)
        {
            if (i < count)
            {
                emit_global_data(section, elems->items[i], elem_size);
            }
            else
            {
                masm_section_append_zero(section, elem_size);
            }
        }
    }
    else if (expr->kind == AST_EXPR_UNARY)
    {
        int64_t val = 0;
        if (expr->unary_expr.op == TOKEN_MINUS && expr->unary_expr.expr->kind == AST_EXPR_LIT && expr->unary_expr.expr->lit_expr.kind == TOKEN_LIT_INT)
        {
            val = -((int64_t)expr->unary_expr.expr->lit_expr.int_val);
        }
        size_t write_size = size > 8 ? 8 : size;
        masm_section_append_data(section, &val, write_size);
        if (size > 8)
        {
            masm_section_append_zero(section, size - 8);
        }
    }
    else if (expr->kind == AST_EXPR_BINARY)
    {
        int64_t val = 0;
        if (expr->binary_expr.op == TOKEN_MINUS)
        {
            if (expr->binary_expr.left->kind == AST_EXPR_LIT && expr->binary_expr.right->kind == AST_EXPR_LIT)
            {
                val = (int64_t)expr->binary_expr.left->lit_expr.int_val - (int64_t)expr->binary_expr.right->lit_expr.int_val;
            }
        }
        size_t write_size = size > 8 ? 8 : size;
        masm_section_append_data(section, &val, write_size);
        if (size > 8)
        {
            masm_section_append_zero(section, size - 8);
        }
    }
    else
    {
        masm_section_append_zero(section, size);
    }
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
    bool is_bss = true;

    if (stmt->var_stmt.init)
    {
        AstKind k = stmt->var_stmt.init->kind;
        if (k == AST_EXPR_LIT || k == AST_EXPR_STRUCT || k == AST_EXPR_ARRAY || k == AST_EXPR_UNARY || k == AST_EXPR_BINARY)
        {
            is_bss = false;
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
        emit_global_data(section, stmt->var_stmt.init, size);
    }
}
