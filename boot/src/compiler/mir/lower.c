#include "compiler/mir/lower.h"
#include "compiler/mir/function.h"
#include "compiler/mir/block.h"
#include "compiler/mir/inst.h"
#include "compiler/mir/global.h"
#include "compiler/mir/opcode.h"
#include "compiler/symbol.h"
#include "compiler/token.h"
#include "compiler/type.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Stack slot mapping for local variables
typedef struct
{
    AstNode *node;      // Variable AST node
    int32_t  offset;    // Stack offset from RBP (negative)
} StackSlot;

// lowering context tracks state during lowering
typedef struct LowerContext
{
    MIRModule   *module;
    MIRFunction *current_function;
    MIRBlock    *current_block;
    
    // value map: ast node -> mir value (for expressions)
    struct {
        AstNode  *node;
        MIRValue *value;
    } *value_map;
    int value_map_count;
    int value_map_capacity;
    
    // Stack frame management
    StackSlot *stack_slots;
    int        stack_slot_count;
    int        stack_slot_capacity;
    int32_t    frame_size;  // Total stack space needed (in bytes)
} LowerContext;

static LowerContext *lower_context_create(MIRModule *module)
{
    LowerContext *ctx = malloc(sizeof(LowerContext));
    if (!ctx)
    {
        return NULL;
    }
    
    ctx->module = module;
    ctx->current_function = NULL;
    ctx->current_block = NULL;
    ctx->value_map = NULL;
    ctx->value_map_count = 0;
    ctx->value_map_capacity = 0;
    ctx->stack_slots = NULL;
    ctx->stack_slot_count = 0;
    ctx->stack_slot_capacity = 0;
    ctx->frame_size = 0;
    
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
    
    free(ctx);
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
        int new_capacity = ctx->value_map_capacity == 0 ? 64 : ctx->value_map_capacity * 2;
        void *new_map = realloc(ctx->value_map, new_capacity * sizeof(*ctx->value_map));
        if (!new_map)
        {
            return;
        }
        ctx->value_map = new_map;
        ctx->value_map_capacity = new_capacity;
    }
    
    ctx->value_map[ctx->value_map_count].node = node;
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
static int32_t lower_context_alloc_stack_slot(LowerContext *ctx, AstNode *node, size_t size)
{
    if (!ctx || !node)
    {
        return 0;
    }
    
    // Expand capacity if needed
    if (ctx->stack_slot_count >= ctx->stack_slot_capacity)
    {
        int new_capacity = ctx->stack_slot_capacity == 0 ? 16 : ctx->stack_slot_capacity * 2;
        void *new_slots = realloc(ctx->stack_slots, new_capacity * sizeof(StackSlot));
        if (!new_slots)
        {
            return 0;
        }
        ctx->stack_slots = new_slots;
        ctx->stack_slot_capacity = new_capacity;
    }
    
    // Allocate new stack space (stack grows down)
    ctx->frame_size += size;
    int32_t offset = -(int32_t)ctx->frame_size;
    
    // Record the slot
    ctx->stack_slots[ctx->stack_slot_count].node = node;
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
    
    return 0; // Not found
}

// forward declarations
static int lower_stmt(LowerContext *ctx, AstNode *node);
static MIRValue *lower_expr(LowerContext *ctx, AstNode *node);

