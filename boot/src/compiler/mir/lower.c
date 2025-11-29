#include "compiler/mir/lower.h"
#include "compiler/mir/block.h"
#include "compiler/mir/function.h"
#include "compiler/mir/global.h"
#include "compiler/mir/inst.h"
#include "compiler/mir/opcode.h"
#include "compiler/symbol.h"
#include "compiler/token.h"
#include "compiler/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stack slot mapping for local variables
typedef struct
{
    AstNode *node;   // Variable AST node
    int32_t  offset; // Stack offset from RBP (negative)
} StackSlot;

// Deferred statement frame for tracking fin statements per block
typedef struct DeferredFrame
{
    AstList              *stmts; // deferred statements for this block
    struct DeferredFrame *prev;  // link to enclosing block's frame
} DeferredFrame;

// lowering context tracks state during lowering
typedef struct LowerContext
{
    MIRModule   *module;
    MIRFunction *current_function;
    MIRBlock    *current_block;

    // value map: ast node -> mir value (for expressions)
    struct
    {
        AstNode  *node;
        MIRValue *value;
    }  *value_map;
    int value_map_count;
    int value_map_capacity;

    // Stack frame management
    StackSlot *stack_slots;
    int        stack_slot_count;
    int        stack_slot_capacity;
    int32_t    frame_size; // Total stack space needed (in bytes)

    // Loop control flow
    MIRBlock      *loop_start_block;    // For continue
    MIRBlock      *loop_end_block;      // For break
    DeferredFrame *loop_deferred_frame; // deferred frame at loop entry (for brk/cnt)

    // Deferred statements (fin)
    DeferredFrame *deferred_stack; // stack of deferred statement frames

    // Constant counters
    int float_const_count;
    int string_const_count;
} LowerContext;

static LowerContext *lower_context_create(MIRModule *module)
{
    LowerContext *ctx = malloc(sizeof(LowerContext));
    if (!ctx)
    {
        return NULL;
    }

    ctx->module              = module;
    ctx->current_function    = NULL;
    ctx->current_block       = NULL;
    ctx->value_map           = NULL;
    ctx->value_map_count     = 0;
    ctx->value_map_capacity  = 0;
    ctx->stack_slots         = NULL;
    ctx->stack_slot_count    = 0;
    ctx->stack_slot_capacity = 0;
    ctx->frame_size          = 0;
    ctx->loop_start_block    = NULL;
    ctx->loop_end_block      = NULL;
    ctx->loop_deferred_frame = NULL;
    ctx->deferred_stack      = NULL;
    ctx->float_const_count   = 0;

    return ctx;
}

static void lower_context_destroy(LowerContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    if (ctx->value_map)
    {
        free(ctx->value_map);
    }

    if (ctx->stack_slots)
    {
        free(ctx->stack_slots);
    }

    // free any remaining deferred frames
    while (ctx->deferred_stack)
    {
        DeferredFrame *frame = ctx->deferred_stack;
        ctx->deferred_stack  = frame->prev;
        free(frame);
    }

    free(ctx);
}

// push a new deferred frame for a block
static void lower_push_deferred_frame(LowerContext *ctx, AstList *stmts)
{
    DeferredFrame *frame = malloc(sizeof(DeferredFrame));
    if (!frame)
    {
        return;
    }

    frame->stmts        = stmts;
    frame->prev         = ctx->deferred_stack;
    ctx->deferred_stack = frame;
}

// pop the current deferred frame
static void lower_pop_deferred_frame(LowerContext *ctx)
{
    if (!ctx->deferred_stack)
    {
        return;
    }

    DeferredFrame *frame = ctx->deferred_stack;
    ctx->deferred_stack  = frame->prev;
    free(frame);
}

// forward declarations
static int       lower_stmt(LowerContext *ctx, AstNode *node);
static MIRValue *lower_expr(LowerContext *ctx, AstNode *node);
static MIRValue *lower_address(LowerContext *ctx, AstNode *node);

// check if an expression tree contains a function call
static bool expr_contains_call(AstNode *node)
{
    if (!node)
    {
        return false;
    }

    switch (node->kind)
    {
    case AST_EXPR_CALL:
        return true;

    case AST_EXPR_BINARY:
        return expr_contains_call(node->binary_expr.left) || expr_contains_call(node->binary_expr.right);

    case AST_EXPR_UNARY:
        return expr_contains_call(node->unary_expr.expr);

    case AST_EXPR_CAST:
        return expr_contains_call(node->cast_expr.expr);

    case AST_EXPR_INDEX:
        return expr_contains_call(node->index_expr.array) || expr_contains_call(node->index_expr.index);

    case AST_EXPR_FIELD:
        return expr_contains_call(node->field_expr.object);

    case AST_EXPR_STRUCT:
        if (node->struct_expr.fields)
        {
            for (int i = 0; i < node->struct_expr.fields->count; i++)
            {
                AstNode *field = node->struct_expr.fields->items[i];
                if (field && expr_contains_call(field->field_expr.object))
                {
                    return true;
                }
            }
        }
        return false;

    default:
        return false;
    }
}

// emit deferred statements for current block only (in LIFO order)
static void lower_emit_deferred_current(LowerContext *ctx)
{
    if (!ctx->deferred_stack || !ctx->deferred_stack->stmts)
    {
        return;
    }

    AstList *stmts = ctx->deferred_stack->stmts;
    // emit in reverse order (LIFO)
    for (int i = stmts->count - 1; i >= 0; i--)
    {
        lower_stmt(ctx, stmts->items[i]);
    }
}

// emit all deferred statements up to function scope (for ret)
static void lower_emit_deferred_all(LowerContext *ctx)
{
    DeferredFrame *frame = ctx->deferred_stack;
    while (frame)
    {
        if (frame->stmts)
        {
            // emit in reverse order (LIFO)
            for (int i = frame->stmts->count - 1; i >= 0; i--)
            {
                lower_stmt(ctx, frame->stmts->items[i]);
            }
        }
        frame = frame->prev;
    }
}

// emit deferred statements up to loop boundary (for brk/cnt)
static void lower_emit_deferred_to_loop(LowerContext *ctx)
{
    DeferredFrame *frame      = ctx->deferred_stack;
    DeferredFrame *loop_frame = ctx->loop_deferred_frame;

    while (frame && frame != loop_frame)
    {
        if (frame->stmts)
        {
            // emit in reverse order (LIFO)
            for (int i = frame->stmts->count - 1; i >= 0; i--)
            {
                lower_stmt(ctx, frame->stmts->items[i]);
            }
        }
        frame = frame->prev;
    }
}

static void lower_context_map_value(LowerContext *ctx, AstNode *node, MIRValue *value)
{
    if (!ctx || !node || !value)
    {
        return;
    }

    // expand capacity if needed
    if (ctx->value_map_count >= ctx->value_map_capacity)
    {
        int   new_capacity = ctx->value_map_capacity == 0 ? 64 : ctx->value_map_capacity * 2;
        void *new_map      = realloc(ctx->value_map, new_capacity * sizeof(*ctx->value_map));
        if (!new_map)
        {
            return;
        }
        ctx->value_map          = new_map;
        ctx->value_map_capacity = new_capacity;
    }

    ctx->value_map[ctx->value_map_count].node  = node;
    ctx->value_map[ctx->value_map_count].value = value;
    ctx->value_map_count++;
}

static MIRValue *lower_context_get_value(LowerContext *ctx, AstNode *node)
{
    if (!ctx || !node)
    {
        return NULL;
    }

    for (int i = 0; i < ctx->value_map_count; i++)
    {
        if (ctx->value_map[i].node == node)
        {
            return ctx->value_map[i].value;
        }
    }

    return NULL;
}

// Allocate a stack slot for a local variable
// Returns the negative offset from RBP (e.g., -8, -16, -24...)
// If node is NULL, allocates an anonymous temporary slot
static int32_t lower_context_alloc_stack_slot(LowerContext *ctx, AstNode *node, size_t size)
{
    if (!ctx)
    {
        return 0;
    }

    // Expand capacity if needed
    if (ctx->stack_slot_count >= ctx->stack_slot_capacity)
    {
        int   new_capacity = ctx->stack_slot_capacity == 0 ? 16 : ctx->stack_slot_capacity * 2;
        void *new_slots    = realloc(ctx->stack_slots, new_capacity * sizeof(StackSlot));
        if (!new_slots)
        {
            return 0;
        }
        ctx->stack_slots         = new_slots;
        ctx->stack_slot_capacity = new_capacity;
    }

    // Allocate new stack space (stack grows down)
    ctx->frame_size += size;
    int32_t offset = -(int32_t)ctx->frame_size;

    // Record the slot (node can be NULL for anonymous temporaries)
    ctx->stack_slots[ctx->stack_slot_count].node   = node;
    ctx->stack_slots[ctx->stack_slot_count].offset = offset;
    ctx->stack_slot_count++;

    return offset;
}

// Get stack offset for a variable
static int32_t lower_context_get_stack_offset(LowerContext *ctx, AstNode *node)
{
    if (!ctx || !node)
    {
        return 0;
    }

    for (int i = 0; i < ctx->stack_slot_count; i++)
    {
        if (ctx->stack_slots[i].node == node)
        {
            return ctx->stack_slots[i].offset;
        }
    }

    return 0; // Not found (probably a global variable)
}

// lower function to MIR
static int lower_function(LowerContext *ctx, AstNode *node)
{
    if (node->kind != AST_STMT_FUN)
    {
        return -1;
    }

    // create mir function
    const char *func_name = symbol_get_linkage_name(node->symbol);
    if (!func_name)
    {
        func_name = node->fun_stmt.name;
    }

    MIRFunction *func = mir_function_create(func_name, NULL, node->fun_stmt.is_public);
    if (!func)
    {
        return -1;
    }

    // reset function-specific context state
    ctx->current_function = func;
    ctx->frame_size       = 0;
    ctx->stack_slot_count = 0;

    // create entry block
    MIRBlock *entry    = mir_function_add_block(func, "entry");
    ctx->current_block = entry;

    // Handle hidden pointer for large struct returns (> 16 bytes)
    Type *ret_type       = node->symbol->type;
    bool  has_hidden_ptr = (ret_type && ret_type->kind == TYPE_STRUCT && ret_type->size > 16);

    if (has_hidden_ptr)
    {
        // Add hidden pointer as first parameter
        MIRValue *hidden_ptr = mir_function_add_param(func, NULL, ".hidden_ret_ptr");

        // Spill to stack (optional, but consistent with other params)
        int32_t offset = lower_context_alloc_stack_slot(ctx, NULL, 8);

        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
        mir_inst_add_operand(store, mir_operand_value(hidden_ptr->id));
        mir_inst_add_operand(store, mir_operand_imm_int(offset));
        mir_block_append_inst(ctx->current_block, store);

        // We need to store this offset or value somewhere to use it in RET
        // For now, we can find it by name or assume it's the first param if has_hidden_ptr is true
        // But lower_stmt doesn't know about has_hidden_ptr easily without checking the function symbol again.
    }

    // add parameters and spill to stack
    if (node->fun_stmt.params)
    {
        for (int i = 0; i < node->fun_stmt.params->count; i++)
        {
            AstNode *param = node->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                MIRValue *param_val = mir_function_add_param(func, NULL, param->param_stmt.name);

                // Allocate stack slot for parameter
                size_t size = (param->type) ? param->type->size : 8;
                if (size == 0)
                {
                    size = 8;
                }

                int32_t offset = lower_context_alloc_stack_slot(ctx, param, size);

                // Store parameter register to stack slot
                MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                mir_inst_add_operand(store, mir_operand_value(param_val->id));
                mir_inst_add_operand(store, mir_operand_imm_int(offset));
                mir_block_append_inst(ctx->current_block, store);
            }
        }
    }

    // lower function body
    if (node->fun_stmt.body)
    {
        lower_stmt(ctx, node->fun_stmt.body);
    }

    // set function frame size
    func->frame_size = ctx->frame_size;

    // ensure block is terminated
    if (ctx->current_block && !ctx->current_block->last_inst)
    {
        // add implicit return void
        MIRInst *ret = mir_inst_ret_void();
        mir_block_append_inst(ctx->current_block, ret);
    }

    // add function to module
    mir_module_add_function(ctx->module, func);

    ctx->current_function = NULL;
    ctx->current_block    = NULL;

    return 0;
}

