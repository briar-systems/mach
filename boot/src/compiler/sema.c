#include "compiler/sema.h"
#include "compiler/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler/comptime.h"

// semantic analyzer context
struct Sema
{
    SymbolTable *root_table;
    SymbolTable *current_table;
    int          error_count;
    char        *module_path;
};

Sema *sema_create(const char *module_path)
{
    Sema *sema = malloc(sizeof(Sema));
    if (!sema)
    {
        return NULL;
    }

    sema->root_table = symbol_table_create(NULL);
    sema->current_table = sema->root_table;
    sema->error_count = 0;
    sema->module_path = module_path ? strdup(module_path) : NULL;

    return sema;
}

void sema_destroy(Sema *sema)
{
    if (!sema)
    {
        return;
    }

    if (sema->root_table)
    {
        symbol_table_destroy(sema->root_table);
    }
    if (sema->module_path)
    {
        free(sema->module_path);
    }

    free(sema);
}

SymbolTable *sema_get_root_table(Sema *sema)
{
    return sema ? sema->root_table : NULL;
}

int sema_get_error_count(Sema *sema)
{
    return sema ? sema->error_count : 0;
}

void sema_print_errors(Sema *sema)
{
    if (!sema || sema->error_count == 0)
    {
        return;
    }

    fprintf(stderr, "%d semantic error(s) found\n", sema->error_count);
}

static void sema_error(Sema *sema, Token *token, const char *message)
{
    if (!sema)
    {
        return;
    }

    sema->error_count++;

    if (token)
    {
        fprintf(stderr, "semantic error at position %d: %s\n", token->pos, message);
    }
    else
    {
        fprintf(stderr, "semantic error: %s\n", message);
    }
}

// forward declarations for mutual recursion
static int sema_analyze_stmt(Sema *sema, AstNode *node);
static int sema_analyze_expr(Sema *sema, AstNode *node);
static Type *sema_resolve_type(Sema *sema, AstNode *type_node);

// analyze function declaration
static int sema_analyze_fun(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_FUN)
    {
        return -1;
    }

    // create symbol for function
    Symbol *sym = symbol_create(node->fun_stmt.name, SYMBOL_FUNCTION, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    // check for generics
    if (node->fun_stmt.generics)
    {
        sym->is_generic = true;
        sym->decl = node;
        // Add to symbol table
        if (symbol_table_insert(sema->current_table, sym) < 0)
        {
            sema_error(sema, node->token, "duplicate function declaration");
            symbol_destroy(sym);
            return -1;
        }
        node->symbol = sym;
        return 0;
    }

    // resolve return type
    Type *ret_type = NULL;
    if (node->fun_stmt.return_type)
    {
        ret_type = sema_resolve_type(sema, node->fun_stmt.return_type);
        if (!ret_type)
        {
            sema_error(sema, node->fun_stmt.return_type->token, "failed to resolve return type");
            symbol_destroy(sym);
            return -1;
        }
    }
    
    // Resolve parameter types to build function type
    Type **param_types = NULL;
    int param_count = 0;
    
    if (node->fun_stmt.params)
    {
        param_count = node->fun_stmt.params->count;
        param_types = malloc(sizeof(Type*) * param_count);
        
        for (int i = 0; i < param_count; i++)
        {
            AstNode *param = node->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Type *pt = NULL; // default? or error?
                if (param->param_stmt.type)
                {
                    pt = sema_resolve_type(sema, param->param_stmt.type);
                    if (!pt)
                    {
                        sema_error(sema, param->token, "failed to resolve parameter type");
                        free(param_types);
                        symbol_destroy(sym);
                        return -1;
                    }
                }
                else
                {
                     // Parameter must have type?
                     // In Mach, yes? "val: T"
                     sema_error(sema, param->token, "parameter must have type");
                     free(param_types);
                     symbol_destroy(sym);
                     return -1;
                }
                param_types[i] = pt;
                param->type = pt; // Store for later use in body
            }
        }
    }
    
    sym->type = type_create_function(ret_type, param_types, param_count);
    // type_create_function copies the array? No, usually it takes ownership or copies.
    // Looking at type.c would confirm, but usually safe to free if it copies.
    // If it takes ownership, I shouldn't free.
    // Let's assume it copies for now or I'll check type.c later.
    // Actually, to be safe, I'll check type.c.
    
    // add function to symbol table
    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate function declaration");
        symbol_destroy(sym);
        // free(param_types); // depends on ownership
        return -1;
    }

    node->symbol = sym;

    // analyze function body if present
    if (node->fun_stmt.body)
    {
        // create new scope for function body
        SymbolTable *prev_table = sema->current_table;
        sema->current_table = symbol_table_create(prev_table);

        // add parameters to scope
        if (node->fun_stmt.params)
        {
            for (int i = 0; i < node->fun_stmt.params->count; i++)
            {
                AstNode *param = node->fun_stmt.params->items[i];
                if (param->kind == AST_STMT_PARAM)
                {
                    Symbol *param_sym = symbol_create(param->param_stmt.name, SYMBOL_VARIABLE, sema->module_path);
                    param_sym->type = param->type; // Use resolved type
                    
                    symbol_table_insert(sema->current_table, param_sym);
                    param->symbol = param_sym;
                    param_sym->decl = param;
                }
            }
        }

        sema_analyze_stmt(sema, node->fun_stmt.body);

        sema->current_table = prev_table;
    }
    
    // cleanup param_types array if type_create_function copied it
    // I'll assume it did for now.
    // if (param_types) free(param_types);

    return 0;
}

