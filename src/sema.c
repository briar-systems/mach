#include "sema.h"
#include "type.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool init_type_cache(TypeCache *cache)
{
    cache->void_type = type_make(TYPE_VOID);
    cache->u8_type = type_make(TYPE_U8);
    cache->u16_type = type_make(TYPE_U16);
    cache->u32_type = type_make(TYPE_U32);
    cache->u64_type = type_make(TYPE_U64);
    cache->i8_type = type_make(TYPE_I8);
    cache->i16_type = type_make(TYPE_I16);
    cache->i32_type = type_make(TYPE_I32);
    cache->i64_type = type_make(TYPE_I64);
    cache->f32_type = type_make(TYPE_F32);
    cache->f64_type = type_make(TYPE_F64);

    return cache->void_type && cache->u8_type && cache->u16_type && cache->u32_type && cache->u64_type && cache->i8_type && cache->i16_type && cache->i32_type && cache->i64_type && cache->f32_type && cache->f64_type;
}

void free_type_cache(TypeCache *cache)
{
    type_free(cache->void_type);
    type_free(cache->u8_type);
    type_free(cache->u16_type);
    type_free(cache->u32_type);
    type_free(cache->u64_type);
    type_free(cache->i8_type);
    type_free(cache->i16_type);
    type_free(cache->i32_type);
    type_free(cache->i64_type);
    type_free(cache->f32_type);
    type_free(cache->f64_type);
}

bool sema_init(SEMAContext *sema, Target target)
{
    if (!sema)
    {
        return false;
    }

    sema->scope_current = calloc(sizeof(Scope), 1);
    if (!scope_init(sema->scope_current, NULL))
    {
        return false;
    }

    sema->type_table = calloc(sizeof(NodeTable), 1);
    if (!node_table_init(sema->type_table))
    {
        scope_free(sema->scope_current);
        return false;
    }

    sema->const_table = calloc(sizeof(NodeTable), 1);
    if (!node_table_init(sema->const_table))
    {
        node_table_free(sema->type_table);
        scope_free(sema->scope_current);
        return false;
    }

    sema->errors.errors = NULL;
    sema->errors.len = 0;
    sema->errors.cap = 0;

    if (!init_type_cache(&sema->type_cache))
    {
        node_table_free(sema->const_table);
        node_table_free(sema->type_table);
        scope_free(sema->scope_current);
        return false;
    }

    sema->in_const_context = false;
    sema->in_loop_context = false;
    sema->in_function_context = false;
    sema->current_function_return_type = NULL;

    sema->target = target;

    return true;
}

void sema_free(SEMAContext *sema)
{
    if (!sema)
    {
        return;
    }

    scope_free(sema->scope_current);
    sema->scope_current = NULL;

    node_table_free(sema->type_table);
    sema->type_table = NULL;

    node_table_free(sema->const_table);
    sema->const_table = NULL;

    free(sema->errors.errors);
    sema->errors.errors = NULL;

    free_type_cache(&sema->type_cache);

    free(sema);
}

void sema_report_error(SEMAContext *sema, Token token, const char *format, ...)
{
    if (sema->errors.len >= sema->errors.cap)
    {
        int new_capacity = sema->errors.cap == 0 ? 16 : sema->errors.cap * 2;
        SEMAError *new_errors = realloc(sema->errors.errors, new_capacity * sizeof(SEMAError));
        if (!new_errors)
        {
            fprintf(stderr, "error: failed to allocate memory for error list\n");
            return;
        }
        sema->errors.errors = new_errors;
        sema->errors.cap = new_capacity;
    }

    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy) + 1;
    va_end(args_copy);

    char *buffer = malloc(size);
    if (buffer)
    {
        vsnprintf(buffer, size, format, args);
    }
    va_end(args);

    SEMAError *error = &sema->errors.errors[sema->errors.len++];
    error->token = token;
    error->message = buffer ? buffer : "memory allocation failed for error message";
}

void sema_print_errors(SEMAContext *sema, Lexer *lexer, const char *filename)
{
    for (int i = 0; i < sema->errors.len; i++)
    {
        SEMAError error = sema->errors.errors[i];

        int line = lexer_line_at(lexer, error.token.start);
        int column = lexer_column_at(lexer, error.token.start);

        fprintf(stderr, "%s:%d:%d: error: %s\n", filename, line, column, error.message);
    }
}

Type *sema_get_type(SEMAContext *sema, Node *node)
{
    if (!sema || !node)
    {
        return NULL;
    }

    return node_table_get(sema->type_table, node);
}

bool sema_set_type(SEMAContext *sema, Node *node, Type *type)
{
    if (!sema || !node || !type)
    {
        return false;
    }

    return node_table_set(sema->type_table, node, type);
}

bool sema_is_constant(SEMAContext *sema, Node *node)
{
    if (!sema || !node)
    {
        return false;
    }

    return node_table_get(sema->const_table, node) != NULL;
}

Node *sema_get_constant(SEMAContext *sema, Node *node)
{
    if (!sema || !node)
    {
        return NULL;
    }

    return node_table_get(sema->const_table, node);
}

bool sema_set_constant(SEMAContext *sema, Node *node, Node *constant_value)
{
    if (!sema || !node || !constant_value)
    {
        return false;
    }

    return node_table_set(sema->const_table, node, constant_value);
}

Type *sema_get_builtin_type(SEMAContext *sema, const char *name)
{
    if (!sema || !name)
    {
        return NULL;
    }

    if (strcmp(name, "void") == 0)
        return sema->type_cache.void_type;
    if (strcmp(name, "u8") == 0)
        return sema->type_cache.u8_type;
    if (strcmp(name, "u16") == 0)
        return sema->type_cache.u16_type;
    if (strcmp(name, "u32") == 0)
        return sema->type_cache.u32_type;
    if (strcmp(name, "u64") == 0)
        return sema->type_cache.u64_type;
    if (strcmp(name, "i8") == 0)
        return sema->type_cache.i8_type;
    if (strcmp(name, "i16") == 0)
        return sema->type_cache.i16_type;
    if (strcmp(name, "i32") == 0)
        return sema->type_cache.i32_type;
    if (strcmp(name, "i64") == 0)
        return sema->type_cache.i64_type;
    if (strcmp(name, "f32") == 0)
        return sema->type_cache.f32_type;
    if (strcmp(name, "f64") == 0)
        return sema->type_cache.f64_type;

    return NULL;
}