// lower variable/value declaration
static int lower_var(LowerContext *ctx, AstNode *node)
{
    if (node->kind != AST_STMT_VAR && node->kind != AST_STMT_VAL)
    {
        return -1;
    }

    // check if global or local
    if (!ctx->current_function)
    {
        // global variable
        const char *var_name = symbol_get_linkage_name(node->symbol);
        if (!var_name)
        {
            var_name = node->var_stmt.name;
        }

        MIRGlobalKind kind   = (node->kind == AST_STMT_VAL) ? MIR_GLOBAL_VAL : MIR_GLOBAL_VAR;
        MIRGlobal    *global = mir_global_create(var_name, node->type, kind, node->var_stmt.is_public);

        // handle initializer
        if (node->var_stmt.init)
        {
            if (node->var_stmt.init->kind == AST_EXPR_LIT && node->var_stmt.init->token)
            {
                if (node->var_stmt.init->token->kind == TOKEN_LIT_INT)
                {
                    mir_global_set_int_init(global, node->var_stmt.init->lit_expr.int_val);
                }
                else if (node->var_stmt.init->token->kind == TOKEN_LIT_FLOAT)
                {
                    mir_global_set_float_init(global, node->var_stmt.init->lit_expr.float_val);
                }
                else if (node->var_stmt.init->token->kind == TOKEN_LIT_STRING)
                {
                    mir_global_set_string_init(global, node->var_stmt.init->lit_expr.string_val);
                }
            }
        }

        mir_module_add_global(ctx->module, global);
    }
    else
    {
        // Local variable - allocate stack slot
        size_t size = node->type ? node->type->size : 8;
        if (size == 0)
        {
            size = 8; // default if unknown
        }
        int32_t offset = lower_context_alloc_stack_slot(ctx, node, size);

        // Handle initializer
        if (node->var_stmt.init)
        {
            if (node->var_stmt.init->kind == AST_EXPR_STRUCT)
            {
                // In-place initialization for struct literal
                AstNode *struct_expr = node->var_stmt.init;
                Type    *type        = struct_expr->type;

                if (struct_expr->struct_expr.fields)
                {
                    for (int i = 0; i < struct_expr->struct_expr.fields->count; i++)
                    {
                        AstNode *field_init = struct_expr->struct_expr.fields->items[i];
                        // find field offset
                        int32_t field_offset = 0;
                        for (int j = 0; j < type->structure.field_count; j++)
                        {
                            if (strcmp(type->structure.fields[j].name, field_init->field_expr.field) == 0)
                            {
                                field_offset = (int32_t)type->structure.fields[j].offset;
                                break;
                            }
                        }

                        // lower init expr
                        MIRValue *val = lower_expr(ctx, field_init->field_expr.object);
                        if (val)
                        {
                            // store to stack at offset + field_offset
                            MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                            mir_inst_add_operand(store, mir_operand_value(val->id));
                            mir_inst_add_operand(store, mir_operand_imm_int(offset + field_offset));
                            mir_block_append_inst(ctx->current_block, store);
                        }
                    }
                }
            }
            else if (node->var_stmt.init->kind == AST_EXPR_ARRAY)
            {
                // In-place initialization for array literal
                AstNode *array_expr = node->var_stmt.init;
                Type    *type       = node->type;
                size_t   elem_size  = type->array.elem_type->size;

                if (array_expr->array_expr.elems)
                {
                    for (int i = 0; i < array_expr->array_expr.elems->count; i++)
                    {
                        AstNode  *item_expr = array_expr->array_expr.elems->items[i];
                        MIRValue *val       = lower_expr(ctx, item_expr);
                        if (val)
                        {
                            // store to stack at offset + i * elem_size
                            MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                            mir_inst_add_operand(store, mir_operand_value(val->id));
                            mir_inst_add_operand(store, mir_operand_imm_int(offset + i * elem_size));
                            mir_block_append_inst(ctx->current_block, store);
                        }
                    }
                }
            }
            else
            {
                MIRValue *val = lower_expr(ctx, node->var_stmt.init);
                if (val)
                {
                    // Check if large struct copy
                    if (node->type && node->type->size > 8)
                    {
                        // val is source address
                        // offset is dest stack offset

                        // Emit memcpy: [offset] = [val]
                        size_t size = node->type->size;

                        MIRValue *src_addr = mir_function_alloc_value(ctx->current_function, NULL, "src_addr");
                        MIRValue *tmp      = mir_function_alloc_value(ctx->current_function, NULL, "copy_tmp");
                        MIRValue *val_copy = mir_function_alloc_value(ctx->current_function, NULL, "val_copy");

                        // Copy val to val_copy to preserve it across loop (avoid reg collision with tmp)
                        MIRInst *mov = mir_inst_create(MIR_OP_MOV, NULL);
                        mir_inst_add_operand(mov, mir_operand_value(val->id));
                        mir_inst_set_result(mov, val_copy);
                        mir_block_append_inst(ctx->current_block, mov);

                        printf("MEMCPY: copying %zu bytes from val (id=%u) to offset %d\n", size, val->id, offset);

                        for (size_t i = 0; i < size; i += 8)
                        {
                            printf("  MEMCPY iteration i=%zu\n", i);
                            // Calculate src address: val_copy + i
                            MIRValue *curr_src = val_copy;
                            if (i > 0)
                            {
                                // Reuse src_addr value
                                MIRInst *add = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(val_copy->id), mir_operand_imm_int(i));
                                mir_inst_set_result(add, src_addr);
                                mir_block_append_inst(ctx->current_block, add);
                                curr_src = src_addr;
                            }

                            // Load from src
                            MIRInst *load = mir_inst_create(MIR_OP_LOAD, NULL);
                            mir_inst_add_operand(load, mir_operand_value(curr_src->id));
                            mir_inst_set_result(load, tmp);
                            mir_block_append_inst(ctx->current_block, load);

                            // Store to dst: [rbp + offset + i]
                            MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                            mir_inst_add_operand(store, mir_operand_value(tmp->id));
                            mir_inst_add_operand(store, mir_operand_imm_int(offset + i));
                            mir_block_append_inst(ctx->current_block, store);
                        }
                    }
                    else
                    {
                        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                        mir_inst_add_operand(store, mir_operand_value(val->id));
                        mir_inst_add_operand(store, mir_operand_imm_int(offset));
                        mir_block_append_inst(ctx->current_block, store);
                    }
                }
                else
                {
                    // failed to lower init expr
                }
            }
        }
        // No init value means uninitialized - stack slot is already allocated
    }

    return 0;
}

static int lower_if_stmt(LowerContext *ctx, AstNode *node)
{
    if (node->kind != AST_STMT_IF && node->kind != AST_STMT_OR)
    {
        return -1;
    }

    // If it's an OR node without condition (else), just lower the body
    if (node->kind == AST_STMT_OR && !node->cond_stmt.cond)
    {
        return lower_stmt(ctx, node->cond_stmt.body);
    }

    // create blocks
    MIRBlock *then_block  = mir_function_add_block(ctx->current_function, "if.then");
    MIRBlock *else_block  = node->cond_stmt.stmt_or ? mir_function_add_block(ctx->current_function, "if.else") : NULL;
    MIRBlock *merge_block = mir_function_add_block(ctx->current_function, "if.end");

    // lower condition
    MIRValue *cond = lower_expr(ctx, node->cond_stmt.cond);
    if (!cond)
    {
        return -1;
    }

    // branch
    MIRBlock *false_target = else_block ? else_block : merge_block;
    MIRInst  *br           = mir_inst_brcond(mir_operand_value(cond->id), then_block->id, false_target->id);
    mir_block_append_inst(ctx->current_block, br);

    // then block
    ctx->current_block = then_block;
    if (lower_stmt(ctx, node->cond_stmt.body) < 0)
    {
        return -1;
    }

    if (!ctx->current_block->last_inst || !mir_op_is_terminator(ctx->current_block->last_inst->op))
    {
        mir_block_append_inst(ctx->current_block, mir_inst_br(merge_block->id));
    }

    // else block
    if (else_block)
    {
        ctx->current_block = else_block;
        if (lower_stmt(ctx, node->cond_stmt.stmt_or) < 0)
        {
            return -1;
        }

        if (!ctx->current_block->last_inst || !mir_op_is_terminator(ctx->current_block->last_inst->op))
        {
            mir_block_append_inst(ctx->current_block, mir_inst_br(merge_block->id));
        }
    }

    ctx->current_block = merge_block;
    return 0;
}

static int lower_for_stmt(LowerContext *ctx, AstNode *node)
{
    if (node->kind != AST_STMT_FOR)
    {
        return -1;
    }

    // Save previous loop context
    MIRBlock      *prev_start    = ctx->loop_start_block;
    MIRBlock      *prev_end      = ctx->loop_end_block;
    DeferredFrame *prev_deferred = ctx->loop_deferred_frame;

    // create blocks
    MIRBlock *cond_block = mir_function_add_block(ctx->current_function, "for.cond");
    MIRBlock *body_block = mir_function_add_block(ctx->current_function, "for.body");
    MIRBlock *end_block  = mir_function_add_block(ctx->current_function, "for.end");

    // Set current loop context (save deferred frame at loop entry for brk/cnt)
    ctx->loop_start_block    = cond_block;
    ctx->loop_end_block      = end_block;
    ctx->loop_deferred_frame = ctx->deferred_stack;

    // jump to condition
    mir_block_append_inst(ctx->current_block, mir_inst_br(cond_block->id));

    // condition block
    ctx->current_block = cond_block;
    if (node->for_stmt.cond)
    {
        MIRValue *cond = lower_expr(ctx, node->for_stmt.cond);
        if (!cond)
        {
            return -1;
        }

        MIRInst *br = mir_inst_brcond(mir_operand_value(cond->id), body_block->id, end_block->id);
        mir_block_append_inst(ctx->current_block, br);
    }
    else
    {
        // infinite loop
        mir_block_append_inst(ctx->current_block, mir_inst_br(body_block->id));
    }

    // body block
    ctx->current_block = body_block;
    if (lower_stmt(ctx, node->for_stmt.body) < 0)
    {
        return -1;
    }

    if (!ctx->current_block->last_inst || !mir_op_is_terminator(ctx->current_block->last_inst->op))
    {
        mir_block_append_inst(ctx->current_block, mir_inst_br(cond_block->id));
    }

    ctx->current_block = end_block;

    // Restore previous loop context
    ctx->loop_start_block    = prev_start;
    ctx->loop_end_block      = prev_end;
    ctx->loop_deferred_frame = prev_deferred;

    return 0;
}

