#include "compiler/masm/lower.h"
#include "compiler/masm/abi/spec.h"
#include "compiler/masm/instruction.h"
#include "compiler/masm/isa/spec.h"
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
    MasmTarget target;
    const MasmISASpec *isa;
    const MasmABISpec *abi;
    uint8_t    ptr_size;
    uint8_t    stack_align;
    uint8_t    int_arg_count;
    uint32_t   fp_reg;
    uint32_t   sp_reg;

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
} LowerContext;

// Select ISA spec via shared selector
static const MasmISASpec *lower_select_isa(MasmTarget target)
{
    return masm_isa_spec_select(target);
}

static inline MasmOperand isa_result(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_result(size); }
static inline MasmOperand isa_tmp(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_tmp0(size); }
static inline __attribute__((unused)) MasmOperand isa_tmp2(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_tmp1(size); }
static inline __attribute__((unused)) MasmOperand isa_div_hi(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_div_hi(size); }
static inline __attribute__((unused)) MasmOperand isa_div_lo(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_div_lo(size); }
static inline __attribute__((unused)) MasmOperand isa_arg0(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_arg(0, size); }
static inline __attribute__((unused)) MasmOperand isa_arg1(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_arg(1, size); }
static inline __attribute__((unused)) MasmOperand isa_sp(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_sp(size); }
static inline __attribute__((unused)) MasmOperand isa_fp(LowerContext *ctx, uint8_t size) { return ctx->isa->reg_fp(size); }
static inline uint32_t isa_result_id(LowerContext *ctx) { return isa_result(ctx, ctx->ptr_size).reg.id; }
static inline __attribute__((unused)) uint32_t isa_tmp_id(LowerContext *ctx) { return isa_tmp(ctx, ctx->ptr_size).reg.id; }

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
    ctx->target       = target;
    ctx->ptr_size     = ctx->abi->pointer_size;
    ctx->stack_align  = ctx->abi->stack_align;
    ctx->int_arg_count = ctx->abi->int_arg_count;
    ctx->fp_reg       = ctx->isa->reg_fp(ctx->ptr_size).reg.id;
    ctx->sp_reg       = ctx->isa->reg_sp(ctx->ptr_size).reg.id;
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

    ctx->loop_defer_stack     = malloc(sizeof(int) * 8);
    ctx->loop_defer_count     = 0;
    ctx->loop_defer_capacity  = 8;
    ctx->loop_start_label = NULL;
    ctx->loop_end_label   = NULL;
    regalloc_init(&ctx->regalloc, ctx->isa, ctx->fp_reg, ctx->sp_reg, ctx->ptr_size);
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

    if (expr->kind == AST_EXPR_LIT)
    {
        if (expr->lit_expr.kind == TOKEN_LIT_INT)
        {
            return masm_operand_imm((int64_t)expr->lit_expr.int_val);
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
                        if (store_size > 8) store_size = 8;
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
                    int size = store_size;
                    MasmOperand dst = masm_operand_memory_simple(ptr_reg.reg.id, 0, size);
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

                // 2. Load effective address into result
                MasmOperand addr = isa_result(ctx, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, addr, left_mem));

                // 3. Push address
                masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, addr));

                // 4. Evaluate RHS
                MasmOperand val = lower_expr(masm, text, expr->binary_expr.right, ctx);

                // 5. Move val to tmp (to free result) using correct width
                int store_size = expr->binary_expr.left->type ? expr->binary_expr.left->type->size : 8;
                if (store_size == 0) store_size = 8;
                if (store_size > 8) store_size = 8;

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
                int         size = expr->binary_expr.left->type->size;
                if (size == 0) size = store_size;
                MasmOperand dst  = masm_operand_memory_simple(addr.reg.id, 0, size);
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

                    // Store right_val into var_mem
                    if (right_val.kind == MASM_OPERAND_REGISTER)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, right_val));
                    }
                    else if (right_val.kind == MASM_OPERAND_IMM)
                    {
                        MasmOperand temp = isa_result(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, right_val));
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, temp));

                        // return value in result
                        return temp;
                    }

                    // if right_val was register, it's already in a register (likely RAX), so return it
                    return right_val;
                }
            }
            return masm_operand_none();
        }

        MasmOperand left = lower_expr(masm, text, expr->binary_expr.left, ctx);

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

        // free the save register if we used one
        if (left_save_reg != UINT32_MAX)
        {
            regalloc_free(&ctx->regalloc, left_save_reg);
        }

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
        // evaluate arguments
        AstList *args             = expr->call_expr.args;
        int      stack_args_count = 0;
        int      padding          = 0;

        if (args)
        {
            // Calculate stack space needed
            if (args->count > 6)
            {
                stack_args_count = args->count - 6;
                // Ensure 16-byte alignment before call
                // RSP is 16-byte aligned here (inside function body)
                // We need RSP to be 16-byte aligned BEFORE the call instruction
                // The call instruction pushes 8 bytes, so on entry to callee RSP is 8-byte aligned (correct)
                // So we just need to ensure we subtract a multiple of 16 from RSP?
                // No, ABI says (RSP + 8) is multiple of 16 on entry.
                // Which means RSP is multiple of 16 before CALL.

                // If we push odd number of args (8 bytes each), RSP is misaligned.
                // So if stack_args_count is odd, we need 8 bytes padding.
                if (stack_args_count % 2 != 0)
                {
                    padding = 8;
                }
            }

            int total_stack_space = (stack_args_count * 8) + padding;

            if (total_stack_space > 0)
            {
                MasmOperand rsp = sp_reg_op(ctx);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_SUB, rsp, masm_operand_imm(total_stack_space)));
            }

            // push stack arguments (reverse order to preserve evaluation order)
            for (int i = args->count - 1; i >= 6; i--)
            {
                MasmOperand arg_op = lower_expr(masm, text, args->items[i], ctx);

                // Calculate offset from current RSP
                // i goes from count-1 down to 6.
                // The last arg (count-1) is at the highest address (bottom of stack frame)
                // The first stack arg (6) is at [rsp]
                // So arg[6] is at [rsp + 0]
                // arg[7] is at [rsp + 8]
                // arg[i] is at [rsp + (i-6)*8]

                int         offset = (i - ctx->int_arg_count) * ctx->ptr_size;
                MasmOperand dst    = masm_operand_memory_simple(ctx->sp_reg, offset, ctx->ptr_size);

                if (arg_op.kind == MASM_OPERAND_MEMORY)
                {
                    // memory-to-memory move requires intermediate register
                    MasmOperand tmp = isa_tmp(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, arg_op));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, arg_op));
                }
            }

            // register arguments (forward order)
            for (int i = 0; i < args->count && i < ctx->int_arg_count; i++)
            {
                // evaluate arg
                MasmOperand arg_op = lower_expr(masm, text, args->items[i], ctx);

                // move to register
                uint32_t    reg = (i < ctx->abi->int_arg_count) ? ctx->abi->int_arg_regs[i] : UINT32_MAX;
                MasmOperand dst = masm_operand_register(reg, ctx->ptr_size);

                if (arg_op.kind == MASM_OPERAND_REGISTER && arg_op.reg.id == reg)
                {
                    // already in correct register
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, arg_op));
                }
            }
        }

        // determine if callee is variadic (last param type sentinel NULL)
        bool is_variadic = false;
        if (expr->call_expr.func && expr->call_expr.func->type && expr->call_expr.func->type->kind == TYPE_FUNCTION)
        {
            Type *ft = expr->call_expr.func->type;
            if (ft->function.param_count > 0 && ft->function.param_types[ft->function.param_count - 1] == NULL)
            {
                is_variadic = true;
            }
        }

        // for System V variadic calls, AL holds the number of XMM registers used (ISA role result low byte)
        if (is_variadic)
        {
            int sse_args = 0;
            if (args)
            {
                for (int i = 0; i < args->count && i < 8; i++)
                {
                    AstNode *arg = args->items[i];
                    if (arg && arg->type && type_is_float(arg->type))
                    {
                        sse_args++;
                    }
                }
            }

            MasmOperand al = isa_result(ctx, 1);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, al, masm_operand_imm(sse_args & 0xFF)));
        }

        // emit call
        AstNode *func = expr->call_expr.func;
        if (func->kind == AST_EXPR_IDENT)
        {
            const char *call_name = func->ident_expr.name;
            Symbol     *call_sym  = func->symbol;

            if (call_sym)
            {
                const char *link_name = symbol_get_linkage_name(call_sym);
                if (link_name)
                {
                    call_name = link_name;
                }
            }

            masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, masm_operand_label(strdup(call_name))));
        }
        else
        {
            // indirect call: evaluate function pointer expression
            MasmOperand func_ptr = lower_expr(masm, text, func, ctx);

                // ensure function pointer is in a register
                if (func_ptr.kind != MASM_OPERAND_REGISTER)
                {
                    MasmOperand r_res = isa_result(ctx, ctx->ptr_size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, r_res, func_ptr));
                    func_ptr = r_res;
                }

            masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, func_ptr));
        }

        // clean up stack arguments
        int total_cleanup = (stack_args_count * 8) + padding;
        if (total_cleanup > 0)
        {
            MasmOperand rsp = sp_reg_op(ctx);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, rsp, masm_operand_imm(total_cleanup)));
        }

        return isa_result(ctx, ctx->ptr_size);
    }
    else if (expr->kind == AST_EXPR_UNARY)
    {
        if (expr->unary_expr.op == TOKEN_QUESTION)
        {
            // Address-of: ?expr (produce pointer to lvalue)
            MasmOperand val = lower_expr(masm, text, expr->unary_expr.expr, ctx);

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
                // already a pointer
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

        if (struct_type->kind == TYPE_STRUCT)
        {
            // Find field offset
            int32_t    offset = 0;
            TypeField *field  = NULL;
            for (int i = 0; i < struct_type->structure.field_count; i++)
            {
                if (strcmp(struct_type->structure.fields[i].name, expr->field_expr.field) == 0)
                {
                    field  = &struct_type->structure.fields[i];
                    offset = (int32_t)field->offset;
                    break;
                }
            }

            if (field)
            {
                if (is_pointer)
                {
                    // obj is a register containing the address
                    if (obj.kind != MASM_OPERAND_REGISTER)
                    {
                        MasmOperand r_res = isa_result(ctx, ctx->ptr_size);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, r_res, obj));
                        obj = r_res;
                    }

                    return masm_operand_memory_simple(obj.reg.id, offset, field->type->size);
                }
                else
                {
                    // obj is a memory operand (struct on stack)
                    if (obj.kind == MASM_OPERAND_MEMORY)
                    {
                        // Adjust displacement
                        return masm_operand_memory_simple(obj.mem.base.id, obj.mem.disp + offset, field->type->size);
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
                mem = masm_operand_memory(base, index, elem_size, 0, elem_type->size);
            }
            else if (arr.kind == MASM_OPERAND_MEMORY)
            {
                // [base + disp + index * scale]
                // arr.mem.base is usually RBP
                MasmRegister base  = arr.mem.base;
                MasmRegister index = {idx_reg.reg.id, 8};
                mem = masm_operand_memory(base, index, elem_size, arr.mem.disp, elem_type->size);
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
        if (!type || type->kind != TYPE_STRUCT)
        {
            return masm_operand_none();
        }

        // allocate stack space
        ctx->stack_offset -= type->size;
        // align to pointer size
        int align = ctx->ptr_size ? ctx->ptr_size : 1;
        if (align > 1 && (abs(ctx->stack_offset) % align) != 0)
        {
            ctx->stack_offset -= (align - (abs(ctx->stack_offset) % align));
        }

        int32_t base_offset = ctx->stack_offset;

        // initialize fields
        AstList *fields = expr->struct_expr.fields;
        if (fields)
        {
            for (int i = 0; i < fields->count; i++)
            {
                AstNode *field_node = fields->items[i];
                // field_node is AST_EXPR_FIELD
                char    *field_name = field_node->field_expr.field;
                AstNode *init_expr  = field_node->field_expr.object;

                // find field offset
                size_t offset     = 0;
                Type  *field_type = NULL;
                for (int j = 0; j < type->structure.field_count; j++)
                {
                    if (strcmp(type->structure.fields[j].name, field_name) == 0)
                    {
                        offset     = type->structure.fields[j].offset;
                        field_type = type->structure.fields[j].type;
                        break;
                    }
                }

                // evaluate init expr
                MasmOperand init_op = lower_expr(masm, text, init_expr, ctx);

                // store to stack
                // [rbp + base_offset + offset]
                int32_t     dest_disp = base_offset + (int32_t)offset;
                MasmOperand dest      = frame_mem(ctx, dest_disp, field_type->size > ctx->ptr_size ? ctx->ptr_size : field_type->size);

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

        return frame_mem(ctx, base_offset, ctx->ptr_size);
    }
    return masm_operand_none();
}

static void lower_stmt(Masm *masm, MasmSection *text, AstNode *stmt, LowerContext *ctx)
{
    if (stmt->kind == AST_STMT_RET)
    {
        AstNode *expr = stmt->ret_stmt.expr;
        if (expr)
        {
            MasmOperand op = lower_expr(masm, text, expr, ctx);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, isa_result(ctx, ctx->ptr_size), op));
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
    else if (stmt->kind == AST_STMT_VAR)
    {
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
                if (reg_size < 1) reg_size = 8;
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
                    if (reg_size == 0) reg_size = 8;
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
            char  inner[64];
            size_t copy_len = len - 2 < sizeof(inner) - 1 ? len - 2 : sizeof(inner) - 1;
            memcpy(inner, str + 1, copy_len);
            inner[copy_len] = '\0';

            // split on '+' if present
            char *plus = strchr(inner, '+');
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

    char *func_name_copy = strdup(func_name);
    MasmSymbol *sym = masm_symbol_create(func_name_copy, MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
    masm_add_symbol(masm, sym);

    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);

    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(strdup(func_name))));

    // create context for this function
    LowerContext *ctx = create_context(masm->target);

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

    // handle parameters (skip for _start)
    if (!is_entry && func_node->fun_stmt.params)
    {
        AstList *params = func_node->fun_stmt.params;
        for (int i = 0; i < params->count; i++)
        {
            AstNode *param = params->items[i];
            // allocate stack space
            ctx->stack_offset -= (int32_t)ctx->ptr_size;
            int32_t offset = ctx->stack_offset;

            // add to symbol table
            add_local_var(ctx, param->param_stmt.name, offset, 8);

            // move register to stack
            if (i < ctx->int_arg_count)
            {
                uint32_t    reg = ctx->abi->int_arg_regs[i];
                MasmOperand src = masm_operand_register(reg, ctx->ptr_size);
                MasmOperand dst = frame_mem(ctx, offset, ctx->ptr_size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
            }
            else
            {
                int32_t     stack_param_offset = (int32_t)(2 * ctx->ptr_size + (i - ctx->int_arg_count) * ctx->ptr_size);
                MasmOperand src                = frame_mem(ctx, stack_param_offset, ctx->ptr_size);
                MasmOperand dst                = frame_mem(ctx, offset, ctx->ptr_size);
                MasmOperand tmp                = isa_tmp(ctx, ctx->ptr_size);

                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, tmp));
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
Masm *masm_lower_module(AstNode *ast, SymbolTable *symbols)
{
    MasmTarget target = masm_target_native();
    Masm      *masm   = masm_create(target);

    if (ast->kind == AST_PROGRAM)
    {
        AstList *stmts = ast->program.stmts;
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *decl = stmts->items[i];
            if (decl->kind == AST_STMT_FUN)
            {
                lower_function(masm, decl, symbols);
            }
            else if (decl->kind == AST_STMT_VAR)
            {
                lower_global_var(masm, decl);
            }
        }

        // TODO: lower instantiated generic functions not present in the original AST
        // disabled for now to avoid duplicate lowering of functions that are already present
        // in the program AST.
    }

    return masm;
}

static void lower_global_var(Masm *masm, AstNode *stmt)
{
    const char *name = stmt->var_stmt.name;

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