bool sema_types_compatible(Type *a, Type *b)
{
    if (!a || !b)
    {
        return false;
    }

    if (type_equals(a, b))
    {
        return true;
    }

    if ((a->kind >= TYPE_U8 && a->kind <= TYPE_F64) && (b->kind >= TYPE_U8 && b->kind <= TYPE_F64))
    {
        if ((a->kind >= TYPE_U8 && a->kind <= TYPE_I64) && (b->kind >= TYPE_U8 && b->kind <= TYPE_I64))
        {
            return true;
        }

        if ((a->kind >= TYPE_U8 && a->kind <= TYPE_I64) && (b->kind == TYPE_F32 || b->kind == TYPE_F64))
        {
            return true;
        }

        if ((a->kind == TYPE_F32 || a->kind == TYPE_F64) && (b->kind >= TYPE_U8 && b->kind <= TYPE_I64))
        {
            return true;
        }

        if ((a->kind == TYPE_F32 || a->kind == TYPE_F64) && (b->kind == TYPE_F32 || b->kind == TYPE_F64))
        {
            return true;
        }
    }

    if (a->kind == TYPE_PTR && b->kind == TYPE_PTR)
    {
        if (a->ptr.target->kind == TYPE_VOID || b->ptr.target->kind == TYPE_VOID)
        {
            return true;
        }

        return sema_types_compatible(a->ptr.target, b->ptr.target);
    }

    if (a->kind == TYPE_ARR && b->kind == TYPE_ARR)
    {
        if (!sema_types_compatible(a->arr.element_type, b->arr.element_type))
        {
            return false;
        }

        if (a->arr.is_slice && b->arr.is_slice)
        {
            return true;
        }
        
        if (a->arr.is_slice && !b->arr.is_slice)
        {
            return true;
        }

        if (a->arr.len == b->arr.len)
        {
            return true;
        }

        return false;
    }

    if (a->kind == TYPE_FUN && b->kind == TYPE_FUN)
    {
        if (!sema_types_compatible(a->fun.return_type, b->fun.return_type))
        {
            return false;
        }

        if (a->fun.param_count != b->fun.param_count)
        {
            return false;
        }

        for (int i = 0; i < a->fun.param_count; i++)
        {
            if (!sema_types_compatible(a->fun.param_types[i], b->fun.param_types[i]))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool sema_can_convert_type(Type *from, Type *to) { return sema_types_compatible(from, to); }

Type *sema_common_type(SEMAContext *sema, Type *a, Type *b)
{
    if (!a || !b)
    {
        return NULL;
    }

    if (type_equals(a, b))
    {
        return a;
    }

    if ((a->kind >= TYPE_U8 && a->kind <= TYPE_F64) && (b->kind >= TYPE_U8 && b->kind <= TYPE_F64))
    {

        // if either is a float, the result is the larger float
        if (a->kind == TYPE_F64 || b->kind == TYPE_F64)
        {
            return sema->type_cache.f64_type;
        }

        if (a->kind == TYPE_F32 || b->kind == TYPE_F32)
        {
            return sema->type_cache.f32_type;
        }

        // both are integers, use the larger signed type
        if (a->kind == TYPE_I64 || b->kind == TYPE_I64)
        {
            return sema->type_cache.i64_type;
        }

        if (a->kind == TYPE_U64 || b->kind == TYPE_U64)
        {
            return sema->type_cache.u64_type;
        }

        if (a->kind == TYPE_I32 || b->kind == TYPE_I32)
        {
            return sema->type_cache.i32_type;
        }

        if (a->kind == TYPE_U32 || b->kind == TYPE_U32)
        {
            return sema->type_cache.u32_type;
        }

        if (a->kind == TYPE_I16 || b->kind == TYPE_I16)
        {
            return sema->type_cache.i16_type;
        }

        if (a->kind == TYPE_U16 || b->kind == TYPE_U16)
        {
            return sema->type_cache.u16_type;
        }

        if (a->kind == TYPE_I8 || b->kind == TYPE_I8)
        {
            return sema->type_cache.i8_type;
        }

        return sema->type_cache.u8_type;
    }

    if (a->kind == TYPE_PTR && b->kind == TYPE_PTR)
    {
        // if one is void*, use the other type
        if (a->ptr.target->kind == TYPE_VOID)
        {
            return b;
        }

        if (b->ptr.target->kind == TYPE_VOID)
        {
            return a;
        }

        // otherwise, if they're compatible, use the first
        if (sema_types_compatible(a, b))
        {
            return a;
        }
    }

    return NULL;
}

void sema_enter_scope(SEMAContext *sema)
{
    if (!sema)
    {
        return;
    }

    Scope *new_scope = calloc(sizeof(Scope), 1);
    if (!new_scope)
    {
        return;
    }

    if (!scope_init(new_scope, sema->scope_current))
    {
        free(new_scope);
        return;
    }

    sema->scope_current = new_scope;
}

void sema_exit_scope(SEMAContext *sema)
{
    if (!sema || !sema->scope_current || !sema->scope_current->parent)
    {
        return;
    }

    Scope *parent = sema->scope_current->parent;

    scope_free(sema->scope_current);

    sema->scope_current = parent;
}

Node *evaluate_constant_binary(Node *left, Node *right, Operator op)
{
    if (!left || !right)
    {
        return NULL;
    }

    if (left->kind == NODE_LIT_INT && right->kind == NODE_LIT_INT)
    {
        int64_t lval = left->lit_int.value;
        int64_t rval = right->lit_int.value;
        int64_t result = 0;

        switch (op)
        {
        case OP_ADD:
            result = lval + rval;
            break;
        case OP_SUB:
            result = lval - rval;
            break;
        case OP_MUL:
            result = lval * rval;
            break;
        case OP_DIV:
            if (rval == 0)
            {
                return NULL; // division by zero
            }
            result = lval / rval;
            break;
        case OP_MOD:
            if (rval == 0)
            {
                return NULL; // division by zero
            }
            result = lval % rval;
            break;
        case OP_BITWISE_AND:
            result = lval & rval;
            break;
        case OP_BITWISE_OR:
            result = lval | rval;
            break;
        case OP_BITWISE_XOR:
            result = lval ^ rval;
            break;
        case OP_BITWISE_SHL:
            result = lval << rval;
            break;
        case OP_BITWISE_SHR:
            result = lval >> rval;
            break;
        case OP_EQUAL:
            result = lval == rval;
            break;
        case OP_NOT_EQUAL:
            result = lval != rval;
            break;
        case OP_LESS:
            result = lval < rval;
            break;
        case OP_LESS_EQUAL:
            result = lval <= rval;
            break;
        case OP_GREATER:
            result = lval > rval;
            break;
        case OP_GREATER_EQUAL:
            result = lval >= rval;
            break;
        case OP_LOGICAL_AND:
            result = lval && rval;
            break;
        case OP_LOGICAL_OR:
            result = lval || rval;
            break;
        default:
            return NULL; // unsupported operator for integers
        }

        Node *result_node = calloc(sizeof(Node), 1);
        if (!result_node)
        {
            return NULL;
        }

        node_init(result_node, NODE_LIT_INT, left->token);
        result_node->lit_int.value = result;
        return result_node;
    }

    if ((left->kind == NODE_LIT_FLOAT || left->kind == NODE_LIT_INT) && (right->kind == NODE_LIT_FLOAT || right->kind == NODE_LIT_INT))
    {

        double lval = left->kind == NODE_LIT_FLOAT ? left->lit_float.value : (double)left->lit_int.value;
        double rval = right->kind == NODE_LIT_FLOAT ? right->lit_float.value : (double)right->lit_int.value;
        double result = 0;

        switch (op)
        {
        case OP_ADD:
            result = lval + rval;
            break;
        case OP_SUB:
            result = lval - rval;
            break;
        case OP_MUL:
            result = lval * rval;
            break;
        case OP_DIV:
            if (rval == 0)
                return NULL; // division by zero
            result = lval / rval;
            break;
        case OP_EQUAL:
            result = lval == rval;
            break;
        case OP_NOT_EQUAL:
            result = lval != rval;
            break;
        case OP_LESS:
            result = lval < rval;
            break;
        case OP_LESS_EQUAL:
            result = lval <= rval;
            break;
        case OP_GREATER:
            result = lval > rval;
            break;
        case OP_GREATER_EQUAL:
            result = lval >= rval;
            break;
        case OP_LOGICAL_AND:
            result = lval && rval;
            break;
        case OP_LOGICAL_OR:
            result = lval || rval;
            break;
        default:
            return NULL; // unsupported operator for floats
        }

        Node *result_node = calloc(sizeof(Node), 1);
        if (!result_node)
        {
            return NULL;
        }

        node_init(result_node, NODE_LIT_FLOAT, left->token);
        result_node->lit_float.value = result;
        return result_node;
    }

    return NULL; // unsupported operation or types
}

Node *evaluate_constant_unary(Node *operand, Operator op)
{
    if (!operand)
    {
        return NULL;
    }

    if (operand->kind == NODE_LIT_INT)
    {
        int64_t val = operand->lit_int.value;
        int64_t result = 0;

        switch (op)
        {
        case OP_SUB:
            result = -val;
            break;
        case OP_BITWISE_NOT:
            result = ~val;
            break;
        case OP_LOGICAL_NOT:
            result = !val;
            break;
        default:
            return NULL; // unsupported operator for ints
        }

        Node *result_node = calloc(sizeof(Node), 1);
        if (!result_node)
        {
            return NULL;
        }

        node_init(result_node, NODE_LIT_INT, operand->token);
        result_node->lit_int.value = result;
        return result_node;
    }

    if (operand->kind == NODE_LIT_FLOAT)
    {
        double val = operand->lit_float.value;
        double result = 0;

        switch (op)
        {
        case OP_SUB:
            result = -val;
            break;
        case OP_LOGICAL_NOT:
            result = !val;
            break;
        default:
            return NULL; // unsupported operator for floats
        }

        Node *result_node = calloc(sizeof(Node), 1);
        if (!result_node)
        {
            return NULL;
        }

        node_init(result_node, NODE_LIT_FLOAT, operand->token);
        result_node->lit_float.value = result;
        return result_node;
    }

    return NULL; // unsupported operation or type
}

void visit_module(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    if (node->module.stmts)
    {
        for (int i = 0; i < node->module.stmts->len; i++)
        {
            Node *stmt = node_list_get(node->module.stmts, i);

            // check valid statements at module root
            switch (stmt->kind)
            {
            case NODE_STMT_USE:
            case NODE_STMT_DEF:
            case NODE_STMT_VOL:
            case NODE_STMT_VAL:
            case NODE_STMT_FUN:
            case NODE_STMT_STR:
            case NODE_STMT_UNI:
                visitor_visit(visitor, stmt);
                break;
            default:
                sema_report_error(sema, stmt->token, "invalid statement at module root");
                break;
            }
        }
    }
}

void visit_identifier(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    char *name = strndup(node->token.start, node->token.len);

    // check against builtins
    Type *builtin_type = sema_get_builtin_type(sema, name);
    if (builtin_type)
    {
        sema_set_type(sema, node, builtin_type);
        free(name);
        return;
    }

    Symbol *symbol = scope_lookup(sema->scope_current, name);
    if (!symbol)
    {
        sema_report_error(sema, node->token, "undefined identifier '%s'", name);
        free(name);
        return;
    }

    sema_set_type(sema, node, symbol->type);

    if (symbol->value && sema_is_constant(sema, symbol->value))
    {
        Node *const_value = sema_get_constant(sema, symbol->value);
        if (const_value)
        {
            sema_set_constant(sema, node, const_value);
        }
    }

    free(name);
}

void visit_lit_int(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    // default integer literal type is i64.
    // TODO: dynamically change this based on the desired type so we don't have
    // to have extra cast instructions injected for cases like these:
    // `var i: u32 = 42;`
    // `0xFF::u16`
    sema_set_type(sema, node, sema->type_cache.i64_type);
    sema_set_constant(sema, node, node);
}

void visit_lit_float(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    // float literals have type f64 by default
    // TODO: see `visit_lit_int` for the issue. Need to dynamically decide type.
    sema_set_type(sema, node, sema->type_cache.f64_type);
    sema_set_constant(sema, node, node);
}

void visit_lit_char(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    sema_set_type(sema, node, sema->type_cache.u8_type);
    sema_set_constant(sema, node, node);
}

void visit_lit_string(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    // string literals are arrays of u8
    Type *char_array = type_make(TYPE_ARR);
    if (!char_array)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for string literal type");
        return;
    }
    char_array->arr.element_type = sema->type_cache.u8_type;
    char_array->arr.len = strlen(node->lit_string.value) + 1;
    char_array->arr.cap = char_array->arr.len;
    char_array->arr.is_slice = false;

    sema_set_type(sema, node, char_array);
    sema_set_constant(sema, node, node);
}

void visit_expr_unary(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    bool old_const_context = sema->in_const_context;

    visitor_visit(visitor, node->expr_unary.expr);

    sema->in_const_context = old_const_context;

    Type *operand_type = sema_get_type(sema, node->expr_unary.expr);
    if (!operand_type)
    {
        return;
    }

    switch (node->expr_unary.op)
    {
    case OP_SUB:
    case OP_BITWISE_NOT:
    case OP_LOGICAL_NOT:
        if (operand_type->kind < TYPE_U8 || operand_type->kind > TYPE_F64)
        {
            sema_report_error(sema, node->token, "unary operator '%s' cannot be applied to type '%s'", op_to_string(node->expr_unary.op), type_to_string(operand_type));
            return;
        }

        // result has the same type as the operand
        sema_set_type(sema, node, operand_type);

        // check for constant folding
        if (sema_is_constant(sema, node->expr_unary.expr))
        {
            Node *const_operand = sema_get_constant(sema, node->expr_unary.expr);
            Node *const_result = evaluate_constant_unary(const_operand, node->expr_unary.op);
            if (const_result)
            {
                sema_set_constant(sema, node, const_result);
            }
        }
        break;

    case OP_ADDRESS:
        // need to check if we're taking the address of a variable or other lvalue
        // TODO: validate that this is all we need... may add some more?
        if (node->expr_unary.expr->kind != NODE_IDENTIFIER && node->expr_unary.expr->kind != NODE_POST_MEMBER && node->expr_unary.expr->kind != NODE_POST_IDX_ARR)
        {
            sema_report_error(sema, node->token, "cannot take the address of this expression");
            return;
        }

        Type *type_ptr = type_make(TYPE_PTR);
        if (!type_ptr)
        {
            sema_report_error(sema, node->token, "failed to allocate memory for pointer type");
            return;
        }
        type_ptr->ptr.target = operand_type;

        sema_set_type(sema, node, type_ptr);
        break;

    case OP_DEREFERENCE:
        if (operand_type->kind != TYPE_PTR)
        {
            sema_report_error(sema, node->expr_unary.expr->token, "cannot dereference non-pointer type '%s'", type_to_string(operand_type));
            return;
        }

        sema_set_type(sema, node, operand_type->ptr.target);
        break;

    default:
        sema_report_error(sema, node->token, "unsupported unary operator: %s", op_to_string(node->expr_unary.op));
        return;
    }
}

void visit_expr_binary(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    bool old_const_context = sema->in_const_context;

    visitor_visit(visitor, node->expr_binary.left);
    visitor_visit(visitor, node->expr_binary.right);

    sema->in_const_context = old_const_context;

    Type *left_type = sema_get_type(sema, node->expr_binary.left);
    Type *right_type = sema_get_type(sema, node->expr_binary.right);
    if (!left_type || !right_type)
    {
        return;
    }

    switch (node->expr_binary.op)
    {
    case OP_ASSIGN:
        if (node->expr_binary.left->kind != NODE_IDENTIFIER && node->expr_binary.left->kind != NODE_POST_MEMBER && node->expr_binary.left->kind != NODE_POST_IDX_ARR)
        {
            sema_report_error(sema, node->expr_binary.left->token, "left side of assignment must be an lvalue");
            return;
        }

        if (!sema_types_compatible(left_type, right_type))
        {
            sema_report_error(sema, node->token, "cannot assign value of type '%s' to variable of type '%s'", type_to_string(right_type), type_to_string(left_type));
            return;
        }

        // result type is the left operand's type
        sema_set_type(sema, node, left_type);
        break;

    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
    case OP_BITWISE_AND:
    case OP_BITWISE_OR:
    case OP_BITWISE_XOR:
    case OP_BITWISE_SHL:
    case OP_BITWISE_SHR:
        // for numeric operators, operands must be numeric
        if ((left_type->kind < TYPE_U8 || left_type->kind > TYPE_F64) || (right_type->kind < TYPE_U8 || right_type->kind > TYPE_F64))
        {
            sema_report_error(sema, node->token, "binary operator '%s' cannot be applied to types '%s' and '%s'", op_to_string(node->expr_binary.op), type_to_string(left_type), type_to_string(right_type));
            return;
        }

        // get common type between the operands
        Type *result_type = sema_common_type(sema, left_type, right_type);
        if (!result_type)
        {
            sema_report_error(sema, node->token, "incompatible operand types '%s' and '%s' for operator '%s'", type_to_string(left_type), type_to_string(right_type), op_to_string(node->expr_binary.op));
            return;
        }

        sema_set_type(sema, node, result_type);

        // constant folding
        if (sema_is_constant(sema, node->expr_binary.left) && sema_is_constant(sema, node->expr_binary.right))
        {

            Node *left_val = sema_get_constant(sema, node->expr_binary.left);
            Node *right_val = sema_get_constant(sema, node->expr_binary.right);

            Node *result = evaluate_constant_binary(left_val, right_val, node->expr_binary.op);
            if (result)
            {
                sema_set_constant(sema, node, result);
            }
        }
        break;

    case OP_EQUAL:
    case OP_NOT_EQUAL:
        // most types can be compared for equality
        if (!sema_types_compatible(left_type, right_type))
        {
            sema_report_error(sema, node->token, "cannot compare values of types '%s' and '%s'", type_to_string(left_type), type_to_string(right_type));
            return;
        }

        // result is always boolean (u8)
        sema_set_type(sema, node, sema->type_cache.u8_type);

        // constant folding
        if (sema_is_constant(sema, node->expr_binary.left) && sema_is_constant(sema, node->expr_binary.right))
        {

            Node *left_val = sema_get_constant(sema, node->expr_binary.left);
            Node *right_val = sema_get_constant(sema, node->expr_binary.right);

            Node *result = evaluate_constant_binary(left_val, right_val, node->expr_binary.op);
            if (result)
            {
                sema_set_constant(sema, node, result);
            }
        }
        break;

    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREATER:
    case OP_GREATER_EQUAL:
        // comparison operators require numeric types or pointers of the same type
        if (((left_type->kind < TYPE_U8 || left_type->kind > TYPE_F64) && left_type->kind != TYPE_PTR) || ((right_type->kind < TYPE_U8 || right_type->kind > TYPE_F64) && right_type->kind != TYPE_PTR))
        {
            sema_report_error(sema, node->token, "binary operator '%s' cannot be applied to types '%s' and '%s'", op_to_string(node->expr_binary.op), type_to_string(left_type), type_to_string(right_type));
            return;
        }

        // for pointers, they must point to the same type
        if (left_type->kind == TYPE_PTR && right_type->kind == TYPE_PTR)
        {
            if (!type_equals(left_type->ptr.target, right_type->ptr.target))
            {
                sema_report_error(sema, node->token, "cannot compare pointers to different types '%s' and '%s'", type_to_string(left_type), type_to_string(right_type));
                return;
            }
        }
        else if ((left_type->kind == TYPE_PTR) != (right_type->kind == TYPE_PTR))
        {
            // one is a pointer and one is not
            sema_report_error(sema, node->token, "cannot compare pointer to non-pointer type");
            return;
        }

        // result is always boolean (u8)
        sema_set_type(sema, node, sema->type_cache.u8_type);

        // constant folding
        if (sema_is_constant(sema, node->expr_binary.left) && sema_is_constant(sema, node->expr_binary.right))
        {

            Node *left_val = sema_get_constant(sema, node->expr_binary.left);
            Node *right_val = sema_get_constant(sema, node->expr_binary.right);

            Node *result = evaluate_constant_binary(left_val, right_val, node->expr_binary.op);
            if (result)
            {
                sema_set_constant(sema, node, result);
            }
        }
        break;

    case OP_LOGICAL_AND:
    case OP_LOGICAL_OR:
        // logical operators convert both operands to boolean
        sema_set_type(sema, node, sema->type_cache.u8_type);

        // constant folding
        if (sema_is_constant(sema, node->expr_binary.left) && sema_is_constant(sema, node->expr_binary.right))
        {

            Node *left_val = sema_get_constant(sema, node->expr_binary.left);
            Node *right_val = sema_get_constant(sema, node->expr_binary.right);

            Node *result = evaluate_constant_binary(left_val, right_val, node->expr_binary.op);
            if (result)
            {
                sema_set_constant(sema, node, result);
            }
        }
        break;

    default:
        sema_report_error(sema, node->token, "unsupported binary operator: '%s'", op_to_string(node->expr_binary.op));
        return;
    }
}

void visit_post_member(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->post_member.target);

    Type *target_type = sema_get_type(sema, node->post_member.target);
    if (!target_type)
    {
        return;
    }

    // handle direct or pointer access
    if (target_type->kind == TYPE_PTR)
    {
        target_type = target_type->ptr.target;
    }

    // target must be a struct or union
    if (target_type->kind != TYPE_STR && target_type->kind != TYPE_UNI)
    {
        sema_report_error(sema, node->post_member.target->token, "cannot access member of non-struct/union type '%s'", type_to_string(target_type));
        return;
    }

    // member must be an identifier
    if (node->post_member.member->kind != NODE_IDENTIFIER)
    {
        sema_report_error(sema, node->post_member.member->token, "struct/union member access requires an identifier");
        return;
    }

    char *member_name = strndup(node->post_member.member->token.start, node->post_member.member->token.len);

    // find the member in the struct/union
    int field_index = -1;
    Type *field_type = NULL;

    if (target_type->kind == TYPE_STR)
    {
        for (int i = 0; i < target_type->str.field_count; i++)
        {
            if (target_type->str.field_names[i] && strcmp(member_name, target_type->str.field_names[i]) == 0)
            {
                field_index = i;
                field_type = target_type->str.field_types[i];
                break;
            }
        }
    }
    else
    {
        // TYPE_UNI (checked above)
        for (int i = 0; i < target_type->uni.field_count; i++)
        {
            if (target_type->uni.field_names[i] && strcmp(member_name, target_type->uni.field_names[i]) == 0)
            {
                field_index = i;
                field_type = target_type->uni.field_types[i];
                break;
            }
        }
    }

    if (field_index == -1)
    {
        sema_report_error(sema, node->post_member.member->token, "'%s' has no member named '%s'", type_to_string(target_type), member_name);
        free(member_name);
        return;
    }

    sema_set_type(sema, node, field_type);
    free(member_name);
}

void visit_post_call(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->post_call.target);

    Type *target_type = sema_get_type(sema, node->post_call.target);
    if (!target_type)
    {
        return;
    }

    if (target_type->kind != TYPE_FUN)
    {
        sema_report_error(sema, node->post_call.target->token, "cannot call non-function type '%s'", type_to_string(target_type));
        return;
    }

    int expected_param_count = target_type->fun.param_count;
    int actual_arg_count = node->post_call.args ? node->post_call.args->len : 0;

    if (expected_param_count != actual_arg_count)
    {
        sema_report_error(sema, node->token, "function call with '%d' arguments but function takes '%d' parameters", actual_arg_count, expected_param_count);
        return;
    }

    // check each argument
    for (int i = 0; i < actual_arg_count; i++)
    {
        Node *arg = node_list_get(node->post_call.args, i);
        visitor_visit(visitor, arg);

        Type *arg_type = sema_get_type(sema, arg);
        Type *param_type = target_type->fun.param_types[i];

        if (!sema_types_compatible(param_type, arg_type))
        {
            sema_report_error(sema, arg->token, "parameter '%d': cannot convert from '%s' to '%s'", i, type_to_string(arg_type), type_to_string(param_type));
        }
    }

    sema_set_type(sema, node, target_type->fun.return_type);
}