// lower statement
static int lower_stmt(LowerContext *ctx, AstNode *node)
{
    if (!node)
    {
        return 0;
    }

    switch (node->kind)
    {
    case AST_STMT_FUN:
        return lower_function(ctx, node);

    case AST_STMT_VAR:
    case AST_STMT_VAL:
        return lower_var(ctx, node);

    case AST_STMT_BLOCK:
    {
        // push deferred frame for this block
        lower_push_deferred_frame(ctx, node->block_stmt.deferred_stmts);

        if (node->block_stmt.stmts)
        {
            for (int i = 0; i < node->block_stmt.stmts->count; i++)
            {
                if (lower_stmt(ctx, node->block_stmt.stmts->items[i]) < 0)
                {
                    lower_pop_deferred_frame(ctx);
                    return -1;
                }
            }
        }

        // emit deferred statements at normal block exit (LIFO order)
        lower_emit_deferred_current(ctx);
        lower_pop_deferred_frame(ctx);
        return 0;
    }

    case AST_STMT_COMPTIME_IF:
    case AST_STMT_COMPTIME_OR:
        if (node->comptime_if_stmt.taken_branch)
        {
            return lower_stmt(ctx, node->comptime_if_stmt.taken_branch);
        }
        return 0;

    case AST_STMT_RET:
        if (node->ret_stmt.expr)
        {
            MIRValue *val = lower_expr(ctx, node->ret_stmt.expr);
            if (val)
            {
                // Check if we need to return via hidden pointer
                MIRFunction *func           = ctx->current_function;
                bool         has_hidden_ptr = (func->param_count > 0 && strcmp(func->params[0]->name, ".hidden_ret_ptr") == 0);

                if (has_hidden_ptr)
                {
                    // Load hidden pointer
                    MIRValue *hidden_ptr = func->params[0];

                    // Copy from val (src addr) to hidden_ptr (dst addr)
                    // val is address because lower_expr returns address for large structs

                    Type  *ret_type = node->ret_stmt.expr->type;
                    size_t size     = ret_type ? ret_type->size : 16; // Default to 16 if unknown, but should be known

                    MIRValue *src_addr = mir_function_alloc_value(ctx->current_function, NULL, "src_addr");
                    MIRValue *dst_addr = mir_function_alloc_value(ctx->current_function, NULL, "dst_addr");
                    MIRValue *tmp      = mir_function_alloc_value(ctx->current_function, NULL, "copy_tmp");

                    MIRValue *val_copy        = mir_function_alloc_value(ctx->current_function, NULL, "val_copy");
                    MIRValue *hidden_ptr_copy = mir_function_alloc_value(ctx->current_function, NULL, "hidden_ptr_copy");

                    // Copy val and hidden_ptr to temps to preserve them across loop
                    MIRInst *mov1 = mir_inst_create(MIR_OP_MOV, NULL);
                    mir_inst_add_operand(mov1, mir_operand_value(val->id));
                    mir_inst_set_result(mov1, val_copy);
                    mir_block_append_inst(ctx->current_block, mov1);

                    MIRInst *mov2 = mir_inst_create(MIR_OP_MOV, NULL);
                    mir_inst_add_operand(mov2, mir_operand_value(hidden_ptr->id));
                    mir_inst_set_result(mov2, hidden_ptr_copy);
                    mir_block_append_inst(ctx->current_block, mov2);

                    for (size_t i = 0; i < size; i += 8)
                    {
                        // Calculate src address: val_copy + i
                        MIRValue *curr_src = val_copy;
                        if (i > 0)
                        {
                            MIRInst *add = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(val_copy->id), mir_operand_imm_int(i));
                            mir_inst_set_result(add, src_addr);
                            mir_block_append_inst(ctx->current_block, add);
                            curr_src = src_addr;
                        }

                        // Load from src
                        MIRInst *load = mir_inst_create(MIR_OP_LOAD, NULL);
                        mir_inst_add_operand(load, mir_operand_value(curr_src->id));
                        mir_inst_set_result(load, tmp);
                        mir_block_append_inst(ctx->current_block, load);

                        // Calculate dst address: hidden_ptr_copy + i
                        MIRValue *curr_dst = hidden_ptr_copy;
                        if (i > 0)
                        {
                            MIRInst *add = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(hidden_ptr_copy->id), mir_operand_imm_int(i));
                            mir_inst_set_result(add, dst_addr);
                            mir_block_append_inst(ctx->current_block, add);
                            curr_dst = dst_addr;
                        }

                        // Store to dst: [dst_addr] = tmp
                        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                        mir_inst_add_operand(store, mir_operand_value(tmp->id));
                        mir_inst_add_operand(store, mir_operand_value(curr_dst->id));
                        mir_block_append_inst(ctx->current_block, store);
                    }

                    // emit all deferred statements before return
                    lower_emit_deferred_all(ctx);

                    // Return the hidden pointer
                    MIRInst *ret = mir_inst_ret(NULL, mir_operand_value(hidden_ptr->id));
                    mir_block_append_inst(ctx->current_block, ret);
                }
                else
                {
                    // emit all deferred statements before return
                    lower_emit_deferred_all(ctx);

                    MIRInst *ret = mir_inst_ret(NULL, mir_operand_value(val->id));
                    mir_block_append_inst(ctx->current_block, ret);
                }
            }
        }
        else
        {
            // emit all deferred statements before return
            lower_emit_deferred_all(ctx);

            MIRInst *ret = mir_inst_ret_void();
            mir_block_append_inst(ctx->current_block, ret);
        }
        return 0;

    case AST_STMT_IF:
        return lower_if_stmt(ctx, node);

    case AST_STMT_FOR:
        return lower_for_stmt(ctx, node);

    case AST_STMT_EXPR:
        lower_expr(ctx, node->expr_stmt.expr);
        return 0;

        return 0;

    case AST_STMT_OR:
        return lower_if_stmt(ctx, node);

    case AST_STMT_BRK:
        if (ctx->loop_end_block)
        {
            // emit deferred statements for blocks inside the loop
            lower_emit_deferred_to_loop(ctx);

            mir_block_append_inst(ctx->current_block, mir_inst_br(ctx->loop_end_block->id));
            // Create a new unreachable block for subsequent instructions (dead code)
            MIRBlock *unreachable = mir_function_add_block(ctx->current_function, "unreachable");
            ctx->current_block    = unreachable;
        }
        return 0;

    case AST_STMT_CNT:
        if (ctx->loop_start_block)
        {
            // emit deferred statements for blocks inside the loop
            lower_emit_deferred_to_loop(ctx);

            mir_block_append_inst(ctx->current_block, mir_inst_br(ctx->loop_start_block->id));
            // Create a new unreachable block for subsequent instructions (dead code)
            MIRBlock *unreachable = mir_function_add_block(ctx->current_function, "unreachable");
            ctx->current_block    = unreachable;
        }
        return 0;

    case AST_STMT_MIR:
        // inline MIR block
        if (node->mir_stmt.content && ctx->current_function && ctx->current_block)
        {
            mir_parse_inline_block(ctx, ctx->current_function, ctx->current_block, node->mir_stmt.content);
        }
        return 0;

    default:
        // unimplemented statement
        return 0;
    }
}

// lower expression to MIR value
// lower unary expression
static MIRValue *lower_unary_expr(LowerContext *ctx, AstNode *node)
{
    if (!node || node->kind != AST_EXPR_UNARY)
    {
        return NULL;
    }

    if (node->unary_expr.op == TOKEN_QUESTION)
    {
        // Address-of: ?expr
        AstNode *operand = node->unary_expr.expr;

        // Handle local variable address
        if (operand->kind == AST_EXPR_IDENT && operand->symbol && operand->symbol->kind == SYMBOL_VARIABLE)
        {
            int32_t offset = lower_context_get_stack_offset(ctx, operand->symbol->decl);
            if (offset != 0)
            {
                // Local variable: emit MIR_OP_ADDR
                MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "addr");
                MIRInst  *inst   = mir_inst_create(MIR_OP_ADDR, NULL);
                mir_inst_add_operand(inst, mir_operand_imm_int(offset));
                mir_inst_set_result(inst, result);
                mir_block_append_inst(ctx->current_block, inst);
                return result;
            }
            else
            {
                // Global variable: emit MIR_OP_MOV with global operand (LEA)
                MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                MIRInst  *inst   = mir_inst_create(MIR_OP_MOV, NULL);
                mir_inst_add_operand(inst, mir_operand_global(operand->symbol->name));
                mir_inst_set_result(inst, result);
                mir_block_append_inst(ctx->current_block, inst);
                return result;
            }
        }
        else if (operand->kind == AST_EXPR_FIELD)
        {
            // Address of field: ?obj.field
            MIRValue *base_addr = NULL;
            AstNode  *obj       = operand->field_expr.object;

            if (obj->kind == AST_EXPR_IDENT && obj->symbol && obj->symbol->kind == SYMBOL_VARIABLE)
            {
                int32_t offset = lower_context_get_stack_offset(ctx, obj->symbol->decl);
                if (offset != 0)
                {
                    base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                    MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                    mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                    mir_inst_set_result(addr, base_addr);
                    mir_block_append_inst(ctx->current_block, addr);
                }
                else
                {
                    base_addr    = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                    MIRInst *mov = mir_inst_create(MIR_OP_MOV, NULL);
                    mir_inst_add_operand(mov, mir_operand_global(obj->symbol->name));
                    mir_inst_set_result(mov, base_addr);
                    mir_block_append_inst(ctx->current_block, mov);
                }
            }
            else if (obj->kind == AST_EXPR_UNARY && obj->unary_expr.op == TOKEN_AT)
            {
                base_addr = lower_expr(ctx, obj->unary_expr.expr);
            }

            if (base_addr)
            {
                Type *obj_type = obj->type;
                if (obj_type->kind == TYPE_POINTER)
                {
                    obj_type = obj_type->pointer.base;
                }

                int32_t field_offset = 0;
                if (obj_type->kind == TYPE_STRUCT)
                {
                    for (int i = 0; i < obj_type->structure.field_count; i++)
                    {
                        if (strcmp(obj_type->structure.fields[i].name, operand->field_expr.field) == 0)
                        {
                            field_offset = (int32_t)obj_type->structure.fields[i].offset;
                            break;
                        }
                    }
                }

                if (field_offset != 0)
                {
                    MIRValue *field_addr = mir_function_alloc_value(ctx->current_function, NULL, "field_addr");
                    MIRInst  *add        = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_imm_int(field_offset));
                    mir_inst_set_result(add, field_addr);
                    mir_block_append_inst(ctx->current_block, add);
                    return field_addr;
                }
                return base_addr;
            }
        }
        else if (operand->kind == AST_EXPR_INDEX)
        {
            // Address of array element: ?arr[index]
            MIRValue *base_addr = NULL;
            AstNode  *arr       = operand->index_expr.array;

            if (arr->kind == AST_EXPR_IDENT && arr->symbol && arr->symbol->kind == SYMBOL_VARIABLE)
            {
                int32_t offset = lower_context_get_stack_offset(ctx, arr->symbol->decl);
                if (offset != 0)
                {
                    base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                    MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                    mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                    mir_inst_set_result(addr, base_addr);
                    mir_block_append_inst(ctx->current_block, addr);
                }
                else
                {
                    base_addr    = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                    MIRInst *mov = mir_inst_create(MIR_OP_MOV, NULL);
                    mir_inst_add_operand(mov, mir_operand_global(arr->symbol->name));
                    mir_inst_set_result(mov, base_addr);
                    mir_block_append_inst(ctx->current_block, mov);
                }
            }
            else
            {
                base_addr = lower_expr(ctx, arr);
            }

            if (base_addr)
            {
                MIRValue *index_val = lower_expr(ctx, operand->index_expr.index);
                if (index_val)
                {
                    Type *obj_type  = arr->type;
                    Type *elem_type = NULL;
                    if (obj_type->kind == TYPE_ARRAY)
                    {
                        elem_type = obj_type->array.elem_type;
                    }
                    else if (obj_type->kind == TYPE_POINTER)
                    {
                        elem_type = obj_type->pointer.base;
                    }

                    size_t elem_size = elem_type ? elem_type->size : 8;

                    MIRValue *offset_val = mir_function_alloc_value(ctx->current_function, NULL, "index_offset");
                    MIRInst  *mul        = mir_inst_binary(MIR_OP_MUL, NULL, mir_operand_value(index_val->id), mir_operand_imm_int(elem_size));
                    mir_inst_set_result(mul, offset_val);
                    mir_block_append_inst(ctx->current_block, mul);

                    MIRValue *elem_addr = mir_function_alloc_value(ctx->current_function, NULL, "elem_addr");
                    MIRInst  *add       = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_value(offset_val->id));
                    mir_inst_set_result(add, elem_addr);
                    mir_block_append_inst(ctx->current_block, add);

                    return elem_addr;
                }
            }
        }
        return NULL; // Unsupported operand for address-of
    }
    else if (node->unary_expr.op == TOKEN_AT)
    {
        // Dereference: @expr
        MIRValue *ptr = lower_expr(ctx, node->unary_expr.expr);
        if (!ptr)
        {
            return NULL;
        }

        // Emit MIR_OP_LOAD with pointer value
        MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "deref");
        MIRInst  *inst   = mir_inst_create(MIR_OP_LOAD, NULL);
        mir_inst_add_operand(inst, mir_operand_value(ptr->id));
        mir_inst_set_result(inst, result);
        mir_block_append_inst(ctx->current_block, inst);
        return result;
    }

    return NULL;
}

