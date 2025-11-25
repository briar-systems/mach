#include "compiler/sema.h"
#include "compiler/type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// semantic analyzer context
struct Sema
{
    SymbolTable *root_table;
    SymbolTable *current_table;
    int          error_count;
};

Sema *sema_create()
{
    Sema *sema = malloc(sizeof(Sema));
    if (!sema)
    {
        return NULL;
    }

    sema->root_table = symbol_table_create(NULL);
    sema->current_table = sema->root_table;
    sema->error_count = 0;

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
    Symbol *sym = symbol_create(node->fun_stmt.name, SYMBOL_FUNCTION);
    if (!sym)
    {
        return -1;
    }

    // resolve return type
    if (node->fun_stmt.return_type)
    {
        Type *ret_type = sema_resolve_type(sema, node->fun_stmt.return_type);
        if (!ret_type)
        {
            sema_error(sema, node->token, "failed to resolve return type");
            symbol_destroy(sym);
            return -1;
        }
        sym->type = ret_type;
    }

    // add function to symbol table
    if (symbol_table_insert(sema->current_table, sym) < 0)
    {
        sema_error(sema, node->token, "duplicate function declaration");
        symbol_destroy(sym);
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
                    Symbol *param_sym = symbol_create(param->param_stmt.name, SYMBOL_VARIABLE);
                    if (param->param_stmt.type)
                    {
                        param_sym->type = sema_resolve_type(sema, param->param_stmt.type);
                    }
                    symbol_table_insert(sema->current_table, param_sym);
                    param->symbol = param_sym;
                }
            }
        }

        sema_analyze_stmt(sema, node->fun_stmt.body);

        sema->current_table = prev_table;
    }

    return 0;
}

// analyze variable/value declaration
static int sema_analyze_var(Sema *sema, AstNode *node)
{
    if (node->kind != AST_STMT_VAR && node->kind != AST_STMT_VAL)
    {
        return -1;
    }

    Symbol *sym = symbol_create(node->var_stmt.name, SYMBOL_VARIABLE);
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

    return 0;
}

// analyze statement
static int sema_analyze_stmt(Sema *sema, AstNode *node)
{
    if (!node)
    {
        return 0;
    }

    switch (node->kind)
    {
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
static int sema_analyze_expr(Sema *sema, AstNode *node)
{
    if (!node)
    {
        return 0;
    }

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
                node->type = type_get_primitive(TYPE_PTR);
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
        node->type = node->unary_expr.expr->type;
        return 0;

    case AST_EXPR_CALL:
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
        // result type is return type of function
        if (node->call_expr.func->symbol && node->call_expr.func->symbol->type)
        {
            node->type = node->call_expr.func->symbol->type;
        }
        return 0;

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
    {
        const char *name = type_node->type_name.name;
        
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
        
        // look up user-defined types
        Symbol *sym = symbol_table_lookup(sema->current_table, name);
        if (sym && sym->type)
        {
            return sym->type;
        }
        
        return NULL;
    }

    case AST_TYPE_PTR:
        // pointer type - simplified implementation
        return type_get_primitive(TYPE_PTR);

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
