#include "compiler/masm/lower.h"
#include "compiler/masm/abi/sysv64.h"
#include "compiler/masm/instruction.h"
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

static LowerContext *create_context()
{
    LowerContext *ctx = malloc(sizeof(LowerContext));
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
    regalloc_init(&ctx->regalloc);
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
        free(ctx);
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
    ctx->vars[ctx->var_count].name   = name;
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

            // Append data
            masm_section_append_data(rodata, expr->lit_expr.string_val, len + 1);

            // Return address of string
            MasmOperand res = masm_operand_register(MASM_X86_RAX, 8);
            MasmOperand src = masm_operand_label(strdup(label));

            // MOV RAX, label (absolute address)
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, res, src));
            return res;
        }
    }
    else if (expr->kind == AST_EXPR_IDENT)
    {
        // variable access - load from stack
        LocalVar *var = find_local_var(ctx, expr->ident_expr.name);
        if (var)
        {
            // load variable from [rbp + offset] into RAX
            MasmOperand var_mem = masm_operand_memory_simple(MASM_X86_RBP, var->offset, var->size);
            MasmOperand result  = masm_operand_register(MASM_X86_RAX, var->size);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, var_mem));
            return result;
        }
        else
        {
            // Check global
            MasmSymbol *sym = masm_get_symbol(masm, expr->ident_expr.name);
            if (sym)
            {
                // Load address
                MasmOperand addr     = masm_operand_register(MASM_X86_RAX, 8);
                MasmOperand label_op = masm_operand_label(strdup(sym->name));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, addr, label_op));

                // Load value
                // [RAX]
                MasmOperand mem    = masm_operand_memory_simple(MASM_X86_RAX, 0, sym->size > 8 ? 8 : sym->size);
                MasmOperand result = masm_operand_register(MASM_X86_RAX, sym->size > 8 ? 8 : sym->size);
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
            MasmOperand result = masm_operand_register(MASM_X86_RAX, 8);

            if (left.kind != MASM_OPERAND_REGISTER || left.reg.id != MASM_X86_RAX)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }

            // check left
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, result, result));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(strdup(false_label))));

            // evaluate right
            MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);
            if (right.kind != MASM_OPERAND_REGISTER || right.reg.id != MASM_X86_RAX)
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
            MasmOperand result = masm_operand_register(MASM_X86_RAX, 8);

            if (left.kind != MASM_OPERAND_REGISTER || left.reg.id != MASM_X86_RAX)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }

            // check left
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, result, result));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JNE, masm_operand_label(strdup(true_label))));

            // evaluate right
            MasmOperand right = lower_expr(masm, text, expr->binary_expr.right, ctx);
            if (right.kind != MASM_OPERAND_REGISTER || right.reg.id != MASM_X86_RAX)
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
                    // 1. Evaluate ptr -> RAX
                    MasmOperand ptr = lower_expr(masm, text, expr->binary_expr.left->unary_expr.expr, ctx);

                    if (ptr.kind != MASM_OPERAND_REGISTER)
                    {
                        MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, ptr));
                        ptr = rax;
                    }

                    // Push ptr
                    masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, ptr));

                    // 2. Evaluate RHS -> RAX
                    MasmOperand val = lower_expr(masm, text, expr->binary_expr.right, ctx);

                    // Move val to RCX
                    MasmOperand val_reg;
                    if (val.kind == MASM_OPERAND_REGISTER)
                    {
                        if (val.reg.id == MASM_X86_RAX)
                        {
                            val_reg = masm_operand_register(MASM_X86_RCX, 8);
                            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                        }
                        else
                        {
                            val_reg = val;
                        }
                    }
                    else
                    {
                        val_reg = masm_operand_register(MASM_X86_RCX, 8);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                    }

                    // 3. Pop ptr -> RAX
                    MasmOperand ptr_reg = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, ptr_reg));

                    // 4. Store [ptr] = val
                    int size = 4;
                    if (expr->binary_expr.left->type)
                    {
                        size = expr->binary_expr.left->type->size;
                    }

                    MasmOperand dst = masm_operand_memory_simple(ptr_reg.reg.id, 0, size);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val_reg));

                    // Return val (in RCX) moved to RAX
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, ptr_reg, val_reg));
                    return ptr_reg;
                }
            }
            else if (expr->binary_expr.left->kind == AST_EXPR_INDEX || expr->binary_expr.left->kind == AST_EXPR_FIELD)
            {
                // Generic lvalue assignment (index, field)
                // 1. Evaluate LHS to memory operand
                MasmOperand left_mem = lower_expr(masm, text, expr->binary_expr.left, ctx);

                // 2. Load effective address into RAX
                MasmOperand addr = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, addr, left_mem));

                // 3. Push address
                masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, addr));

                // 4. Evaluate RHS
                MasmOperand val = lower_expr(masm, text, expr->binary_expr.right, ctx);

                // 5. Move val to RCX (to free RAX)
                MasmOperand val_reg = masm_operand_register(MASM_X86_RCX, 8);
                if (val.kind == MASM_OPERAND_REGISTER)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                }
                else if (val.kind == MASM_OPERAND_IMM)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                }
                else
                {
                    // memory to register
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, val_reg, val));
                }

                // 6. Pop address to RAX
                masm_section_append_inst(text, masm_inst_1(MASM_OP_POP, addr));

                // 7. Store [RAX] = RCX
                int         size = expr->binary_expr.left->type->size;
                MasmOperand dst  = masm_operand_memory_simple(MASM_X86_RAX, 0, size);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, val_reg));

                // 8. Return result in RAX
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, addr, val_reg));
                return addr;
            }

            MasmOperand right_val = lower_expr(masm, text, expr->binary_expr.right, ctx);

            if (expr->binary_expr.left->kind == AST_EXPR_IDENT)
            {
                LocalVar *var = find_local_var(ctx, expr->binary_expr.left->ident_expr.name);
                if (var)
                {
                    MasmOperand var_mem = masm_operand_memory_simple(MASM_X86_RBP, var->offset, var->size);

                    // Store right_val into var_mem
                    if (right_val.kind == MASM_OPERAND_REGISTER)
                    {
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, right_val));
                    }
                    else if (right_val.kind == MASM_OPERAND_IMM)
                    {
                        MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, right_val));
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, temp));

                        // return value in RAX
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
        bool       pushed        = false;
        MasmX86Reg left_save_reg = MASM_X86_REG_COUNT;

        if (left.kind == MASM_OPERAND_REGISTER && left.reg.id == MASM_X86_RAX)
        {
            // try to allocate a register to save left value
            left_save_reg = regalloc_alloc(&ctx->regalloc);
            if (left_save_reg != MASM_X86_REG_COUNT)
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
        if (left_save_reg != MASM_X86_REG_COUNT)
        {
            regalloc_free(&ctx->regalloc, left_save_reg);
        }

        MasmOperand result = masm_operand_register(MASM_X86_RAX, 8);
        if (left.kind == MASM_OPERAND_REGISTER)
        {
            result.reg.size = left.reg.size;
        }
        else if (left.kind == MASM_OPERAND_MEMORY)
        {
            result.reg.size = left.mem.size;
        }

        MasmOperand right_op = right;
        MasmOperand rcx      = masm_operand_register(MASM_X86_RCX, result.reg.size);

        if (pushed)
        {
            // right is in RAX (if register) or immediate.
            // Move right to RCX to free up RAX for left.
            if (right.kind == MASM_OPERAND_REGISTER && right.reg.id == MASM_X86_RAX)
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
        else if (left_save_reg != MASM_X86_REG_COUNT)
        {
            // left is in an allocated register, right may be in RAX
            // We want result in RAX for compatibility with existing code

            // If right is in RAX, move it to RCX
            if (right.kind == MASM_OPERAND_REGISTER && right.reg.id == MASM_X86_RAX)
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
            if (right.kind == MASM_OPERAND_REGISTER && right.reg.id == MASM_X86_RAX)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rcx, right));
                right_op = rcx;
            }

            // Load left into RAX
            if (left.kind == MASM_OPERAND_IMM)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }
            else if (left.kind == MASM_OPERAND_REGISTER && left.reg.id != MASM_X86_RAX)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
            }
            else if (left.kind == MASM_OPERAND_MEMORY)
            {
                if (left.mem.size == 4)
                {
                    MasmOperand eax = masm_operand_register(MASM_X86_RAX, 4);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, eax, left));
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, left));
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
            MasmOperand al = masm_operand_register(MASM_X86_RAX, 1);
            masm_section_append_inst(text, masm_inst_1(setcc_opcode, al));

            // zero-extend AL to RAX using AND RAX, 1
            // This assumes boolean result is 0 or 1.
            // High bytes of RAX are preserved from 'left' operand, so we must clear them.
            // AND RAX, 1 keeps only the LSB (which is the result of SETcc).
            masm_section_append_inst(text, masm_inst_2(MASM_OP_AND, result, masm_operand_imm(1)));
        }
        else if (opcode == MASM_OP_IDIV)
        {
            masm_section_append_inst(text, masm_inst_0(MASM_OP_CQO));
            masm_section_append_inst(text, masm_inst_1(opcode, right_op));
        }
        else
        {
            masm_section_append_inst(text, masm_inst_2(opcode, result, right_op));
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
                MasmOperand rsp = masm_operand_register(MASM_X86_RSP, 8);
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

                int         offset = (i - 6) * 8;
                MasmOperand dst    = masm_operand_memory_simple(MASM_X86_RSP, offset, 8);

                if (arg_op.kind == MASM_OPERAND_MEMORY)
                {
                    // memory-to-memory move requires intermediate register
                    MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, arg_op));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, rax));
                }
                else
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, arg_op));
                }
            }

            // register arguments (forward order)
            for (int i = 0; i < args->count && i < 6; i++)
            {
                // evaluate arg
                MasmOperand arg_op = lower_expr(masm, text, args->items[i], ctx);

                // move to register
                MasmX86Reg  reg = masm_sysv64_arg_reg(i);
                MasmOperand dst = masm_operand_register(reg, 8);

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

        // for System V variadic calls, AL holds the number of XMM registers used
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

            MasmOperand al = masm_operand_register(MASM_X86_RAX, 1);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, al, masm_operand_imm(sse_args & 0xFF)));
        }

        // emit call
        AstNode *func = expr->call_expr.func;
        if (func->kind == AST_EXPR_IDENT)
        {
            masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, masm_operand_label(strdup(func->ident_expr.name))));
        }
        else
        {
            // indirect call: evaluate function pointer expression
            MasmOperand func_ptr = lower_expr(masm, text, func, ctx);

            // ensure function pointer is in a register
            if (func_ptr.kind != MASM_OPERAND_REGISTER)
            {
                MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, func_ptr));
                func_ptr = rax;
            }

            masm_section_append_inst(text, masm_inst_1(MASM_OP_CALL, func_ptr));
        }

        // clean up stack arguments
        int total_cleanup = (stack_args_count * 8) + padding;
        if (total_cleanup > 0)
        {
            MasmOperand rsp = masm_operand_register(MASM_X86_RSP, 8);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_ADD, rsp, masm_operand_imm(total_cleanup)));
        }

        return masm_operand_register(MASM_X86_RAX, 8);
    }
    else if (expr->kind == AST_EXPR_UNARY)
    {
        if (expr->unary_expr.op == TOKEN_QUESTION)
        {
            // Address-of: ?expr
            AstNode *operand_node = expr->unary_expr.expr;

            if (operand_node->kind == AST_EXPR_IDENT)
            {
                LocalVar *var = find_local_var(ctx, operand_node->ident_expr.name);
                if (var)
                {
                    MasmOperand src = masm_operand_memory_simple(MASM_X86_RBP, var->offset, var->size);
                    MasmOperand dst = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_LEA, dst, src));
                    return dst;
                }
            }
            return masm_operand_none();
        }
        else if (expr->unary_expr.op == TOKEN_AT)
        {
            // Dereference: @expr
            MasmOperand ptr = lower_expr(masm, text, expr->unary_expr.expr, ctx);

            if (ptr.kind != MASM_OPERAND_REGISTER)
            {
                MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, ptr));
                ptr = rax;
            }

            int size = 8;
            if (expr->type)
            {
                size = expr->type->size;
            }

            return masm_operand_memory_simple(ptr.reg.id, 0, size);
        }

        MasmOperand operand = lower_expr(masm, text, expr->unary_expr.expr, ctx);
        MasmOperand result  = masm_operand_register(MASM_X86_RAX, 8);

        if (operand.kind != MASM_OPERAND_REGISTER || operand.reg.id != MASM_X86_RAX)
        {
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, operand));
        }

        if (expr->unary_expr.op == TOKEN_BANG)
        {
            // !expr -> cmp rax, 0; sete al; and rax, 1
            masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, result, masm_operand_imm(0)));

            MasmOperand al = masm_operand_register(MASM_X86_RAX, 1);
            masm_section_append_inst(text, masm_inst_1(MASM_OP_SETE, al));

            masm_section_append_inst(text, masm_inst_2(MASM_OP_AND, result, masm_operand_imm(1)));
        }
        else if (expr->unary_expr.op == TOKEN_MINUS)
        {
            // -expr -> sub 0, rax
            MasmOperand rcx = masm_operand_register(MASM_X86_RCX, 8);
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
                        MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                        masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, obj));
                        obj = rax;
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
        // align to 8 bytes if needed (simple alignment for now)
        if (ctx->stack_offset % 8 != 0)
        {
            ctx->stack_offset -= (8 - (abs(ctx->stack_offset) % 8));
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
                MasmOperand dst         = masm_operand_memory_simple(MASM_X86_RBP, elem_offset, elem_size);

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
                    MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, val));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, rax));
                }
            }
        }

        return masm_operand_memory_simple(MASM_X86_RBP, base_offset, type->size);
    }

    else if (expr->kind == AST_EXPR_INDEX)
    {
        // arr[i]
        MasmOperand arr = lower_expr(masm, text, expr->index_expr.array, ctx);
        MasmOperand idx = lower_expr(masm, text, expr->index_expr.index, ctx);

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

            if (arr.kind == MASM_OPERAND_REGISTER)
            {
                // [reg + offset]
                return masm_operand_memory_simple(arr.reg.id, offset, elem_size);
            }
            else if (arr.kind == MASM_OPERAND_MEMORY)
            {
                // [base + disp + offset]
                return masm_operand_memory_simple(arr.mem.base.id, arr.mem.disp + offset, elem_size);
            }
        }
        else
        {
            // Register index
            MasmOperand idx_reg = idx;
            if (idx.kind != MASM_OPERAND_REGISTER)
            {
                MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, idx));
                idx_reg = rax;
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

                if (idx_reg.reg.id != MASM_X86_RAX && idx_reg.reg.id != MASM_X86_RCX)
                {
                    MasmOperand rax = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, idx_reg));
                    idx_reg = rax;
                }

                masm_section_append_inst(text, masm_inst_3(MASM_OP_IMUL, idx_reg, idx_reg, masm_operand_imm(elem_size)));
                elem_size = 1; // scale is now 1
            }

            if (arr.kind == MASM_OPERAND_REGISTER)
            {
                // [base + index * scale]
                MasmRegister base  = {arr.reg.id, 8};
                MasmRegister index = {idx_reg.reg.id, 8};
                return masm_operand_memory(base, index, elem_size, 0, elem_type->size);
            }
            else if (arr.kind == MASM_OPERAND_MEMORY)
            {
                // [base + disp + index * scale]
                // arr.mem.base is usually RBP
                MasmRegister base  = arr.mem.base;
                MasmRegister index = {idx_reg.reg.id, 8};
                return masm_operand_memory(base, index, elem_size, arr.mem.disp, elem_type->size);
            }
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
        // align to 8 bytes
        if (ctx->stack_offset % 8 != 0)
        {
            ctx->stack_offset -= (8 - (ctx->stack_offset % 8));
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
                MasmOperand dest      = masm_operand_memory_simple(MASM_X86_RBP, dest_disp, field_type->size > 8 ? 8 : field_type->size);

                if (init_op.kind == MASM_OPERAND_REGISTER)
                {
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dest, init_op));
                }
                else if (init_op.kind == MASM_OPERAND_IMM)
                {
                    if (init_op.imm > 2147483647 || init_op.imm < -2147483648)
                    {
                        MasmOperand tmp = masm_operand_register(MASM_X86_RAX, 8);
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
                    MasmOperand tmp = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, init_op));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dest, tmp));
                }
            }
        }

        return masm_operand_memory_simple(MASM_X86_RBP, base_offset, 8);
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
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, masm_operand_register(MASM_X86_RAX, 8), op));
        }

        // run all deferred statements before returning
        emit_deferred_from(masm, text, ctx, 0);

        // emit epilogue before return
        MasmOperand rbp = masm_operand_register(MASM_X86_RBP, 8);
        MasmOperand rsp = masm_operand_register(MASM_X86_RSP, 8);
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
        // determine size
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
            size = 8; // fallback
        }

        // align size to 8 bytes for stack slots
        if (size % 8 != 0)
        {
            size += (8 - (size % 8));
        }

        // allocate space on stack for variable
        ctx->stack_offset -= size;
        int32_t offset = ctx->stack_offset;

        // add to symbol table
        add_local_var(ctx, stmt->var_stmt.name, offset, size);

        // if there's an initializer, evaluate and store it
        if (stmt->var_stmt.init)
        {
            MasmOperand value = lower_expr(masm, text, stmt->var_stmt.init, ctx);

            // store value to stack location [rbp + offset]
            MasmOperand var_mem = masm_operand_memory_simple(MASM_X86_RBP, offset, size > 8 ? 8 : size);

            // if value is in a register, store it directly
            if (value.kind == MASM_OPERAND_REGISTER)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, value));
            }
            // if value is immediate, move to register first
            else if (value.kind == MASM_OPERAND_IMM)
            {
                MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, value));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, var_mem, temp));
            }
            else if (value.kind == MASM_OPERAND_MEMORY)
            {
                // Memory to Memory copy
                int32_t src_disp = value.mem.disp;

                // copy in 8-byte chunks
                for (size_t i = 0; i < size; i += 8)
                {
                    MasmOperand src_chunk = masm_operand_memory_simple(value.mem.base.id, src_disp + i, 8);
                    MasmOperand dst_chunk = masm_operand_memory_simple(MASM_X86_RBP, offset + i, 8);

                    MasmOperand tmp = masm_operand_register(MASM_X86_RAX, 8);
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, tmp, src_chunk));
                    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst_chunk, tmp));
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
        snprintf(else_label, sizeof(else_label), ".Lif_else_%d", label_counter);
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
            MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
            masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
            masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
        }
        else
        {
            // memory or other
            MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
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
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(ctx->loop_start_label)));

        // Condition
        if (stmt->for_stmt.cond)
        {
            MasmOperand cond   = lower_expr(masm, text, stmt->for_stmt.cond, ctx);
            MasmOperand result = masm_operand_register(MASM_X86_RAX, 8);

            if (cond.kind != MASM_OPERAND_REGISTER || cond.reg.id != MASM_X86_RAX)
            {
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, result, cond));
            }

            masm_section_append_inst(text, masm_inst_2(MASM_OP_CMP, result, masm_operand_imm(0)));
            masm_section_append_inst(text, masm_inst_1(MASM_OP_JE, masm_operand_label(ctx->loop_end_label)));
        }

        // Body
        lower_stmt(masm, text, stmt->for_stmt.body, ctx);

        // Jump back to start
        masm_section_append_inst(text, masm_inst_1(MASM_OP_JMP, masm_operand_label(ctx->loop_start_label)));

        // Emit end label
        masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(ctx->loop_end_label)));

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
                MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
            }
            else
            {
                MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
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
                MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, temp, cond));
                masm_section_append_inst(text, masm_inst_2(MASM_OP_TEST, temp, temp));
            }
            else
            {
                MasmOperand temp = masm_operand_register(MASM_X86_RAX, 8);
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
        // skip whitespace
        while (*token == ' ' || *token == '\t')
        {
            token++;
        }

        if (strncmp(token, "syscall", 7) == 0)
        {
            masm_section_append_inst(text, masm_inst_0(MASM_OP_SYSCALL));
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

        token = strtok_r(NULL, "\n;", &saveptr);
    }

    free(line);
}