// compute the address of an expression without loading its value
// used for nested field access (p.x.y) and address-of operations
static MIRValue *lower_address(LowerContext *ctx, AstNode *node)
{
    if (!ctx || !node)
    {
        return NULL;
    }

    switch (node->kind)
    {
    case AST_EXPR_IDENT:
    {
        if (!node->symbol || node->symbol->kind != SYMBOL_VARIABLE)
        {
            return NULL;
        }

        int32_t offset = lower_context_get_stack_offset(ctx, node->symbol->decl);
        if (offset != 0)
        {
            // local variable: compute stack address
            MIRValue *addr      = mir_function_alloc_value(ctx->current_function, NULL, "var_addr");
            MIRInst  *addr_inst = mir_inst_create(MIR_OP_ADDR, NULL);
            mir_inst_add_operand(addr_inst, mir_operand_imm_int(offset));
            mir_inst_set_result(addr_inst, addr);
            mir_block_append_inst(ctx->current_block, addr_inst);
            return addr;
        }
        else
        {
            // global variable
            MIRValue   *addr     = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
            MIRInst    *mov      = mir_inst_create(MIR_OP_MOV, NULL);
            const char *sym_name = symbol_get_linkage_name(node->symbol);
            mir_inst_add_operand(mov, mir_operand_global(sym_name));
            mir_inst_set_result(mov, addr);
            mir_block_append_inst(ctx->current_block, mov);
            return addr;
        }
    }

    case AST_EXPR_UNARY:
    {
        if (node->unary_expr.op == TOKEN_AT)
        {
            // dereference: address of @ptr is ptr itself
            return lower_expr(ctx, node->unary_expr.expr);
        }
        return NULL;
    }

    case AST_EXPR_FIELD:
    {
        // field access: obj.field
        AstNode *obj      = node->field_expr.object;
        Type    *obj_type = obj->type;

        // check if object is a pointer
        bool obj_is_pointer = (obj_type && obj_type->kind == TYPE_POINTER);

        MIRValue *base_addr = NULL;

        if (obj_is_pointer)
        {
            // pointer: load the pointer value
            base_addr = lower_expr(ctx, obj);
        }
        else
        {
            // value: get the address of the object (recursive for nested access)
            base_addr = lower_address(ctx, obj);
        }

        if (!base_addr)
        {
            return NULL;
        }

        // find field offset
        Type *struct_type = obj_type;
        if (struct_type->kind == TYPE_POINTER)
        {
            struct_type = struct_type->pointer.base;
        }

        int32_t field_offset = 0;
        if (struct_type->kind == TYPE_STRUCT)
        {
            for (int i = 0; i < struct_type->structure.field_count; i++)
            {
                if (strcmp(struct_type->structure.fields[i].name, node->field_expr.field) == 0)
                {
                    field_offset = (int32_t)struct_type->structure.fields[i].offset;
                    break;
                }
            }
        }
        else if (struct_type->kind == TYPE_UNION)
        {
            // unions have all fields at offset 0
            field_offset = 0;
        }

        // add offset to base
        if (field_offset != 0)
        {
            MIRValue *field_addr = mir_function_alloc_value(ctx->current_function, NULL, "field_addr");
            MIRInst  *add        = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_imm_int(field_offset));
            mir_inst_set_result(add, field_addr);
            mir_block_append_inst(ctx->current_block, add);
            return field_addr;
        }

        return base_addr;
    }

    case AST_EXPR_INDEX:
    {
        // array indexing: arr[index]
        AstNode *arr = node->index_expr.array;

        MIRValue *base_addr = NULL;

        if (arr->type && arr->type->kind == TYPE_POINTER)
        {
            // pointer: load the pointer value
            base_addr = lower_expr(ctx, arr);
        }
        else
        {
            // array: get address
            base_addr = lower_address(ctx, arr);
        }

        if (!base_addr)
        {
            return NULL;
        }

        MIRValue *index_val = lower_expr(ctx, node->index_expr.index);
        if (!index_val)
        {
            return NULL;
        }

        // get element size
        Type  *elem_type = NULL;
        size_t elem_size = 8;
        if (arr->type)
        {
            if (arr->type->kind == TYPE_ARRAY)
            {
                elem_type = arr->type->array.elem_type;
            }
            else if (arr->type->kind == TYPE_POINTER)
            {
                elem_type = arr->type->pointer.base;
            }
            if (elem_type)
            {
                elem_size = elem_type->size;
            }
        }

        // calculate offset: index * elem_size
        MIRValue *offset_val = mir_function_alloc_value(ctx->current_function, NULL, "index_offset");
        MIRInst  *mul        = mir_inst_binary(MIR_OP_MUL, NULL, mir_operand_value(index_val->id), mir_operand_imm_int(elem_size));
        mir_inst_set_result(mul, offset_val);
        mir_block_append_inst(ctx->current_block, mul);

        // add offset to base
        MIRValue *elem_addr = mir_function_alloc_value(ctx->current_function, NULL, "elem_addr");
        MIRInst  *add       = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_value(offset_val->id));
        mir_inst_set_result(add, elem_addr);
        mir_block_append_inst(ctx->current_block, add);

        return elem_addr;
    }

    default:
        return NULL;
    }
}

