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
                    param_sym->decl = param;
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

    Symbol *sym = symbol_create(node->rec_stmt.name, SYMBOL_TYPE);
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