// analyze variable/value declaration
static int sema_analyze_var(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_VAR && node->kind != AST_STMT_VAL)
    {
        return -1;
    }

    Symbol *sym = symbol_create(node->var_stmt.name, SYMBOL_VARIABLE, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    // resolve explicit type if provided
    if (node->var_stmt.type)
    {
        Type *var_type = sema_resolve_type(sema, node->var_stmt.type);
        if (!var_type)
        {
            sema_error(sema, node->token, "failed to resolve variable type");
            symbol_destroy(sym);
            return -1;
        }
        sym->type = var_type;
    }

    // analyze initializer
    if (node->var_stmt.init)
    {
        if (sema_analyze_expr(sema, node->var_stmt.init) < 0)
        {
            symbol_destroy(sym);
            return -1;
        }

        // infer type from initializer if not explicit
        if (!sym->type && node->var_stmt.init->type)
        {
            sym->type = node->var_stmt.init->type;
        }
        // check type compatibility if type was explicit
        else if (sym->type && node->var_stmt.init->type)
        {
            if (!type_equals(sym->type, node->var_stmt.init->type))
            {
                sema_error(sema, node->token, "type mismatch: cannot assign type to variable");
                symbol_destroy(sym);
                return -1;
            }
        }
    }

    // add to symbol table
    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate variable declaration");
        symbol_destroy(sym);
        return -1;
    }

    node->symbol = sym;
    node->type = sym->type;
    sym->decl = node; // Link symbol back to declaration node

    return 0;
}

// analyze statement
// analyze record definition
static int sema_analyze_rec(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_REC)
    {
        return -1;
    }

    Symbol *sym = symbol_create(node->rec_stmt.name, SYMBOL_TYPE, sema->module_path);
    if (!sym)
    {
        return -1;
    }

    // process fields
    int field_count = node->rec_stmt.fields ? node->rec_stmt.fields->count : 0;
    TypeField *fields = NULL;
    
    if (field_count > 0)
    {
        fields = malloc(sizeof(TypeField) * field_count);
        if (!fields)
        {
            symbol_destroy(sym);
            return -1;
        }

        for (int i = 0; i < field_count; i++)
        {
            AstNode *field_node = node->rec_stmt.fields->items[i];
            if (field_node->kind != AST_STMT_FIELD)
            {
                free(fields);
                symbol_destroy(sym);
                return -1;
            }

            fields[i].name = strdup(field_node->field_stmt.name);
            fields[i].type = sema_resolve_type(sema, field_node->field_stmt.type);
            fields[i].offset = 0; // calculated in type_create_struct

            if (!fields[i].type)
            {
                sema_error(sema, field_node->token, "failed to resolve field type");
                // cleanup
                for (int j = 0; j <= i; j++) free(fields[j].name);
                free(fields);
                symbol_destroy(sym);
                return -1;
            }
        }
    }

    // create struct type
    Type *rec_type = type_create_struct(node->rec_stmt.name, fields, field_count);
    if (!rec_type)
    {
        // cleanup
        if (fields)
        {
            for (int i = 0; i < field_count; i++) free(fields[i].name);
            free(fields);
        }
        symbol_destroy(sym);
        return -1;
    }

    sym->type = rec_type;
    node->symbol = sym;
    node->type = rec_type;

    // add to symbol table
    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate type definition");
        symbol_destroy(sym);
        return -1;
    }

    return 0;
}

static bool sema_eval_comptime_int(Sema *sema, AstNode *node, int64_t *out_val)
{
    if (!node) return false;

    if (node->kind == AST_COMPTIME)
    {
        if (node->comptime.value_kind == COMPTIME_INT)
        {
            *out_val = node->comptime.int_value;
            return true;
        }
        return false;
    }
    
    if (node->kind == AST_EXPR_LIT)
    {
        if (node->lit_expr.kind == TOKEN_LIT_INT)
        {
            *out_val = (int64_t)node->lit_expr.int_val;
            return true;
        }
        return false;
    }
    
    if (node->kind == AST_EXPR_BINARY)
    {
        int64_t left, right;
        if (!sema_eval_comptime_int(sema, node->binary_expr.left, &left)) return false;
        if (!sema_eval_comptime_int(sema, node->binary_expr.right, &right)) return false;
        
        switch (node->binary_expr.op)
        {
            case TOKEN_PLUS: *out_val = left + right; return true;
            case TOKEN_MINUS: *out_val = left - right; return true;
            case TOKEN_STAR: *out_val = left * right; return true;
            case TOKEN_SLASH: *out_val = (right != 0) ? (left / right) : 0; return true;
            case TOKEN_EQUAL_EQUAL: *out_val = (left == right); return true;
            case TOKEN_BANG_EQUAL: *out_val = (left != right); return true;
            case TOKEN_LESS: *out_val = (left < right); return true;
            case TOKEN_GREATER: *out_val = (left > right); return true;
            case TOKEN_LESS_EQUAL: *out_val = (left <= right); return true;
            case TOKEN_GREATER_EQUAL: *out_val = (left >= right); return true;
            default: return false;
        }
    }
    
    return false;
}