void visit_post_idx_arr(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->post_idx_arr.target);
    visitor_visit(visitor, node->post_idx_arr.index);

    Type *target_type = sema_get_type(sema, node->post_idx_arr.target);
    if (!target_type)
    {
        return;
    }

    // must be an array type or pointer
    Type *element_type = NULL;

    if (target_type->kind == TYPE_ARR)
    {
        element_type = target_type->arr.element_type;
    }
    else if (target_type->kind == TYPE_PTR)
    {
        element_type = target_type->ptr.target;
    }
    else
    {
        sema_report_error(sema, node->post_idx_arr.target->token, "indexing requires array or pointer type, got '%s'", type_to_string(target_type));
        return;
    }

    // index must be an integer type
    Type *index_type = sema_get_type(sema, node->post_idx_arr.index);
    if (!index_type || index_type->kind < TYPE_U8 || index_type->kind > TYPE_I64)
    {
        sema_report_error(sema, node->post_idx_arr.index->token, "array index must be an integer type, got '%s'", index_type ? type_to_string(index_type) : "unknown");
        return;
    }

    // if target is a fixed-size array and index is constant, check bounds
    if (target_type->kind == TYPE_ARR && target_type->arr.len >= 0 && sema_is_constant(sema, node->post_idx_arr.index))
    {
        Node *index_node = sema_get_constant(sema, node->post_idx_arr.index);
        if (index_node->lit_int.value < 0 || index_node->lit_int.value >= target_type->arr.len)
        {
            sema_report_error(sema, node->token, "array index out of bounds");
            return;
        }
    }

    sema_set_type(sema, node, element_type);
}