static MIRValue *lower_expr(LowerContext *ctx, AstNode *node)
{
    if (!ctx || !node)
    {
        return NULL;
    }

    // check if we already lowered this node
    MIRValue *val = lower_context_get_value(ctx, node);
    if (val)
    {
        return val;
    }

    switch (node->kind)
    {
    case AST_EXPR_UNARY:
        return lower_unary_expr(ctx, node);

    case AST_EXPR_LIT:
        if (node->token && node->token->kind == TOKEN_LIT_INT)
        {
            // allocate result value
            MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "lit");

            // create const instruction
            MIRInst *inst = mir_inst_const(NULL, node->lit_expr.int_val);
            mir_inst_set_result(inst, result);
            mir_block_append_inst(ctx->current_block, inst);

            lower_context_map_value(ctx, node, result);
            return result;
        }
        else if (node->token && node->token->kind == TOKEN_LIT_FLOAT)
        {
            // Create a synthetic global for the float constant
            char name[64];
            snprintf(name, sizeof(name), "__float_const_%d", ctx->float_const_count++);

            MIRGlobal *global = mir_global_create(name, type_get_primitive(TYPE_F64), MIR_GLOBAL_VAL, false);
            mir_global_set_float_init(global, node->lit_expr.float_val);
            mir_module_add_global(ctx->module, global);

            // Load address of global
            MIRValue *addr = mir_function_alloc_value(ctx->current_function, NULL, "float_addr");
            MIRInst  *mov  = mir_inst_create(MIR_OP_MOV, NULL);
            mir_inst_add_operand(mov, mir_operand_global(global->name));
            mir_inst_set_result(mov, addr);
            mir_block_append_inst(ctx->current_block, mov);

            // Load value from address
            MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "lit_f");
            MIRInst  *load   = mir_inst_create(MIR_OP_LOAD, type_get_primitive(TYPE_F64));
            mir_inst_add_operand(load, mir_operand_value(addr->id));
            mir_inst_set_result(load, result);
            mir_block_append_inst(ctx->current_block, load);

            lower_context_map_value(ctx, node, result);
            return result;
        }
        else if (node->token && node->token->kind == TOKEN_LIT_STRING)
        {
            // Create a synthetic global for the string constant
            char name[64];
            snprintf(name, sizeof(name), "__str_const_%d", ctx->string_const_count++);

            MIRGlobal *global = mir_global_create(name, type_get_primitive(TYPE_PTR), MIR_GLOBAL_VAL, false);
            mir_global_set_string_init(global, node->lit_expr.string_val);
            mir_module_add_global(ctx->module, global);

            // Load address of global as &u8
            MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "str");
            MIRInst  *mov    = mir_inst_create(MIR_OP_MOV, NULL);
            mir_inst_add_operand(mov, mir_operand_global(global->name));
            mir_inst_set_result(mov, result);
            mir_block_append_inst(ctx->current_block, mov);

            lower_context_map_value(ctx, node, result);
            return result;
        }
        return NULL;

    case AST_COMPTIME:
    {
        // Lower compile-time constant (already evaluated in sema)
        if (node->comptime.value_kind == COMPTIME_INT)
        {
            // Integer constant
            MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "comptime_int");
            MIRInst  *mov    = mir_inst_create(MIR_OP_MOV, NULL);
            mir_inst_add_operand(mov, mir_operand_imm_int(node->comptime.int_value));
            mir_inst_set_result(mov, result);
            mir_block_append_inst(ctx->current_block, mov);

            lower_context_map_value(ctx, node, result);
            return result;
        }
        else if (node->comptime.value_kind == COMPTIME_STRING)
        {
            // String constant - create global like string literals
            char name[64];
            snprintf(name, sizeof(name), "__comptime_str_%d", ctx->string_const_count++);

            MIRGlobal *global = mir_global_create(name, type_get_primitive(TYPE_PTR), MIR_GLOBAL_VAL, false);
            mir_global_set_string_init(global, node->comptime.string_value);
            mir_module_add_global(ctx->module, global);

            // Load address as &u8
            MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "comptime_str");
            MIRInst  *mov    = mir_inst_create(MIR_OP_MOV, NULL);
            mir_inst_add_operand(mov, mir_operand_global(global->name));
            mir_inst_set_result(mov, result);
            mir_block_append_inst(ctx->current_block, mov);

            lower_context_map_value(ctx, node, result);
            return result;
        }
        return NULL;
    }

    case AST_EXPR_CAST:
    {
        // Lower expression
        MIRValue *val = lower_expr(ctx, node->cast_expr.expr);
        if (!val)
        {
            return NULL;
        }

        // Cast is raw bit reinterpretation (bitcast), not type conversion
        // Emit a MOV to ensure type safety in MIR
        MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "cast");
        MIRInst  *inst   = mir_inst_create(MIR_OP_MOV, NULL);
        mir_inst_add_operand(inst, mir_operand_value(val->id));
        mir_inst_set_result(inst, result);
        mir_block_append_inst(ctx->current_block, inst);

        lower_context_map_value(ctx, node, result);
        return result;
    }

    case AST_EXPR_IDENT:
        // look up value from symbol
        if (node->symbol)
        {
            // check if it's a variable symbol
            if (node->symbol->kind == SYMBOL_VARIABLE)
            {
                // Check if it's a local variable on the stack
                int32_t offset = lower_context_get_stack_offset(ctx, node->symbol->decl);
                if (offset != 0)
                {
                    // It's a local variable on stack
                    // If it's a large struct (> 8 bytes), return the address (LEA)
                    // Otherwise, load the value (LOAD)
                    if (node->type && node->type->size > 8)
                    {
                        MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "local_addr");
                        MIRInst  *addr   = mir_inst_create(MIR_OP_ADDR, NULL);
                        mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                        mir_inst_set_result(addr, result);
                        mir_block_append_inst(ctx->current_block, addr);
                        return result;
                    }
                    else
                    {
                        MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "local_load");
                        MIRInst  *load   = mir_inst_create(MIR_OP_LOAD, NULL);
                        mir_inst_add_operand(load, mir_operand_imm_int(offset));
                        mir_inst_set_result(load, result);
                        mir_block_append_inst(ctx->current_block, load);
                        return result;
                    }
                }

                // check if we've already lowered this as a value (e.g. function param)
                MIRValue *local_val = lower_context_get_value(ctx, node->symbol->decl);
                if (!local_val)
                {
                    // not a local, must be a global
                    // Get address of global
                    MIRValue   *addr     = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                    MIRInst    *mov      = mir_inst_create(MIR_OP_MOV, NULL);
                    const char *sym_name = symbol_get_linkage_name(node->symbol);
                    mir_inst_add_operand(mov, mir_operand_global(sym_name));
                    mir_inst_set_result(mov, addr);
                    mir_block_append_inst(ctx->current_block, mov);

                    // For globals, return the address directly (no automatic dereferencing)
                    // The user must explicitly dereference if they want the value
                    lower_context_map_value(ctx, node, addr);
                    return addr;
                }
                else
                {
                    return local_val;
                }
            }

            // for functions or other symbols, check if we've already lowered this
            MIRValue *val = lower_context_get_value(ctx, node->symbol->decl);
            if (val)
            {
                return val;
            }
        }
        return NULL;

    case AST_EXPR_FIELD:
    {
        // Field access: obj.field
        // We need the address of obj to add the offset

        MIRValue *base_addr = NULL;
        AstNode  *obj       = node->field_expr.object;
        Type     *obj_type  = obj->type;

        // Check if object is a pointer - if so, we need to load it first
        bool obj_is_pointer = (obj_type && obj_type->kind == TYPE_POINTER);

        if (obj->kind == AST_EXPR_IDENT && obj->symbol && obj->symbol->kind == SYMBOL_VARIABLE)
        {
            int32_t offset = lower_context_get_stack_offset(ctx, obj->symbol->decl);
            if (offset != 0)
            {
                if (obj_is_pointer)
                {
                    // Pointer variable: load the pointer value from the stack
                    base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "ptr_val");
                    MIRInst *load = mir_inst_create(MIR_OP_LOAD, NULL);
                    mir_inst_add_operand(load, mir_operand_imm_int(offset));
                    mir_inst_add_operand(load, mir_operand_imm_int(8)); // pointer size
                    mir_inst_set_result(load, base_addr);
                    mir_block_append_inst(ctx->current_block, load);
                }
                else
                {
                    // Value variable: get stack address
                    base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                    MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                    mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                    mir_inst_set_result(addr, base_addr);
                    mir_block_append_inst(ctx->current_block, addr);
                }
            }
            else
            {
                // Global variable
                base_addr    = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                MIRInst *mov = mir_inst_create(MIR_OP_MOV, NULL);
                mir_inst_add_operand(mov, mir_operand_global(obj->symbol->name));
                mir_inst_set_result(mov, base_addr);
                mir_block_append_inst(ctx->current_block, mov);
            }
        }
        else if (obj->kind == AST_EXPR_UNARY && obj->unary_expr.op == TOKEN_AT)
        {
            // Dereference: @ptr.field -> ptr is the base address
            base_addr = lower_expr(ctx, obj->unary_expr.expr);
        }
        else
        {
            // nested field access or other complex expression
            // use lower_address to get the address without loading
            if (obj_is_pointer)
            {
                // pointer expression: load the pointer value
                base_addr = lower_expr(ctx, obj);
            }
            else
            {
                // value expression: get its address
                base_addr = lower_address(ctx, obj);
            }
        }

        if (!base_addr)
        {
            return NULL;
        }

        // Find field offset - unwrap pointer if needed
        Type *struct_type = obj_type;
        if (struct_type->kind == TYPE_POINTER)
        {
            struct_type = struct_type->pointer.base;
        }

        int32_t field_offset = 0;
        if (struct_type->kind == TYPE_STRUCT)
        {
            for (int i = 0; i < struct_type->structure.field_count; i++)
            {
                if (strcmp(struct_type->structure.fields[i].name, node->field_expr.field) == 0)
                {
                    field_offset = (int32_t)struct_type->structure.fields[i].offset;
                    break;
                }
            }
        }

        // Add offset
        MIRValue *field_addr = base_addr;
        if (field_offset != 0)
        {
            field_addr   = mir_function_alloc_value(ctx->current_function, NULL, "field_addr");
            MIRInst *add = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_imm_int(field_offset));
            mir_inst_set_result(add, field_addr);
            mir_block_append_inst(ctx->current_block, add);
        }

        // Load value or return address
        if (node->type && node->type->size > 8)
        {
            // Return address for large structs
            return field_addr;
        }
        else
        {
            MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "field_val");
            MIRInst  *load   = mir_inst_create(MIR_OP_LOAD, NULL);
            mir_inst_add_operand(load, mir_operand_value(field_addr->id));
            mir_inst_set_result(load, result);
            mir_block_append_inst(ctx->current_block, load);

            return result;
        }
    }

    case AST_EXPR_INDEX:
    {
        // Array indexing: arr[index]
        // Address = base_addr + index * elem_size

        MIRValue *base_addr = NULL;
        AstNode  *arr       = node->index_expr.array;

        if (arr->kind == AST_EXPR_IDENT && arr->symbol && arr->symbol->kind == SYMBOL_VARIABLE && arr->type && arr->type->kind == TYPE_ARRAY)
        {
            // Local array: get stack address
            int32_t offset = lower_context_get_stack_offset(ctx, arr->symbol->decl);
            if (offset != 0)
            {
                base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                mir_inst_set_result(addr, base_addr);
                mir_block_append_inst(ctx->current_block, addr);
            }
            else
            {
                // Global variable
                base_addr    = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                MIRInst *mov = mir_inst_create(MIR_OP_MOV, NULL);
                mir_inst_add_operand(mov, mir_operand_global(arr->symbol->name));
                mir_inst_set_result(mov, base_addr);
                mir_block_append_inst(ctx->current_block, mov);
            }
        }
        else
        {
            // Pointer or complex expression: evaluate to get address
            base_addr = lower_expr(ctx, arr);
        }

        if (!base_addr)
        {
            return NULL;
        }

        MIRValue *index_val = lower_expr(ctx, node->index_expr.index);
        if (!index_val)
        {
            return NULL;
        }

        // Get element size
        Type *obj_type  = arr->type;
        Type *elem_type = NULL;
        if (obj_type->kind == TYPE_ARRAY)
        {
            elem_type = obj_type->array.elem_type;
        }
        else if (obj_type->kind == TYPE_POINTER)
        {
            elem_type = obj_type->pointer.base;
        }

        size_t elem_size = elem_type ? elem_type->size : 8;

        // Calculate offset: index * elem_size
        MIRValue *offset_val = mir_function_alloc_value(ctx->current_function, NULL, "index_offset");
        MIRInst  *mul        = mir_inst_binary(MIR_OP_MUL, NULL, mir_operand_value(index_val->id), mir_operand_imm_int(elem_size));
        mir_inst_set_result(mul, offset_val);
        mir_block_append_inst(ctx->current_block, mul);

        // Add offset to base
        MIRValue *elem_addr = mir_function_alloc_value(ctx->current_function, NULL, "elem_addr");
        MIRInst  *add       = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_value(offset_val->id));
        mir_inst_set_result(add, elem_addr);
        mir_block_append_inst(ctx->current_block, add);

        // Load value
        MIRValue *result = mir_function_alloc_value(ctx->current_function, elem_type, "elem_val");
        MIRInst  *load   = mir_inst_create(MIR_OP_LOAD, NULL);
        mir_inst_add_operand(load, mir_operand_value(elem_addr->id));
        mir_inst_set_result(load, result);
        mir_block_append_inst(ctx->current_block, load);

        return result;
    }

    case AST_EXPR_BINARY:
    {
        // Handle assignment separately
        if (node->binary_expr.op == TOKEN_EQUAL)
        {
            AstNode *lhs = node->binary_expr.left;

            if (lhs->kind == AST_EXPR_IDENT)
            {
                // Variable assignment: var = val
                MIRValue *rhs_val = lower_expr(ctx, node->binary_expr.right);
                if (!rhs_val)
                {
                    return NULL;
                }

                if (lhs->symbol && lhs->symbol->kind == SYMBOL_VARIABLE)
                {
                    int32_t offset = lower_context_get_stack_offset(ctx, lhs->symbol->decl);
                    if (offset != 0)
                    {
                        // Local variable: store to stack
                        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                        mir_inst_add_operand(store, mir_operand_value(rhs_val->id));
                        mir_inst_add_operand(store, mir_operand_imm_int(offset));
                        mir_block_append_inst(ctx->current_block, store);
                    }
                    else
                    {
                        // Global variable: store to global
                        // Load global address
                        MIRValue   *addr     = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                        MIRInst    *mov      = mir_inst_create(MIR_OP_MOV, NULL);
                        const char *sym_name = symbol_get_linkage_name(lhs->symbol);
                        mir_inst_add_operand(mov, mir_operand_global(sym_name));
                        mir_inst_set_result(mov, addr);
                        mir_block_append_inst(ctx->current_block, mov);

                        // Store value to global (operands: value, address)
                        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                        mir_inst_add_operand(store, mir_operand_value(rhs_val->id));
                        mir_inst_add_operand(store, mir_operand_value(addr->id));
                        mir_block_append_inst(ctx->current_block, store);
                    }
                }
                return rhs_val;
            }
            else if (lhs->kind == AST_EXPR_INDEX)
            {
                // Array element assignment: arr[i] = val
                MIRValue *rhs_val = lower_expr(ctx, node->binary_expr.right);
                if (!rhs_val)
                {
                    return NULL;
                }

                AstNode *base  = lhs->index_expr.array;
                AstNode *index = lhs->index_expr.index;

                // Lower base address
                MIRValue *base_addr = NULL;
                if (base->kind == AST_EXPR_IDENT && base->symbol && base->symbol->kind == SYMBOL_VARIABLE)
                {
                    int32_t offset = lower_context_get_stack_offset(ctx, base->symbol->decl);
                    if (offset != 0)
                    {
                        base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                        MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                        mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                        mir_inst_set_result(addr, base_addr);
                        mir_block_append_inst(ctx->current_block, addr);
                    }
                }

                if (!base_addr)
                {
                    return NULL; // Only local arrays supported for now
                }

                // Lower index
                MIRValue *index_val = lower_expr(ctx, index);
                if (!index_val)
                {
                    return NULL;
                }

                // Element size
                size_t elem_size = base->type->array.elem_type->size;

                // Calculate offset: index * elem_size
                MIRValue *offset_val = mir_function_alloc_value(ctx->current_function, NULL, "index_offset");
                MIRInst  *mul        = mir_inst_binary(MIR_OP_MUL, NULL, mir_operand_value(index_val->id), mir_operand_imm_int(elem_size));
                mir_inst_set_result(mul, offset_val);
                mir_block_append_inst(ctx->current_block, mul);

                // Add offset to base
                MIRValue *elem_addr = mir_function_alloc_value(ctx->current_function, NULL, "elem_addr");
                MIRInst  *add       = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_value(offset_val->id));
                mir_inst_set_result(add, elem_addr);
                mir_block_append_inst(ctx->current_block, add);

                // Store value
                MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                mir_inst_add_operand(store, mir_operand_value(rhs_val->id));
                mir_inst_add_operand(store, mir_operand_value(elem_addr->id)); // Store to address in register
                mir_block_append_inst(ctx->current_block, store);

                return rhs_val;
            }
            else if (lhs->kind == AST_EXPR_UNARY && lhs->unary_expr.op == TOKEN_AT)
            {
                // Pointer assignment: @ptr = val
                // Evaluate ptr first
                MIRValue *ptr_val = lower_expr(ctx, lhs->unary_expr.expr);
                if (!ptr_val)
                {
                    return NULL;
                }

                MIRValue *rhs_val = lower_expr(ctx, node->binary_expr.right);
                if (!rhs_val)
                {
                    return NULL;
                }

                MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                mir_inst_add_operand(store, mir_operand_value(rhs_val->id));
                mir_inst_add_operand(store, mir_operand_value(ptr_val->id));
                mir_block_append_inst(ctx->current_block, store);

                return rhs_val;
            }
            else if (lhs->kind == AST_EXPR_FIELD)
            {
                // Field assignment: obj.field = val
                MIRValue *base_addr      = NULL;
                AstNode  *obj            = lhs->field_expr.object;
                Type     *obj_type       = obj->type;
                bool      obj_is_pointer = (obj_type && obj_type->kind == TYPE_POINTER);

                if (obj->kind == AST_EXPR_IDENT && obj->symbol && obj->symbol->kind == SYMBOL_VARIABLE)
                {
                    int32_t offset = lower_context_get_stack_offset(ctx, obj->symbol->decl);
                    if (offset != 0)
                    {
                        if (obj_is_pointer)
                        {
                            // Pointer variable: load the pointer value from the stack
                            base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "ptr_val");
                            MIRInst *load = mir_inst_create(MIR_OP_LOAD, NULL);
                            mir_inst_add_operand(load, mir_operand_imm_int(offset));
                            mir_inst_add_operand(load, mir_operand_imm_int(8)); // pointer size
                            mir_inst_set_result(load, base_addr);
                            mir_block_append_inst(ctx->current_block, load);
                        }
                        else
                        {
                            // Value variable: get stack address
                            base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                            MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                            mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                            mir_inst_set_result(addr, base_addr);
                            mir_block_append_inst(ctx->current_block, addr);
                        }
                    }
                    else
                    {
                        base_addr    = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                        MIRInst *mov = mir_inst_create(MIR_OP_MOV, NULL);
                        mir_inst_add_operand(mov, mir_operand_global(obj->symbol->name));
                        mir_inst_set_result(mov, base_addr);
                        mir_block_append_inst(ctx->current_block, mov);
                    }
                }
                else if (obj->kind == AST_EXPR_UNARY && obj->unary_expr.op == TOKEN_AT)
                {
                    base_addr = lower_expr(ctx, obj->unary_expr.expr);
                }

                if (base_addr)
                {
                    Type *struct_type = obj_type;
                    if (struct_type && struct_type->kind == TYPE_POINTER)
                    {
                        struct_type = struct_type->pointer.base;
                    }

                    int32_t field_offset = 0;
                    if (struct_type && struct_type->kind == TYPE_STRUCT)
                    {
                        for (int i = 0; i < struct_type->structure.field_count; i++)
                        {
                            if (strcmp(struct_type->structure.fields[i].name, lhs->field_expr.field) == 0)
                            {
                                field_offset = (int32_t)struct_type->structure.fields[i].offset;
                                break;
                            }
                        }
                    }

                    MIRValue *field_addr = base_addr;
                    if (field_offset != 0)
                    {
                        field_addr   = mir_function_alloc_value(ctx->current_function, NULL, "field_addr");
                        MIRInst *add = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_imm_int(field_offset));
                        mir_inst_set_result(add, field_addr);
                        mir_block_append_inst(ctx->current_block, add);
                    }

                    MIRValue *rhs_val = lower_expr(ctx, node->binary_expr.right);
                    if (!rhs_val)
                    {
                        return NULL;
                    }

                    MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                    mir_inst_add_operand(store, mir_operand_value(rhs_val->id));
                    mir_inst_add_operand(store, mir_operand_value(field_addr->id));
                    mir_block_append_inst(ctx->current_block, store);

                    return rhs_val;
                }
            }
            else if (lhs->kind == AST_EXPR_INDEX)
            {
                // Array assignment: arr[index] = val
                MIRValue *base_addr = NULL;
                AstNode  *arr       = lhs->index_expr.array;

                if (arr->kind == AST_EXPR_IDENT && arr->symbol && arr->symbol->kind == SYMBOL_VARIABLE)
                {
                    int32_t offset = lower_context_get_stack_offset(ctx, arr->symbol->decl);
                    if (offset != 0)
                    {
                        base_addr     = mir_function_alloc_value(ctx->current_function, NULL, "base_addr");
                        MIRInst *addr = mir_inst_create(MIR_OP_ADDR, NULL);
                        mir_inst_add_operand(addr, mir_operand_imm_int(offset));
                        mir_inst_set_result(addr, base_addr);
                        mir_block_append_inst(ctx->current_block, addr);
                    }
                    else
                    {
                        base_addr            = mir_function_alloc_value(ctx->current_function, NULL, "global_addr");
                        MIRInst    *mov      = mir_inst_create(MIR_OP_MOV, NULL);
                        const char *sym_name = symbol_get_linkage_name(arr->symbol);
                        mir_inst_add_operand(mov, mir_operand_global(sym_name));
                        mir_inst_set_result(mov, base_addr);
                        mir_block_append_inst(ctx->current_block, mov);
                    }
                }
                else
                {
                    base_addr = lower_expr(ctx, arr);
                }

                if (base_addr)
                {
                    MIRValue *index_val = lower_expr(ctx, lhs->index_expr.index);
                    if (index_val)
                    {
                        Type *obj_type  = arr->type;
                        Type *elem_type = NULL;
                        if (obj_type->kind == TYPE_ARRAY)
                        {
                            elem_type = obj_type->array.elem_type;
                        }
                        else if (obj_type->kind == TYPE_POINTER)
                        {
                            elem_type = obj_type->pointer.base;
                        }

                        size_t elem_size = elem_type ? elem_type->size : 8;

                        MIRValue *offset_val = mir_function_alloc_value(ctx->current_function, NULL, "index_offset");
                        MIRInst  *mul        = mir_inst_binary(MIR_OP_MUL, NULL, mir_operand_value(index_val->id), mir_operand_imm_int(elem_size));
                        mir_inst_set_result(mul, offset_val);
                        mir_block_append_inst(ctx->current_block, mul);

                        MIRValue *elem_addr = mir_function_alloc_value(ctx->current_function, NULL, "elem_addr");
                        MIRInst  *add       = mir_inst_binary(MIR_OP_ADD, NULL, mir_operand_value(base_addr->id), mir_operand_value(offset_val->id));
                        mir_inst_set_result(add, elem_addr);
                        mir_block_append_inst(ctx->current_block, add);

                        MIRValue *rhs_val = lower_expr(ctx, node->binary_expr.right);
                        if (!rhs_val)
                        {
                            return NULL;
                        }

                        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                        mir_inst_add_operand(store, mir_operand_value(rhs_val->id));
                        mir_inst_add_operand(store, mir_operand_value(elem_addr->id));
                        mir_block_append_inst(ctx->current_block, store);

                        return rhs_val;
                    }
                }
            }

            return NULL;
        }

        MIRValue *left  = lower_expr(ctx, node->binary_expr.left);
        MIRValue *right = lower_expr(ctx, node->binary_expr.right);

        if (!left || !right)
        {
            return NULL;
        }

        // allocate result
        MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "binop");

        // determine opcode from operator
        MIROp op = MIR_OP_ADD; // default
        switch (node->binary_expr.op)
        {
        case TOKEN_PLUS:
            op = MIR_OP_ADD;
            break;
        case TOKEN_MINUS:
            op = MIR_OP_SUB;
            break;
        case TOKEN_STAR:
            op = MIR_OP_MUL;
            break;
        case TOKEN_SLASH:
            op = MIR_OP_DIV;
            break;
        case TOKEN_PERCENT:
            op = MIR_OP_MOD;
            break;
        case TOKEN_EQUAL_EQUAL:
            op = MIR_OP_EQ;
            break;
        case TOKEN_BANG_EQUAL:
            op = MIR_OP_NE;
            break;
        case TOKEN_LESS:
            op = MIR_OP_LT;
            break;
        case TOKEN_GREATER:
            op = MIR_OP_GT;
            break;
        case TOKEN_LESS_EQUAL:
            op = MIR_OP_LE;
            break;
        case TOKEN_GREATER_EQUAL:
            op = MIR_OP_GE;
            break;
        case TOKEN_AMPERSAND:
            op = MIR_OP_AND;
            break;
        case TOKEN_PIPE:
            op = MIR_OP_OR;
            break;
        case TOKEN_CARET:
            op = MIR_OP_XOR;
            break;
        case TOKEN_LESS_LESS:
            op = MIR_OP_SHL;
            break;
        case TOKEN_GREATER_GREATER:
            op = MIR_OP_SHR;
            break;
        default:
            break;
        }

        // create instruction
        MIRInst *inst = mir_inst_binary(op, NULL, mir_operand_value(left->id), mir_operand_value(right->id));
        mir_inst_set_result(inst, result);
        mir_block_append_inst(ctx->current_block, inst);

        lower_context_map_value(ctx, node, result);
        return result;
    }

    case AST_EXPR_CALL:
    {
        // get function name
        const char *func_name = NULL;
        if (node->call_expr.func->kind == AST_EXPR_IDENT)
        {
            func_name = node->call_expr.func->ident_expr.name;
            if (node->call_expr.func->symbol)
            {
                func_name = symbol_get_linkage_name(node->call_expr.func->symbol);
            }
        }
        else if (node->call_expr.func->kind == AST_EXPR_FIELD && node->call_expr.func->field_expr.is_method)
        {
            // method call - get function name from the field's symbol
            if (node->call_expr.func->symbol)
            {
                func_name = symbol_get_linkage_name(node->call_expr.func->symbol);
            }
            else
            {
                func_name = node->call_expr.func->field_expr.field;
            }
        }

        if (!func_name)
        {
            return NULL;
        }

        // Check return type size
        Type *ret_type       = node->type;
        bool  has_hidden_ptr = (ret_type && ret_type->kind == TYPE_STRUCT && ret_type->size > 16);

        MIRValue *hidden_ptr_slot   = NULL;
        int32_t   hidden_ptr_offset = 0;

        if (has_hidden_ptr)
        {
            // Allocate stack slot for return value
            // Pass node to properly track this allocation and prevent stack slot overlaps
            hidden_ptr_offset = lower_context_alloc_stack_slot(ctx, node, ret_type->size);

            // Get address of slot
            hidden_ptr_slot = mir_function_alloc_value(ctx->current_function, NULL, "hidden_ret_addr");
            MIRInst *addr   = mir_inst_create(MIR_OP_ADDR, NULL);
            mir_inst_add_operand(addr, mir_operand_imm_int(hidden_ptr_offset));
            mir_inst_set_result(addr, hidden_ptr_slot);
            mir_block_append_inst(ctx->current_block, addr);
        }

        // lower arguments
        // To handle nested calls correctly, we need to spill arguments to stack
        // before evaluating any argument that contains a call. This prevents
        // register corruption when nested calls use the same registers.
        MIROperand *args      = NULL;
        int         arg_count = 0;
        if (node->call_expr.args)
        {
            arg_count = node->call_expr.args->count;
            if (has_hidden_ptr)
            {
                arg_count++; // Add hidden pointer arg
            }

            args = malloc(arg_count * sizeof(MIROperand));

            int arg_idx = 0;
            if (has_hidden_ptr)
            {
                args[arg_idx++] = mir_operand_value(hidden_ptr_slot->id);
            }

            // Check if any argument contains a call
            bool has_nested_call = false;
            for (int i = 0; i < node->call_expr.args->count; i++)
            {
                if (expr_contains_call(node->call_expr.args->items[i]))
                {
                    has_nested_call = true;
                    break;
                }
            }

            if (has_nested_call)
            {
                // Spill strategy: evaluate each argument and immediately spill to stack
                // then reload all arguments from stack before the call
                int32_t   *spill_offsets = malloc(node->call_expr.args->count * sizeof(int32_t));
                MIRValue **arg_values    = malloc(node->call_expr.args->count * sizeof(MIRValue *));

                for (int i = 0; i < node->call_expr.args->count; i++)
                {
                    // Evaluate argument
                    MIRValue *arg = lower_expr(ctx, node->call_expr.args->items[i]);
                    arg_values[i] = arg;

                    if (arg)
                    {
                        // Allocate stack slot for this argument
                        spill_offsets[i] = lower_context_alloc_stack_slot(ctx, NULL, 8);

                        // Spill to stack
                        MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                        mir_inst_add_operand(store, mir_operand_value(arg->id));
                        mir_inst_add_operand(store, mir_operand_imm_int(spill_offsets[i]));
                        mir_block_append_inst(ctx->current_block, store);
                    }
                    else
                    {
                        spill_offsets[i] = 0;
                    }
                }

                // Reload all arguments from stack
                for (int i = 0; i < node->call_expr.args->count; i++)
                {
                    if (arg_values[i] && spill_offsets[i] != 0)
                    {
                        MIRValue *reloaded = mir_function_alloc_value(ctx->current_function, NULL, "arg_reload");
                        MIRInst  *load     = mir_inst_create(MIR_OP_LOAD, NULL);
                        mir_inst_add_operand(load, mir_operand_imm_int(spill_offsets[i]));
                        mir_inst_set_result(load, reloaded);
                        mir_block_append_inst(ctx->current_block, load);

                        args[arg_idx++] = mir_operand_value(reloaded->id);
                    }
                    else if (arg_values[i])
                    {
                        args[arg_idx++] = mir_operand_value(arg_values[i]->id);
                    }
                }

                free(spill_offsets);
                free(arg_values);
            }
            else
            {
                // No nested calls - just evaluate arguments normally
                for (int i = 0; i < node->call_expr.args->count; i++)
                {
                    MIRValue *arg = lower_expr(ctx, node->call_expr.args->items[i]);
                    if (arg)
                    {
                        args[arg_idx++] = mir_operand_value(arg->id);
                    }
                }
            }
        }
        else if (has_hidden_ptr)
        {
            arg_count = 1;
            args      = malloc(arg_count * sizeof(MIROperand));
            args[0]   = mir_operand_value(hidden_ptr_slot->id);
        }

        // allocate result
        MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "call");

        // create call instruction
        MIRInst *inst = mir_inst_call(NULL, func_name, args, arg_count);
        mir_inst_set_result(inst, result);
        mir_block_append_inst(ctx->current_block, inst);

        if (args)
        {
            free(args);
        }

        if (has_hidden_ptr)
        {
            // Return address of stack slot
            // CRITICAL: The register holding hidden_ptr_slot might have been clobbered by the call.
            // We must re-calculate the address after the call to ensure we have a valid register.
            MIRValue *ret_val = mir_function_alloc_value(ctx->current_function, NULL, "hidden_ret_addr_reload");
            MIRInst  *addr    = mir_inst_create(MIR_OP_ADDR, NULL);
            mir_inst_add_operand(addr, mir_operand_imm_int(hidden_ptr_offset));
            mir_inst_set_result(addr, ret_val);
            mir_block_append_inst(ctx->current_block, addr);

            lower_context_map_value(ctx, node, ret_val);
            return ret_val;
        }
        else
        {
            lower_context_map_value(ctx, node, result);
            return result;
        }
    }

    default:
        return NULL;
    }
}