static int sema_analyze_comptime_stmt(Sema *sema, AstNode *node)
{
    // node is AST_COMPTIME
    AstNode *inner = node->comptime.inner;

    // Check if it is an assignment
    if (inner->kind == AST_EXPR_BINARY && inner->binary_expr.op == TOKEN_EQUAL)
    {
        AstNode *lhs = inner->binary_expr.left;
        AstNode *rhs = inner->binary_expr.right;

        // Evaluate RHS
        if (sema_analyze_expr(sema, rhs) < 0)
        {
            return -1;
        }

        // RHS must be constant
        // For now, only string literals for name attribute
        if (rhs->kind != AST_EXPR_LIT || rhs->lit_expr.kind != TOKEN_LIT_STRING)
        {
            sema_error(sema, rhs->token, "expected string literal for attribute value");
            return -1;
        }

        // LHS must be field access on symbol
        // $foo.name
        // LHS is AST_EXPR_FIELD(object=foo, field=name)

        if (lhs->kind != AST_EXPR_FIELD)
        {
            sema_error(sema, lhs->token, "expected attribute access (e.g. $foo.name)");
            return -1;
        }

        AstNode *object = lhs->field_expr.object;
        char *field     = lhs->field_expr.field;

        // Object must be identifier (symbol)
        if (object->kind != AST_EXPR_IDENT)
        {
            sema_error(sema, object->token, "expected symbol identifier");
            return -1;
        }

        // Look up symbol
        Symbol *sym = symbol_table_lookup(sema->current_table, object->ident_expr.name);
        if (!sym)
        {
            sema_error(sema, object->token, "undefined symbol");
            return -1;
        }

        // Handle attributes
        if (strcmp(field, "name") == 0)
        {
            if (sym->export_name)
            {
                free(sym->export_name);
            }
            sym->export_name = strdup(rhs->lit_expr.string_val);
        }
        else
        {
            sema_error(sema, lhs->token, "unknown attribute");
            return -1;
        }

        return 0;
    }
    else if (inner->kind == AST_EXPR_CALL)
    {
        // Handle $error(...) and $assert(...)
        AstNode *func = inner->call_expr.func;
        if (func->kind == AST_EXPR_IDENT)
        {
            char *name = func->ident_expr.name;
            if (strcmp(name, "error") == 0)
            {
                // $error("message")
                if (!inner->call_expr.args || inner->call_expr.args->count != 1)
                {
                    sema_error(sema, inner->token, "expected 1 argument for $error");
                    return -1;
                }
                
                AstNode *arg = inner->call_expr.args->items[0];
                if (sema_analyze_expr(sema, arg) < 0) return -1;
                
                if (arg->kind != AST_EXPR_LIT || arg->lit_expr.kind != TOKEN_LIT_STRING)
                {
                    sema_error(sema, arg->token, "expected string literal for error message");
                    return -1;
                }
                
                sema_error(sema, inner->token, arg->lit_expr.string_val);
                return -1;
            }
            else if (strcmp(name, "assert") == 0)
            {
                // $assert(cond, "message")
                if (!inner->call_expr.args || inner->call_expr.args->count != 2)
                {
                    sema_error(sema, inner->token, "expected 2 arguments for $assert");
                    return -1;
                }
                
                AstNode *cond = inner->call_expr.args->items[0];
                AstNode *msg = inner->call_expr.args->items[1];
                
                if (sema_analyze_expr(sema, cond) < 0) return -1;
                if (sema_analyze_expr(sema, msg) < 0) return -1;
                
                int64_t val = 0;
                if (!sema_eval_comptime_int(sema, cond, &val))
                {
                    sema_error(sema, cond->token, "condition is not a compile-time constant");
                    return -1;
                }
                
                if (msg->kind != AST_EXPR_LIT || msg->lit_expr.kind != TOKEN_LIT_STRING)
                {
                    sema_error(sema, msg->token, "expected string literal for assertion message");
                    return -1;
                }
                
                if (val == 0)
                {
                    sema_error(sema, inner->token, msg->lit_expr.string_val);
                    return -1;
                }
                
                return 0;
            }
        }
    }

    // If not assignment, it's a read (e.g. $mach.os.id).
    // As a statement, it does nothing.
    // But we should analyze it to ensure it's valid.
    return sema_analyze_expr(sema, inner);
}