void visit_post_cast(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->post_cast.target);
    visitor_visit(visitor, node->post_cast.type_node);

    Type *expr_type = sema_get_type(sema, node->post_cast.target);
    Type *target_type = sema_get_type(sema, node->post_cast.type_node);

    if (!expr_type || !target_type)
    {
        return;
    }

    // check if the cast is valid
    if (!sema_can_convert_type(expr_type, target_type))
    {
        sema_report_error(sema, node->token, "invalid cast from '%s' to '%s'", type_to_string(expr_type), type_to_string(target_type));
        return;
    }

    sema_set_type(sema, node, target_type);

    // if the expression is a constant, try to evaluate the cast
    if (sema_is_constant(sema, node->post_cast.target))
    {
        Node *const_expr = sema_get_constant(sema, node->post_cast.target);

        if ((expr_type->kind >= TYPE_U8 && expr_type->kind <= TYPE_F64) && (target_type->kind >= TYPE_U8 && target_type->kind <= TYPE_F64))
        {

            if (const_expr->kind == NODE_LIT_INT && (target_type->kind == TYPE_F32 || target_type->kind == TYPE_F64))
            {
                // int to float
                Node *float_node = calloc(1, sizeof(Node));
                if (float_node)
                {
                    node_init(float_node, NODE_LIT_FLOAT, const_expr->token);
                    float_node->lit_float.value = (double)const_expr->lit_int.value;
                    sema_set_constant(sema, node, float_node);
                }
            }
            else if (const_expr->kind == NODE_LIT_FLOAT && (target_type->kind >= TYPE_U8 && target_type->kind <= TYPE_I64))
            {
                // float to int
                Node *int_node = calloc(1, sizeof(Node));
                if (int_node)
                {
                    node_init(int_node, NODE_LIT_INT, const_expr->token);
                    int_node->lit_int.value = (int64_t)const_expr->lit_float.value;
                    sema_set_constant(sema, node, int_node);
                }
            }
            else
            {
                sema_set_constant(sema, node, const_expr);
            }
        }
    }
}