static MasmOperand parse_operand(const char *str, LowerContext *ctx)
{
    // parse register or immediate
    if (strcmp(str, "rax") == 0)
    {
        return masm_operand_register(MASM_X86_RAX, 8);
    }
    if (strcmp(str, "eax") == 0)
    {
        return masm_operand_register(MASM_X86_RAX, 4);
    }
    if (strcmp(str, "al") == 0)
    {
        return masm_operand_register(MASM_X86_RAX, 1);
    }
    if (strcmp(str, "rdi") == 0)
    {
        return masm_operand_register(MASM_X86_RDI, 8);
    }
    if (strcmp(str, "rsi") == 0)
    {
        return masm_operand_register(MASM_X86_RSI, 8);
    }
    if (strcmp(str, "rdx") == 0)
    {
        return masm_operand_register(MASM_X86_RDX, 8);
    }
    if (strcmp(str, "rcx") == 0)
    {
        return masm_operand_register(MASM_X86_RCX, 8);
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
            return masm_operand_memory_simple(MASM_X86_RBP, var->offset, var->size);
        }
    }

    return masm_operand_none();
}

static void lower_function(Masm *masm, AstNode *func_node, SymbolTable *symbols)
{
    (void)symbols;

    // use export_name if available (from $funcname.name = "..."), otherwise use AST name
    const char *func_name = func_node->fun_stmt.name;
    if (func_node->symbol)
    {
        const char *link_name = symbol_get_linkage_name(func_node->symbol);
        if (link_name)
        {
            func_name = link_name;
        }
    }

    MasmSymbol *sym = masm_symbol_create(func_name, MASM_SYMBOL_FUNCTION, MASM_BIND_GLOBAL);
    masm_add_symbol(masm, sym);

    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);

    masm_section_append_inst(text, masm_inst_1(MASM_OP_LABEL, masm_operand_label(func_name)));

    // create context for this function
    LowerContext *ctx = create_context();

    // emit prologue: push rbp; mov rbp, rsp
    MasmOperand rbp = masm_operand_register(MASM_X86_RBP, 8);
    MasmOperand rsp = masm_operand_register(MASM_X86_RSP, 8);
    masm_section_append_inst(text, masm_inst_1(MASM_OP_PUSH, rbp));
    masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rbp, rsp));

    // reserve space for locals (placeholder, will be patched)
    size_t sub_rsp_idx = text->inst_count;
    masm_section_append_inst(text, masm_inst_2(MASM_OP_SUB, rsp, masm_operand_imm(0)));

    // handle parameters
    if (func_node->fun_stmt.params)
    {
        AstList *params = func_node->fun_stmt.params;
        for (int i = 0; i < params->count; i++)
        {
            AstNode *param = params->items[i];
            // allocate stack space
            ctx->stack_offset -= 8;
            int32_t offset = ctx->stack_offset;

            // add to symbol table
            add_local_var(ctx, param->param_stmt.name, offset, 8);

            // move register to stack
            if (i < 6)
            {
                MasmX86Reg  reg = masm_sysv64_arg_reg(i);
                MasmOperand src = masm_operand_register(reg, 8);
                MasmOperand dst = masm_operand_memory_simple(MASM_X86_RBP, offset, 8);
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, src));
            }
            else
            {
                // stack parameter
                // [rbp + 16 + (i-6)*8]
                int32_t     stack_param_offset = 16 + (i - 6) * 8;
                MasmOperand src                = masm_operand_memory_simple(MASM_X86_RBP, stack_param_offset, 8);
                MasmOperand dst                = masm_operand_memory_simple(MASM_X86_RBP, offset, 8);
                MasmOperand rax                = masm_operand_register(MASM_X86_RAX, 8);

                // mov rax, [rbp + offset]
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, rax, src));
                // mov [rbp - local], rax
                masm_section_append_inst(text, masm_inst_2(MASM_OP_MOV, dst, rax));
            }
        }
    }

    if (func_node->fun_stmt.body)
    {
        lower_stmt(masm, text, func_node->fun_stmt.body, ctx);
    }

    // patch stack allocation
    // stack grows down, so stack_offset is negative.
    // size = -stack_offset
    // align to 16 bytes
    int32_t frame_size = -ctx->stack_offset;
    if (frame_size % 16 != 0)
    {
        frame_size = (frame_size + 15) & ~15;
    }

    if (frame_size > 0)
    {
        text->instructions[sub_rsp_idx].operands[1].imm = frame_size;
    }
    else
    {
        // if no locals, we can make this a NOP or just SUB RSP, 0 (which is harmless but wasteful)
        // For now, SUB RSP, 0 is fine, or we could overwrite with NOP if we had one.
        // Actually, we can just leave it as SUB RSP, 0.
    }

    // emit epilogue: mov rsp, rbp; pop rbp; ret
    // Note: ret statements will already emit ret, so we only add epilogue if function doesn't end with ret
    // For now, always add epilogue before ret in lower_stmt for AST_STMT_RET

    destroy_context(ctx);
}

static void lower_global_var(Masm *masm, AstNode *stmt);
static bool is_function_decl_in_program(AstNode *program, AstNode *decl)
{
    if (!program || program->kind != AST_PROGRAM || !decl)
    {
        return false;
    }

    AstList *stmts = program->program.stmts;
    if (!stmts)
    {
        return false;
    }

    for (int i = 0; i < stmts->count; i++)
    {
        if (stmts->items[i] == decl)
        {
            return true;
        }
    }

    return false;
}

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

        // lower any instantiated generic functions that are not part of the original AST
        if (symbols)
        {
            for (Symbol *sym = symbols->symbols; sym; sym = sym->next)
            {
                if (sym->kind != SYMBOL_FUNCTION)
                {
                    continue;
                }

                if (sym->is_generic)
                {
                    // skip generic templates; only instantiated copies are lowered
                    continue;
                }

                if (!sym->decl || sym->decl->kind != AST_STMT_FUN)
                {
                    continue;
                }

                if (is_function_decl_in_program(ast, sym->decl))
                {
                    continue; // already lowered from the program AST
                }

                // skip external/FFI functions with no body
                if (!sym->decl->fun_stmt.body)
                {
                    continue;
                }

                lower_function(masm, sym->decl, symbols);
            }
        }
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