static int sema_analyze_stmt(Sema *sema, AstNode *node)
{
    if (!node)
    {
        return 0;
    }

    switch (node->kind)
    {
    case AST_COMPTIME:
        return sema_analyze_comptime_stmt(sema, node);

    case AST_STMT_COMPTIME_IF:
    case AST_STMT_COMPTIME_OR:
    {
        bool cond_val = true;
        if (node->comptime_if_stmt.cond)
        {
            if (sema_analyze_expr(sema, node->comptime_if_stmt.cond) < 0) return -1;
            
            int64_t val = 0;
            if (!sema_eval_comptime_int(sema, node->comptime_if_stmt.cond, &val))
            {
                sema_error(sema, node->token, "expression is not a compile-time constant");
                return -1;
            }
            cond_val = (val != 0);
        }
        
        if (cond_val)
        {
            node->comptime_if_stmt.taken_branch = node->comptime_if_stmt.body;
            return sema_analyze_stmt(sema, node->comptime_if_stmt.body);
        }
        else
        {
            node->comptime_if_stmt.taken_branch = node->comptime_if_stmt.stmt_or;
            if (node->comptime_if_stmt.stmt_or)
            {
                return sema_analyze_stmt(sema, node->comptime_if_stmt.stmt_or);
            }
        }
        return 0;
    }

    case AST_STMT_REC:
        return sema_analyze_rec(sema, node);

    case AST_STMT_FUN:
        return sema_analyze_fun(sema, node);


    case AST_STMT_VAL:
    case AST_STMT_VAR:
        return sema_analyze_var(sema, node);

    case AST_STMT_BLOCK:
        if (node->block_stmt.stmts)
        {
            for (int i = 0; i < node->block_stmt.stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, node->block_stmt.stmts->items[i]) < 0)
                {
                    return -1;
                }
            }
        }
        return 0;

    case AST_STMT_RET:
        if (node->ret_stmt.expr)
        {
            return sema_analyze_expr(sema, node->ret_stmt.expr);
        }
        return 0;

    case AST_STMT_EXPR:
        return sema_analyze_expr(sema, node->expr_stmt.expr);

    case AST_STMT_IF:
        if (sema_analyze_expr(sema, node->cond_stmt.cond) < 0)
        {
            return -1;
        }
        if (sema_analyze_stmt(sema, node->cond_stmt.body) < 0)
        {
            return -1;
        }
        if (node->cond_stmt.stmt_or)
        {
            return sema_analyze_stmt(sema, node->cond_stmt.stmt_or);
        }
        return 0;

    case AST_STMT_OR:
        if (node->cond_stmt.cond && sema_analyze_expr(sema, node->cond_stmt.cond) < 0)
        {
            return -1;
        }
        if (sema_analyze_stmt(sema, node->cond_stmt.body) < 0)
        {
            return -1;
        }
        if (node->cond_stmt.stmt_or)
        {
            return sema_analyze_stmt(sema, node->cond_stmt.stmt_or);
        }
        return 0;

    case AST_STMT_FOR:
        if (node->for_stmt.cond && sema_analyze_expr(sema, node->for_stmt.cond) < 0)
        {
            return -1;
        }
        return sema_analyze_stmt(sema, node->for_stmt.body);

    case AST_STMT_MIR:
        // inline MIR blocks are opaque to semantic analysis
        return 0;

    default:
        // other statements not implemented yet
        return 0;
    }
}

// analyze expression
static Type *sema_resolve_type(Sema *sema, AstNode *type_node);