void visit_stmt_expr(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->stmt_expr.expr);

    // check that expression is valid then move on
    Type *expr_type = sema_get_type(sema, node->stmt_expr.expr);
    if (!expr_type)
    {
        return;
    }
}

void visit_stmt_block(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    sema_enter_scope(sema);

    if (node->stmt_block.stmts)
    {
        for (int i = 0; i < node->stmt_block.stmts->len; i++)
        {
            Node *stmt = node_list_get(node->stmt_block.stmts, i);
            visitor_visit(visitor, stmt);
        }
    }

    sema_exit_scope(sema);
}

void visit_stmt_val(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    // for 'val' declarations, initializer must be a constant expression
    bool old_const_context = sema->in_const_context;
    sema->in_const_context = true;

    // process the type (must be provided)
    Type *var_type = NULL;
    if (!node->stmt_val.type_node)
    {
        sema_report_error(sema, node->token, "'val' declaration must have a type");
    }
    else
    {
        visitor_visit(visitor, node->stmt_val.type_node);
        var_type = sema_get_type(sema, node->stmt_val.type_node);
    }

    if (!node->stmt_val.expr)
    {
        sema_report_error(sema, node->token, "'val' declaration requires an initializer");
    }
    else
    {
        visitor_visit(visitor, node->stmt_val.expr);
    }

    // initializer must match declared type
    Type *init_type = sema_get_type(sema, node->stmt_val.expr);
    if (!sema_types_compatible(var_type, init_type))
    {
        sema_report_error(sema, node->token, "cannot initialize value of type '%s' with expression of type '%s'", type_to_string(var_type), type_to_string(init_type));
    }

    // ensure initializer is constant
    if (!sema_is_constant(sema, node->stmt_val.expr))
    {
        sema_report_error(sema, node->token, "'val' declaration requires a constant initializer");
    }

    sema->in_const_context = old_const_context;

    if (var_type && node->stmt_val.ident->kind == NODE_IDENTIFIER)
    {
        char *name = strndup(node->stmt_val.ident->token.start, node->stmt_val.ident->token.len);
        if (!scope_define(sema->scope_current, name, node, var_type))
        {
            sema_report_error(sema, node->stmt_val.ident->token, "redefinition of '%s'", name);
        }
        free(name);
    }
}