// check if a symbol name contains Itanium-style template args (I...E pattern)
static bool has_template_args(const char *name)
{
    if (!name)
    {
        return false;
    }

    const char *i_pos = strchr(name, 'I');
    if (!i_pos)
    {
        return false;
    }

    // check there's content between I and E
    const char *e_pos = strrchr(name, 'E');
    if (!e_pos || e_pos <= i_pos + 1)
    {
        return false;
    }

    return true;
}

// helper to lower instantiated generic functions from symbol table
static void lower_instantiated_generics(LowerContext *ctx, SymbolTable *table)
{
    if (!table)
    {
        return;
    }

    for (Symbol *sym = table->symbols; sym; sym = sym->next)
    {
        // look for function symbols that are instantiated generics:
        // - kind is SYMBOL_FUNCTION
        // - is_generic is false (it's an instantiation, not a template)
        // - decl is set and is a function
        // - name contains I...E pattern (Itanium-style template args)
        if (sym->kind == SYMBOL_FUNCTION && !sym->is_generic && sym->decl && sym->decl->kind == AST_STMT_FUN && has_template_args(sym->name))
        {
            lower_function(ctx, sym->decl);
        }
    }
}

MIRModule *mir_lower_module(AstNode *ast_module, SymbolTable *symbols)
{
    if (!ast_module)
    {
        return NULL;
    }

    // extract module name
    const char *module_name = "module";
    if (ast_module->kind == AST_MODULE && ast_module->module.name)
    {
        module_name = ast_module->module.name;
    }
    else if (ast_module->kind == AST_PROGRAM)
    {
        module_name = "program";
    }

    // create mir module
    MIRModule *module = mir_module_create(module_name);
    if (!module)
    {
        return NULL;
    }

    // create lowering context
    LowerContext *ctx = lower_context_create(module);
    if (!ctx)
    {
        mir_module_destroy(module);
        return NULL;
    }

    // lower all statements
    AstList *stmts = NULL;
    if (ast_module->kind == AST_PROGRAM)
    {
        stmts = ast_module->program.stmts;
    }
    else if (ast_module->kind == AST_MODULE)
    {
        stmts = ast_module->module.stmts;
    }

    if (stmts)
    {
        // First pass: lower all globals (variables)
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *stmt = stmts->items[i];
            if (stmt->kind == AST_STMT_VAR || stmt->kind == AST_STMT_VAL)
            {
                lower_stmt(ctx, stmt);
            }
        }

        // Second pass: lower all non-generic functions
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *stmt = stmts->items[i];
            if (stmt->kind == AST_STMT_FUN)
            {
                // skip generic template functions - they are instantiated on demand
                if (stmt->fun_stmt.generics && stmt->fun_stmt.generics->count > 0)
                {
                    continue;
                }
                lower_stmt(ctx, stmt);
            }
        }

        // Third pass: lower everything else (types, etc.)
        for (int i = 0; i < stmts->count; i++)
        {
            AstNode *stmt = stmts->items[i];
            if (stmt->kind != AST_STMT_VAR && stmt->kind != AST_STMT_VAL && stmt->kind != AST_STMT_FUN)
            {
                lower_stmt(ctx, stmt);
            }
        }
    }

    // Fourth pass: lower instantiated generic functions from symbol table
    if (symbols)
    {
        lower_instantiated_generics(ctx, symbols);
    }

    lower_context_destroy(ctx);
    return module;
}