// analyze expression
int sema_analyze_expr(Sema *sema, AstNode *node)
{
    if (!sema || !node)
    {
        return -1;
    }
    
    // printf("DEBUG: Analyze expr kind %d\n", node->kind);

    switch (node->kind)
    {
    case AST_EXPR_LIT:
        // literals have types based on their token
        if (node->token)
        {
            switch (node->token->kind)
            {
            case TOKEN_LIT_INT:
                node->type = type_get_primitive(TYPE_I64);
                break;
            case TOKEN_LIT_FLOAT:
                node->type = type_get_primitive(TYPE_F64);
                break;
            case TOKEN_LIT_STRING:
                // simplified: string is pointer to u8
                node->type = type_create_pointer(type_get_primitive(TYPE_U8), false);
                break;
            default:
                break;
            }
        }
        return 0;

    case AST_EXPR_IDENT:
    {
        // look up identifier in symbol table
        Symbol *sym = symbol_table_lookup(sema->current_table, node->ident_expr.name);

        if (!sym)
        {
            sema_error(sema, node->token, "undefined identifier");
            return -1;
        }
        node->symbol = sym;
        node->type = sym->type;
        return 0;
    }

    case AST_EXPR_BINARY:
        if (sema_analyze_expr(sema, node->binary_expr.left) < 0)
        {
            return -1;
        }
        if (sema_analyze_expr(sema, node->binary_expr.right) < 0)
        {
            return -1;
        }
        // simplified: result type is same as left operand
        node->type = node->binary_expr.left->type;
        return 0;

    case AST_EXPR_UNARY:
        if (sema_analyze_expr(sema, node->unary_expr.expr) < 0)
        {
            return -1;
        }
        
        if (node->unary_expr.op == TOKEN_QUESTION)
        {
            // Address-of: ?T -> *T
            // TODO: check if operand is an l-value (variable)
            node->type = type_create_pointer(node->unary_expr.expr->type, false);
        }
        else if (node->unary_expr.op == TOKEN_AT)
        {
            // Dereference: @T -> T
            Type *operand_type = node->unary_expr.expr->type;
            if (operand_type->kind != TYPE_POINTER)
            {
                sema_error(sema, node->token, "cannot dereference non-pointer type");
                return -1;
            }
            node->type = operand_type->pointer.base;
        }
        else
        {
            // other unary ops (-, !, etc.) preserve type
            node->type = node->unary_expr.expr->type;
        }
        return 0;

    case AST_EXPR_CALL:
    {
        // analyze function
        if (sema_analyze_expr(sema, node->call_expr.func) < 0)
        {
            return -1;
        }

        // analyze arguments
        if (node->call_expr.args)
        {
            for (int i = 0; i < node->call_expr.args->count; i++)
            {
                if (sema_analyze_expr(sema, node->call_expr.args->items[i]) < 0)
                {
                    return -1;
                }
            }
        }

        Type *func_type = node->call_expr.func->type;
        Symbol *func_sym = node->call_expr.func->symbol;

        // Handle generics
        if (func_sym && func_sym->is_generic)
        {
            
            if (!node->call_expr.type_args)
            {
                sema_error(sema, node->token, "generic function call requires type arguments");
                return -1;
            }
            
            Symbol *inst = sema_instantiate_generic(sema, func_sym, node->call_expr.type_args);
            if (!inst)
            {
                sema_error(sema, node->token, "failed to instantiate generic function");
                return -1;
            }
            
            // Update call target to instantiated symbol
            node->call_expr.func->symbol = inst;
            node->call_expr.func->type = inst->type;
            func_type = inst->type;
        }

        if (!func_type || func_type->kind != TYPE_FUNCTION)
        {
            sema_error(sema, node->token, "calling non-function type");
            return -1;
        }

        int arg_count = node->call_expr.args ? node->call_expr.args->count : 0;
        int param_count = func_type->function.param_count;

        // check argument count

        // check argument count
        if (arg_count != param_count)
        {
            if (func_type->function.param_count > 0 && 
                func_type->function.param_types[func_type->function.param_count - 1] == NULL) // Variadic check
            {
                 if (arg_count < param_count - 1)
                 {
                     sema_error(sema, node->token, "too few arguments to function call");
                     return -1;
                 }
            }
            else
            {
                sema_error(sema, node->token, "argument count mismatch");
                return -1;
            }
        }

        // check argument types
        for (int i = 0; i < arg_count; i++)
        {
            AstNode *arg = node->call_expr.args->items[i];
            
            // For variadic functions, stop checking fixed params
            if (i >= param_count) break;
            
            Type *param_type = func_type->function.param_types[i];
            
            // Variadic sentinel check
            if (param_type == NULL) break; 

            if (!type_equals(arg->type, param_type))
            {
                // check for implicit casts (e.g. &T -> *T)
                if (param_type->kind == TYPE_POINTER && arg->type->kind == TYPE_POINTER)
                {
                    // allow &T -> *T (const to mutable is unsafe but allowed for now?)
                    // actually, allow *T -> &T (mutable to const)
                    if (type_equals(param_type->pointer.base, arg->type->pointer.base))
                    {
                         if (param_type->pointer.is_const && !arg->type->pointer.is_const)
                         {
                             // *T -> &T ok
                             continue;
                         }
                    }
                }
                
                // Allow *void -> *T and *T -> *void
                if (param_type->kind == TYPE_PTR || arg->type->kind == TYPE_PTR)
                {
                    continue;
                }
                
                sema_error(sema, arg->token, "argument type mismatch");
                return -1;
            }
        }

        node->type = func_type->function.return_type;
        return 0;
    }

    case AST_EXPR_FIELD:
    {
        if (sema_analyze_expr(sema, node->field_expr.object) < 0)
        {
            return -1;
        }

        Type *obj_type = node->field_expr.object->type;
        
        // auto-dereference pointer to struct
        if (obj_type->kind == TYPE_POINTER && obj_type->pointer.base->kind == TYPE_STRUCT)
        {
            obj_type = obj_type->pointer.base;
        }

        if (obj_type->kind != TYPE_STRUCT)
        {
            sema_error(sema, node->token, "field access on non-struct type");
            return -1;
        }

        // find field
        TypeField *field = NULL;
        for (int i = 0; i < obj_type->structure.field_count; i++)
        {
            if (strcmp(obj_type->structure.fields[i].name, node->field_expr.field) == 0)
            {
                field = &obj_type->structure.fields[i];
                break;
            }
        }

        if (!field)
        {
            sema_error(sema, node->token, "undefined field");
            return -1;
        }

        node->type = field->type;
        return 0;
    }

    case AST_EXPR_STRUCT:
    {
        Type *type = sema_resolve_type(sema, node->struct_expr.type);
        if (!type || type->kind != TYPE_STRUCT)
        {
            sema_error(sema, node->token, "invalid struct type");
            return -1;
        }

        // verify fields
        if (node->struct_expr.fields)
        {
            for (int i = 0; i < node->struct_expr.fields->count; i++)
            {
                AstNode *field_init = node->struct_expr.fields->items[i];
                // field_init is AST_EXPR_FIELD (field: name, object: init_expr)
                
                // find field in struct type
                TypeField *field = NULL;
                for (int j = 0; j < type->structure.field_count; j++)
                {
                    if (strcmp(type->structure.fields[j].name, field_init->field_expr.field) == 0)
                    {
                        field = &type->structure.fields[j];
                        break;
                    }
                }

                if (!field)
                {
                    sema_error(sema, field_init->token, "undefined field in struct literal");
                    return -1;
                }

                // analyze init expression
                if (sema_analyze_expr(sema, field_init->field_expr.object) < 0)
                {
                    return -1;
                }

                if (!type_equals(field->type, field_init->field_expr.object->type))
                {
                    sema_error(sema, field_init->token, "field type mismatch");
                    return -1;
                }
            }
        }
        
        // TODO: Check for missing fields?

        node->type = type;
        return 0;
    }

    case AST_EXPR_CAST:
    {
        if (sema_analyze_expr(sema, node->cast_expr.expr) < 0)
        {
            return -1;
        }
        
        Type *target_type = sema_resolve_type(sema, node->cast_expr.type);
        if (!target_type)
        {
            sema_error(sema, node->token, "invalid cast type");
            return -1;
        }
        
        node->type = target_type;
        return 0;
    }

    case AST_EXPR_INDEX:
    {
        if (sema_analyze_expr(sema, node->index_expr.array) < 0)
        {
            return -1;
        }
        // Check for generic instantiation
        Symbol *sym = node->index_expr.array->symbol;

        if (sym && sym->is_generic)
        {
            // Resolve type arg
            // We create a temporary list for now (single arg)
            AstList *type_args = malloc(sizeof(AstList));
            if (type_args)
            {
                ast_list_init(type_args);
                ast_list_append(type_args, node->index_expr.index);
                
                Symbol *inst = sema_instantiate_generic(sema, sym, type_args);
                
                // Cleanup list shell (items are owned by AST)
                free(type_args);
                
                if (!inst)
                {
                    sema_error(sema, node->token, "failed to instantiate generic");
                    return -1;
                }
                
                node->kind = AST_EXPR_IDENT;
                node->ident_expr.name = strdup(inst->name);
                node->symbol = inst;
                node->type = inst->type;
                
                return 0;
            }
        }

        if (sema_analyze_expr(sema, node->index_expr.index) < 0)
        {
            return -1;
        }

        Type *obj_type = node->index_expr.array->type;
        Type *index_type = node->index_expr.index->type;

        // Check index type (must be integer)
        if (index_type->kind != TYPE_I64 && index_type->kind != TYPE_U64 &&
            index_type->kind != TYPE_I32 && index_type->kind != TYPE_U32 &&
            index_type->kind != TYPE_I16 && index_type->kind != TYPE_U16 &&
            index_type->kind != TYPE_I8 && index_type->kind != TYPE_U8)
        {
            sema_error(sema, node->index_expr.index->token, "array index must be an integer");
            return -1;
        }

        // Check object type (array or pointer)
        if (!obj_type)
        {
             sema_error(sema, node->token, "indexing on unknown type");
             return -1;
        }

        if (obj_type->kind == TYPE_ARRAY)
        {
            node->type = obj_type->array.elem_type;
        }
        else if (obj_type->kind == TYPE_POINTER)
        {
            node->type = obj_type->pointer.base;
        }
        else
        {
            sema_error(sema, node->token, "indexing on non-array/pointer type");
            return -1;
        }

        return 0;
    }
    
    case AST_COMPTIME:
    {
        // Evaluate compile-time expression
        AstNode *inner = node->comptime.inner;
        
        // Try to resolve as $mach constant first
        if (comptime_lookup(sema, node) == 0)
        {
            return 0;
        }
        
        // Handle $Symbol.attribute pattern
        if (inner && inner->kind == AST_EXPR_FIELD)
        {
            AstNode *obj = inner->field_expr.object;
            if (obj->kind != AST_EXPR_IDENT)
            {
                sema_error(sema, node->token, "compiletime attribute access requires symbol name");
                return -1;
            }
            
            // Look up symbol
            Symbol *sym = symbol_table_lookup(sema->current_table, obj->ident_expr.name);
            if (!sym)
            {
                sema_error(sema, obj->token, "undefined symbol in compiletime expression");
                return -1;
            }
            
            const char *attr_name = inner->field_expr.field;
            
            // Evaluate attribute
            if (strcmp(attr_name, "name") == 0)
            {
                // Return symbol name as string
                node->comptime.value_kind = COMPTIME_STRING;
                node->comptime.string_value = strdup(obj->ident_expr.name);
                node->type = type_get_primitive(TYPE_PTR); // &u8
                return 0;
            }
            else if (strcmp(attr_name, "size") == 0)
            {
                // Return type size as integer
                node->comptime.value_kind = COMPTIME_INT;
                node->comptime.int_value = sym->type ? sym->type->size : 0;
                node->type = type_get_primitive(TYPE_I64);
                return 0;
            }
            else if (strcmp(attr_name, "align") == 0)
            {
                // Return type alignment (use size as alignment for now)
                node->comptime.value_kind = COMPTIME_INT;
                node->comptime.int_value = sym->type ? sym->type->size : 0;
                node->type = type_get_primitive(TYPE_I64);
                return 0;
            }
            else if (strcmp(attr_name, "field_count") == 0)
            {
                // Return number of fields (record types only)
                if (sym->type && sym->type->kind == TYPE_STRUCT)
                {
                    node->comptime.value_kind = COMPTIME_INT;
                    node->comptime.int_value = sym->type->structure.field_count;
                    node->type = type_get_primitive(TYPE_I64);
                    return 0;
                }
                sema_error(sema, node->token, "field_count only valid for record types");
                return -1;
            }
            else
            {
                sema_error(sema, node->token, "unknown compiletime attribute");
                return -1;
            }
        }
        
        sema_error(sema, node->token, "unsupported compiletime expression");
        return -1;
    }

    default:
        // other expressions not implemented yet
        return 0;
    }
}