void visit_stmt_var(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    Type *var_type = NULL;
    if (!node->stmt_var.type_node)
    {
        sema_report_error(sema, node->token, "variable declaration must have a type");
    }
    else
    {
        visitor_visit(visitor, node->stmt_var.type_node);
        var_type = sema_get_type(sema, node->stmt_var.type_node);
    }

    if (node->stmt_var.expr)
    {
        visitor_visit(visitor, node->stmt_var.expr);

        // initializer must match declared type
        Type *init_type = sema_get_type(sema, node->stmt_var.expr);
        if (!sema_types_compatible(var_type, init_type))
        {
            sema_report_error(sema, node->token, "cannot initialize variable of type '%s' with expression of type '%s'", type_to_string(var_type), type_to_string(init_type));
        }
    }

    // add to symbol table
    if (var_type && node->stmt_var.ident->kind == NODE_IDENTIFIER)
    {
        char *name = strndup(node->stmt_var.ident->token.start, node->stmt_var.ident->token.len);
        if (!scope_define(sema->scope_current, name, node, var_type))
        {
            sema_report_error(sema, node->stmt_var.ident->token, "redefinition of '%s'", name);
        }
        free(name);
    }
}

void visit_stmt_vol(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    Type *vol_type = NULL;
    if (!node->stmt_var.type_node)
    {
        sema_report_error(sema, node->token, "volatile variable declaration must have a type");
    }
    else
    {
        visitor_visit(visitor, node->stmt_var.type_node);
        vol_type = sema_get_type(sema, node->stmt_var.type_node);
    }

    if (node->stmt_var.expr)
    {
        visitor_visit(visitor, node->stmt_var.expr);

        // initializer must match declared type
        Type *init_type = sema_get_type(sema, node->stmt_var.expr);
        if (!sema_types_compatible(vol_type, init_type))
        {
            sema_report_error(sema, node->token, "cannot initialize variable of type '%s' with expression of type '%s'", type_to_string(vol_type), type_to_string(init_type));
        }
    }

    // add to symbol table
    if (vol_type && node->stmt_var.ident->kind == NODE_IDENTIFIER)
    {
        char *name = strndup(node->stmt_var.ident->token.start, node->stmt_var.ident->token.len);
        if (!scope_define(sema->scope_current, name, node, vol_type))
        {
            sema_report_error(sema, node->stmt_var.ident->token, "redefinition of '%s'", name);
        }
        free(name);
    }
}

void visit_stmt_def(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    Type *var_type = NULL;
    if (node->stmt_def.type_node)
    {
        visitor_visit(visitor, node->stmt_def.type_node);
        var_type = sema_get_type(sema, node->stmt_def.type_node);
    }
    else
    {
        sema_report_error(sema, node->token, "type definition must have a type");
    }

    // add to symbol table
    if (var_type && node->stmt_def.ident->kind == NODE_IDENTIFIER)
    {
        char *name = strndup(node->stmt_def.ident->token.start, node->stmt_def.ident->token.len);
        if (!scope_define(sema->scope_current, name, node, var_type))
        {
            sema_report_error(sema, node->stmt_def.ident->token, "redefinition of '%s'", name);
        }
        free(name);
    }
}

void visit_stmt_str(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    // create a new struct type
    char *struct_name = strndup(node->stmt_str.ident->token.start, node->stmt_str.ident->token.len);

    // set up fields
    int field_count = node->stmt_str.fields ? node->stmt_str.fields->len : 0;
    char **field_names = NULL;
    Type **field_types = NULL;

    if (field_count > 0)
    {
        field_names = calloc(field_count, sizeof(char *));
        field_types = calloc(field_count, sizeof(Type *));

        if (!field_names || !field_types)
        {
            free(struct_name);
            free(field_names);
            free(field_types);
            return;
        }

        // process each field
        for (int i = 0; i < field_count; i++)
        {
            Node *field = node_list_get(node->stmt_str.fields, i);
            visitor_visit(visitor, field);

            if (field->kind == NODE_FIELD)
            {
                char *field_name = strndup(field->field.ident->token.start, field->field.ident->token.len);

                // check for duplicate field names
                for (int j = 0; j < i; j++)
                {
                    if (field_names[j] && strcmp(field_names[j], field_name) == 0)
                    {
                        sema_report_error(sema, field->field.ident->token, "duplicate field name '%s' in struct", field_name);
                        free(field_name);
                        field_name = NULL;
                        break;
                    }
                }

                if (field_name)
                {
                    field_names[i] = field_name;
                    field_types[i] = sema_get_type(sema, field->field.type_node);
                }
            }
        }
    }

    // create the struct type
    Type *struct_type = type_make(TYPE_STR);
    if (!struct_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for struct type");
        free(struct_name);
        free(field_names);
        free(field_types);
        return;
    }
    struct_type->str.field_count = field_count;
    struct_type->str.field_names = field_names;
    struct_type->str.field_types = field_types;
    struct_type->str.name = strdup(struct_name);
    struct_type->str.field_offsets = calloc(field_count, sizeof(int));
    if (!struct_type->str.field_offsets)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for struct field offsets");
        free(struct_name);
        free(field_names);
        free(field_types);
        return;
    }
    type_compute_offsets(struct_type, sema->target);

    if (!scope_define(sema->scope_current, struct_name, node, struct_type))
    {
        sema_report_error(sema, node->stmt_str.ident->token, "redefinition of '%s'", struct_name);
    }

    free(struct_name);
}

void visit_stmt_uni(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    // create a new union type
    char *union_name = strndup(node->stmt_uni.ident->token.start, node->stmt_uni.ident->token.len);

    // set up fields
    int field_count = node->stmt_uni.fields ? node->stmt_uni.fields->len : 0;
    char **field_names = NULL;
    Type **field_types = NULL;

    if (field_count > 0)
    {
        field_names = calloc(field_count, sizeof(char *));
        field_types = calloc(field_count, sizeof(Type *));

        if (!field_names || !field_types)
        {
            free(union_name);
            free(field_names);
            free(field_types);
            return;
        }

        // process each field
        for (int i = 0; i < field_count; i++)
        {
            Node *field = node_list_get(node->stmt_uni.fields, i);
            visitor_visit(visitor, field);

            if (field->kind == NODE_FIELD)
            {
                char *field_name = strndup(field->field.ident->token.start, field->field.ident->token.len);

                // check for duplicate field names
                for (int j = 0; j < i; j++)
                {
                    if (field_names[j] && strcmp(field_names[j], field_name) == 0)
                    {
                        sema_report_error(sema, field->field.ident->token, "duplicate field name '%s' in union", field_name);
                        free(field_name);
                        field_name = NULL;
                        break;
                    }
                }

                if (field_name)
                {
                    field_names[i] = field_name;
                    field_types[i] = sema_get_type(sema, field->field.type_node);
                }
            }
        }
    }

    Type *union_type = type_make(TYPE_UNI);
    if (!union_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for union type");
        free(union_name);
        free(field_names);
        free(field_types);
        return;
    }
    union_type->uni.field_count = field_count;
    union_type->uni.field_names = field_names;
    union_type->uni.field_types = field_types;
    union_type->uni.name = strdup(union_name);

    if (!scope_define(sema->scope_current, union_name, node, union_type))
    {
        sema_report_error(sema, node->stmt_uni.ident->token, "redefinition of '%s'", union_name);
    }

    free(union_name);
}