MIRFunction *mir_lower_function(AstNode *ast_function)
{
    if (!ast_function || ast_function->kind != AST_STMT_FUN)
    {
        return NULL;
    }

    // create temporary module for standalone function lowering
    MIRModule    *temp_module = mir_module_create("temp");
    LowerContext *ctx         = lower_context_create(temp_module);

    lower_function(ctx, ast_function);

    MIRFunction *func = temp_module->functions;
    if (func)
    {
        // detach from module
        temp_module->functions = func->next;
        func->next             = NULL;
    }

    lower_context_destroy(ctx);
    mir_module_destroy(temp_module);

    return func;
}

MIRGlobal *mir_lower_global(AstNode *ast_var)
{
    if (!ast_var || (ast_var->kind != AST_STMT_VAR && ast_var->kind != AST_STMT_VAL))
    {
        return NULL;
    }

    // create temporary module for standalone global lowering
    MIRModule    *temp_module = mir_module_create("temp");
    LowerContext *ctx         = lower_context_create(temp_module);

    lower_var(ctx, ast_var);

    MIRGlobal *global = temp_module->globals;
    if (global)
    {
        // detach from module
        temp_module->globals = global->next;
        global->next         = NULL;
    }

    lower_context_destroy(ctx);
    mir_module_destroy(temp_module);

    return global;
}