// resolve type from AST type node
static Type *sema_resolve_type(Sema *sema, AstNode *type_node)
{
    if (!type_node)
    {
        return NULL;
    }

    switch (type_node->kind)
    {
    case AST_TYPE_NAME:
    case AST_EXPR_IDENT:
    {
        const char *name = (type_node->kind == AST_TYPE_NAME) ? type_node->type_name.name : type_node->ident_expr.name;
        
        // check primitive types
        if (strcmp(name, "i8") == 0) return type_get_primitive(TYPE_I8);
        if (strcmp(name, "i16") == 0) return type_get_primitive(TYPE_I16);
        if (strcmp(name, "i32") == 0) return type_get_primitive(TYPE_I32);
        if (strcmp(name, "i64") == 0) return type_get_primitive(TYPE_I64);
        if (strcmp(name, "u8") == 0) return type_get_primitive(TYPE_U8);
        if (strcmp(name, "u16") == 0) return type_get_primitive(TYPE_U16);
        if (strcmp(name, "u32") == 0) return type_get_primitive(TYPE_U32);
        if (strcmp(name, "u64") == 0) return type_get_primitive(TYPE_U64);
        if (strcmp(name, "f32") == 0) return type_get_primitive(TYPE_F32);
        if (strcmp(name, "f64") == 0) return type_get_primitive(TYPE_F64);
        if (strcmp(name, "ptr") == 0) return type_get_primitive(TYPE_PTR);
        if (strcmp(name, "bool") == 0) return type_get_primitive(TYPE_U8); // bool is u8
        
        // look up user-defined types
        Symbol *sym = symbol_table_lookup(sema->current_table, name);
        if (sym && sym->type)
        {
            return sym->type;
        }
        
        return NULL;
    }

    case AST_TYPE_PTR:
    {
        Type *base = sema_resolve_type(sema, type_node->type_ptr.base);
        if (!base) return NULL;
        return type_create_pointer(base, type_node->type_ptr.is_read_only);
    }

    case AST_TYPE_ARRAY:
    {
        Type *elem = sema_resolve_type(sema, type_node->type_array.elem_type);
        if (!elem) return NULL;
        
        // Evaluate size
        AstNode *size_expr = type_node->type_array.size;
        // Simple check for integer literal
        if (size_expr->kind == AST_EXPR_LIT && size_expr->token->kind == TOKEN_LIT_INT)
        {
            size_t count = (size_t)size_expr->lit_expr.int_val;
            return type_create_array(elem, count);
        }
        else
        {
            sema_error(sema, size_expr->token, "array size must be a constant integer");
            return NULL;
        }
    }

    default:
        return NULL;
    }
}