void visit_stmt_fun(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    char *func_name = strndup(node->stmt_fun.ident->token.start, node->stmt_fun.ident->token.len);

    visitor_visit(visitor, node->stmt_fun.type_node);
    Type *return_type = sema_get_type(sema, node->stmt_fun.type_node);

    int param_count = node->stmt_fun.params ? node->stmt_fun.params->len : 0;
    char **param_names = NULL;
    Type **param_types = NULL;

    if (param_count > 0)
    {
        param_names = calloc(param_count, sizeof(char *));
        if (!param_names)
        {
            free(func_name);
            return;
        }

        param_types = calloc(param_count, sizeof(Type *));
        if (!param_types)
        {
            free(func_name);
            return;
        }
    }

    // create function type now, before processing body
    Type *func_type = type_make(TYPE_FUN);
    if (!func_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for function type");
        free(func_name);
        free(param_names);
        free(param_types);
        return;
    }
    func_type->fun.return_type = return_type;
    func_type->fun.param_count = param_count;
    func_type->fun.param_types = param_types;
    func_type->fun.param_names = param_names;
    func_type->fun.name = strdup(func_name);

    // add to symbol table first so recursive calls work
    if (!scope_define(sema->scope_current, func_name, node, func_type))
    {
        sema_report_error(sema, node->stmt_fun.ident->token, "redefinition of '%s'", func_name);
    }

    sema_enter_scope(sema);

    bool old_func_context = sema->in_function_context;
    Type *old_return_type = sema->current_function_return_type;

    sema->in_function_context = true;
    sema->current_function_return_type = return_type;

    if (param_count > 0 && node->stmt_fun.params)
    {
        for (int i = 0; i < param_count; i++)
        {
            Node *param = node_list_get(node->stmt_fun.params, i);

            if (param->kind == NODE_FIELD)
            {
                visitor_visit(visitor, param->field.type_node);
                Type *param_type = sema_get_type(sema, param->field.type_node);
                param_types[i] = param_type;

                char *param_name = strndup(param->field.ident->token.start, param->field.ident->token.len);
                if (!scope_define(sema->scope_current, param_name, param, param_type))
                {
                    sema_report_error(sema, param->field.ident->token, "duplicate parameter name '%s'", param_name);
                }
                param_names[i] = param_name;
            }
            else
            {
                sema_report_error(sema, param->token, "invalid parameter");
            }
        }
    }

    if (node->stmt_fun.body)
    {
        visitor_visit(visitor, node->stmt_fun.body);
    }

    sema->in_function_context = old_func_context;
    sema->current_function_return_type = old_return_type;

    sema_exit_scope(sema);

    free(func_name);
}

void visit_stmt_if(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->stmt_if.cond);

    Type *cond_type = sema_get_type(sema, node->stmt_if.cond);
    if (cond_type && (cond_type->kind < TYPE_U8 || cond_type->kind > TYPE_F64))
    {
        sema_report_error(sema, node->stmt_if.cond->token, "if condition must have numeric type, got '%s'", type_to_string(cond_type));
    }

    if (sema_is_constant(sema, node->stmt_if.cond))
    {
        Node *cond_val = sema_get_constant(sema, node->stmt_if.cond);
        bool is_true = false;

        if (cond_val->kind == NODE_LIT_INT)
        {
            is_true = cond_val->lit_int.value != 0;
        }
        else if (cond_val->kind == NODE_LIT_FLOAT)
        {
            is_true = cond_val->lit_float.value != 0;
        }

        // don't warn if condition is false and there's a branch
        if (is_true)
        {
            sema_report_error(sema, node->stmt_if.cond->token, "if condition is always true");
        }
        else if (!node->stmt_if.branch)
        {
            sema_report_error(sema, node->stmt_if.cond->token, "if condition is always false");
        }
    }

    visitor_visit(visitor, node->stmt_if.branch);
}

void visit_stmt_or(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->stmt_or.cond);

    Type *cond_type = sema_get_type(sema, node->stmt_or.cond);
    if (cond_type && (cond_type->kind < TYPE_U8 || cond_type->kind > TYPE_F64))
    {
        sema_report_error(sema, node->stmt_or.cond->token, "or condition must have numeric type, got '%s'", type_to_string(cond_type));
    }

    visitor_visit(visitor, node->stmt_or.branch);
}

void visit_stmt_for(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    sema_enter_scope(sema);

    bool old_loop_context = sema->in_loop_context;
    sema->in_loop_context = true;

    if (node->stmt_for.cond)
    {
        visitor_visit(visitor, node->stmt_for.cond);

        Type *cond_type = sema_get_type(sema, node->stmt_for.cond);
        if (cond_type && (cond_type->kind < TYPE_U8 || cond_type->kind > TYPE_F64))
        {
            sema_report_error(sema, node->stmt_for.cond->token, "for loop condition must have numeric type, got '%s'", type_to_string(cond_type));
        }

        if (sema_is_constant(sema, node->stmt_for.cond))
        {
            Node *cond_val = sema_get_constant(sema, node->stmt_for.cond);
            if (cond_val->kind == NODE_LIT_INT && cond_val->lit_int.value == 0)
            {
                sema_report_error(sema, node->stmt_for.cond->token, "for loop condition is always false");
            }
        }
    }

    visitor_visit(visitor, node->stmt_for.body);

    sema->in_loop_context = old_loop_context;

    sema_exit_scope(sema);
}

void visit_stmt_brk(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    if (!sema->in_loop_context)
    {
        sema_report_error(sema, node->token, "break statement not within loop");
    }
}

void visit_stmt_cnt(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    if (!sema->in_loop_context)
    {
        sema_report_error(sema, node->token, "continue statement not within loop");
    }
}

void visit_stmt_ret(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    if (!sema->in_function_context)
    {
        sema_report_error(sema, node->token, "return statement not within function");
        return;
    }

    if (node->stmt_ret.expr)
    {
        visitor_visit(visitor, node->stmt_ret.expr);

        Type *expr_type = sema_get_type(sema, node->stmt_ret.expr);
        if (!expr_type)
        {
            return;
        }

        if (sema->current_function_return_type->kind == TYPE_VOID)
        {
            sema_report_error(sema, node->token, "void function cannot return a value");
            return;
        }

        if (!sema_types_compatible(sema->current_function_return_type, expr_type))
        {
            sema_report_error(sema, node->token, "cannot return value of type '%s' from function with return type '%s'", type_to_string(expr_type), type_to_string(sema->current_function_return_type));
        }
    }
    else if (sema->current_function_return_type->kind != TYPE_VOID)
    {
        sema_report_error(sema, node->token, "non-void function must return a value");
    }
}

void visit_stmt_use(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    if (node->stmt_use.path->kind != NODE_LIT_STRING)
    {
        sema_report_error(sema, node->stmt_use.path->token, "module path must be a string literal");
    }
}

void visit_type_arr(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->type_arr.type_node);

    Type *element_type = sema_get_type(sema, node->type_arr.type_node);
    if (!element_type)
    {
        return;
    }

    int len = -1;
    bool is_slice = false;
    if (node->type_arr.size)
    {
        visitor_visit(visitor, node->type_arr.size);

        Type *len_type = sema_get_type(sema, node->type_arr.size);
        if (len_type && (len_type->kind < TYPE_U8 || len_type->kind > TYPE_I64))
        {
            sema_report_error(sema, node->type_arr.size->token, "array size must have integer type, got '%s'", type_to_string(len_type));
        }

        if (sema_is_constant(sema, node->type_arr.size))
        {
            Node *len_node = sema_get_constant(sema, node->type_arr.size);
            if (len_node->kind == NODE_LIT_INT)
            {
                len = len_node->lit_int.value;
            }
        }
    }
    else
    {
        // default to 0 length
        len = 0;
        is_slice = true;
    }

    Type *arr_type = type_make(TYPE_ARR);
    if (!arr_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for array type");
        return;
    }
    arr_type->arr.element_type = element_type;
    arr_type->arr.len = len;
    arr_type->arr.cap = len;
    arr_type->arr.is_slice = is_slice;

    sema_set_type(sema, node, arr_type);
}