// lower function to MIR
static int lower_function(LowerContext *ctx, AstNode *node)
{
    if (node->kind != AST_STMT_FUN)
    {
        return -1;
    }
    
    // create mir function
    MIRFunction *func = mir_function_create(node->fun_stmt.name, NULL, node->fun_stmt.is_public);
    if (!func)
    {
        return -1;
    }
    
    // reset function-specific context state
    ctx->current_function = func;
    ctx->frame_size = 0;
    ctx->stack_slot_count = 0;
    
    // add parameters
    if (node->fun_stmt.params)
    {
        for (int i = 0; i < node->fun_stmt.params->count; i++)
        {
            AstNode *param = node->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                MIRValue *param_val = mir_function_add_param(func, NULL, param->param_stmt.name);
                lower_context_map_value(ctx, param, param_val);
            }
        }
    }
    
    // create entry block
    MIRBlock *entry = mir_function_add_block(func, "entry");
    ctx->current_block = entry;
    
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
    ctx->current_block = NULL;
    
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
        MIRGlobalKind kind = (node->kind == AST_STMT_VAL) ? MIR_GLOBAL_VAL : MIR_GLOBAL_VAR;
        MIRGlobal *global = mir_global_create(node->var_stmt.name, NULL, kind, node->var_stmt.is_public);
        
        // handle initializer
        if (node->var_stmt.init)
        {
            if (node->var_stmt.init->kind == AST_EXPR_LIT && node->var_stmt.init->token)
            {
                if (node->var_stmt.init->token->kind == TOKEN_LIT_INT)
                {
                    mir_global_set_int_init(global, node->var_stmt.init->lit_expr.int_val);
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
        // For now, assume all variables are 8 bytes (i64/pointer size)
        int32_t offset = lower_context_alloc_stack_slot(ctx, node, 8);
        
        // Handle initializer
        if (node->var_stmt.init)
        {
            MIRValue *init_val = lower_expr(ctx, node->var_stmt.init);
            if (init_val)
            {
                // Generate STORE: store init_val to [rbp + offset]
                MIRInst *store = mir_inst_create(MIR_OP_STORE, NULL);
                mir_inst_add_operand(store, mir_operand_value(init_val->id));
                mir_inst_add_operand(store, mir_operand_imm_int(offset));
                mir_block_append_inst(ctx->current_block, store);
            }
        }
        // No init value means uninitialized - stack slot is already allocated
    }
    
    return 0;
}

static int lower_if_stmt(LowerContext *ctx, AstNode *node)
{
    if (node->kind != AST_STMT_IF) return -1;
    
    // create blocks
    MIRBlock *then_block = mir_function_add_block(ctx->current_function, "if.then");
    MIRBlock *else_block = node->cond_stmt.stmt_or ? mir_function_add_block(ctx->current_function, "if.else") : NULL;
    MIRBlock *merge_block = mir_function_add_block(ctx->current_function, "if.end");
    
    // lower condition
    MIRValue *cond = lower_expr(ctx, node->cond_stmt.cond);
    if (!cond) return -1;
    
    // branch
    MIRBlock *false_target = else_block ? else_block : merge_block;
    MIRInst *br = mir_inst_brcond(mir_operand_value(cond->id), then_block->id, false_target->id);
    mir_block_append_inst(ctx->current_block, br);
    
    // then block
    ctx->current_block = then_block;
    if (lower_stmt(ctx, node->cond_stmt.body) < 0) return -1;
    
    if (!ctx->current_block->last_inst || !mir_op_is_terminator(ctx->current_block->last_inst->op))
    {
        mir_block_append_inst(ctx->current_block, mir_inst_br(merge_block->id));
    }
    
    // else block
    if (else_block)
    {
        ctx->current_block = else_block;
        if (lower_stmt(ctx, node->cond_stmt.stmt_or) < 0) return -1;
        
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
    if (node->kind != AST_STMT_FOR) return -1;
    
    // create blocks
    MIRBlock *cond_block = mir_function_add_block(ctx->current_function, "for.cond");
    MIRBlock *body_block = mir_function_add_block(ctx->current_function, "for.body");
    MIRBlock *end_block = mir_function_add_block(ctx->current_function, "for.end");
    
    // jump to condition
    mir_block_append_inst(ctx->current_block, mir_inst_br(cond_block->id));
    
    // condition block
    ctx->current_block = cond_block;
    if (node->for_stmt.cond)
    {
        MIRValue *cond = lower_expr(ctx, node->for_stmt.cond);
        if (!cond) return -1;
        
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
    if (lower_stmt(ctx, node->for_stmt.body) < 0) return -1;
    
    if (!ctx->current_block->last_inst || !mir_op_is_terminator(ctx->current_block->last_inst->op))
    {
        mir_block_append_inst(ctx->current_block, mir_inst_br(cond_block->id));
    }
    
    ctx->current_block = end_block;
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
        if (node->block_stmt.stmts)
        {
            for (int i = 0; i < node->block_stmt.stmts->count; i++)
            {
                if (lower_stmt(ctx, node->block_stmt.stmts->items[i]) < 0)
                {
                    return -1;
                }
            }
        }
        return 0;
        
    case AST_STMT_RET:
        if (node->ret_stmt.expr)
        {
            MIRValue *val = lower_expr(ctx, node->ret_stmt.expr);
            if (val)
            {
                MIRInst *ret = mir_inst_ret(NULL, mir_operand_value(val->id));
                mir_block_append_inst(ctx->current_block, ret);
            }
        }
        else
        {
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
static MIRValue *lower_expr(LowerContext *ctx, AstNode *node)
{
    if (!node || !ctx->current_function)
    {
        return NULL;
    }
    
    switch (node->kind)
    {
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
        return NULL;
        
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
                    // It's a local variable on stack - generate LOAD
                    MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "local_load");
                    
                    // Generate LOAD: result = load(offset)
                    MIRInst *load = mir_inst_create(MIR_OP_LOAD, NULL);
                    mir_inst_add_operand(load, mir_operand_imm_int(offset));
                    mir_inst_set_result(load, result);
                    mir_block_append_inst(ctx->current_block, load);
                    
                    return result;
                }
                
                // check if we've already lowered this as a value (e.g. function param)
                MIRValue *local_val = lower_context_get_value(ctx, node->symbol->decl);
                if (!local_val)
                {
                    // not a local, must be a global
                    // for globals, we need to load the address
                    MIRValue *result = mir_function_alloc_value(ctx->current_function, NULL, "global_ref");
                    
                    // create a MOV instruction that loads from the global
                    MIRInst *inst = mir_inst_create(MIR_OP_MOV, NULL);
                    mir_inst_add_operand(inst, mir_operand_global(node->symbol->name));
                    mir_inst_set_result(inst, result);
                    mir_block_append_inst(ctx->current_block, inst);
                    
                    lower_context_map_value(ctx, node, result);
                    return result;
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
        
    case AST_EXPR_BINARY:
    {
        MIRValue *left = lower_expr(ctx, node->binary_expr.left);
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
        case TOKEN_PLUS:  op = MIR_OP_ADD; break;
        case TOKEN_MINUS: op = MIR_OP_SUB; break;
        case TOKEN_STAR:  op = MIR_OP_MUL; break;
        case TOKEN_SLASH: op = MIR_OP_DIV; break;
        case TOKEN_EQUAL_EQUAL: op = MIR_OP_EQ; break;
        case TOKEN_BANG_EQUAL: op = MIR_OP_NE; break;
        case TOKEN_LESS:    op = MIR_OP_LT; break;
        case TOKEN_GREATER:    op = MIR_OP_GT; break;
        case TOKEN_LESS_EQUAL: op = MIR_OP_LE; break;
        case TOKEN_GREATER_EQUAL: op = MIR_OP_GE; break;
        case TOKEN_AMPERSAND: op = MIR_OP_AND; break;
        case TOKEN_PIPE: op = MIR_OP_OR; break;
        case TOKEN_CARET: op = MIR_OP_XOR; break;
        case TOKEN_LESS_LESS: op = MIR_OP_SHL; break;
        case TOKEN_GREATER_GREATER: op = MIR_OP_SHR; break;
        default: break;
        }
        
        // create instruction
        MIRInst *inst = mir_inst_binary(op, NULL, 
                                        mir_operand_value(left->id),
                                        mir_operand_value(right->id));
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
        }
        
        if (!func_name)
        {
            return NULL;
        }
        
        // lower arguments
        MIROperand *args = NULL;
        int arg_count = 0;
        if (node->call_expr.args)
        {
            arg_count = node->call_expr.args->count;
            args = malloc(arg_count * sizeof(MIROperand));
            for (int i = 0; i < arg_count; i++)
            {
                MIRValue *arg = lower_expr(ctx, node->call_expr.args->items[i]);
                if (arg)
                {
                    args[i] = mir_operand_value(arg->id);
                }
            }
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
        
        lower_context_map_value(ctx, node, result);
        return result;
    }
    
    default:
        return NULL;
    }
}

MIRModule *mir_lower_module(AstNode *ast_module)
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
        for (int i = 0; i < stmts->count; i++)
        {
            lower_stmt(ctx, stmts->items[i]);
        }
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
    MIRModule *temp_module = mir_module_create("temp");
    LowerContext *ctx = lower_context_create(temp_module);
    
    lower_function(ctx, ast_function);
    
    MIRFunction *func = temp_module->functions;
    if (func)
    {
        // detach from module
        temp_module->functions = func->next;
        func->next = NULL;
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
    MIRModule *temp_module = mir_module_create("temp");
    LowerContext *ctx = lower_context_create(temp_module);
    
    lower_var(ctx, ast_var);
    
    MIRGlobal *global = temp_module->globals;
    if (global)
    {
        // detach from module
        temp_module->globals = global->next;
        global->next = NULL;
    }
    
    lower_context_destroy(ctx);
    mir_module_destroy(temp_module);
    
    return global;
}

// simple inline MIR parser for basic instructions
int mir_parse_inline_block(LowerContext *ctx, MIRFunction *func, MIRBlock *current_block, const char *mir_text)
{
    if (!func || !current_block || !mir_text)
    {
        return -1;
    }
    
    // parse SSA-style inline MIR with immediates as operands
    const char *p = mir_text;
    
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
        
        // parse instruction
        if (strncmp(p, "syscall(", 8) == 0)
        {
            // SSA-style: syscall(60, 42) - immediates directly as operands
            p += 8;
            
            MIRInst *inst = mir_inst_create(MIR_OP_SYSCALL, NULL);
            
            // parse arguments
            while (*p && *p != ')')
            {
                while (*p == ' ' || *p == '\t' || *p == ',')
                {
                    p++;
                }
                
                if (*p >= '0' && *p <= '9')
                {
                    int64_t value = atoll(p);
                    mir_inst_add_operand(inst, mir_operand_imm_int(value));
                    
                    while (*p >= '0' && *p <= '9')
                    {
                        p++;
                    }
                }
                else if (*p == '%')
                {
                    // SSA value: %0, %1, etc. - refers to function parameters
                    p++;
                    int param_idx = atoi(p);
                    
                    // Map %N to the Nth function parameter's value ID
                    if (param_idx >= 0 && param_idx < (int)func->param_count && func->params[param_idx])
                    {
                        mir_inst_add_operand(inst, mir_operand_value(func->params[param_idx]->id));
                    }
                    else
                    {
                        // Invalid parameter reference, use 0 as fallback
                        mir_inst_add_operand(inst, mir_operand_value(0));
                    }
                    
                    while (*p >= '0' &&  *p <= '9')
                    {
                        p++;
                    }
                }
                else if (*p == '@')
                {
                    // global reference: @name
                    p++;
                    const char *name_start = p;
                    while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || 
                                 (*p >= '0' && *p <= '9') || *p == '_'))
                    {
                        p++;
                    }
                    
                    // copy name
                    int name_len = p - name_start;
                    char *name = malloc(name_len + 1);
                    memcpy(name, name_start, name_len);
                    name[name_len] = '\0';
                    
                    mir_inst_add_operand(inst, mir_operand_global(name));
                }
                else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_')
                {
                    // identifier (local variable)
                    const char *name_start = p;
                    while (*p && ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || 
                                 (*p >= '0' && *p <= '9') || *p == '_'))
                    {
                        p++;
                    }
                    
                    int name_len = p - name_start;
                    char *name = malloc(name_len + 1);
                    memcpy(name, name_start, name_len);
                    name[name_len] = '\0';
                    
                    // Look up stack offset
                    int32_t offset = 0;
                    if (ctx && ctx->stack_slots)
                    {
                        for (int i = 0; i < ctx->stack_slot_count; i++)
                        {
                            if (ctx->stack_slots[i].node && 
                                (ctx->stack_slots[i].node->kind == AST_STMT_VAR || ctx->stack_slots[i].node->kind == AST_STMT_VAL) &&
                                ctx->stack_slots[i].node->var_stmt.name &&
                                strcmp(ctx->stack_slots[i].node->var_stmt.name, name) == 0)
                            {
                                offset = ctx->stack_slots[i].offset;
                                break;
                            }
                        }
                    }
                    
                    if (offset != 0)
                    {
                        // Generate LOAD
                        MIRValue *val = mir_function_alloc_value(func, NULL, "inline_load");
                        MIRInst *load = mir_inst_create(MIR_OP_LOAD, NULL);
                        mir_inst_add_operand(load, mir_operand_imm_int(offset));
                        mir_inst_set_result(load, val);
                        mir_block_append_inst(current_block, load);
                        
                        mir_inst_add_operand(inst, mir_operand_value(val->id));
                        free(name); // name not needed for value operand
                    }
                    else
                    {
                        // Not found locally, assume global reference
                        mir_inst_add_operand(inst, mir_operand_global(name));
                    }
                }
                else if (*p == ')')
                {
                    break;
                }
                else
                {
                    p++;
                }
            }
            
            if (*p == ')')
            {
                p++;
            }
            
            mir_block_append_inst(current_block, inst);
        }
        else if (strncmp(p, "ret", 3) == 0)
        {
            p += 3;
            
            while (*p == ' ' || *p == '\t')
            {
                p++;
            }
            
            if (*p == '\n' || *p == '\0' || *p == ';')
            {
                // ret void
                MIRInst *inst = mir_inst_ret_void();
                mir_block_append_inst(current_block, inst);
            }
            else if (*p >= '0' && *p <= '9')
            {
                // ret with immediate - materialize it
                int64_t value = atoll(p);
                MIRValue *result = mir_function_alloc_value(func, NULL, "const");
                MIRInst *const_inst = mir_inst_const(NULL, value);
                mir_inst_set_result(const_inst, result);
                mir_block_append_inst(current_block, const_inst);
                
                MIRInst *ret_inst = mir_inst_ret(NULL, mir_operand_value(result->id));
                mir_block_append_inst(current_block, ret_inst);
                
                while (*p >= '0' && *p <= '9')
                {
                    p++;
                }
            }
            else if (*p == '%')
            {
                // ret with SSA value
                p++;
                int val_id = atoi(p);
                MIRInst *inst = mir_inst_ret(NULL, mir_operand_value(val_id));
                mir_block_append_inst(current_block, inst);
                
                while (*p >= '0' && *p <= '9')
                {
                    p++;
                }
            }
        }
        else
        {
            // skip unknown instruction
            while (*p && *p != '\n')
            {
                p++;
            }
        }
        
        // skip optional semicolon
        if (*p == ';')
        {
            p++;
        }
    }
    
    return 0;
}

// helper to add string data to module