// analyze program root
int sema_analyze(Sema *sema, AstNode *ast)
{
    if (!sema || !ast)
    {
        return -1;
    }

    if (ast->kind == AST_PROGRAM)
    {
        if (ast->program.stmts)
        {
            for (int i = 0; i < ast->program.stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, ast->program.stmts->items[i]) < 0)
                {
                    // continue analyzing to find more errors
                }
            }
        }
    }
    else if (ast->kind == AST_MODULE)
    {
        if (ast->module.stmts)
        {
            for (int i = 0; i < ast->module.stmts->count; i++)
            {
                if (sema_analyze_stmt(sema, ast->module.stmts->items[i]) < 0)
                {
                    // continue analyzing to find more errors
                }
            }
        }
    }

    return sema->error_count > 0 ? -1 : 0;
}

// Instantiate a generic function with type arguments
Symbol *sema_instantiate_generic(Sema *sema, Symbol *generic_sym, AstList *type_args)
{
    if (!sema || !generic_sym || !type_args)
    {
        return NULL;
    }

    if (!generic_sym->is_generic || !generic_sym->decl)
    {
        return NULL;
    }

    AstNode *decl = generic_sym->decl;
    AstList *generic_params = NULL;
    
    if (decl->kind == AST_STMT_FUN)
    {
        generic_params = decl->fun_stmt.generics;
    }
    // TODO: Handle struct/union generics
    
    if (!generic_params)
    {
        return NULL;
    }
    
    if (generic_params->count != type_args->count)
    {
        return NULL;
    }

    // 1. Generate mangled name for this instantiation
    // Format: original_name + "_inst" + type_arg_names...
    // This is a simplified mangling for now
    char mangled_name[256];
    snprintf(mangled_name, sizeof(mangled_name), "%s_inst", generic_sym->name);
    
    for (int i = 0; i < type_args->count; i++)
    {
        // Append simplified type representation
        // Ideally we should use a proper type mangler
        strncat(mangled_name, "_T", sizeof(mangled_name) - strlen(mangled_name) - 1);
    }

    // 2. Check if already instantiated
    Symbol *inst = symbol_table_lookup(sema->root_table, mangled_name);
    if (inst)
    {
        return inst;
    }

    // 3. Clone the AST
    AstNode *cloned_decl = ast_clone(decl);
    if (!cloned_decl)
    {
        return NULL;
    }

    // Rename the cloned declaration
    if (cloned_decl->kind == AST_STMT_FUN)
    {
        free(cloned_decl->fun_stmt.name);
        cloned_decl->fun_stmt.name = strdup(mangled_name);
        // Clear generics list on clone so it's treated as a normal function
        // (We don't free the list itself as it's a shallow copy of the list structure, 
        // but we set it to NULL so sema treats it as non-generic)
        cloned_decl->fun_stmt.generics = NULL; 
    }

    // 4. Analyze the cloned AST
    
    // Create symbol manually with mangled name.
    Symbol *inst_sym = symbol_create(mangled_name, SYMBOL_FUNCTION, sema->module_path);
    inst_sym->decl = cloned_decl;  // Link symbol to cloned AST for MIR lowering
    symbol_table_insert(sema->root_table, inst_sym);
    cloned_decl->symbol = inst_sym;
    
    // Create scope for instantiation (used for return type, params, and body)
    SymbolTable *scope = symbol_table_create(sema->root_table);
    SymbolTable *prev_table = sema->current_table;
    
    // Bind generic params
    for (int i = 0; i < generic_params->count; i++)
    {
        AstNode *param_node = generic_params->items[i];
        AstNode *arg_node = type_args->items[i];
         if (param_node->kind == AST_TYPE_PARAM)
        {
            Type *arg_type = sema_resolve_type(sema, arg_node); // Resolve in current context (caller)
            // Note: arg_node might need to be resolved in caller's context, 
            // but here we are using sema which has current_table set to caller's scope (or whatever it was).
            // Yes, sema->current_table is restored at end of this function.
            
            Symbol *type_sym = symbol_create(param_node->type_param.name, SYMBOL_TYPE, sema->module_path);
            type_sym->type = arg_type;
            symbol_table_insert(scope, type_sym);
        }
    }
    
    sema->current_table = scope;
    
    // Resolve return type
    Type *ret_type = NULL;
    if (cloned_decl->fun_stmt.return_type)
    {
        ret_type = sema_resolve_type(sema, cloned_decl->fun_stmt.return_type);
        if (!ret_type)
        {
             // Error handling?
             // sema_error(sema, ...);
        }
    }
    
    // Resolve parameter types
    Type **param_types = NULL;
    int param_count = 0;
    
    if (cloned_decl->fun_stmt.params)
    {
        param_count = cloned_decl->fun_stmt.params->count;
        param_types = malloc(sizeof(Type*) * param_count);
        
        for (int i = 0; i < param_count; i++)
        {
            AstNode *param = cloned_decl->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Type *pt = NULL;
                if (param->param_stmt.type)
                {
                    pt = sema_resolve_type(sema, param->param_stmt.type);
                }
                param_types[i] = pt;
                param->type = pt;
            }
        }
    }
    
    inst_sym->type = type_create_function(ret_type, param_types, param_count);
    
    // Add params to scope (as variables)
    if (cloned_decl->fun_stmt.params)
    {
         for (int i = 0; i < cloned_decl->fun_stmt.params->count; i++)
        {
            AstNode *param = cloned_decl->fun_stmt.params->items[i];
            if (param->kind == AST_STMT_PARAM)
            {
                Symbol *param_sym = symbol_create(param->param_stmt.name, SYMBOL_VARIABLE, sema->module_path);
                param_sym->type = param->type;
                symbol_table_insert(sema->current_table, param_sym);
                param->symbol = param_sym;
                param_sym->decl = param;
            }
        }
    }
    
    // Analyze body
    if (cloned_decl->fun_stmt.body)
    {
        sema_analyze_stmt(sema, cloned_decl->fun_stmt.body);
    }
    
    sema->current_table = prev_table;

    return inst_sym;
}