void visit_type_ptr(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->type_ptr.type_node);

    Type *target_type = sema_get_type(sema, node->type_ptr.type_node);
    if (!target_type)
    {
        return;
    }

    Type *ptr_type = type_make(TYPE_PTR);
    if (!ptr_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for pointer type");
        return;
    }
    ptr_type->ptr.target = target_type;

    sema_set_type(sema, node, ptr_type);
}

void visit_type_fun(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    visitor_visit(visitor, node->type_fun.type_node);

    Type *return_type = sema_get_type(sema, node->type_fun.type_node);
    if (!return_type)
    {
        return;
    }

    int param_count = node->type_fun.params ? node->type_fun.params->len : 0;
    char **param_names = NULL;
    Type **param_types = NULL;

    if (param_count > 0)
    {
        param_names = calloc(param_count, sizeof(char *));
        if (!param_names)
        {
            return;
        }

        param_types = calloc(param_count, sizeof(Type *));
        if (!param_types)
        {
            free(param_names);
            return;
        }
    }

    for (int i = 0; i < param_count; i++)
    {
        Node *param = node_list_get(node->type_fun.params, i);

        if (param->kind == NODE_FIELD)
        {
            visitor_visit(visitor, param->field.type_node);
            Type *param_type = sema_get_type(sema, param->field.type_node);
            param_types[i] = param_type;

            char *param_name = strndup(param->field.ident->token.start, param->field.ident->token.len);
            param_names[i] = param_name;
        }
        else
        {
            sema_report_error(sema, param->token, "invalid parameter");
        }
    }

    Type *fun_type = type_make(TYPE_FUN);
    if (!fun_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for function type");
        free(param_names);
        free(param_types);
        return;
    }
    fun_type->fun.return_type = return_type;
    fun_type->fun.param_count = param_count;
    fun_type->fun.param_types = param_types;
    fun_type->fun.param_names = param_names;
    fun_type->fun.name = NULL;
    
    sema_set_type(sema, node, fun_type);
}

void visit_type_str(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    int field_count = node->type_str.fields ? node->type_str.fields->len : 0;
    char **field_names = NULL;
    Type **field_types = NULL;

    if (field_count > 0)
    {
        field_names = calloc(field_count, sizeof(char *));
        field_types = calloc(field_count, sizeof(Type *));

        if (!field_names || !field_types)
        {
            free(field_names);
            free(field_types);
            return;
        }

        for (int i = 0; i < field_count; i++)
        {
            Node *field = node_list_get(node->type_str.fields, i);
            visitor_visit(visitor, field);

            if (field->kind == NODE_FIELD)
            {
                char *field_name = strndup(field->field.ident->token.start, field->field.ident->token.len);

                for (int j = 0; j < i; j++)
                {
                    if (field_names[j] && strcmp(field_names[j], field_name) == 0)
                    {
                        sema_report_error(sema, field->field.ident->token, "duplicate field name '%s' in struct", field_name);
                        free(field_name);
                        field_name = NULL;
                        break;
                    }
                }

                if (field_name)
                {
                    field_names[i] = field_name;
                    field_types[i] = sema_get_type(sema, field->field.type_node);
                }
            }
        }
    }

    Type *struct_type = type_make(TYPE_STR);
    if (!struct_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for struct type");
        free(field_names);
        free(field_types);
        return;
    }
    struct_type->str.field_count = field_count;
    struct_type->str.field_names = field_names;
    struct_type->str.field_types = field_types;
    struct_type->str.name = NULL;

    sema_set_type(sema, node, struct_type);
}

void visit_type_uni(Visitor *visitor, Node *node)
{
    SEMAContext *sema = visitor->context;

    int field_count = node->type_uni.fields ? node->type_uni.fields->len : 0;
    char **field_names = NULL;
    Type **field_types = NULL;

    if (field_count > 0)
    {
        field_names = calloc(field_count, sizeof(char *));
        field_types = calloc(field_count, sizeof(Type *));

        if (!field_names || !field_types)
        {
            free(field_names);
            free(field_types);
            return;
        }

        for (int i = 0; i < field_count; i++)
        {
            Node *field = node_list_get(node->type_uni.fields, i);
            visitor_visit(visitor, field);

            if (field->kind == NODE_FIELD)
            {
                char *field_name = strndup(field->field.ident->token.start, field->field.ident->token.len);

                for (int j = 0; j < i; j++)
                {
                    if (field_names[j] && strcmp(field_names[j], field_name) == 0)
                    {
                        sema_report_error(sema, field->field.ident->token, "duplicate field name '%s' in union", field_name);
                        free(field_name);
                        field_name = NULL;
                        break;
                    }
                }

                if (field_name)
                {
                    field_names[i] = field_name;
                    field_types[i] = sema_get_type(sema, field->field.type_node);
                }
            }
        }
    }

    Type *uni_type = type_make(TYPE_UNI);
    if (!uni_type)
    {
        sema_report_error(sema, node->token, "failed to allocate memory for union type");
        free(field_names);
        free(field_types);
        return;
    }
    uni_type->uni.field_count = field_count;
    uni_type->uni.field_names = field_names;
    uni_type->uni.field_types = field_types;
    uni_type->uni.name = NULL;

    sema_set_type(sema, node, uni_type);
}

void visit_field(Visitor *visitor, Node *node)
{
    visitor_visit(visitor, node->field.type_node);
}


bool visitor_init_sema(Visitor *visitor, SEMAContext *context)
{
    if (!visitor)
    {
        return NULL;
    }

    visitor->context = context;

    visitor->visit_module = visit_module;
    visitor->visit_identifier = visit_identifier;
    visitor->visit_lit_int = visit_lit_int;
    visitor->visit_lit_float = visit_lit_float;
    visitor->visit_lit_char = visit_lit_char;
    visitor->visit_lit_string = visit_lit_string;
    visitor->visit_expr_unary = visit_expr_unary;
    visitor->visit_expr_binary = visit_expr_binary;
    visitor->visit_post_member = visit_post_member;
    visitor->visit_post_call = visit_post_call;
    visitor->visit_post_idx_arr = visit_post_idx_arr;
    visitor->visit_post_cast = visit_post_cast;
    visitor->visit_stmt_expr = visit_stmt_expr;
    visitor->visit_stmt_block = visit_stmt_block;
    visitor->visit_stmt_val = visit_stmt_val;
    visitor->visit_stmt_var = visit_stmt_var;
    visitor->visit_stmt_vol = visit_stmt_vol;
    visitor->visit_stmt_def = visit_stmt_def;
    visitor->visit_stmt_str = visit_stmt_str;
    visitor->visit_stmt_uni = visit_stmt_uni;
    visitor->visit_stmt_fun = visit_stmt_fun;
    visitor->visit_stmt_if = visit_stmt_if;
    visitor->visit_stmt_or = visit_stmt_or;
    visitor->visit_stmt_for = visit_stmt_for;
    visitor->visit_stmt_brk = visit_stmt_brk;
    visitor->visit_stmt_cnt = visit_stmt_cnt;
    visitor->visit_stmt_ret = visit_stmt_ret;
    visitor->visit_stmt_use = visit_stmt_use;
    visitor->visit_type_arr = visit_type_arr;
    visitor->visit_type_ptr = visit_type_ptr;
    visitor->visit_type_fun = visit_type_fun;
    visitor->visit_type_str = visit_type_str;
    visitor->visit_type_uni = visit_type_uni;
    visitor->visit_field = visit_field;

    return true;
}