// simple inline MIR parser
int mir_parse_inline_block(LowerContext *ctx, MIRFunction *func, MIRBlock *current_block, const char *mir_text)
{
    if (!func || !current_block || !mir_text)
    {
        return -1;
    }

    const char *p = mir_text;

    // Map for local SSA values within the block: name -> value_id
    // Simple fixed-size map for now
    struct
    {
        char    *name;
        uint32_t id;
    } local_values[64];
    int local_value_count = 0;

    while (*p)
    {
        // skip whitespace
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        {
            p++;
        }
        if (*p == '\0')
        {
            break;
        }

        // Check for assignment: %name = ...
        char *dest_name = NULL;
        if (*p == '%')
        {
            const char *start = p;
            while (*p && *p != ' ' && *p != '=' && *p != '\t')
            {
                p++;
            }

            // Check if it's an assignment
            const char *check = p;
            while (*check == ' ' || *check == '\t')
            {
                check++;
            }

            if (*check == '=')
            {
                int len   = p - start;
                dest_name = malloc(len + 1);
                memcpy(dest_name, start, len);
                dest_name[len] = '\0';
                p              = check + 1; // skip =
                while (*p == ' ' || *p == '\t')
                {
                    p++;
                }
            }
            else
            {
                // Not an assignment, reset
                p = start;
            }
        }

        // Parse opcode
        const char *op_start = p;
        while (*p && (*p >= 'a' && *p <= 'z'))
        {
            p++;
        }
        int  op_len = p - op_start;
        char op_name[32];
        if (op_len >= 32)
        {
            op_len = 31;
        }
        memcpy(op_name, op_start, op_len);
        op_name[op_len] = '\0';

        MIROp op = MIR_OP_ADD; // default/invalid
        if (strcmp(op_name, "syscall") == 0)
        {
            op = MIR_OP_SYSCALL;
        }
        else if (strcmp(op_name, "store") == 0)
        {
            op = MIR_OP_STORE;
        }
        else if (strcmp(op_name, "load") == 0)
        {
            op = MIR_OP_LOAD;
        }
        else if (strcmp(op_name, "ret") == 0)
        {
            op = MIR_OP_RET;
        }
        else if (strcmp(op_name, "mov") == 0)
        {
            op = MIR_OP_MOV;
        }
        else if (strcmp(op_name, "add") == 0)
        {
            op = MIR_OP_ADD;
        }
        else if (strcmp(op_name, "sub") == 0)
        {
            op = MIR_OP_SUB;
        }
        else if (strcmp(op_name, "mul") == 0)
        {
            op = MIR_OP_MUL;
        }
        else if (strcmp(op_name, "div") == 0)
        {
            op = MIR_OP_DIV;
        }
        else if (strcmp(op_name, "addr") == 0)
        {
            op = MIR_OP_ADDR;
        }

        MIRInst *inst = mir_inst_create(op, NULL);

        // Parse operands
        // Expect operands separated by comma or space, possibly in parens?
        // The example used `syscall(1, ...)` but `store %val, %ptr`.
        // Let's support both: optional parens, comma/space separators.

        if (*p == '(')
        {
            p++;
        }

        while (*p && *p != ')' && *p != ';' && *p != '\n')
        {
            while (*p == ' ' || *p == '\t' || *p == ',')
            {
                p++;
            }
            if (*p == '\0' || *p == ')' || *p == ';' || *p == '\n')
            {
                break;
            }

            if (*p == '%')
            {
                // SSA value or parameter
                const char *start = p;
                p++;
                if (*p >= '0' && *p <= '9')
                {
                    // %0, %1 -> function parameters - need to load from stack
                    int param_idx = atoi(p);
                    while (*p >= '0' && *p <= '9')
                    {
                        p++;
                    }

                    if (param_idx >= 0 && param_idx < (int)func->param_count && func->params[param_idx])
                    {
                        // Parameters are spilled to stack, so we need to load them
                        // Find the parameter's stack slot
                        int32_t offset = 0;
                        if (ctx && ctx->stack_slots)
                        {
                            // Parameters are at the beginning of stack_slots
                            // stack_slots[0] is param 0, stack_slots[1] is param 1, etc.
                            if (param_idx < ctx->stack_slot_count)
                            {
                                offset = ctx->stack_slots[param_idx].offset;
                            }
                        }

                        if (offset != 0)
                        {
                            // Generate a LOAD instruction to read the parameter from its stack slot
                            MIRValue *loaded = mir_function_alloc_value(func, NULL, "param_load");
                            MIRInst  *load   = mir_inst_create(MIR_OP_LOAD, NULL);
                            mir_inst_add_operand(load, mir_operand_imm_int(offset));
                            mir_inst_set_result(load, loaded);
                            mir_block_append_inst(current_block, load);

                            mir_inst_add_operand(inst, mir_operand_value(loaded->id));
                        }
                        else
                        {
                            // Fallback: use the parameter value directly (might not work)
                            mir_inst_add_operand(inst, mir_operand_value(func->params[param_idx]->id));
                        }
                    }
                    else
                    {
                        mir_inst_add_operand(inst, mir_operand_value(0));
                    }
                }
                else
                {
                    // Named SSA value: %name
                    while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
                    {
                        p++;
                    }

                    int   len  = p - start;
                    char *name = malloc(len + 1);
                    memcpy(name, start, len);
                    name[len] = '\0';

                    // Look up in local values
                    uint32_t id = 0;
                    for (int i = 0; i < local_value_count; i++)
                    {
                        if (strcmp(local_values[i].name, name) == 0)
                        {
                            id = local_values[i].id;
                            break;
                        }
                    }

                    // Also check parameters by name?
                    if (id == 0)
                    {
                        for (size_t i = 0; i < func->param_count; i++)
                        {
                            // Param names don't have %, but we might use %param
                            if (func->params[i] && func->params[i]->name && strcmp(func->params[i]->name, name + 1) == 0)
                            {
                                id = func->params[i]->id;
                                break;
                            }
                        }
                    }

                    mir_inst_add_operand(inst, mir_operand_value(id));
                    free(name);
                }
            }
            else if (*p == '@')
            {
                // Global - need to find the mangled name
                p++;
                const char *start = p;
                while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
                {
                    p++;
                }
                int   len  = p - start;
                char *name = malloc(len + 1);
                memcpy(name, start, len);
                name[len] = '\0';

                // Look up the global in the module to get its mangled name
                const char *mangled_name = NULL;
                if (ctx && ctx->module)
                {
                    for (MIRGlobal *g = ctx->module->globals; g; g = g->next)
                    {
                        // Check if the mangled name contains the unmangled name
                        if (strstr(g->name, name) != NULL)
                        {
                            mangled_name = g->name;
                            break;
                        }
                    }
                }

                // Use the mangled name if found, otherwise use the original name
                mir_inst_add_operand(inst, mir_operand_global(mangled_name ? mangled_name : name));
                free(name);
            }
            else if ((*p >= '0' && *p <= '9') || *p == '-')
            {
                // Integer literal
                int64_t val = atoll(p);
                if (*p == '-')
                {
                    p++;
                }
                while (*p >= '0' && *p <= '9')
                {
                    p++;
                }
                mir_inst_add_operand(inst, mir_operand_imm_int(val));
            }
            else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
            {
                // Identifier (local variable)
                const char *start = p;
                while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_'))
                {
                    p++;
                }
                int   len  = p - start;
                char *name = malloc(len + 1);
                memcpy(name, start, len);
                name[len] = '\0';

                // Look up stack offset
                int32_t offset = 0;
                if (ctx && ctx->stack_slots)
                {
                    for (int i = 0; i < ctx->stack_slot_count; i++)
                    {
                        if (!ctx->stack_slots[i].node)
                        {
                            continue;
                        }
                        const char *slot_name = NULL;
                        if (ctx->stack_slots[i].node->kind == AST_STMT_VAR || ctx->stack_slots[i].node->kind == AST_STMT_VAL)
                        {
                            slot_name = ctx->stack_slots[i].node->var_stmt.name;
                        }
                        else if (ctx->stack_slots[i].node->kind == AST_STMT_PARAM)
                        {
                            slot_name = ctx->stack_slots[i].node->param_stmt.name;
                        }

                        if (slot_name && strcmp(slot_name, name) == 0)
                        {
                            offset = ctx->stack_slots[i].offset;
                            break;
                        }
                    }
                }

                if (offset != 0)
                {
                    // Load address or value?
                    // For 'store %val, var', var is address.
                    // For 'mov %val, var', var is value (load).
                    // This is ambiguous.
                    // Convention: identifiers in MIR block refer to the VALUE of the variable (LOAD).
                    // If we want address, we should use specific syntax or context.
                    // But for 'store %val, %ptr', %ptr is address.
                    // If we do 'store %val, var', we probably mean store to var's address.

                    // However, 'store' takes (value, address).
                    // If second operand is identifier, we want its address.
                    // If first operand is identifier, we want its value.

                    // Let's implement a heuristic:
                    // Always emit LOAD for identifier.
                    // If we need address, we should have used `addr var` or similar?
                    // Or maybe we check opcode?

                    // For simplicity: Emit LOAD.
                    // If user wants address, they should use `&var` (not supported yet) or `alloca`.

                    MIRValue *val  = mir_function_alloc_value(func, NULL, "inline_load");
                    MIRInst  *load = mir_inst_create(MIR_OP_LOAD, NULL);
                    mir_inst_add_operand(load, mir_operand_imm_int(offset));
                    mir_inst_set_result(load, val);
                    mir_block_append_inst(current_block, load);

                    mir_inst_add_operand(inst, mir_operand_value(val->id));
                }
                else
                {
                    // Global?
                    mir_inst_add_operand(inst, mir_operand_global(name));
                }
                free(name);
            }
            else
            {
                p++; // skip unknown
            }
        }

        if (*p == ')')
        {
            p++;
        }
        if (*p == ';')
        {
            p++;
        }

        // Handle result assignment
        if (dest_name)
        {
            MIRValue *res = mir_function_alloc_value(func, NULL, dest_name);
            mir_inst_set_result(inst, res);

            if (local_value_count < 64)
            {
                local_values[local_value_count].name = dest_name;
                local_values[local_value_count].id   = res->id;
                local_value_count++;
            }
            else
            {
                free(dest_name);
            }
        }

        mir_block_append_inst(current_block, inst);
    }

    // cleanup
    for (int i = 0; i < local_value_count; i++)
    {
        free(local_values[i].name);
    }

    return 0;
}

// helper to add string data to module
