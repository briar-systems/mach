#include "codegen.h"
#include "symbol.h"
#include "token.h"
#include "type.h"
#include <limits.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Transforms/PassBuilder.h>
#include <llvm-c/DebugInfo.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#endif

static LLVMValueRef    codegen_load_rvalue(CodegenContext *ctx, LLVMValueRef value, Type *type, AstNode *source_expr);
static void            codegen_debug_init(CodegenContext *ctx);
static void            codegen_debug_finalize(CodegenContext *ctx);
static void            codegen_set_debug_location(CodegenContext *ctx, AstNode *node);
static LLVMMetadataRef codegen_get_current_scope(CodegenContext *ctx);
static LLVMMetadataRef codegen_debug_get_unknown_type(CodegenContext *ctx);
static LLVMMetadataRef codegen_debug_get_type(CodegenContext *ctx, Type *type);
static bool            codegen_debug_push_scope(CodegenContext *ctx, AstNode *node, LLVMMetadataRef *out_prev_scope);
static void            codegen_debug_pop_scope(CodegenContext *ctx, LLVMMetadataRef previous_scope, bool pushed);
static void            codegen_debug_declare_symbol(CodegenContext *ctx, Symbol *symbol, Type *type, LLVMValueRef storage, AstNode *node, bool is_parameter, unsigned arg_index);
static LLVMMetadataRef codegen_debug_create_subprogram(CodegenContext *ctx, AstNode *stmt, LLVMValueRef func, const char *display_name, const char *link_name, size_t param_count);

// Helper function to add a function with required attributes
// Prevents LLVM from optimizing code into library calls (e.g., loop -> strlen)
// This is critical since we don't link to libc and provide our own implementations
static LLVMValueRef codegen_add_function(CodegenContext *ctx, const char *name, LLVMTypeRef function_type)
{
    LLVMValueRef func = LLVMAddFunction(ctx->module, name, function_type);
    LLVMAddAttributeAtIndex(func, LLVMAttributeFunctionIndex, LLVMCreateStringAttribute(ctx->context, "no-builtins", 11, "", 0));
    return func;
}

// simple constant folding for integers/booleans used in global initializers
static bool codegen_eval_const_i64(CodegenContext *ctx, AstNode *expr, int64_t *out)
{
    (void)ctx;
    if (!expr || !out)
    {
        return false;
    }

    switch (expr->kind)
    {
    case AST_EXPR_LIT:
        switch (expr->lit_expr.kind)
        {
        case TOKEN_LIT_INT:
            *out = (int64_t)expr->lit_expr.int_val;
            return true;
        case TOKEN_LIT_CHAR:
            *out = (int64_t)expr->lit_expr.char_val;
            return true;
        default:
            return false;
        }

    case AST_EXPR_NULL:
        *out = 0;
        return true;

    case AST_EXPR_IDENT:
        if (!expr->symbol)
        {
            return false;
        }

        if (expr->symbol->has_const_i64)
        {
            *out = expr->symbol->const_i64;
            return true;
        }

        if ((expr->symbol->kind == SYMBOL_VAL || expr->symbol->kind == SYMBOL_VAR) && expr->symbol->decl)
        {
            AstNode *decl = expr->symbol->decl;
            AstNode *init = NULL;
            if (decl->kind == AST_STMT_VAL || decl->kind == AST_STMT_VAR)
            {
                init = decl->var_stmt.init;
            }

            if (init && codegen_eval_const_i64(ctx, init, out))
            {
                if (expr->symbol->kind == SYMBOL_VAL)
                {
                    expr->symbol->has_const_i64 = true;
                    expr->symbol->const_i64     = *out;
                }
                return true;
            }
        }
        return false;

    case AST_EXPR_UNARY:
    {
        int64_t value = 0;
        if (!codegen_eval_const_i64(ctx, expr->unary_expr.expr, &value))
        {
            return false;
        }

        switch (expr->unary_expr.op)
        {
        case TOKEN_MINUS:
            *out = -value;
            return true;
        case TOKEN_PLUS:
            *out = value;
            return true;
        case TOKEN_BANG:
            *out = (value == 0) ? 1 : 0;
            return true;
        case TOKEN_TILDE:
            *out = ~value;
            return true;
        default:
            return false;
        }
    }

    case AST_EXPR_BINARY:
    {
        int64_t lhs = 0;
        int64_t rhs = 0;
        if (!codegen_eval_const_i64(ctx, expr->binary_expr.left, &lhs) || !codegen_eval_const_i64(ctx, expr->binary_expr.right, &rhs))
        {
            return false;
        }

        switch (expr->binary_expr.op)
        {
        case TOKEN_PLUS:
            *out = lhs + rhs;
            return true;
        case TOKEN_MINUS:
            *out = lhs - rhs;
            return true;
        case TOKEN_STAR:
            *out = lhs * rhs;
            return true;
        case TOKEN_SLASH:
            if (rhs == 0)
            {
                return false;
            }
            *out = lhs / rhs;
            return true;
        case TOKEN_PERCENT:
            if (rhs == 0)
            {
                return false;
            }
            *out = lhs % rhs;
            return true;
        case TOKEN_LESS_LESS:
            if (rhs < 0 || rhs >= 64)
            {
                return false;
            }
            *out = lhs << rhs;
            return true;
        case TOKEN_GREATER_GREATER:
            if (rhs < 0 || rhs >= 64)
            {
                return false;
            }
            *out = lhs >> rhs;
            return true;
        case TOKEN_PIPE:
            *out = lhs | rhs;
            return true;
        case TOKEN_AMPERSAND:
            *out = lhs & rhs;
            return true;
        case TOKEN_CARET:
            *out = lhs ^ rhs;
            return true;
        case TOKEN_EQUAL_EQUAL:
            *out = (lhs == rhs) ? 1 : 0;
            return true;
        case TOKEN_BANG_EQUAL:
            *out = (lhs != rhs) ? 1 : 0;
            return true;
        case TOKEN_LESS:
            *out = (lhs < rhs) ? 1 : 0;
            return true;
        case TOKEN_LESS_EQUAL:
            *out = (lhs <= rhs) ? 1 : 0;
            return true;
        case TOKEN_GREATER:
            *out = (lhs > rhs) ? 1 : 0;
            return true;
        case TOKEN_GREATER_EQUAL:
            *out = (lhs >= rhs) ? 1 : 0;
            return true;
        case TOKEN_PIPE_PIPE:
            *out = ((lhs != 0) || (rhs != 0)) ? 1 : 0;
            return true;
        case TOKEN_AMPERSAND_AMPERSAND:
            *out = ((lhs != 0) && (rhs != 0)) ? 1 : 0;
            return true;
        default:
            return false;
        }
    }

    case AST_EXPR_FIELD:
        // for module.constant access
        if (expr->symbol && expr->symbol->has_const_i64)
        {
            *out = expr->symbol->const_i64;
            return true;
        }
        // try evaluating via initializer
        if (expr->symbol && (expr->symbol->kind == SYMBOL_VAL || expr->symbol->kind == SYMBOL_VAR) && expr->symbol->decl)
        {
            AstNode *decl = expr->symbol->decl;
            AstNode *init = NULL;
            if (decl->kind == AST_STMT_VAL || decl->kind == AST_STMT_VAR)
            {
                init = decl->var_stmt.init;
            }

            if (init && codegen_eval_const_i64(ctx, init, out))
            {
                if (expr->symbol->kind == SYMBOL_VAL)
                {
                    expr->symbol->has_const_i64 = true;
                    expr->symbol->const_i64     = *out;
                }
                return true;
            }
        }
        return false;

    case AST_EXPR_CAST:
    {
        int64_t value = 0;
        if (!codegen_eval_const_i64(ctx, expr->cast_expr.expr, &value))
        {
            return false;
        }

        Type *target = type_resolve_alias(expr->type);
        if (!target)
        {
            return false;
        }

        if (type_is_integer(target))
        {
            size_t bits = target->size * 8;
            if (bits == 0 || bits > 64)
            {
                return false;
            }

            if (type_is_signed(target))
            {
                if (bits < 64)
                {
                    int shift = (int)(64 - bits);
                    value     = (value << shift) >> shift;
                }
                *out = value;
                return true;
            }

            uint64_t uvalue = (uint64_t)value;
            if (bits < 64)
            {
                uint64_t mask = (1ULL << bits) - 1ULL;
                uvalue &= mask;
            }
            *out = (int64_t)uvalue;
            return true;
        }

        if (type_is_pointer_like(target))
        {
            *out = value;
            return true;
        }

        return false;
    }

    case AST_EXPR_CALL:
        if (expr->call_expr.func && expr->call_expr.func->kind == AST_EXPR_IDENT && expr->call_expr.args)
        {
            const char *name = expr->call_expr.func->ident_expr.name;

            if (strcmp(name, "size_of") == 0)
            {
                if (expr->call_expr.args->count != 1)
                {
                    return false;
                }

                AstNode *arg = expr->call_expr.args->items[0];
                if (!arg || !arg->type)
                {
                    return false;
                }

                Type *arg_type = type_resolve_alias(arg->type);
                if (!arg_type)
                {
                    return false;
                }

                *out = (int64_t)type_sizeof(arg_type);
                return true;
            }

            if (strcmp(name, "align_of") == 0)
            {
                if (expr->call_expr.args->count != 1)
                {
                    return false;
                }

                AstNode *arg = expr->call_expr.args->items[0];
                if (!arg || !arg->type)
                {
                    return false;
                }

                Type *arg_type = type_resolve_alias(arg->type);
                if (!arg_type)
                {
                    return false;
                }

                *out = (int64_t)type_alignof(arg_type);
                return true;
            }

            if (strcmp(name, "offset_of") == 0)
            {
                if (expr->call_expr.args->count != 2)
                {
                    return false;
                }

                AstNode *type_arg  = expr->call_expr.args->items[0];
                AstNode *field_arg = expr->call_expr.args->items[1];
                if (!type_arg || !field_arg || field_arg->kind != AST_EXPR_IDENT || !type_arg->type)
                {
                    return false;
                }

                Type *struct_type = type_resolve_alias(type_arg->type);
                if (!struct_type || struct_type->kind != TYPE_STRUCT || !struct_type->composite.fields)
                {
                    return false;
                }

                const char *field_name = field_arg->ident_expr.name;
                for (size_t i = 0; i < struct_type->composite.field_count; i++)
                {
                    Symbol *field = &struct_type->composite.fields[i];
                    if (strcmp(field->name, field_name) == 0)
                    {
                        *out = (int64_t)field->field.offset;
                        return true;
                    }
                }

                return false;
            }

            if (strcmp(name, "min") == 0)
            {
                if (expr->call_expr.args->count != 1)
                {
                    return false;
                }

                AstNode *arg = expr->call_expr.args->items[0];
                if (!arg || !arg->type)
                {
                    return false;
                }

                Type *arg_type = type_resolve_alias(arg->type);
                if (!arg_type)
                {
                    return false;
                }

                switch (arg_type->kind)
                {
                case TYPE_U8:
                case TYPE_U16:
                case TYPE_U32:
                case TYPE_U64:
                    *out = 0;
                    return true;
                case TYPE_I8:
                    *out = -128;
                    return true;
                case TYPE_I16:
                    *out = -32768;
                    return true;
                case TYPE_I32:
                    *out = -2147483648LL;
                    return true;
                case TYPE_I64:
                    *out = (int64_t)(1ULL << 63); // -9223372036854775808
                    return true;
                default:
                    return false;
                }
            }

            if (strcmp(name, "max") == 0)
            {
                if (expr->call_expr.args->count != 1)
                {
                    return false;
                }

                AstNode *arg = expr->call_expr.args->items[0];
                if (!arg || !arg->type)
                {
                    return false;
                }

                Type *arg_type = type_resolve_alias(arg->type);
                if (!arg_type)
                {
                    return false;
                }

                switch (arg_type->kind)
                {
                case TYPE_U8:
                    *out = 255;
                    return true;
                case TYPE_U16:
                    *out = 65535;
                    return true;
                case TYPE_U32:
                    *out = 4294967295U;
                    return true;
                case TYPE_U64:
                    *out = (int64_t)0xFFFFFFFFFFFFFFFFULL;
                    return true;
                case TYPE_I8:
                    *out = 127;
                    return true;
                case TYPE_I16:
                    *out = 32767;
                    return true;
                case TYPE_I32:
                    *out = 2147483647;
                    return true;
                case TYPE_I64:
                    *out = 9223372036854775807LL;
                    return true;
                default:
                    return false;
                }
            }

            if (strcmp(name, "iota") == 0)
            {
                if (expr->call_expr.args->count != 0)
                {
                    return false;
                }

                *out = (int64_t)ctx->iota_counter++;
                return true;
            }
        }
        return false;

    default:
        return false;
    }
}

static void codegen_debug_split_path(const char *path, char **dir_out, char **file_out)
{
    if (!path)
    {
        *dir_out  = strdup(".");
        *file_out = strdup("<unknown>");
        return;
    }

    char *tmp = strdup(path);
    if (!tmp)
    {
        *dir_out  = strdup(".");
        *file_out = strdup(path);
        return;
    }

    char *slash = strrchr(tmp, '/');
    if (!slash)
    {
        *dir_out  = strdup(".");
        *file_out = strdup(tmp);
    }
    else
    {
        *slash    = '\0';
        *dir_out  = strdup(tmp);
        *file_out = strdup(slash + 1);
    }

    free(tmp);
}

static void codegen_debug_init(CodegenContext *ctx)
{
    if (!ctx->debug_info || ctx->di_builder)
        return;

    const char *source_path = ctx->source_file;
    if (!source_path || source_path[0] == '\0')
    {
        ctx->debug_info = false;
        return;
    }

    char  resolved[PATH_MAX];
    char *full_path = NULL;
#ifdef _WIN32
    if (_fullpath(resolved, source_path, PATH_MAX))
        full_path = strdup(resolved);
    else
        full_path = strdup(source_path);
#else
    if (realpath(source_path, resolved))
        full_path = strdup(resolved);
    else
        full_path = strdup(source_path);
#endif

    if (!full_path)
    {
        ctx->debug_info = false;
        return;
    }

    char *dir  = NULL;
    char *file = NULL;
    codegen_debug_split_path(full_path, &dir, &file);
    if (!dir || !file)
    {
        free(full_path);
        free(dir);
        free(file);
        ctx->debug_info = false;
        return;
    }

    ctx->debug_full_path = full_path;
    ctx->debug_dir       = dir;
    ctx->debug_file      = file;

    ctx->di_builder = LLVMCreateDIBuilder(ctx->module);
    if (!ctx->di_builder)
    {
        ctx->debug_info = false;
        return;
    }

    LLVMSetIsNewDbgInfoFormat(ctx->module, 1);

    ctx->di_unknown_type = NULL;

    LLVMValueRef    debug_version_val = LLVMConstInt(LLVMInt32TypeInContext(ctx->context), LLVMDebugMetadataVersion(), false);
    LLVMMetadataRef debug_version     = LLVMValueAsMetadata(debug_version_val);
    LLVMAddModuleFlag(ctx->module, LLVMModuleFlagBehaviorWarning, "Debug Info Version", strlen("Debug Info Version"), debug_version);
    LLVMValueRef    dwarf_version_val = LLVMConstInt(LLVMInt32TypeInContext(ctx->context), 5, false);
    LLVMMetadataRef dwarf_version     = LLVMValueAsMetadata(dwarf_version_val);
    LLVMAddModuleFlag(ctx->module, LLVMModuleFlagBehaviorWarning, "Dwarf Version", strlen("Dwarf Version"), dwarf_version);

    ctx->di_file         = LLVMDIBuilderCreateFile(ctx->di_builder, ctx->debug_file, strlen(ctx->debug_file), ctx->debug_dir, strlen(ctx->debug_dir));
    ctx->di_compile_unit = LLVMDIBuilderCreateCompileUnit(ctx->di_builder, LLVMDWARFSourceLanguageC99, ctx->di_file, "mach-c", strlen("mach-c"), ctx->opt_level > 0, "", 0, 0, "", 0, LLVMDWARFEmissionFull, 0, false, false, "", 0, "", 0);

    ctx->current_di_scope = ctx->di_compile_unit;
    ctx->debug_finalized  = false;
}

static void codegen_debug_finalize(CodegenContext *ctx)
{
    if (!ctx->debug_info || !ctx->di_builder || ctx->debug_finalized)
        return;
    LLVMDIBuilderFinalize(ctx->di_builder);
    ctx->debug_finalized = true;
}

static LLVMMetadataRef codegen_get_current_scope(CodegenContext *ctx)
{
    if (ctx->current_di_scope)
        return ctx->current_di_scope;
    return ctx->di_compile_unit;
}

static void codegen_set_debug_location(CodegenContext *ctx, AstNode *node)
{
    if (!ctx->debug_info || !ctx->di_builder)
        return;
    if (!node || !node->token || !ctx->source_lexer)
        return;

    int             line  = lexer_get_pos_line(ctx->source_lexer, node->token->pos);
    int             col   = lexer_get_pos_line_offset(ctx->source_lexer, node->token->pos);
    LLVMMetadataRef scope = codegen_get_current_scope(ctx);
    if (!scope)
        return;

    LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(ctx->context, (unsigned)(line + 1), (unsigned)(col + 1), scope, NULL);
    LLVMSetCurrentDebugLocation2(ctx->builder, loc);
}

static LLVMMetadataRef codegen_debug_get_unknown_type(CodegenContext *ctx)
{
    if (!ctx->di_builder)
        return NULL;
    if (!ctx->di_unknown_type)
    {
        const char *name     = "u64";
        ctx->di_unknown_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, name, strlen(name), 0, 64, 0);
    }
    return ctx->di_unknown_type;
}

static LLVMMetadataRef codegen_debug_get_type(CodegenContext *ctx, Type *type)
{
    if (!ctx->debug_info || !ctx->di_builder)
        return codegen_debug_get_unknown_type(ctx);
    if (!type)
        return codegen_debug_get_unknown_type(ctx);

    Type *resolved = type_resolve_alias(type);
    if (!resolved)
        resolved = type;

    // check cache - including entries being built (NULL means in-progress)
    for (int i = 0; i < ctx->di_type_cache.count; i++)
    {
        if (ctx->di_type_cache.types[i] == resolved)
        {
            // if we found it with a NULL di_type, we're in a recursive call
            // return unknown type to break the cycle
            if (!ctx->di_type_cache.di_types[i])
                return codegen_debug_get_unknown_type(ctx);
            return ctx->di_type_cache.di_types[i];
        }
    }

    if (ctx->di_type_cache.count >= ctx->di_type_cache.capacity)
    {
        ctx->di_type_cache.capacity = ctx->di_type_cache.capacity ? ctx->di_type_cache.capacity * 2 : 16;
        ctx->di_type_cache.types    = realloc(ctx->di_type_cache.types, sizeof(Type *) * ctx->di_type_cache.capacity);
        ctx->di_type_cache.di_types = realloc(ctx->di_type_cache.di_types, sizeof(LLVMMetadataRef) * ctx->di_type_cache.capacity);
    }

    // reserve entry immediately with NULL to detect cycles
    const int index                    = ctx->di_type_cache.count++;
    ctx->di_type_cache.types[index]    = resolved;
    ctx->di_type_cache.di_types[index] = NULL;

    const size_t    size_bytes  = type_sizeof(resolved);
    const uint64_t  size_bits   = (uint64_t)size_bytes * 8;
    const size_t    align_bytes = type_alignof(resolved);
    const unsigned  align_bits  = (unsigned)(align_bytes * 8);
    LLVMMetadataRef di_type     = NULL;

    // standard DWARF type encodings from DWARF spec
    const unsigned DW_ATE_unsigned = 0x07;
    const unsigned DW_ATE_signed   = 0x05;
    const unsigned DW_ATE_float    = 0x04;

    switch (resolved->kind)
    {
    case TYPE_U8:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "u8", 2, 8, DW_ATE_unsigned, 0);
        break;
    case TYPE_U16:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "u16", 3, 16, DW_ATE_unsigned, 0);
        break;
    case TYPE_U32:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "u32", 3, 32, DW_ATE_unsigned, 0);
        break;
    case TYPE_U64:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "u64", 3, 64, DW_ATE_unsigned, 0);
        break;
    case TYPE_I8:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "i8", 2, 8, DW_ATE_signed, 0);
        break;
    case TYPE_I16:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "i16", 3, 16, DW_ATE_signed, 0);
        break;
    case TYPE_I32:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "i32", 3, 32, DW_ATE_signed, 0);
        break;
    case TYPE_I64:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "i64", 3, 64, DW_ATE_signed, 0);
        break;
    case TYPE_F16:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "f16", 3, 16, DW_ATE_float, 0);
        break;
    case TYPE_F32:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "f32", 3, 32, DW_ATE_float, 0);
        break;
    case TYPE_F64:
        di_type = LLVMDIBuilderCreateBasicType(ctx->di_builder, "f64", 3, 64, DW_ATE_float, 0);
        break;
    case TYPE_PTR:
    case TYPE_POINTER:
    {
        LLVMMetadataRef base = resolved->kind == TYPE_POINTER && resolved->pointer.base ? codegen_debug_get_type(ctx, resolved->pointer.base) : codegen_debug_get_unknown_type(ctx);
        if (resolved->kind == TYPE_POINTER && resolved->pointer.is_read_only)
        {
            base = LLVMDIBuilderCreateQualifiedType(ctx->di_builder, 38, base); // 38 = DW_TAG_const_type
        }
        di_type = LLVMDIBuilderCreatePointerType(ctx->di_builder, base, size_bits, align_bits, 0, "", 0);
        break;
    }
    case TYPE_ARRAY:
    {
        LLVMMetadataRef elem_type     = codegen_debug_get_type(ctx, resolved->array.elem_type);
        LLVMMetadataRef subrange      = LLVMDIBuilderGetOrCreateSubrange(ctx->di_builder, 0, (int64_t)resolved->array.size);
        LLVMMetadataRef subscripts[1] = {subrange};
        di_type                       = LLVMDIBuilderCreateArrayType(ctx->di_builder, size_bits, align_bits, elem_type, subscripts, 1);
        break;
    }
    case TYPE_STRUCT:
    case TYPE_STR:
    {
        const char      *name       = resolved->name ? resolved->name : "anon.struct";
        LLVMMetadataRef  scope      = ctx->di_compile_unit ? ctx->di_compile_unit : ctx->di_file;
        LLVMMetadataRef  file       = ctx->di_file ? ctx->di_file : scope;
        LLVMMetadataRef *members    = NULL;
        unsigned         member_cnt = 0;
        if (resolved->composite.field_count > 0 && resolved->composite.fields)
        {
            member_cnt = (unsigned)resolved->composite.field_count;
            members    = malloc(sizeof(LLVMMetadataRef) * member_cnt);
            if (members)
            {
                Symbol  *field_iter = resolved->composite.fields;
                unsigned i          = 0;
                while (field_iter && i < member_cnt)
                {
                    Symbol *field                     = field_iter;
                    field_iter                        = field_iter->next;
                    const char     *fname             = field->name ? field->name : "field";
                    Type           *field_type        = field->type ? field->type : type_u64();
                    LLVMMetadataRef field_di_type     = codegen_debug_get_type(ctx, field_type);
                    uint64_t        field_size_bits   = (uint64_t)type_sizeof(field_type) * 8;
                    uint32_t        field_align_bits  = (uint32_t)(type_alignof(field_type) * 8);
                    uint64_t        field_offset_bits = (uint64_t)field->field.offset * 8;

                    unsigned field_line = 0;
                    if (field->decl && field->decl->token && ctx->source_lexer)
                        field_line = (unsigned)(lexer_get_pos_line(ctx->source_lexer, field->decl->token->pos) + 1);

                    members[i] = LLVMDIBuilderCreateMemberType(ctx->di_builder, scope, fname, strlen(fname), file, field_line, field_size_bits, field_align_bits, field_offset_bits, LLVMDIFlagZero, field_di_type);
                    i++;
                }
                member_cnt = i;
            }
        }

        di_type = LLVMDIBuilderCreateStructType(ctx->di_builder, scope, name, strlen(name), file, 0, size_bits, align_bits, LLVMDIFlagZero, NULL, members, members ? member_cnt : 0, 0, NULL, "", 0);
        free(members);
        break;
    }
    case TYPE_UNION:
    {
        const char      *name       = resolved->name ? resolved->name : "anon.union";
        LLVMMetadataRef  scope      = ctx->di_compile_unit ? ctx->di_compile_unit : ctx->di_file;
        LLVMMetadataRef  file       = ctx->di_file ? ctx->di_file : scope;
        LLVMMetadataRef *members    = NULL;
        unsigned         member_cnt = 0;
        if (resolved->composite.field_count > 0 && resolved->composite.fields)
        {
            member_cnt = (unsigned)resolved->composite.field_count;
            members    = malloc(sizeof(LLVMMetadataRef) * member_cnt);
            if (members)
            {
                Symbol  *field_iter = resolved->composite.fields;
                unsigned i          = 0;
                while (field_iter && i < member_cnt)
                {
                    Symbol *field                    = field_iter;
                    field_iter                       = field_iter->next;
                    const char     *fname            = field->name ? field->name : "field";
                    Type           *field_type       = field->type ? field->type : type_u64();
                    LLVMMetadataRef field_di_type    = codegen_debug_get_type(ctx, field_type);
                    uint64_t        field_size_bits  = (uint64_t)type_sizeof(field_type) * 8;
                    uint32_t        field_align_bits = (uint32_t)(type_alignof(field_type) * 8);

                    unsigned field_line = 0;
                    if (field->decl && field->decl->token && ctx->source_lexer)
                        field_line = (unsigned)(lexer_get_pos_line(ctx->source_lexer, field->decl->token->pos) + 1);

                    members[i] = LLVMDIBuilderCreateMemberType(ctx->di_builder, scope, fname, strlen(fname), file, field_line, field_size_bits, field_align_bits, 0, LLVMDIFlagZero, field_di_type);
                    i++;
                }
                member_cnt = i;
            }
        }

        di_type = LLVMDIBuilderCreateUnionType(ctx->di_builder, scope, name, strlen(name), file, 0, size_bits, align_bits, LLVMDIFlagZero, members, members ? member_cnt : 0, 0, "", 0);
        free(members);
        break;
    }
    case TYPE_FUNCTION:
    {
        size_t total = resolved->function.param_count + 1;
        if (total == 0)
        {
            di_type = LLVMDIBuilderCreatePointerType(ctx->di_builder, codegen_debug_get_unknown_type(ctx), size_bits, align_bits, 0, "", 0);
            break;
        }

        LLVMMetadataRef *params = malloc(sizeof(LLVMMetadataRef) * total);
        if (!params)
        {
            di_type = codegen_debug_get_unknown_type(ctx);
            break;
        }

        params[0] = resolved->function.return_type ? codegen_debug_get_type(ctx, resolved->function.return_type) : codegen_debug_get_unknown_type(ctx);
        for (size_t i = 0; i < resolved->function.param_count; i++)
        {
            Type *param_type = resolved->function.param_types[i];
            params[i + 1]    = codegen_debug_get_type(ctx, param_type);
        }

        LLVMMetadataRef subroutine = LLVMDIBuilderCreateSubroutineType(ctx->di_builder, ctx->di_file ? ctx->di_file : ctx->di_compile_unit, params, (unsigned)total, 0);
        free(params);
        if (!subroutine)
        {
            di_type = codegen_debug_get_unknown_type(ctx);
            break;
        }

        di_type = LLVMDIBuilderCreatePointerType(ctx->di_builder, subroutine, size_bits, align_bits, 0, "", 0);
        break;
    }
    case TYPE_ALIAS:
        di_type = codegen_debug_get_type(ctx, resolved->alias.target);
        break;
    default:
        di_type = codegen_debug_get_unknown_type(ctx);
        break;
    }

    if (!di_type)
        di_type = codegen_debug_get_unknown_type(ctx);

    ctx->di_type_cache.di_types[index] = di_type;
    return di_type;
}

static bool codegen_debug_push_scope(CodegenContext *ctx, AstNode *node, LLVMMetadataRef *out_prev_scope)
{
    if (!ctx->debug_info || !ctx->di_builder)
        return false;

    LLVMMetadataRef parent_scope = codegen_get_current_scope(ctx);
    if (!parent_scope)
        return false;

    unsigned line = 0;
    unsigned col  = 0;
    if (node && node->token && ctx->source_lexer)
    {
        line = (unsigned)(lexer_get_pos_line(ctx->source_lexer, node->token->pos) + 1);
        col  = (unsigned)(lexer_get_pos_line_offset(ctx->source_lexer, node->token->pos) + 1);
    }

    LLVMMetadataRef file_meta = ctx->di_file ? ctx->di_file : ctx->di_compile_unit;
    LLVMMetadataRef block     = LLVMDIBuilderCreateLexicalBlock(ctx->di_builder, parent_scope, file_meta, line, col);
    if (!block)
        return false;

    if (out_prev_scope)
        *out_prev_scope = ctx->current_di_scope;

    ctx->current_di_scope = block;
    return true;
}

static void codegen_debug_pop_scope(CodegenContext *ctx, LLVMMetadataRef previous_scope, bool pushed)
{
    if (!pushed || !ctx->debug_info || !ctx->di_builder)
        return;

    if (previous_scope)
        ctx->current_di_scope = previous_scope;
    else
        ctx->current_di_scope = ctx->di_compile_unit;
}

static void codegen_debug_declare_symbol(CodegenContext *ctx, Symbol *symbol, Type *type, LLVMValueRef storage, AstNode *node, bool is_parameter, unsigned arg_index)
{
    if (!ctx->debug_info || !ctx->di_builder || !ctx->current_function)
        return;
    if (!storage)
        return;

    const char *name = NULL;
    if (symbol && symbol->name)
        name = symbol->name;
    else if (node && (node->kind == AST_STMT_VAR || node->kind == AST_STMT_VAL))
        name = node->var_stmt.name;
    else if (node && node->kind == AST_STMT_PARAM)
        name = node->param_stmt.name;

    if (!name || !name[0])
        return;

    unsigned line = 0;
    unsigned col  = 0;
    if (node && node->token && ctx->source_lexer)
    {
        line = (unsigned)(lexer_get_pos_line(ctx->source_lexer, node->token->pos) + 1);
        col  = (unsigned)(lexer_get_pos_line_offset(ctx->source_lexer, node->token->pos) + 1);
    }

    LLVMMetadataRef scope = codegen_get_current_scope(ctx);
    if (!scope)
        scope = ctx->di_compile_unit;

    LLVMMetadataRef file_meta = ctx->di_file ? ctx->di_file : scope;
    LLVMMetadataRef di_type   = codegen_debug_get_type(ctx, type);
    if (!di_type)
        di_type = codegen_debug_get_unknown_type(ctx);

    LLVMMetadataRef var_meta = NULL;
    if (is_parameter)
    {
        if (arg_index == 0)
            arg_index = 1;
        var_meta = LLVMDIBuilderCreateParameterVariable(ctx->di_builder, scope, name, strlen(name), arg_index, file_meta, line, di_type, true, LLVMDIFlagZero);
    }
    else
    {
        unsigned align_bits = (unsigned)(type ? type_alignof(type) * 8 : 0);
        var_meta            = LLVMDIBuilderCreateAutoVariable(ctx->di_builder, scope, name, strlen(name), file_meta, line, di_type, true, LLVMDIFlagZero, align_bits);
    }

    if (!var_meta)
        return;

    LLVMMetadataRef   expr      = LLVMDIBuilderCreateExpression(ctx->di_builder, NULL, 0);
    LLVMBasicBlockRef insert_at = LLVMGetInsertBlock(ctx->builder);
    if (!expr || !insert_at)
        return;

    LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(ctx->context, line, col, scope, NULL);
    if (!loc)
        loc = LLVMDIBuilderCreateDebugLocation(ctx->context, line, col, ctx->di_compile_unit ? ctx->di_compile_unit : scope, NULL);

    if (loc)
    {
        LLVMDIBuilderInsertDeclareRecordAtEnd(ctx->di_builder, storage, var_meta, expr, loc, insert_at);
    }
}

static LLVMMetadataRef codegen_debug_create_subprogram(CodegenContext *ctx, AstNode *stmt, LLVMValueRef func, const char *display_name, const char *link_name, size_t param_count)
{
    if (!ctx->debug_info || !ctx->di_builder || !func)
        return NULL;

    LLVMMetadataRef scope = ctx->di_compile_unit ? ctx->di_compile_unit : ctx->di_file;
    if (!scope || !ctx->di_file)
        return NULL;

    unsigned line = 1;
    if (stmt && stmt->token && ctx->source_lexer)
        line = (unsigned)(lexer_get_pos_line(ctx->source_lexer, stmt->token->pos) + 1);

    size_t           total_params = param_count + 1; // include return type slot
    LLVMMetadataRef *param_types  = malloc(sizeof(LLVMMetadataRef) * total_params);
    if (!param_types)
        return NULL;

    LLVMMetadataRef fallback = codegen_debug_get_unknown_type(ctx);

    // set return type (slot 0)
    if (stmt && stmt->type && stmt->type->kind == TYPE_FUNCTION && stmt->type->function.return_type)
    {
        param_types[0] = codegen_debug_get_type(ctx, stmt->type->function.return_type);
    }
    else
    {
        param_types[0] = fallback;
    }

    // set parameter types (slots 1..n)
    if (stmt && stmt->type && stmt->type->kind == TYPE_FUNCTION)
    {
        for (size_t i = 0; i < param_count && i < stmt->type->function.param_count; i++)
        {
            param_types[i + 1] = codegen_debug_get_type(ctx, stmt->type->function.param_types[i]);
        }
        // fill remaining with fallback if param_count > type->function.param_count
        for (size_t i = stmt->type->function.param_count; i < param_count; i++)
        {
            param_types[i + 1] = fallback;
        }
    }
    else
    {
        for (size_t i = 1; i < total_params; i++)
        {
            param_types[i] = fallback;
        }
    }

    LLVMMetadataRef subroutine_type = LLVMDIBuilderCreateSubroutineType(ctx->di_builder, ctx->di_file, param_types, (unsigned)total_params, 0);
    free(param_types);
    if (!subroutine_type)
        return NULL;

    const char *disp = display_name && display_name[0] ? display_name : (link_name && link_name[0] ? link_name : "<function>");
    const char *link = link_name && link_name[0] ? link_name : disp;

    LLVMMetadataRef subprogram = LLVMDIBuilderCreateFunction(ctx->di_builder, scope, disp, strlen(disp), link, strlen(link), ctx->di_file, line, subroutine_type, false, true, line, LLVMDIFlagZero, ctx->opt_level > 0);

    LLVMSetSubprogram(func, subprogram);
    return subprogram;
}

static void codegen_declare_function_symbol(CodegenContext *ctx, Symbol *sym)
{
    if (!ctx || !sym)
        return;

    if (sym->kind != SYMBOL_FUNC)
        return;

    // skip generic templates - only declare concrete functions and specialized instances
    if (sym->func.is_generic && !sym->func.is_specialized_instance)
        return;

    if (codegen_get_symbol_value(ctx, sym))
        return;

    Type *func_type = type_resolve_alias(sym->type);
    if (!func_type || func_type->kind != TYPE_FUNCTION)
        return;

    LLVMTypeRef return_type = func_type->function.return_type ? codegen_get_llvm_type(ctx, func_type->function.return_type) : LLVMVoidTypeInContext(ctx->context);

    bool   uses_mach_varargs = sym->func.uses_mach_varargs;
    size_t fixed_params      = func_type->function.param_count;
    size_t llvm_param_count  = fixed_params + (uses_mach_varargs ? 2 : 0);

    LLVMTypeRef *param_types = NULL;
    if (llvm_param_count > 0)
    {
        param_types = malloc(sizeof(LLVMTypeRef) * llvm_param_count);
        if (!param_types)
            return;

        for (size_t i = 0; i < fixed_params; i++)
        {
            param_types[i] = codegen_get_llvm_type(ctx, func_type->function.param_types[i]);
        }

        if (uses_mach_varargs)
        {
            LLVMTypeRef i64ty             = LLVMInt64TypeInContext(ctx->context);
            LLVMTypeRef i8_ptr            = LLVMPointerTypeInContext(ctx->context, 0);
            param_types[fixed_params]     = i64ty;
            param_types[fixed_params + 1] = LLVMPointerType(i8_ptr, 0);
        }
    }

    LLVMTypeRef llvm_func_type = LLVMFunctionType(return_type, param_types, llvm_param_count, uses_mach_varargs ? false : func_type->function.is_variadic);

    const char *llvm_name = NULL;
    if (sym->func.mangled_name && sym->func.mangled_name[0])
        llvm_name = sym->func.mangled_name;
    else if (sym->func.extern_name && sym->func.extern_name[0])
        llvm_name = sym->func.extern_name;
    else
        llvm_name = sym->name;

    LLVMValueRef func = NULL;
    if (llvm_name && llvm_name[0])
    {
        func = LLVMGetNamedFunction(ctx->module, llvm_name);
        if (!func)
            func = codegen_add_function(ctx, llvm_name, llvm_func_type);
    }

    if (param_types)
        free(param_types);

    if (func)
        codegen_set_symbol_value(ctx, sym, func);
}

static void codegen_declare_functions_in_scope(CodegenContext *ctx, Scope *scope)
{
    if (!scope)
        return;

    for (Symbol *sym = scope->symbols; sym; sym = sym->next)
    {
        codegen_declare_function_symbol(ctx, sym);
    }
}

static void codegen_generate_specialized_function(Symbol *sym, void *user_data)
{
    CodegenContext *ctx = (CodegenContext *)user_data;

    // only generate specialized functions
    if (sym->kind != SYMBOL_FUNC || !sym->func.is_specialized_instance || !sym->func.is_defined || !sym->decl)
        return;

    // check if the function already exists in the LLVM module by name
    // (multiple Symbol objects may exist for the same specialized function)
    const char  *func_name          = sym->func.mangled_name ? sym->func.mangled_name : sym->name;
    LLVMValueRef existing_in_module = LLVMGetNamedFunction(ctx->module, func_name);
    if (existing_in_module && !LLVMIsDeclaration(existing_in_module))
    {
        // already has a body in the module, skip and update symbol mapping
        codegen_set_symbol_value(ctx, sym, existing_in_module);
        return;
    }

    // check if function has already been declared (we'll generate the body anyway if it's just a declaration)
    LLVMValueRef existing = codegen_get_symbol_value(ctx, sym);
    if (existing && !LLVMIsDeclaration(existing))
    {
        // already has a body, skip
        return;
    }

    // generate the function using its own cloned, analyzed AST
    // (each specialization has its own AST with correct scope bindings)
    codegen_stmt_fun(ctx, sym->decl);
}

static void codegen_generate_specialized_functions_in_scope(CodegenContext *ctx, Scope *scope)
{
    if (!scope)
        return;

    // walk all symbols and generate bodies for specialized function instances
    for (Symbol *sym = scope->symbols; sym; sym = sym->next)
    {
        codegen_generate_specialized_function(sym, ctx);
    }
}

void codegen_context_init(CodegenContext *ctx, const char *module_name, bool no_pie)
{
    ctx->context = LLVMContextCreate();

    // normalize module name for LLVM module: replace dots and slashes with underscores
    char *normalized_name = NULL;
    if (module_name)
    {
        size_t len      = strlen(module_name);
        normalized_name = malloc(len + 1);
        for (size_t i = 0; i < len; i++)
        {
            char ch            = module_name[i];
            normalized_name[i] = (ch == '.' || ch == '/') ? '_' : ch;
        }
        normalized_name[len] = '\0';
    }

    ctx->module = LLVMModuleCreateWithNameInContext(normalized_name ? normalized_name : "module", ctx->context);
    free(normalized_name);

    ctx->builder = LLVMCreateBuilderInContext(ctx->context);
    ctx->no_pie  = no_pie;

    // initialize target
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    char         *error = NULL;
    LLVMTargetRef target;
    if (LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &error) != 0)
    {
        fprintf(stderr, "error: failed to get target: %s\n", error);
        LLVMDisposeMessage(error);
        exit(EXIT_FAILURE);
    }

    ctx->target_machine = LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), LLVMGetHostCPUName(), LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, ctx->no_pie ? LLVMRelocStatic : LLVMRelocPIC, LLVMCodeModelDefault);

    ctx->data_layout = LLVMCreateTargetDataLayout(ctx->target_machine);
    LLVMSetModuleDataLayout(ctx->module, ctx->data_layout);
    LLVMSetTarget(ctx->module, LLVMGetDefaultTargetTriple());

    ctx->spec_cache = NULL;

    // initialize deferred stack
    ctx->deferred_stack.blocks   = NULL;
    ctx->deferred_stack.count    = 0;
    ctx->deferred_stack.capacity = 0;

    // initialize maps
    ctx->symbol_map.symbols  = NULL;
    ctx->symbol_map.values   = NULL;
    ctx->symbol_map.count    = 0;
    ctx->symbol_map.capacity = 0;

    ctx->type_cache.types      = NULL;
    ctx->type_cache.llvm_types = NULL;
    ctx->type_cache.count      = 0;
    ctx->type_cache.capacity   = 0;

    ctx->di_type_cache.types    = NULL;
    ctx->di_type_cache.di_types = NULL;
    ctx->di_type_cache.count    = 0;
    ctx->di_type_cache.capacity = 0;

    ctx->current_function        = NULL;
    ctx->current_function_type   = NULL;
    ctx->break_block             = NULL;
    ctx->continue_block          = NULL;
    ctx->generating_mutable_init = false;
    ctx->module_inline_asm       = NULL;
    ctx->module_inline_asm_len   = 0;

    ctx->errors     = NULL;
    ctx->has_errors = false;

    ctx->opt_level             = 2;
    ctx->debug_info            = false;
    ctx->debug_finalized       = false;
    ctx->di_builder            = NULL;
    ctx->di_compile_unit       = NULL;
    ctx->di_file               = NULL;
    ctx->current_di_scope      = NULL;
    ctx->current_di_subprogram = NULL;
    ctx->debug_full_path       = NULL;
    ctx->debug_dir             = NULL;
    ctx->debug_file            = NULL;
    ctx->di_unknown_type       = NULL;

    ctx->current_vararg_count_value = NULL;
    ctx->current_vararg_array       = NULL;
    ctx->current_fixed_param_count  = 0;
    ctx->source_file                = NULL;

    ctx->iota_counter = 0;
    ctx->source_lexer = NULL;
}

void codegen_context_dnit(CodegenContext *ctx)
{
    // clean up errors
    CodegenError *error = ctx->errors;
    while (error)
    {
        CodegenError *next = error->next;
        free(error->message);
        free(error);
        error = next;
    }

    // clean up maps
    free(ctx->symbol_map.symbols);
    free(ctx->symbol_map.values);
    free(ctx->type_cache.types);
    free(ctx->type_cache.llvm_types);
    free(ctx->di_type_cache.types);
    free(ctx->di_type_cache.di_types);
    free(ctx->deferred_stack.blocks);

    if (ctx->di_builder)
    {
        codegen_debug_finalize(ctx);
        LLVMDisposeDIBuilder(ctx->di_builder);
    }
    free(ctx->debug_full_path);
    free(ctx->debug_dir);
    free(ctx->debug_file);

    // clean up llvm
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->module);
    LLVMDisposeTargetMachine(ctx->target_machine);
    LLVMContextDispose(ctx->context);
    free(ctx->module_inline_asm);

    // no heap state for varargs
}

static void codegen_generate_specialized_functions_in_scope(CodegenContext *ctx, Scope *scope);

bool codegen_generate(CodegenContext *ctx, AstNode *root, SymbolTable *symbols)
{
    if (root->kind != AST_PROGRAM)
    {
        codegen_error(ctx, root, "expected program node");
        return false;
    }

    if (ctx->debug_info)
        codegen_debug_init(ctx);

    if (symbols)
    {
        codegen_declare_functions_in_scope(ctx, symbols->global_scope);
        codegen_declare_functions_in_scope(ctx, symbols->module_scope);
    }

    // generate top-level statements
    for (int i = 0; i < root->program.stmts->count; i++)
    {
        AstNode *stmt = root->program.stmts->items[i];
        codegen_stmt(ctx, stmt);
    }

    // generate specialized generic function instances from local scopes
    if (symbols)
    {
        codegen_generate_specialized_functions_in_scope(ctx, symbols->global_scope);
        codegen_generate_specialized_functions_in_scope(ctx, symbols->module_scope);
    }

    // generate ALL specialized functions from global cache
    // Each specialization has its own cloned AST with proper scope bindings,
    // so we can safely generate them in any module. The linker will deduplicate
    // via linkonce_odr linkage (standard C++ template model)
    if (ctx->spec_cache)
    {
        specialization_cache_foreach(ctx->spec_cache, codegen_generate_specialized_function, ctx);
    }

    if (ctx->debug_info)
        codegen_debug_finalize(ctx);

    // verify module (do not abort process; print diagnostics)
    char *error = NULL;
    if (LLVMVerifyModule(ctx->module, LLVMPrintMessageAction, &error) != 0)
    {
        fprintf(stderr, "error: module verification failed: %s\n", error);
        LLVMDisposeMessage(error);
        const char *debug_path = "/tmp/cmach_verify.ll";
        char       *dump_error = NULL;
        if (LLVMPrintModuleToFile(ctx->module, debug_path, &dump_error) != 0)
        {
            fprintf(stderr, "error: failed to dump module IR: %s\n", dump_error);
            LLVMDisposeMessage(dump_error);
        }
        else
        {
            fprintf(stderr, "note: wrote failing module IR to %s\n", debug_path);
        }
        ctx->has_errors = true;
    }

    // run optimization passes
    // note: optimizations now run even with debug info enabled, as modern LLVM handles debug info correctly
    if (ctx->opt_level > 0 && !ctx->has_errors)
    {
        LLVMPassBuilderOptionsRef options = LLVMCreatePassBuilderOptions();

        char opt_str[16];
        snprintf(opt_str, sizeof(opt_str), "default<O%d>", ctx->opt_level);

        LLVMRunPasses(ctx->module, opt_str, ctx->target_machine, options);
        LLVMDisposePassBuilderOptions(options);
    }

    return !ctx->has_errors;
}

bool codegen_emit_object(CodegenContext *ctx, const char *filename)
{
    char *error = NULL;
    if (LLVMTargetMachineEmitToFile(ctx->target_machine, ctx->module, (char *)filename, LLVMObjectFile, &error) != 0)
    {
        fprintf(stderr, "error: failed to emit object file: %s\n", error);
        LLVMDisposeMessage(error);
        return false;
    }
    return true;
}

bool codegen_emit_llvm_ir(CodegenContext *ctx, const char *filename)
{
    char *error = NULL;
    if (LLVMPrintModuleToFile(ctx->module, filename, &error) != 0)
    {
        fprintf(stderr, "error: failed to emit llvm ir: %s\n", error);
        LLVMDisposeMessage(error);
        return false;
    }
    return true;
}

bool codegen_emit_assembly(CodegenContext *ctx, const char *filename)
{
    char *error = NULL;
    if (LLVMTargetMachineEmitToFile(ctx->target_machine, ctx->module, (char *)filename, LLVMAssemblyFile, &error) != 0)
    {
        fprintf(stderr, "error: failed to emit assembly: %s\n", error);
        LLVMDisposeMessage(error);
        return false;
    }
    return true;
}

void codegen_error(CodegenContext *ctx, AstNode *node, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    size_t len = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char *message = malloc(len);
    va_start(args, fmt);
    vsnprintf(message, len, fmt, args);
    va_end(args);

    CodegenError *error = malloc(sizeof(CodegenError));
    error->message      = message;
    error->node         = node;
    error->next         = ctx->errors;
    ctx->errors         = error;
    ctx->has_errors     = true;
}

void codegen_print_errors(CodegenContext *ctx)
{
    for (CodegenError *error = ctx->errors; error; error = error->next)
    {
        if (ctx->source_lexer && error->node && error->node->token)
        {
            int   line = lexer_get_pos_line(ctx->source_lexer, error->node->token->pos);
            int   col  = lexer_get_pos_line_offset(ctx->source_lexer, error->node->token->pos) + 1;
            char *text = lexer_get_line_text(ctx->source_lexer, line);
            fprintf(stderr, "%s:%d:%d: codegen error: %s\n", ctx->source_file ? ctx->source_file : "<unknown>", line + 1, col, error->message);
            if (text)
            {
                fprintf(stderr, "%s\n", text);
                // simple caret underline
                for (int i = 0; i < col - 1; i++)
                    fputc(' ', stderr);
                fputc('^', stderr);
                fputc('\n', stderr);
                free(text);
            }
        }
        else
        {
            fprintf(stderr, "codegen error: %s\n", error->message);
        }
    }
}

static LLVMTypeRef codegen_pointer_int_type(CodegenContext *ctx)
{
    if (!ctx)
    {
        return NULL;
    }

    LLVMTypeRef raw_ptr_type = LLVMPointerTypeInContext(ctx->context, 0);

    if (!ctx->data_layout)
    {
        return LLVMInt64TypeInContext(ctx->context);
    }

    uint64_t pointer_bits = LLVMSizeOfTypeInBits(ctx->data_layout, raw_ptr_type);

    if (pointer_bits <= 16)
        return LLVMInt16TypeInContext(ctx->context);
    if (pointer_bits <= 32)
        return LLVMInt32TypeInContext(ctx->context);
    if (pointer_bits <= 64)
        return LLVMInt64TypeInContext(ctx->context);

    return LLVMInt64TypeInContext(ctx->context);
}

LLVMTypeRef codegen_get_llvm_type(CodegenContext *ctx, Type *type)
{
    if (!type)
        return NULL;

    // check cache
    LLVMTypeRef reused_llvm = NULL;
    for (int i = 0; i < ctx->type_cache.count; i++)
    {
        if (ctx->type_cache.types[i] == type)
        {
            return ctx->type_cache.llvm_types[i];
        }

        // reuse struct/union types by name even if Type instances differ
        if (!reused_llvm)
        {
            Type *cached_type = ctx->type_cache.types[i];
            if (type->name && cached_type && cached_type->name && type->kind == cached_type->kind && (type->kind == TYPE_STRUCT || type->kind == TYPE_UNION) && strcmp(type->name, cached_type->name) == 0)
            {
                reused_llvm = ctx->type_cache.llvm_types[i];
            }
        }
    }

    // if we found a reusable type, cache this Type pointer and return
    if (reused_llvm)
    {
        if (ctx->type_cache.count >= ctx->type_cache.capacity)
        {
            ctx->type_cache.capacity   = ctx->type_cache.capacity ? ctx->type_cache.capacity * 2 : 16;
            ctx->type_cache.types      = realloc(ctx->type_cache.types, sizeof(Type *) * ctx->type_cache.capacity);
            ctx->type_cache.llvm_types = realloc(ctx->type_cache.llvm_types, sizeof(LLVMTypeRef) * ctx->type_cache.capacity);
        }

        ctx->type_cache.types[ctx->type_cache.count]      = type;
        ctx->type_cache.llvm_types[ctx->type_cache.count] = reused_llvm;
        ctx->type_cache.count++;

        return reused_llvm;
    }

    LLVMTypeRef llvm_type = NULL;

    switch (type->kind)
    {
    case TYPE_U8:
        llvm_type = LLVMInt8TypeInContext(ctx->context);
        break;
    case TYPE_U16:
        llvm_type = LLVMInt16TypeInContext(ctx->context);
        break;
    case TYPE_U32:
        llvm_type = LLVMInt32TypeInContext(ctx->context);
        break;
    case TYPE_U64:
        llvm_type = LLVMInt64TypeInContext(ctx->context);
        break;
    case TYPE_I8:
        llvm_type = LLVMInt8TypeInContext(ctx->context);
        break;
    case TYPE_I16:
        llvm_type = LLVMInt16TypeInContext(ctx->context);
        break;
    case TYPE_I32:
        llvm_type = LLVMInt32TypeInContext(ctx->context);
        break;
    case TYPE_I64:
        llvm_type = LLVMInt64TypeInContext(ctx->context);
        break;
    case TYPE_F16:
        llvm_type = LLVMHalfTypeInContext(ctx->context);
        break;
    case TYPE_F32:
        llvm_type = LLVMFloatTypeInContext(ctx->context);
        break;
    case TYPE_F64:
        llvm_type = LLVMDoubleTypeInContext(ctx->context);
        break;
    case TYPE_PTR:
        llvm_type = LLVMPointerTypeInContext(ctx->context, 0);
        break;
    case TYPE_POINTER:
        llvm_type = LLVMPointerTypeInContext(ctx->context, 0);
        break;
    case TYPE_ARRAY:
    {
        LLVMTypeRef elem_type = codegen_get_llvm_type(ctx, type->array.elem_type);
        llvm_type             = LLVMArrayType(elem_type, (unsigned)type->array.size);
    }
    break;
    case TYPE_STR:
    {
        LLVMTypeRef field_types[2] = {
            LLVMPointerTypeInContext(ctx->context, 0),
            codegen_pointer_int_type(ctx),
        };
        llvm_type = LLVMStructTypeInContext(ctx->context, field_types, 2, false);
    }
    break;
    case TYPE_STRUCT:
    {
        // create opaque struct type first
        llvm_type = LLVMStructCreateNamed(ctx->context, type->name);

        // count fields
        int field_count = 0;
        for (Symbol *field = type->composite.fields; field; field = field->next)
        {
            field_count++;
        }

        // collect field types
        LLVMTypeRef *field_types = malloc(sizeof(LLVMTypeRef) * field_count);
        int          i           = 0;
        for (Symbol *field = type->composite.fields; field; field = field->next)
        {
            field_types[i++] = codegen_get_llvm_type(ctx, field->type);
        }

        LLVMStructSetBody(llvm_type, field_types, field_count, false);
        free(field_types);
    }
    break;
    case TYPE_UNION:
    {
        // unions are represented as a struct with a single field of the largest member type
        size_t      max_size       = 0;
        LLVMTypeRef largest_type   = NULL;
        bool        has_any_fields = false;

        for (Symbol *field = type->composite.fields; field; field = field->next)
        {
            has_any_fields = true;

            size_t field_size = type_sizeof(field->type);
            if (!largest_type || field_size > max_size)
            {
                LLVMTypeRef candidate = codegen_get_llvm_type(ctx, field->type);
                if (candidate)
                {
                    max_size     = field_size;
                    largest_type = candidate;
                }
            }
        }

        if (largest_type)
        {
            llvm_type = LLVMStructTypeInContext(ctx->context, &largest_type, 1, false);
        }
        else if (has_any_fields)
        {
            size_t      storage_size = max_size > 0 ? max_size : 1;
            LLVMTypeRef byte_array   = LLVMArrayType(LLVMInt8TypeInContext(ctx->context), (unsigned)storage_size);
            llvm_type                = LLVMStructTypeInContext(ctx->context, &byte_array, 1, false);
        }
        else
        {
            llvm_type = LLVMStructTypeInContext(ctx->context, NULL, 0, false);
        }
    }
    break;
    case TYPE_FUNCTION:
    {
        // functions are always represented as pointers in LLVM
        llvm_type = LLVMPointerTypeInContext(ctx->context, 0);
    }
    break;
    case TYPE_ALIAS:
        llvm_type = codegen_get_llvm_type(ctx, type->alias.target);
        break;
    case TYPE_ERROR:
        llvm_type = NULL;
        break;
    }

    // cache the result
    if (llvm_type)
    {
        if (ctx->type_cache.count >= ctx->type_cache.capacity)
        {
            ctx->type_cache.capacity   = ctx->type_cache.capacity ? ctx->type_cache.capacity * 2 : 16;
            ctx->type_cache.types      = realloc(ctx->type_cache.types, sizeof(Type *) * ctx->type_cache.capacity);
            ctx->type_cache.llvm_types = realloc(ctx->type_cache.llvm_types, sizeof(LLVMTypeRef) * ctx->type_cache.capacity);
        }

        ctx->type_cache.types[ctx->type_cache.count]      = type;
        ctx->type_cache.llvm_types[ctx->type_cache.count] = llvm_type;
        ctx->type_cache.count++;
    }

    return llvm_type;
}

LLVMValueRef codegen_get_symbol_value(CodegenContext *ctx, Symbol *symbol)
{
    for (int i = 0; i < ctx->symbol_map.count; i++)
    {
        if (ctx->symbol_map.symbols[i] == symbol)
        {
            return ctx->symbol_map.values[i];
        }
    }
    return NULL;
}

void codegen_set_symbol_value(CodegenContext *ctx, Symbol *symbol, LLVMValueRef value)
{
    // check if already exists
    for (int i = 0; i < ctx->symbol_map.count; i++)
    {
        if (ctx->symbol_map.symbols[i] == symbol)
        {
            ctx->symbol_map.values[i] = value;
            return;
        }
    }

    // add new mapping
    if (ctx->symbol_map.count >= ctx->symbol_map.capacity)
    {
        ctx->symbol_map.capacity = ctx->symbol_map.capacity ? ctx->symbol_map.capacity * 2 : 16;
        ctx->symbol_map.symbols  = realloc(ctx->symbol_map.symbols, sizeof(Symbol *) * ctx->symbol_map.capacity);
        ctx->symbol_map.values   = realloc(ctx->symbol_map.values, sizeof(LLVMValueRef) * ctx->symbol_map.capacity);
    }

    ctx->symbol_map.symbols[ctx->symbol_map.count] = symbol;
    ctx->symbol_map.values[ctx->symbol_map.count]  = value;
    ctx->symbol_map.count++;
}

LLVMValueRef codegen_stmt(CodegenContext *ctx, AstNode *stmt)
{
    if (!stmt)
        return NULL;

    codegen_set_debug_location(ctx, stmt);

    switch (stmt->kind)
    {
    case AST_STMT_VAR:
    case AST_STMT_VAL:
        return codegen_stmt_var(ctx, stmt);
    case AST_STMT_FUN:
        return codegen_stmt_fun(ctx, stmt);
    case AST_STMT_RET:
        return codegen_stmt_ret(ctx, stmt);
    case AST_COMPTIME:
        // compile-time directives do not emit runtime code
        return NULL;
    case AST_STMT_IF:
        return codegen_stmt_if(ctx, stmt);
    case AST_STMT_OR:
        return codegen_stmt_if(ctx, stmt); // or statements have same structure as if
    case AST_STMT_FOR:
        return codegen_stmt_for(ctx, stmt);
    case AST_STMT_BLOCK:
        return codegen_stmt_block(ctx, stmt);
    case AST_STMT_EXPR:
        return codegen_stmt_expr(ctx, stmt);
    case AST_STMT_BRK:
        if (ctx->break_block)
        {
            // execute deferred statements for all active blocks in reverse order
            // only down to the loop's scope
            for (int i = ctx->deferred_stack.count - 1; i >= ctx->break_depth; i--)
            {
                AstList *deferred = ctx->deferred_stack.blocks[i];
                if (deferred)
                {
                    for (int j = deferred->count - 1; j >= 0; j--)
                    {
                        codegen_stmt(ctx, deferred->items[j]);
                    }
                }
            }
            LLVMBuildBr(ctx->builder, ctx->break_block);
        }
        else
        {
            codegen_error(ctx, stmt, "break outside loop");
        }
        return NULL;
    case AST_STMT_CNT:
        if (ctx->continue_block)
        {
            // execute deferred statements for all active blocks in reverse order
            // only down to the loop's scope
            for (int i = ctx->deferred_stack.count - 1; i >= ctx->continue_depth; i--)
            {
                AstList *deferred = ctx->deferred_stack.blocks[i];
                if (deferred)
                {
                    for (int j = deferred->count - 1; j >= 0; j--)
                    {
                        codegen_stmt(ctx, deferred->items[j]);
                    }
                }
            }
            LLVMBuildBr(ctx->builder, ctx->continue_block);
        }
        else
        {
            codegen_error(ctx, stmt, "continue outside loop");
        }
        return NULL;
    case AST_STMT_EXT:
        return codegen_stmt_ext(ctx, stmt);
    case AST_STMT_DEF:
        // type definitions don't generate code
        return NULL;
    case AST_STMT_REC:
    case AST_STMT_UNI:
        // record/union definitions don't generate code directly
        return NULL;
    case AST_STMT_USE:
        return codegen_stmt_use(ctx, stmt);
    case AST_STMT_ASM:
        if (stmt->asm_stmt.code)
        {
            // default clobbers to be safe for syscalls and general side-effect asm
            const char *default_clobbers = "~{memory},~{dirflag},~{fpsr},~{flags}";
            const char *constr           = stmt->asm_stmt.constraints && stmt->asm_stmt.constraints[0] ? stmt->asm_stmt.constraints : default_clobbers;

            if (ctx->current_function)
            {
                // inline asm in function: side-effect call with no operands
                LLVMTypeRef  void_ty    = LLVMVoidTypeInContext(ctx->context);
                LLVMTypeRef  asm_fn_ty  = LLVMFunctionType(void_ty, NULL, 0, false);
                LLVMValueRef inline_asm = LLVMGetInlineAsm(asm_fn_ty, stmt->asm_stmt.code, strlen(stmt->asm_stmt.code), constr, strlen(constr), true, false, LLVMInlineAsmDialectATT, false);
                LLVMBuildCall2(ctx->builder, asm_fn_ty, inline_asm, NULL, 0, "");
            }
            else
            {
                size_t code_len = strlen(stmt->asm_stmt.code);
                if (code_len > 0)
                {
                    size_t existing_len          = ctx->module_inline_asm_len;
                    size_t need_leading_newline  = (existing_len > 0 && ctx->module_inline_asm[existing_len - 1] != '\n') ? 1 : 0;
                    size_t need_trailing_newline = (stmt->asm_stmt.code[code_len - 1] != '\n') ? 1 : 0;
                    size_t new_len               = existing_len + need_leading_newline + code_len + need_trailing_newline;

                    char *combined = malloc(new_len + 1);
                    if (!combined)
                    {
                        codegen_error(ctx, stmt, "failed to allocate module inline asm buffer");
                        return NULL;
                    }

                    size_t offset = 0;
                    if (existing_len > 0)
                    {
                        memcpy(combined, ctx->module_inline_asm, existing_len);
                        offset = existing_len;
                    }

                    if (need_leading_newline)
                    {
                        combined[offset++] = '\n';
                    }

                    memcpy(combined + offset, stmt->asm_stmt.code, code_len);
                    offset += code_len;

                    if (need_trailing_newline)
                    {
                        combined[offset++] = '\n';
                    }

                    combined[offset] = '\0';

                    free(ctx->module_inline_asm);
                    ctx->module_inline_asm     = combined;
                    ctx->module_inline_asm_len = offset;

                    LLVMSetModuleInlineAsm2(ctx->module, ctx->module_inline_asm, ctx->module_inline_asm_len);
                }
            }
        }
        return NULL;
    default:
        codegen_error(ctx, stmt, "unimplemented statement kind: %s", ast_node_kind_to_string(stmt->kind));
        return NULL;
    }
}

LLVMValueRef codegen_stmt_use(CodegenContext *ctx, AstNode *stmt)
{
    (void)ctx;
    (void)stmt;
    return NULL;
}

LLVMValueRef codegen_stmt_ext(CodegenContext *ctx, AstNode *stmt)
{
    // generate LLVM function declaration for external function
    Type *func_type = stmt->symbol ? stmt->symbol->type : NULL;
    if (!func_type || func_type->kind != TYPE_FUNCTION)
    {
        codegen_error(ctx, stmt, "external declaration must have function type");
        return NULL;
    }

    // get the target symbol name (may be different from function name)
    const char *symbol_name = stmt->ext_stmt.symbol ? stmt->ext_stmt.symbol : stmt->ext_stmt.name;

    // check if function already exists
    LLVMValueRef func = LLVMGetNamedFunction(ctx->module, symbol_name);
    if (func)
    {
        // already declared, just update symbol mapping
        codegen_set_symbol_value(ctx, stmt->symbol, func);
        return func;
    }

    // create LLVM function type
    LLVMTypeRef return_type = func_type->function.return_type ? codegen_get_llvm_type(ctx, func_type->function.return_type) : LLVMVoidTypeInContext(ctx->context);

    LLVMTypeRef *param_types       = NULL;
    bool         uses_mach_varargs = stmt->symbol && stmt->symbol->func.uses_mach_varargs;
    size_t       fixed_params      = func_type->function.param_count;
    size_t       llvm_param_count  = fixed_params + (uses_mach_varargs ? 2 : 0);
    if (llvm_param_count > 0)
    {
        param_types = malloc(sizeof(LLVMTypeRef) * llvm_param_count);
        for (size_t i = 0; i < fixed_params; i++)
        {
            param_types[i] = codegen_get_llvm_type(ctx, func_type->function.param_types[i]);
        }
        if (uses_mach_varargs)
        {
            LLVMTypeRef i64ty             = LLVMInt64TypeInContext(ctx->context);
            LLVMTypeRef i8_ptr            = LLVMPointerTypeInContext(ctx->context, 0);
            param_types[fixed_params]     = i64ty;
            param_types[fixed_params + 1] = LLVMPointerType(i8_ptr, 0);
        }
    }

    LLVMTypeRef llvm_func_type = LLVMFunctionType(return_type, param_types, llvm_param_count, uses_mach_varargs ? false : func_type->function.is_variadic);

    // create function declaration with target symbol name
    func = codegen_add_function(ctx, symbol_name, llvm_func_type);

    free(param_types);

    // add to symbol map
    codegen_set_symbol_value(ctx, stmt->symbol, func);

    return func;
}

LLVMValueRef codegen_stmt_block(CodegenContext *ctx, AstNode *stmt)
{
    LLVMValueRef last = NULL;

    LLVMMetadataRef prev_scope = NULL;
    bool            pushed     = codegen_debug_push_scope(ctx, stmt, &prev_scope);

    // push deferred statements list to stack
    if (ctx->deferred_stack.count >= ctx->deferred_stack.capacity)
    {
        ctx->deferred_stack.capacity = ctx->deferred_stack.capacity == 0 ? 8 : ctx->deferred_stack.capacity * 2;
        ctx->deferred_stack.blocks   = realloc(ctx->deferred_stack.blocks, ctx->deferred_stack.capacity * sizeof(AstList *));
    }
    ctx->deferred_stack.blocks[ctx->deferred_stack.count++] = stmt->block_stmt.deferred_stmts;

    if (stmt->block_stmt.stmts)
    {
        for (int i = 0; i < stmt->block_stmt.stmts->count; i++)
        {
            last = codegen_stmt(ctx, stmt->block_stmt.stmts->items[i]);
        }
    }

    // execute deferred statements in reverse order
    if (stmt->block_stmt.deferred_stmts && !LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder)))
    {
        for (int i = stmt->block_stmt.deferred_stmts->count - 1; i >= 0; i--)
        {
            codegen_stmt(ctx, stmt->block_stmt.deferred_stmts->items[i]);
        }
    }

    // pop deferred statements list
    ctx->deferred_stack.count--;

    codegen_debug_pop_scope(ctx, prev_scope, pushed);
    return last;
}

LLVMValueRef codegen_stmt_var(CodegenContext *ctx, AstNode *stmt)
{
    if (!stmt->symbol || !stmt->type)
    {
        codegen_error(ctx, stmt, "variable declaration missing symbol or type info");
        return NULL;
    }

    LLVMTypeRef llvm_type = codegen_get_llvm_type(ctx, stmt->type);
    if (!llvm_type)
    {
        codegen_error(ctx, stmt, "failed to get llvm type for variable");
        return NULL;
    }

    if (!ctx->current_function)
    {
        // top-level: create a true LLVM global
        const char  *gname  = stmt->symbol && stmt->symbol->var.mangled_name && stmt->symbol->var.mangled_name[0] ? stmt->symbol->var.mangled_name : stmt->var_stmt.name;
        LLVMValueRef global = LLVMGetNamedGlobal(ctx->module, gname);
        if (!global)
        {
            global = LLVMAddGlobal(ctx->module, llvm_type, gname);
        }
        if (stmt->var_stmt.is_val)
        {
            LLVMSetGlobalConstant(global, true);
        }
        else
        {
            LLVMSetGlobalConstant(global, false);
        }
        // initializer
        if (stmt->var_stmt.init)
        {
            // handle constants specially without emitting instructions
            AstNode     *init       = stmt->var_stmt.init;
            LLVMValueRef const_init = NULL;
            if (init->kind == AST_EXPR_LIT)
            {
                switch (init->lit_expr.kind)
                {
                case TOKEN_LIT_INT:
                {
                    const_init = LLVMConstInt(llvm_type, init->lit_expr.int_val, type_is_signed(stmt->type));
                    break;
                }
                case TOKEN_LIT_FLOAT:
                {
                    const_init = LLVMConstReal(llvm_type, init->lit_expr.float_val);
                    break;
                }
                case TOKEN_LIT_CHAR:
                {
                    const_init = LLVMConstInt(llvm_type, init->lit_expr.char_val, false);
                    break;
                }
                case TOKEN_LIT_STRING:
                {
                    Type *resolved = type_resolve_alias(stmt->type);
                    if (resolved && resolved->kind == TYPE_STR)
                    {
                        size_t      len         = strlen(init->lit_expr.string_val);
                        size_t      storage_len = len > 0 ? len : 1;
                        LLVMTypeRef char_ty     = LLVMInt8TypeInContext(ctx->context);
                        LLVMTypeRef arr_ty      = LLVMArrayType(char_ty, (unsigned)storage_len);
                        char        gbuf[128];
                        snprintf(gbuf, sizeof(gbuf), "%s.str", gname);
                        LLVMValueRef data_g = LLVMGetNamedGlobal(ctx->module, gbuf);
                        if (!data_g)
                        {
                            data_g = LLVMAddGlobal(ctx->module, arr_ty, gbuf);
                            LLVMSetGlobalConstant(data_g, true);
                            LLVMSetLinkage(data_g, LLVMPrivateLinkage);
                            LLVMValueRef str_c;
                            if (len > 0)
                            {
                                str_c = LLVMConstStringInContext(ctx->context, init->lit_expr.string_val, (unsigned)len, true);
                            }
                            else
                            {
                                LLVMValueRef zero_elem = LLVMConstInt(char_ty, 0, false);
                                str_c                  = LLVMConstArray(char_ty, &zero_elem, 1);
                            }
                            LLVMSetInitializer(data_g, str_c);
                        }
                        LLVMValueRef idxs[2]   = {LLVMConstInt(LLVMInt32TypeInContext(ctx->context), 0, false), LLVMConstInt(LLVMInt32TypeInContext(ctx->context), 0, false)};
                        LLVMValueRef data_ptr  = LLVMConstGEP2(arr_ty, data_g, idxs, 2);
                        data_ptr               = LLVMConstPointerCast(data_ptr, LLVMPointerTypeInContext(ctx->context, 0));
                        LLVMTypeRef  len_ty    = codegen_pointer_int_type(ctx);
                        LLVMValueRef len_val   = LLVMConstInt(len_ty ? len_ty : LLVMInt64TypeInContext(ctx->context), len, false);
                        LLVMValueRef fields[2] = {data_ptr, len_val};
                        LLVMTypeRef  str_ty    = codegen_get_llvm_type(ctx, stmt->type);
                        const_init             = str_ty ? LLVMConstNamedStruct(str_ty, fields, 2) : LLVMConstStructInContext(ctx->context, fields, 2, false);
                    }
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                // try simple integer/boolean constant folding
                int64_t folded = 0;
                if (codegen_eval_const_i64(ctx, init, &folded))
                {
                    const_init = LLVMConstInt(llvm_type, (unsigned long long)folded, type_is_signed(stmt->type));
                }
            }
            if (!const_init)
            {
                codegen_error(ctx, stmt, "global variable initializer must be constant");
                return NULL;
            }
            LLVMSetInitializer(global, const_init);
        }
        codegen_set_symbol_value(ctx, stmt->symbol, global);
        return global;
    }
    else
    {
        // function-local: allocate and store
        LLVMValueRef alloca = codegen_create_alloca(ctx, llvm_type, stmt->var_stmt.name);
        codegen_set_symbol_value(ctx, stmt->symbol, alloca);

        // always initialize to zero first to avoid undef in SROA'd variables
        LLVMValueRef zero_val = LLVMConstNull(llvm_type);
        LLVMBuildStore(ctx->builder, zero_val, alloca);

        if (stmt->var_stmt.init)
        {
            bool old_mutable_init        = ctx->generating_mutable_init;
            ctx->generating_mutable_init = !stmt->var_stmt.is_val;
            LLVMValueRef init_value      = codegen_expr(ctx, stmt->var_stmt.init);
            ctx->generating_mutable_init = old_mutable_init;
            if (!init_value)
            {
                codegen_error(ctx, stmt, "failed to generate initializer");
                return NULL;
            }
            if (stmt->var_stmt.init->kind == AST_EXPR_STRUCT)
            {
                LLVMValueRef loaded_struct = LLVMBuildLoad2(ctx->builder, llvm_type, init_value, "");
                LLVMBuildStore(ctx->builder, loaded_struct, alloca);
            }
            else
            {
                init_value = codegen_load_rvalue(ctx, init_value, stmt->var_stmt.init->type, stmt->var_stmt.init);
                LLVMBuildStore(ctx->builder, init_value, alloca);
            }
        }

        codegen_debug_declare_symbol(ctx, stmt->symbol, stmt->type, alloca, stmt, false, 0);
        return alloca;
    }
}

LLVMValueRef codegen_stmt_fun(CodegenContext *ctx, AstNode *stmt)
{
    // skip generic function templates (they're instantiated on demand)
    // specialized instances have their own cloned ASTs and should be generated
    if (stmt->symbol)
    {
        if (stmt->symbol->func.is_generic && !stmt->symbol->func.is_specialized_instance)
        {
            // this is a generic template - skip it
            return NULL;
        }
    }
    else if (stmt->fun_stmt.generics && stmt->fun_stmt.generics->count > 0)
    {
        // AST has generics but no symbol - skip it
        return NULL;
    }

    if (!stmt->symbol || !stmt->type)
    {
        codegen_error(ctx, stmt, "function declaration missing symbol or type info");
        return NULL;
    }

    const char *func_name = NULL;
    if (stmt->symbol && stmt->symbol->func.mangled_name && stmt->symbol->func.mangled_name[0])
        func_name = stmt->symbol->func.mangled_name;
    else
        func_name = stmt->fun_stmt.name;

    Type       *func_type         = stmt->type;
    bool        uses_mach_varargs = stmt->symbol && stmt->symbol->func.uses_mach_varargs;
    size_t      fixed_param_count = func_type->function.param_count;
    size_t      llvm_param_count  = fixed_param_count + (uses_mach_varargs ? 2 : 0);
    LLVMTypeRef return_type       = func_type->function.return_type ? codegen_get_llvm_type(ctx, func_type->function.return_type) : LLVMVoidTypeInContext(ctx->context);

    LLVMTypeRef *param_types = NULL;
    if (llvm_param_count > 0)
    {
        param_types = malloc(sizeof(LLVMTypeRef) * llvm_param_count);
        for (size_t i = 0; i < fixed_param_count; i++)
        {
            param_types[i] = codegen_get_llvm_type(ctx, func_type->function.param_types[i]);
        }
        if (uses_mach_varargs)
        {
            LLVMTypeRef i64ty                  = LLVMInt64TypeInContext(ctx->context);
            LLVMTypeRef i8_ptr                 = LLVMPointerTypeInContext(ctx->context, 0);
            param_types[fixed_param_count]     = i64ty;                      // hidden count parameter
            param_types[fixed_param_count + 1] = LLVMPointerType(i8_ptr, 0); // hidden i8** array pointer
        }
    }

    if (!func_name || !func_name[0])
    {
        free(param_types);
        codegen_error(ctx, stmt, "function has no resolvable name");
        return NULL;
    }

    LLVMTypeRef  llvm_func_type = LLVMFunctionType(return_type, param_types, llvm_param_count, uses_mach_varargs ? false : func_type->function.is_variadic);
    LLVMValueRef func           = LLVMGetNamedFunction(ctx->module, func_name);
    if (!func)
    {
        func = codegen_add_function(ctx, func_name, llvm_func_type);
    }
    free(param_types);

    // set linkage for specialized generic instances
    // linkonce_odr = keep one definition, discard duplicates (like C++ templates)
    if (stmt->symbol && stmt->symbol->func.is_specialized_instance)
    {
        LLVMSetLinkage(func, LLVMLinkOnceODRLinkage);
    }

    // clean up mangled name
    codegen_set_symbol_value(ctx, stmt->symbol, func);

    if (stmt->symbol && !stmt->symbol->func.mangled_name && func_name)
        stmt->symbol->func.mangled_name = strdup(func_name);

    // generate body if present
    if (stmt->fun_stmt.body)
    {
        LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx->context, func, "entry");
        LLVMPositionBuilderAtEnd(ctx->builder, entry);

        LLVMMetadataRef prev_scope      = ctx->current_di_scope;
        LLVMMetadataRef prev_subprogram = ctx->current_di_subprogram;
        LLVMMetadataRef subprogram      = NULL;
        if (ctx->debug_info && ctx->di_builder)
        {
            const char *disp = stmt->fun_stmt.name ? stmt->fun_stmt.name : func_name;
            const char *link = func_name ? func_name : disp;
            subprogram       = codegen_debug_create_subprogram(ctx, stmt, func, disp, link, fixed_param_count);
            if (subprogram)
            {
                ctx->current_di_scope      = subprogram;
                ctx->current_di_subprogram = subprogram;
                codegen_set_debug_location(ctx, stmt);
            }
        }

        LLVMValueRef prev_function           = ctx->current_function;
        Type        *prev_ftype              = ctx->current_function_type;
        LLVMValueRef prev_vararg_count_value = ctx->current_vararg_count_value;
        LLVMValueRef prev_vararg_array       = ctx->current_vararg_array;
        size_t       prev_fixed_param_count  = ctx->current_fixed_param_count;
        ctx->current_function                = func;
        ctx->current_function_type           = stmt->type;

        size_t fixed_params            = fixed_param_count;
        ctx->current_fixed_param_count = fixed_params;
        if (stmt->fun_stmt.params)
        {
            size_t fixed_index = 0;
            for (int i = 0; i < stmt->fun_stmt.params->count; i++)
            {
                AstNode *param = stmt->fun_stmt.params->items[i];
                if (param->param_stmt.is_variadic)
                    continue;
                LLVMValueRef param_value = LLVMGetParam(func, (unsigned)fixed_index);
                if (!param_value)
                {
                    codegen_error(ctx, stmt, "failed to access parameter '%s'", param->param_stmt.name ? param->param_stmt.name : "<anon>");
                    fixed_index++;
                    continue;
                }

                if (!param->type)
                {
                    codegen_error(ctx, param, "parameter '%s' missing resolved type", param->param_stmt.name ? param->param_stmt.name : "<anon>");
                    fixed_index++;
                    continue;
                }

                LLVMTypeRef  param_type   = codegen_get_llvm_type(ctx, param->type);
                LLVMValueRef param_alloca = codegen_create_alloca(ctx, param_type, param->param_stmt.name);
                LLVMBuildStore(ctx->builder, param_value, param_alloca);
                if (param->symbol)
                    codegen_set_symbol_value(ctx, param->symbol, param_alloca);
                codegen_debug_declare_symbol(ctx, param->symbol, param->type, param_alloca, param, true, (unsigned)(fixed_index + 1));
                fixed_index++;
            }
        }

        if (uses_mach_varargs)
        {
            unsigned count_index            = (unsigned)fixed_params;
            ctx->current_vararg_count_value = LLVMGetParam(func, count_index);
            ctx->current_vararg_array       = LLVMGetParam(func, count_index + 1);
            LLVMSetValueName2(ctx->current_vararg_count_value, "__mach_vararg_count", strlen("__mach_vararg_count"));
            LLVMSetValueName2(ctx->current_vararg_array, "__mach_vararg_data", strlen("__mach_vararg_data"));
        }
        else
        {
            ctx->current_vararg_count_value = NULL;
            ctx->current_vararg_array       = NULL;
        }

        codegen_stmt(ctx, stmt->fun_stmt.body);

        if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder)))
        {
            if (stmt->type->function.return_type == NULL)
            {
                LLVMBuildRetVoid(ctx->builder);
            }
            else
            {
                LLVMTypeRef  ret_ty = codegen_get_llvm_type(ctx, stmt->type->function.return_type);
                LLVMValueRef zero   = LLVMConstNull(ret_ty);
                LLVMBuildRet(ctx->builder, zero);
            }
        }

        ctx->current_vararg_count_value = prev_vararg_count_value;
        ctx->current_vararg_array       = prev_vararg_array;
        ctx->current_fixed_param_count  = prev_fixed_param_count;
        ctx->current_function           = prev_function;
        ctx->current_function_type      = prev_ftype;
        if (subprogram)
        {
            ctx->current_di_scope      = prev_scope;
            ctx->current_di_subprogram = prev_subprogram;
            LLVMSetCurrentDebugLocation2(ctx->builder, NULL);
        }
    }

    return func;
}

LLVMValueRef codegen_stmt_ret(CodegenContext *ctx, AstNode *stmt)
{
    codegen_set_debug_location(ctx, stmt);

    LLVMValueRef value = NULL;
    if (stmt->ret_stmt.expr)
    {
        value = codegen_expr(ctx, stmt->ret_stmt.expr);
        if (!value)
        {
            codegen_error(ctx, stmt, "failed to generate return value");
            return NULL;
        }
        value = codegen_load_rvalue(ctx, value, stmt->ret_stmt.expr->type, stmt->ret_stmt.expr);
        // cast to declared return type if available (prefer Mach function type)
        LLVMTypeRef rty = NULL;
        if (ctx->current_function_type && ctx->current_function_type->kind == TYPE_FUNCTION && ctx->current_function_type->function.return_type)
        {
            rty = codegen_get_llvm_type(ctx, ctx->current_function_type->function.return_type);
        }
        // safe fallback: derive from current llvm function
        if (!rty && ctx->current_function)
        {
            LLVMTypeRef fty = LLVMTypeOf(ctx->current_function);
            if (fty && LLVMGetTypeKind(fty) == LLVMFunctionTypeKind)
                rty = LLVMGetReturnType(fty);
            else if (fty && LLVMGetTypeKind(fty) == LLVMPointerTypeKind)
            {
                LLVMTypeRef el = LLVMGetElementType(fty);
                if (el && LLVMGetTypeKind(el) == LLVMFunctionTypeKind)
                    rty = LLVMGetReturnType(el);
            }
        }
        LLVMTypeRef vty = LLVMTypeOf(value);
        if (rty && vty)
        {
            LLVMTypeKind rk = LLVMGetTypeKind(rty);
            LLVMTypeKind vk = LLVMGetTypeKind(vty);
            if (vk == LLVMIntegerTypeKind && rk == LLVMIntegerTypeKind)
            {
                unsigned vw = LLVMGetIntTypeWidth(vty);
                unsigned rw = LLVMGetIntTypeWidth(rty);
                if (vw < rw)
                    value = LLVMBuildZExt(ctx->builder, value, rty, "zext_ret");
                else if (vw > rw)
                    value = LLVMBuildTrunc(ctx->builder, value, rty, "trunc_ret");
            }
            else if (vk == LLVMPointerTypeKind && rk == LLVMIntegerTypeKind)
            {
                value = LLVMBuildPtrToInt(ctx->builder, value, rty, "ptrtoint_ret");
            }
            else if (vk == LLVMIntegerTypeKind && rk == LLVMPointerTypeKind)
            {
                value = LLVMBuildIntToPtr(ctx->builder, value, rty, "inttoptr_ret");
            }
        }
    }

    // execute deferred statements for all active blocks in reverse order
    for (int i = ctx->deferred_stack.count - 1; i >= 0; i--)
    {
        AstList *deferred = ctx->deferred_stack.blocks[i];
        if (deferred)
        {
            for (int j = deferred->count - 1; j >= 0; j--)
            {
                codegen_stmt(ctx, deferred->items[j]);
            }
        }
    }

    if (value)
    {
        return LLVMBuildRet(ctx->builder, value);
    }
    else
    {
        return LLVMBuildRetVoid(ctx->builder);
    }
}

LLVMValueRef codegen_stmt_if(CodegenContext *ctx, AstNode *stmt)
{
    LLVMValueRef cond_value = NULL;
    if (stmt->cond_stmt.cond)
    {
        cond_value = codegen_expr(ctx, stmt->cond_stmt.cond);
        if (!cond_value)
        {
            codegen_error(ctx, stmt, "failed to generate if condition");
            return NULL;
        }
        cond_value = codegen_load_if_needed(ctx, cond_value, stmt->cond_stmt.cond->type, stmt->cond_stmt.cond);
        Type *ct   = type_resolve_alias(stmt->cond_stmt.cond->type);
        if (type_is_integer(ct) || type_is_pointer_like(ct))
        {
            LLVMValueRef zero = LLVMConstNull(LLVMTypeOf(cond_value));
            cond_value        = LLVMBuildICmp(ctx->builder, LLVMIntNE, cond_value, zero, "ifcond");
        }
        else if (type_is_float(ct))
        {
            LLVMValueRef zero = LLVMConstReal(LLVMTypeOf(cond_value), 0.0);
            cond_value        = LLVMBuildFCmp(ctx->builder, LLVMRealONE, cond_value, zero, "ifcond");
        }
        else
        {
            codegen_error(ctx, stmt, "invalid truthiness for condition (record/union not allowed)");
            return NULL;
        }
    }
    else
    {
        // else-only branch should execute unconditionally when reached
        // create a constant true condition
        cond_value = LLVMConstInt(LLVMInt1TypeInContext(ctx->context), 1, false);
    }

    LLVMBasicBlockRef then_block  = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "then");
    LLVMBasicBlockRef else_block  = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "else");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "ifcont");

    LLVMBuildCondBr(ctx->builder, cond_value, then_block, else_block);

    // generate then block
    LLVMPositionBuilderAtEnd(ctx->builder, then_block);
    LLVMMetadataRef then_prev_scope = NULL;
    bool            then_pushed     = codegen_debug_push_scope(ctx, stmt->cond_stmt.body, &then_prev_scope);
    codegen_set_debug_location(ctx, stmt->cond_stmt.body);
    codegen_stmt(ctx, stmt->cond_stmt.body);
    codegen_debug_pop_scope(ctx, then_prev_scope, then_pushed);
    bool then_falls_through = false;
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder)))
    {
        LLVMBuildBr(ctx->builder, merge_block);
        then_falls_through = true;
    }

    // generate else block (or chain)
    LLVMPositionBuilderAtEnd(ctx->builder, else_block);
    if (stmt->cond_stmt.stmt_or)
    {
        LLVMMetadataRef else_prev_scope = NULL;
        bool            else_pushed     = codegen_debug_push_scope(ctx, stmt->cond_stmt.stmt_or, &else_prev_scope);
        codegen_set_debug_location(ctx, stmt->cond_stmt.stmt_or);
        codegen_stmt(ctx, stmt->cond_stmt.stmt_or);
        codegen_debug_pop_scope(ctx, else_prev_scope, else_pushed);
    }
    bool else_falls_through = false;
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder)))
    {
        LLVMBuildBr(ctx->builder, merge_block);
        else_falls_through = true;
    }

    // continue at merge block
    LLVMPositionBuilderAtEnd(ctx->builder, merge_block);

    // if both branches terminated (no fallthrough), mark merge as unreachable to avoid false missing-return
    if (!then_falls_through && !else_falls_through)
    {
        LLVMBuildUnreachable(ctx->builder);
    }

    return NULL;
}

LLVMValueRef codegen_stmt_for(CodegenContext *ctx, AstNode *stmt)
{
    LLVMBasicBlockRef loop_block = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "loop");
    LLVMBasicBlockRef body_block = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "loop.body");
    LLVMBasicBlockRef exit_block = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "loop.exit");

    // save loop context
    LLVMBasicBlockRef prev_break    = ctx->break_block;
    LLVMBasicBlockRef prev_continue = ctx->continue_block;
    int               prev_break_depth = ctx->break_depth;
    int               prev_continue_depth = ctx->continue_depth;

    ctx->break_block    = exit_block;
    ctx->continue_block = loop_block;
    ctx->break_depth    = ctx->deferred_stack.count;
    ctx->continue_depth = ctx->deferred_stack.count;

    // jump to loop header
    LLVMBuildBr(ctx->builder, loop_block);

    // generate loop header
    LLVMPositionBuilderAtEnd(ctx->builder, loop_block);
    codegen_set_debug_location(ctx, stmt);

    if (stmt->for_stmt.cond)
    {
        LLVMValueRef cond_value = codegen_expr(ctx, stmt->for_stmt.cond);
        if (!cond_value)
        {
            codegen_error(ctx, stmt, "failed to generate loop condition");
            return NULL;
        }
        Type *ct = type_resolve_alias(stmt->for_stmt.cond->type);
        if (type_is_integer(ct) || type_is_pointer_like(ct))
        {
            cond_value = LLVMBuildICmp(ctx->builder, LLVMIntNE, cond_value, LLVMConstNull(LLVMTypeOf(cond_value)), "loopcond");
        }
        else if (type_is_float(ct))
        {
            cond_value = LLVMBuildFCmp(ctx->builder, LLVMRealONE, cond_value, LLVMConstReal(LLVMTypeOf(cond_value), 0.0), "loopcond");
        }
        else
        {
            codegen_error(ctx, stmt, "invalid truthiness for loop condition (record/union not allowed)");
            return NULL;
        }

        LLVMBuildCondBr(ctx->builder, cond_value, body_block, exit_block);
    }
    else
    {
        // infinite loop
        LLVMBuildBr(ctx->builder, body_block);
    }

    // generate body
    LLVMPositionBuilderAtEnd(ctx->builder, body_block);
    LLVMMetadataRef body_prev_scope = NULL;
    bool            body_pushed     = codegen_debug_push_scope(ctx, stmt->for_stmt.body, &body_prev_scope);
    codegen_set_debug_location(ctx, stmt->for_stmt.body);
    codegen_stmt(ctx, stmt->for_stmt.body);
    codegen_debug_pop_scope(ctx, body_prev_scope, body_pushed);
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder)))
    {
        LLVMBuildBr(ctx->builder, loop_block);
    }

    // restore loop context
    ctx->break_block    = prev_break;
    ctx->continue_block = prev_continue;
    ctx->break_depth    = prev_break_depth;
    ctx->continue_depth = prev_continue_depth;

    // continue at exit block
    LLVMPositionBuilderAtEnd(ctx->builder, exit_block);

    return NULL;
}

LLVMValueRef codegen_stmt_expr(CodegenContext *ctx, AstNode *stmt)
{
    // evaluate the expression but discard the result in statement context
    codegen_expr(ctx, stmt->expr_stmt.expr);
    return NULL;
}

LLVMValueRef codegen_expr(CodegenContext *ctx, AstNode *expr)
{
    if (!expr)
        return NULL;

    codegen_set_debug_location(ctx, expr);

    switch (expr->kind)
    {
    case AST_EXPR_LIT:
        return codegen_expr_lit(ctx, expr);
    case AST_EXPR_NULL:
        return codegen_expr_null(ctx, expr);
    case AST_EXPR_IDENT:
        return codegen_expr_ident(ctx, expr);
    case AST_EXPR_BINARY:
        return codegen_expr_binary(ctx, expr);
    case AST_EXPR_UNARY:
        return codegen_expr_unary(ctx, expr);
    case AST_EXPR_CALL:
        return codegen_expr_call(ctx, expr);
    case AST_EXPR_CAST:
        return codegen_expr_cast(ctx, expr);
    case AST_EXPR_FIELD:
        return codegen_expr_field(ctx, expr);
    case AST_EXPR_INDEX:
        return codegen_expr_index(ctx, expr);
    case AST_EXPR_ARRAY:
        return codegen_expr_array(ctx, expr);
    case AST_EXPR_STRUCT:
        return codegen_expr_struct(ctx, expr);
    default:
        codegen_error(ctx, expr, "unimplemented expression kind");
        return NULL;
    }
}

LLVMValueRef codegen_expr_lit(CodegenContext *ctx, AstNode *expr)
{
    switch (expr->lit_expr.kind)
    {
    case TOKEN_LIT_INT:
    {
        Type *t = type_resolve_alias(expr->type);
        // pointer-context: handle null and intptr -> ptr
        if (t && (t->kind == TYPE_PTR || t->kind == TYPE_POINTER))
        {
            if (expr->lit_expr.int_val == 0)
            {
                LLVMTypeRef pty = codegen_get_llvm_type(ctx, expr->type);
                if (!pty)
                    pty = LLVMPointerTypeInContext(ctx->context, 0);
                return LLVMConstNull(pty);
            }
            LLVMValueRef as_int = LLVMConstInt(LLVMInt64TypeInContext(ctx->context), expr->lit_expr.int_val, false);
            LLVMTypeRef  pty    = codegen_get_llvm_type(ctx, expr->type);
            if (!pty)
                pty = LLVMPointerTypeInContext(ctx->context, 0);
            return LLVMConstIntToPtr(as_int, pty);
        }
        // integer literal: use the semantic integer type if available, else i64 fallback
        LLVMTypeRef ity = codegen_get_llvm_type(ctx, expr->type);
        if (!ity || LLVMGetTypeKind(ity) != LLVMIntegerTypeKind)
            ity = LLVMInt64TypeInContext(ctx->context);
        bool is_signed = type_is_signed(expr->type);
        return LLVMConstInt(ity, expr->lit_expr.int_val, is_signed);
    }
    case TOKEN_LIT_FLOAT:
    {
        LLVMTypeRef type = codegen_get_llvm_type(ctx, expr->type);
        if (!type)
            type = LLVMDoubleTypeInContext(ctx->context);
        else
        {
            LLVMTypeKind k = LLVMGetTypeKind(type);
            if (!(k == LLVMHalfTypeKind || k == LLVMFloatTypeKind || k == LLVMDoubleTypeKind))
                type = LLVMDoubleTypeInContext(ctx->context);
        }
        return LLVMConstReal(type, expr->lit_expr.float_val);
    }
    case TOKEN_LIT_CHAR:
    {
        LLVMTypeRef type = codegen_get_llvm_type(ctx, expr->type);
        if (!type || LLVMGetTypeKind(type) != LLVMIntegerTypeKind)
            type = LLVMInt8TypeInContext(ctx->context);
        return LLVMConstInt(type, expr->lit_expr.char_val, false);
    }
    case TOKEN_LIT_STRING:
    {
        LLVMValueRef str_ptr = LLVMBuildGlobalStringPtr(ctx->builder, expr->lit_expr.string_val, "str");
        size_t       str_len = strlen(expr->lit_expr.string_val);
        LLVMTypeRef  len_ty  = codegen_pointer_int_type(ctx);
        LLVMValueRef len_val = LLVMConstInt(len_ty ? len_ty : LLVMInt64TypeInContext(ctx->context), str_len, false);

        LLVMTypeRef str_type = codegen_get_llvm_type(ctx, expr->type);
        if (!str_type)
        {
            LLVMTypeRef field_types[2] = {
                LLVMPointerTypeInContext(ctx->context, 0),
                len_ty ? len_ty : LLVMInt64TypeInContext(ctx->context),
            };
            str_type = LLVMStructTypeInContext(ctx->context, field_types, 2, false);
        }

        LLVMValueRef str_value = LLVMGetUndef(str_type);
        str_value              = LLVMBuildInsertValue(ctx->builder, str_value, str_ptr, 0, "str_data");
        str_value              = LLVMBuildInsertValue(ctx->builder, str_value, len_val, 1, "str_len");
        return str_value;
    }
    default:
        codegen_error(ctx, expr, "unknown literal kind");
        return NULL;
    }
}

LLVMValueRef codegen_expr_null(CodegenContext *ctx, AstNode *expr)
{
    LLVMTypeRef ptr_ty = codegen_get_llvm_type(ctx, expr->type);
    if (!ptr_ty)
        ptr_ty = LLVMPointerTypeInContext(ctx->context, 0);
    return LLVMConstNull(ptr_ty);
}

LLVMValueRef codegen_expr_ident(CodegenContext *ctx, AstNode *expr)
{
    if (!expr->symbol)
    {
        codegen_error(ctx, expr, "identifier has no symbol");
        return NULL;
    }

    LLVMValueRef value = codegen_get_symbol_value(ctx, expr->symbol);
    if (!value && expr->symbol->kind == SYMBOL_FUNC)
    {
        codegen_declare_function_symbol(ctx, expr->symbol);
        value = codegen_get_symbol_value(ctx, expr->symbol);
    }
    if (!value)
    {
        // declare external global for module-level val/var
        if (expr->symbol->kind == SYMBOL_VAR || expr->symbol->kind == SYMBOL_VAL)
        {
            LLVMTypeRef ty    = codegen_get_llvm_type(ctx, expr->symbol->type);
            const char *gname = expr->symbol->var.mangled_name && expr->symbol->var.mangled_name[0] ? expr->symbol->var.mangled_name : expr->ident_expr.name;
            value             = LLVMGetNamedGlobal(ctx->module, gname);
            if (!value)
            {
                value = LLVMAddGlobal(ctx->module, ty, gname);
            }
            codegen_set_symbol_value(ctx, expr->symbol, value);
        }
        else
        {
            codegen_error(ctx, expr, "undefined symbol '%s'", expr->ident_expr.name);
            return NULL;
        }
    }

    // load value if it's a variable
    if (expr->symbol->kind == SYMBOL_VAR || expr->symbol->kind == SYMBOL_VAL || expr->symbol->kind == SYMBOL_PARAM)
    {
        if (codegen_is_lvalue(expr))
        {
            return value; // return address for lvalues
        }
        else
        {
            return LLVMBuildLoad2(ctx->builder, codegen_get_llvm_type(ctx, expr->type), value, "");
        }
    }

    return value;
}

LLVMValueRef codegen_expr_binary(CodegenContext *ctx, AstNode *expr)
{
    // handle assignment specially
    if (expr->binary_expr.op == TOKEN_EQUAL)
    {
        // special-case deref on LHS: @ptr = value
        if (expr->binary_expr.left->kind == AST_EXPR_UNARY && expr->binary_expr.left->unary_expr.op == TOKEN_AT)
        {
            AstNode     *ptr_node = expr->binary_expr.left->unary_expr.expr;
            LLVMValueRef dest     = codegen_expr(ctx, ptr_node);
            if (!dest)
            {
                codegen_error(ctx, expr, "failed to generate destination pointer for store");
                return NULL;
            }
            // ensure dest is the pointer value (load from alloca/global/gep if needed)
            if (LLVMIsAAllocaInst(dest) || LLVMIsAGlobalVariable(dest))
            {
                LLVMTypeRef ptr_ty = codegen_get_llvm_type(ctx, ptr_node->type);
                dest               = LLVMBuildLoad2(ctx->builder, ptr_ty, dest, "");
            }
            LLVMValueRef rhs = codegen_expr(ctx, expr->binary_expr.right);
            if (!rhs)
            {
                codegen_error(ctx, expr, "failed to generate rhs for store");
                return NULL;
            }
            rhs = codegen_load_if_needed(ctx, rhs, expr->binary_expr.right->type, expr->binary_expr.right);
            LLVMBuildStore(ctx->builder, rhs, dest);
            return rhs;
        }

        LLVMValueRef lhs = codegen_expr(ctx, expr->binary_expr.left);
        LLVMValueRef rhs = codegen_expr(ctx, expr->binary_expr.right);

        if (!lhs || !rhs)
        {
            codegen_error(ctx, expr, "failed to generate assignment operands");
            return NULL;
        }

        // load rhs value if needed
        rhs = codegen_load_if_needed(ctx, rhs, expr->binary_expr.right->type, expr->binary_expr.right);

        LLVMBuildStore(ctx->builder, rhs, lhs);
        return rhs;
    }

    // generate operands
    LLVMValueRef lhs = codegen_expr(ctx, expr->binary_expr.left);
    LLVMValueRef rhs = codegen_expr(ctx, expr->binary_expr.right);

    if (!lhs || !rhs)
    {
        codegen_error(ctx, expr, "failed to generate binary operands");
        return NULL;
    }

    // load values if needed
    lhs = codegen_load_if_needed(ctx, lhs, expr->binary_expr.left->type, expr->binary_expr.left);
    rhs = codegen_load_if_needed(ctx, rhs, expr->binary_expr.right->type, expr->binary_expr.right);

    // handle type conversions for comparison operations
    Type *lhs_type = type_resolve_alias(expr->binary_expr.left->type);
    Type *rhs_type = type_resolve_alias(expr->binary_expr.right->type);

    // extend i1 comparison results to match declared type (u8)
    if (lhs && lhs_type && type_is_integer(lhs_type))
    {
        LLVMTypeRef lhs_llvm_ty = LLVMTypeOf(lhs);
        LLVMTypeRef expected_ty = codegen_get_llvm_type(ctx, lhs_type);
        if (LLVMGetTypeKind(lhs_llvm_ty) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(lhs_llvm_ty) == 1)
        {
            lhs = LLVMBuildZExt(ctx->builder, lhs, expected_ty, "boolext");
        }
    }
    if (rhs && rhs_type && type_is_integer(rhs_type))
    {
        LLVMTypeRef rhs_llvm_ty = LLVMTypeOf(rhs);
        LLVMTypeRef expected_ty = codegen_get_llvm_type(ctx, rhs_type);
        if (LLVMGetTypeKind(rhs_llvm_ty) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(rhs_llvm_ty) == 1)
        {
            rhs = LLVMBuildZExt(ctx->builder, rhs, expected_ty, "boolext");
        }
    }

    // handle integer type mismatches by extending smaller type
    if (type_is_integer(lhs_type) && type_is_integer(rhs_type) && lhs_type->kind != rhs_type->kind)
    {
        size_t lhs_size = type_sizeof(lhs_type);
        size_t rhs_size = type_sizeof(rhs_type);

        if (lhs_size < rhs_size)
        {
            bool        is_signed   = type_is_signed(lhs_type);
            LLVMTypeRef target_type = codegen_get_llvm_type(ctx, rhs_type);
            lhs                     = is_signed ? LLVMBuildSExt(ctx->builder, lhs, target_type, "sext") : LLVMBuildZExt(ctx->builder, lhs, target_type, "zext");
        }
        else if (rhs_size < lhs_size)
        {
            bool        is_signed   = type_is_signed(rhs_type);
            LLVMTypeRef target_type = codegen_get_llvm_type(ctx, lhs_type);
            rhs                     = is_signed ? LLVMBuildSExt(ctx->builder, rhs, target_type, "sext") : LLVMBuildZExt(ctx->builder, rhs, target_type, "zext");
        }
    }

    // generate operation based on operator and types
    bool lhs_is_float       = type_is_float(lhs_type);
    bool rhs_is_float       = type_is_float(rhs_type);
    bool operands_are_float = lhs_is_float || rhs_is_float;
    bool lhs_is_signed      = type_is_signed(lhs_type);
    bool rhs_is_signed      = type_is_signed(rhs_type);
    bool is_signed          = type_is_signed(expr->type) || lhs_is_signed || rhs_is_signed;

    // untyped pointer comparisons (ptr == int): cast integer to pointer type
    if ((expr->binary_expr.op == TOKEN_EQUAL_EQUAL || expr->binary_expr.op == TOKEN_BANG_EQUAL))
    {
        if (type_is_pointer_like(lhs_type) && type_is_integer(rhs_type))
        {
            LLVMTypeRef lhs_ty = LLVMTypeOf(lhs);
            rhs                = LLVMBuildIntToPtr(ctx->builder, rhs, lhs_ty, "int2ptr");
        }
        else if (type_is_integer(lhs_type) && type_is_pointer_like(rhs_type))
        {
            LLVMTypeRef rhs_ty = LLVMTypeOf(rhs);
            lhs                = LLVMBuildIntToPtr(ctx->builder, lhs, rhs_ty, "int2ptr");
        }
        else if (type_is_integer(lhs_type) && type_is_integer(rhs_type))
        {
            // align integer widths for icmp
            LLVMTypeRef lty = LLVMTypeOf(lhs);
            LLVMTypeRef rty = LLVMTypeOf(rhs);
            if (LLVMGetTypeKind(lty) == LLVMIntegerTypeKind && LLVMGetTypeKind(rty) == LLVMIntegerTypeKind && lty != rty)
            {
                unsigned lw = LLVMGetIntTypeWidth(lty);
                unsigned rw = LLVMGetIntTypeWidth(rty);
                if (lw != rw)
                {
                    if (rw < lw)
                        rhs = LLVMBuildZExt(ctx->builder, rhs, lty, "zext_icmp");
                    else
                        rhs = LLVMBuildTrunc(ctx->builder, rhs, lty, "trunc_icmp");
                }
            }
        }
    }

    // pointer arithmetic: ptr + int or ptr - int
    if ((expr->binary_expr.op == TOKEN_PLUS || expr->binary_expr.op == TOKEN_MINUS) && type_is_pointer_like(lhs_type) && type_is_integer(rhs_type))
    {
        LLVMValueRef index = rhs;
        if (expr->binary_expr.op == TOKEN_MINUS)
        {
            index = LLVMBuildNeg(ctx->builder, rhs, "negindex");
        }

        // Get the pointee type to scale the index correctly
        Type *pointee_type = NULL;
        if (lhs_type->kind == TYPE_POINTER)
        {
            pointee_type = lhs_type->pointer.base;
        }

        // Use proper element type for GEP scaling
        if (pointee_type)
        {
            LLVMTypeRef  elem_ty = codegen_get_llvm_type(ctx, pointee_type);
            LLVMValueRef gep     = LLVMBuildGEP2(ctx->builder, elem_ty, lhs, &index, 1, "ptradd");
            return gep;
        }
        else
        {
            // Fallback to byte addressing for untyped pointers (TYPE_PTR)
            LLVMTypeRef  i8ty = LLVMInt8TypeInContext(ctx->context);
            LLVMValueRef gep  = LLVMBuildGEP2(ctx->builder, i8ty, lhs, &index, 1, "ptradd");
            return gep;
        }
    }

    switch (expr->binary_expr.op)
    {
    case TOKEN_PLUS:
        return operands_are_float ? LLVMBuildFAdd(ctx->builder, lhs, rhs, "add") : LLVMBuildAdd(ctx->builder, lhs, rhs, "add");
    case TOKEN_MINUS:
        return operands_are_float ? LLVMBuildFSub(ctx->builder, lhs, rhs, "sub") : LLVMBuildSub(ctx->builder, lhs, rhs, "sub");
    case TOKEN_STAR:
        return operands_are_float ? LLVMBuildFMul(ctx->builder, lhs, rhs, "mul") : LLVMBuildMul(ctx->builder, lhs, rhs, "mul");
    case TOKEN_SLASH:
        return operands_are_float ? LLVMBuildFDiv(ctx->builder, lhs, rhs, "div") : (is_signed ? LLVMBuildSDiv(ctx->builder, lhs, rhs, "div") : LLVMBuildUDiv(ctx->builder, lhs, rhs, "div"));
    case TOKEN_PERCENT:
        return is_signed ? LLVMBuildSRem(ctx->builder, lhs, rhs, "rem") : LLVMBuildURem(ctx->builder, lhs, rhs, "rem");
    case TOKEN_AMPERSAND:
        // bitwise and (integers)
        return LLVMBuildAnd(ctx->builder, lhs, rhs, "band");
    case TOKEN_PIPE:
        // bitwise or
        return LLVMBuildOr(ctx->builder, lhs, rhs, "bor");
    case TOKEN_EQUAL_EQUAL:
        return operands_are_float ? LLVMBuildFCmp(ctx->builder, LLVMRealOEQ, lhs, rhs, "feq") : LLVMBuildICmp(ctx->builder, LLVMIntEQ, lhs, rhs, "eq");
    case TOKEN_BANG_EQUAL:
        return operands_are_float ? LLVMBuildFCmp(ctx->builder, LLVMRealUNE, lhs, rhs, "fne") : LLVMBuildICmp(ctx->builder, LLVMIntNE, lhs, rhs, "ne");
    case TOKEN_LESS:
        return operands_are_float ? LLVMBuildFCmp(ctx->builder, LLVMRealOLT, lhs, rhs, "lt") : (is_signed ? LLVMBuildICmp(ctx->builder, LLVMIntSLT, lhs, rhs, "lt") : LLVMBuildICmp(ctx->builder, LLVMIntULT, lhs, rhs, "lt"));
    case TOKEN_LESS_EQUAL:
        return operands_are_float ? LLVMBuildFCmp(ctx->builder, LLVMRealOLE, lhs, rhs, "le") : (is_signed ? LLVMBuildICmp(ctx->builder, LLVMIntSLE, lhs, rhs, "le") : LLVMBuildICmp(ctx->builder, LLVMIntULE, lhs, rhs, "le"));
    case TOKEN_GREATER:
        return operands_are_float ? LLVMBuildFCmp(ctx->builder, LLVMRealOGT, lhs, rhs, "gt") : (is_signed ? LLVMBuildICmp(ctx->builder, LLVMIntSGT, lhs, rhs, "gt") : LLVMBuildICmp(ctx->builder, LLVMIntUGT, lhs, rhs, "gt"));
    case TOKEN_GREATER_EQUAL:
        return operands_are_float ? LLVMBuildFCmp(ctx->builder, LLVMRealOGE, lhs, rhs, "ge") : (is_signed ? LLVMBuildICmp(ctx->builder, LLVMIntSGE, lhs, rhs, "ge") : LLVMBuildICmp(ctx->builder, LLVMIntUGE, lhs, rhs, "ge"));
    case TOKEN_AMPERSAND_AMPERSAND:
    {
        LLVMValueRef l1 = LLVMBuildICmp(ctx->builder, LLVMIntNE, lhs, LLVMConstNull(LLVMTypeOf(lhs)), "l1");
        LLVMValueRef r1 = LLVMBuildICmp(ctx->builder, LLVMIntNE, rhs, LLVMConstNull(LLVMTypeOf(rhs)), "r1");
        return LLVMBuildAnd(ctx->builder, l1, r1, "and");
    }
    case TOKEN_PIPE_PIPE:
    case TOKEN_KW_OR:
    {
        LLVMValueRef l1 = LLVMBuildICmp(ctx->builder, LLVMIntNE, lhs, LLVMConstNull(LLVMTypeOf(lhs)), "l1");
        LLVMValueRef r1 = LLVMBuildICmp(ctx->builder, LLVMIntNE, rhs, LLVMConstNull(LLVMTypeOf(rhs)), "r1");
        return LLVMBuildOr(ctx->builder, l1, r1, "or");
    }
    case TOKEN_CARET:
        return LLVMBuildXor(ctx->builder, lhs, rhs, "xor");
    case TOKEN_LESS_LESS:
    case TOKEN_GREATER_GREATER:
    {
        // llvm requires shift amount to match operand width
        LLVMTypeRef lhs_ty = LLVMTypeOf(lhs);
        LLVMTypeRef rhs_ty = LLVMTypeOf(rhs);
        if (lhs_ty != rhs_ty)
        {
            unsigned lhs_bits = LLVMGetIntTypeWidth(lhs_ty);
            unsigned rhs_bits = LLVMGetIntTypeWidth(rhs_ty);
            if (rhs_bits > lhs_bits)
                rhs = LLVMBuildTrunc(ctx->builder, rhs, lhs_ty, "shamt");
            else if (rhs_bits < lhs_bits)
                rhs = LLVMBuildZExt(ctx->builder, rhs, lhs_ty, "shamt");
        }
        if (expr->binary_expr.op == TOKEN_LESS_LESS)
            return LLVMBuildShl(ctx->builder, lhs, rhs, "shl");
        else
            return is_signed ? LLVMBuildAShr(ctx->builder, lhs, rhs, "shr") : LLVMBuildLShr(ctx->builder, lhs, rhs, "shr");
    }
        {
        default:
            codegen_error(ctx, expr, "unimplemented binary operator");
            return NULL;
        }
    }
}

LLVMValueRef codegen_expr_unary(CodegenContext *ctx, AstNode *expr)
{
    LLVMValueRef operand = codegen_expr(ctx, expr->unary_expr.expr);
    if (!operand)
    {
        codegen_error(ctx, expr, "failed to generate unary operand");
        return NULL;
    }

    switch (expr->unary_expr.op)
    {
    case TOKEN_MINUS:
        operand = codegen_load_if_needed(ctx, operand, expr->unary_expr.expr->type, expr->unary_expr.expr);
        if (type_is_float(expr->type))
        {
            return LLVMBuildFNeg(ctx->builder, operand, "neg");
        }
        else
        {
            return LLVMBuildNeg(ctx->builder, operand, "neg");
        }
    case TOKEN_QUESTION: // address-of
        return operand;
    case TOKEN_AT: // dereference
        // for dereference, we always need to load the pointer value first if it's in an alloca/global
        if (LLVMIsAAllocaInst(operand) || LLVMIsAGlobalVariable(operand))
        {
            LLVMTypeRef ptr_type = codegen_get_llvm_type(ctx, expr->unary_expr.expr->type);
            operand              = LLVMBuildLoad2(ctx->builder, ptr_type, operand, "");
        }
        // then dereference the pointer
        return LLVMBuildLoad2(ctx->builder, codegen_get_llvm_type(ctx, expr->type), operand, "deref");
    case TOKEN_TILDE: // bitwise not
        operand = codegen_load_if_needed(ctx, operand, expr->unary_expr.expr->type, expr->unary_expr.expr);
        return LLVMBuildNot(ctx->builder, operand, "bnot");
    case TOKEN_BANG: // logical not
        operand = codegen_load_if_needed(ctx, operand, expr->unary_expr.expr->type, expr->unary_expr.expr);
        // compare against zero: !x becomes x == 0
        if (type_is_float(expr->unary_expr.expr->type))
        {
            LLVMValueRef zero = LLVMConstReal(LLVMTypeOf(operand), 0.0);
            return LLVMBuildFCmp(ctx->builder, LLVMRealOEQ, operand, zero, "lnot");
        }
        else
        {
            LLVMValueRef zero = LLVMConstInt(LLVMTypeOf(operand), 0, false);
            return LLVMBuildICmp(ctx->builder, LLVMIntEQ, operand, zero, "lnot");
        }
    default:
        codegen_error(ctx, expr, "unimplemented unary operator");
        return NULL;
    }
}

LLVMValueRef codegen_expr_call(CodegenContext *ctx, AstNode *expr)
{
    // check for builtin/intrinsic functions first
    if (expr->call_expr.func->kind == AST_EXPR_IDENT)
    {
        const char *func_name = expr->call_expr.func->ident_expr.name;

        if (strcmp(func_name, "va_count") == 0)
        {
            if (expr->call_expr.args && expr->call_expr.args->count != 0)
            {
                codegen_error(ctx, expr, "va_count() expects no arguments");
                return NULL;
            }
            if (!ctx->current_vararg_count_value)
            {
                codegen_error(ctx, expr, "va_count used outside mach variadic function");
                return NULL;
            }
            return ctx->current_vararg_count_value;
        }

        if (strcmp(func_name, "va_arg") == 0)
        {
            if (!ctx->current_vararg_array || !ctx->current_vararg_count_value)
            {
                codegen_error(ctx, expr, "va_arg used outside mach variadic function");
                return NULL;
            }
            if (!expr->call_expr.args || expr->call_expr.args->count != 1)
            {
                codegen_error(ctx, expr, "va_arg() expects exactly one argument");
                return NULL;
            }

            AstNode     *idx_node = expr->call_expr.args->items[0];
            LLVMValueRef idx_val  = codegen_expr(ctx, idx_node);
            if (!idx_val)
            {
                codegen_error(ctx, expr, "failed to generate va_arg index");
                return NULL;
            }
            idx_val = codegen_load_if_needed(ctx, idx_val, idx_node->type, idx_node);

            LLVMTypeRef idx_ty = LLVMTypeOf(idx_val);
            LLVMTypeRef i64_ty = LLVMInt64TypeInContext(ctx->context);
            if (idx_ty != i64_ty)
            {
                if (LLVMGetTypeKind(idx_ty) == LLVMIntegerTypeKind)
                {
                    unsigned width = LLVMGetIntTypeWidth(idx_ty);
                    if (width < 64)
                        idx_val = LLVMBuildZExt(ctx->builder, idx_val, i64_ty, "mach_va_index_zext");
                    else if (width > 64)
                        idx_val = LLVMBuildTrunc(ctx->builder, idx_val, i64_ty, "mach_va_index_trunc");
                }
                else
                {
                    codegen_error(ctx, expr, "va_arg index must be integer");
                    return NULL;
                }
            }

            // add bounds checking for vararg access
            LLVMBasicBlockRef bounds_fail_block = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "bounds_fail");
            LLVMBasicBlockRef bounds_ok_block   = LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function, "success");

            // check: index < 0 || index >= count
            LLVMValueRef zero          = LLVMConstInt(i64_ty, 0, false);
            LLVMValueRef is_negative   = LLVMBuildICmp(ctx->builder, LLVMIntSLT, idx_val, zero, "lt");
            LLVMValueRef is_too_large  = LLVMBuildICmp(ctx->builder, LLVMIntSGE, idx_val, ctx->current_vararg_count_value, "ge");
            LLVMValueRef out_of_bounds = LLVMBuildOr(ctx->builder, is_negative, is_too_large, "out_of_bounds");

            LLVMBuildCondBr(ctx->builder, out_of_bounds, bounds_fail_block, bounds_ok_block);

            // bounds fail: call abort
            LLVMPositionBuilderAtEnd(ctx->builder, bounds_fail_block);
            LLVMTypeRef  abort_type = LLVMFunctionType(LLVMVoidTypeInContext(ctx->context), NULL, 0, false);
            LLVMValueRef abort_func = LLVMGetNamedFunction(ctx->module, "abort");
            if (!abort_func)
            {
                abort_func = codegen_add_function(ctx, "abort", abort_type);
            }
            LLVMBuildCall2(ctx->builder, abort_type, abort_func, NULL, 0, "");
            LLVMBuildUnreachable(ctx->builder);

            // bounds ok: proceed with vararg access
            LLVMPositionBuilderAtEnd(ctx->builder, bounds_ok_block);
            LLVMTypeRef  i8_ty      = LLVMInt8TypeInContext(ctx->context);
            LLVMTypeRef  i8_ptr_ty  = LLVMPointerType(i8_ty, 0);
            LLVMValueRef indices[1] = {idx_val};
            LLVMValueRef slot       = LLVMBuildInBoundsGEP2(ctx->builder, i8_ptr_ty, ctx->current_vararg_array, indices, 1, "mach_va_slot");
            LLVMValueRef value_ptr  = LLVMBuildLoad2(ctx->builder, i8_ptr_ty, slot, "mach_va_ptr");
            return value_ptr;
        }

        // handle min() intrinsic - returns minimum value for numeric type
        if (strcmp(func_name, "min") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 1)
            {
                codegen_error(ctx, expr, "min() expects exactly one argument");
                return NULL;
            }

            AstNode *arg      = expr->call_expr.args->items[0];
            Type    *arg_type = type_resolve_alias(arg->type);

            // generate min value based on type
            LLVMTypeRef  llvm_type = codegen_get_llvm_type(ctx, arg_type);
            LLVMValueRef min_val   = NULL;

            switch (arg_type->kind)
            {
            case TYPE_U8:
            case TYPE_U16:
            case TYPE_U32:
            case TYPE_U64:
                // unsigned min is always 0
                min_val = LLVMConstInt(llvm_type, 0, false);
                break;
            case TYPE_I8:
                min_val = LLVMConstInt(llvm_type, -128, true);
                break;
            case TYPE_I16:
                min_val = LLVMConstInt(llvm_type, -32768, true);
                break;
            case TYPE_I32:
                min_val = LLVMConstInt(llvm_type, -2147483648LL, true);
                break;
            case TYPE_I64:
                min_val = LLVMConstInt(llvm_type, (1ULL << 63), true); // -9223372036854775808
                break;
            case TYPE_F16:
            case TYPE_F32:
            case TYPE_F64:
                // for floats, return negative infinity or smallest representable value
                min_val = LLVMConstRealOfString(llvm_type, "-inf");
                break;
            default:
                codegen_error(ctx, expr, "min() requires a numeric type");
                return NULL;
            }

            return min_val;
        }

        // handle max() intrinsic - returns maximum value for numeric type
        if (strcmp(func_name, "max") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 1)
            {
                codegen_error(ctx, expr, "max() expects exactly one argument");
                return NULL;
            }

            AstNode *arg      = expr->call_expr.args->items[0];
            Type    *arg_type = type_resolve_alias(arg->type);

            // generate max value based on type
            LLVMTypeRef  llvm_type = codegen_get_llvm_type(ctx, arg_type);
            LLVMValueRef max_val   = NULL;

            switch (arg_type->kind)
            {
            case TYPE_U8:
                max_val = LLVMConstInt(llvm_type, 255, false);
                break;
            case TYPE_U16:
                max_val = LLVMConstInt(llvm_type, 65535, false);
                break;
            case TYPE_U32:
                max_val = LLVMConstInt(llvm_type, 4294967295U, false);
                break;
            case TYPE_U64:
                max_val = LLVMConstInt(llvm_type, 0xFFFFFFFFFFFFFFFFULL, false);
                break;
            case TYPE_I8:
                max_val = LLVMConstInt(llvm_type, 127, true);
                break;
            case TYPE_I16:
                max_val = LLVMConstInt(llvm_type, 32767, true);
                break;
            case TYPE_I32:
                max_val = LLVMConstInt(llvm_type, 2147483647, true);
                break;
            case TYPE_I64:
                max_val = LLVMConstInt(llvm_type, 9223372036854775807LL, true);
                break;
            case TYPE_F16:
            case TYPE_F32:
            case TYPE_F64:
                // for floats, return positive infinity or largest representable value
                max_val = LLVMConstRealOfString(llvm_type, "inf");
                break;
            default:
                codegen_error(ctx, expr, "max() requires a numeric type");
                return NULL;
            }

            return max_val;
        }

        // handle iota() intrinsic - compile-time incrementing counter
        if (strcmp(func_name, "iota") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 0)
            {
                codegen_error(ctx, expr, "iota() expects no arguments");
                return NULL;
            }

            // increment and return the iota counter
            unsigned long iota_value = ctx->iota_counter++;
            expr->type               = type_u64();

            return LLVMConstInt(LLVMInt64TypeInContext(ctx->context), iota_value, false);
        }

        // handle size_of() intrinsic
        if (strcmp(func_name, "size_of") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 1)
            {
                codegen_error(ctx, expr, "size_of() expects exactly one argument");
                return NULL;
            }

            AstNode *arg      = expr->call_expr.args->items[0];
            Type    *arg_type = type_resolve_alias(arg->type);
            size_t   size     = type_sizeof(arg_type);

            // set the expression type for the size_of() call
            expr->type = type_u64();

            return LLVMConstInt(LLVMInt64TypeInContext(ctx->context), size, false);
        }

        // handle align_of() intrinsic
        if (strcmp(func_name, "align_of") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 1)
            {
                codegen_error(ctx, expr, "align_of() expects exactly one argument");
                return NULL;
            }

            AstNode *arg       = expr->call_expr.args->items[0];
            Type    *arg_type  = type_resolve_alias(arg->type);
            size_t   alignment = type_alignof(arg_type);

            // set the expression type for the align_of() call
            expr->type = type_u64();

            return LLVMConstInt(LLVMInt64TypeInContext(ctx->context), alignment, false);
        }

        // handle offset_of() intrinsic
        if (strcmp(func_name, "offset_of") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 2)
            {
                codegen_error(ctx, expr, "offset_of() expects exactly two arguments");
                return NULL;
            }

            AstNode *type_arg  = expr->call_expr.args->items[0];
            AstNode *field_arg = expr->call_expr.args->items[1];

            Type       *struct_type = type_resolve_alias(type_arg->type);
            const char *field_name  = field_arg->ident_expr.name;

            // calculate field offset
            size_t offset = 0;
            bool   found  = false;

            if (struct_type->kind == TYPE_STRUCT && struct_type->composite.fields)
            {
                for (size_t i = 0; i < struct_type->composite.field_count; i++)
                {
                    Symbol *field = &struct_type->composite.fields[i];
                    if (strcmp(field->name, field_name) == 0)
                    {
                        offset = field->field.offset;
                        found  = true;
                        break;
                    }
                }
            }

            if (!found)
            {
                codegen_error(ctx, expr, "field '%s' not found in record", field_name);
                return NULL;
            }

            expr->type = type_u64();
            return LLVMConstInt(LLVMInt64TypeInContext(ctx->context), offset, false);
        }

        // handle type_of() intrinsic
        if (strcmp(func_name, "type_of") == 0)
        {
            if (!expr->call_expr.args || expr->call_expr.args->count != 1)
            {
                codegen_error(ctx, expr, "type_of() expects exactly one argument");
                return NULL;
            }

            AstNode *arg      = expr->call_expr.args->items[0];
            Type    *arg_type = type_resolve_alias(arg->type);

            // generate a simple hash of the type for runtime identification
            // this is a basic implementation - you could make it more sophisticated
            uint64_t type_hash = (uint64_t)arg_type->kind;
            type_hash          = type_hash * 31 + arg_type->size;
            type_hash          = type_hash * 31 + arg_type->alignment;

            expr->type = type_u64();
            return LLVMConstInt(LLVMInt64TypeInContext(ctx->context), type_hash, false);
        }
    }

    Symbol *callee_symbol = expr->call_expr.func->symbol;
    if (callee_symbol && callee_symbol->kind == SYMBOL_FUNC)
    {
        if (!codegen_get_symbol_value(ctx, callee_symbol))
        {
            codegen_declare_function_symbol(ctx, callee_symbol);
        }
    }

    LLVMValueRef func = codegen_expr(ctx, expr->call_expr.func);
    if (!func)
    {
        codegen_error(ctx, expr, "failed to generate function for call");
        return NULL;
    }

    // load function pointer if needed (for function parameters)
    func = codegen_load_if_needed(ctx, func, expr->call_expr.func->type, expr->call_expr.func);

    // get function type early for parameter type checking
    Type *func_type = type_resolve_alias(expr->call_expr.func->type);
    if (!func_type || func_type->kind != TYPE_FUNCTION)
    {
        codegen_error(ctx, expr, "call expression does not have function type");
        return NULL;
    }

    bool   uses_mach_varargs = callee_symbol && callee_symbol->kind == SYMBOL_FUNC && callee_symbol->func.uses_mach_varargs;
    int    arg_expr_count    = expr->call_expr.args ? expr->call_expr.args->count : 0;
    size_t fixed_param_count = func_type->function.param_count;

    bool forwards_varargs = false;
    int  varargs_markers  = 0;
    if (expr->call_expr.args)
    {
        for (int i = 0; i < arg_expr_count; i++)
        {
            AstNode *arg_node = expr->call_expr.args->items[i];
            if (arg_node && arg_node->kind == AST_EXPR_VARARGS)
            {
                forwards_varargs = true;
                varargs_markers++;
            }
        }
    }

    int extra_arg_count = 0;
    if (expr->call_expr.args && arg_expr_count > (int)fixed_param_count)
    {
        for (int i = (int)fixed_param_count; i < arg_expr_count; i++)
        {
            AstNode *arg_node = expr->call_expr.args->items[i];
            if (arg_node->kind != AST_EXPR_VARARGS)
            {
                extra_arg_count++;
            }
        }
    }

    if (uses_mach_varargs && !forwards_varargs && arg_expr_count < (int)fixed_param_count)
    {
        codegen_error(ctx, expr, "not enough arguments for variadic function");
        return NULL;
    }

    // account for varargs markers that will be skipped in actual arg emission
    int           actual_arg_count = arg_expr_count - varargs_markers;
    int           total_args       = uses_mach_varargs ? (int)fixed_param_count + 2 : actual_arg_count;
    LLVMValueRef *args             = NULL;
    if (total_args > 0)
        args = malloc(sizeof(LLVMValueRef) * total_args);

    // evaluate fixed parameters
    int fixed_emit    = 0;
    int arg_src_index = 0;
    for (arg_src_index = 0; arg_src_index < arg_expr_count && fixed_emit < (int)fixed_param_count; arg_src_index++)
    {
        AstNode *arg_node = expr->call_expr.args->items[arg_src_index];

        if (arg_node && arg_node->kind == AST_EXPR_VARARGS)
        {
            continue;
        }

        LLVMValueRef value = codegen_expr(ctx, arg_node);
        if (!value)
        {
            free(args);
            codegen_error(ctx, expr, "failed to generate call argument %d", arg_src_index);
            return NULL;
        }
        value            = codegen_load_if_needed(ctx, value, arg_node->type, arg_node);
        args[fixed_emit] = value;

        Type       *param_type     = func_type->function.param_types[fixed_emit];
        Type       *resolved_param = type_resolve_alias(param_type);
        Type       *resolved_arg   = type_resolve_alias(arg_node->type);
        LLVMTypeRef expected_llvm  = codegen_get_llvm_type(ctx, param_type);
        if (!expected_llvm)
            expected_llvm = LLVMTypeOf(args[fixed_emit]);

        if (resolved_param && resolved_arg)
        {
            if (type_is_numeric(resolved_param) && type_is_numeric(resolved_arg))
            {
                bool        param_is_float = type_is_float(resolved_param);
                bool        arg_is_float   = type_is_float(resolved_arg);
                LLVMTypeRef current_ty     = LLVMTypeOf(args[fixed_emit]);

                if (!param_is_float && !arg_is_float)
                {
                    unsigned from_bits = (unsigned)(resolved_arg->size * 8);
                    unsigned to_bits   = (unsigned)(resolved_param->size * 8);
                    if (from_bits < to_bits)
                    {
                        if (type_is_signed(resolved_arg))
                            args[fixed_emit] = LLVMBuildSExt(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_sext");
                        else
                            args[fixed_emit] = LLVMBuildZExt(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_zext");
                    }
                    else if (from_bits > to_bits)
                    {
                        args[fixed_emit] = LLVMBuildTrunc(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_trunc");
                    }
                    else if (expected_llvm && current_ty != expected_llvm)
                    {
                        args[fixed_emit] = LLVMBuildBitCast(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_bitcast");
                    }
                }
                else if (param_is_float && arg_is_float)
                {
                    unsigned from_bits = (unsigned)(resolved_arg->size * 8);
                    unsigned to_bits   = (unsigned)(resolved_param->size * 8);
                    if (from_bits < to_bits)
                        args[fixed_emit] = LLVMBuildFPExt(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_fpext");
                    else if (from_bits > to_bits)
                        args[fixed_emit] = LLVMBuildFPTrunc(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_fptrunc");
                }
                else if (param_is_float && !arg_is_float)
                {
                    if (type_is_signed(resolved_arg))
                        args[fixed_emit] = LLVMBuildSIToFP(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_sitofp");
                    else
                        args[fixed_emit] = LLVMBuildUIToFP(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_uitofp");
                }
                else if (!param_is_float && arg_is_float)
                {
                    if (type_is_signed(resolved_param))
                        args[fixed_emit] = LLVMBuildFPToSI(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_fptosi");
                    else
                        args[fixed_emit] = LLVMBuildFPToUI(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_fptoui");
                }
            }
            else if (type_is_pointer_like(resolved_param) && type_is_pointer_like(resolved_arg))
            {
                if (expected_llvm && LLVMTypeOf(args[fixed_emit]) != expected_llvm)
                    args[fixed_emit] = LLVMBuildBitCast(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_ptrcast");
            }
            else if (type_is_pointer_like(resolved_param) && type_is_integer(resolved_arg))
            {
                if (expected_llvm)
                    args[fixed_emit] = LLVMBuildIntToPtr(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_inttoptr");
            }
            else if (type_is_integer(resolved_param) && type_is_pointer_like(resolved_arg))
            {
                if (expected_llvm)
                    args[fixed_emit] = LLVMBuildPtrToInt(ctx->builder, args[fixed_emit], expected_llvm, "call_arg_ptrtoint");
            }
        }

        fixed_emit++;
    }

    if (fixed_emit < (int)fixed_param_count)
    {
        free(args);
        codegen_error(ctx, expr, "not enough arguments for function call");
        return NULL;
    }

    if (uses_mach_varargs)
    {
        LLVMTypeRef i64_ty     = LLVMInt64TypeInContext(ctx->context);
        LLVMTypeRef i8_ty      = LLVMInt8TypeInContext(ctx->context);
        LLVMTypeRef i8_ptr_ty  = LLVMPointerType(i8_ty, 0);
        LLVMTypeRef i8_ptr_ptr = LLVMPointerType(i8_ptr_ty, 0);

        if (forwards_varargs)
        {
            LLVMValueRef count_val  = ctx->current_vararg_count_value ? ctx->current_vararg_count_value : LLVMConstInt(i64_ty, 0, false);
            args[fixed_param_count] = count_val;

            if (ctx->current_vararg_array)
            {
                args[fixed_param_count + 1] = ctx->current_vararg_array;
            }
            else
            {
                args[fixed_param_count + 1] = LLVMConstNull(i8_ptr_ptr);
            }
        }
        else
        {
            args[fixed_param_count] = LLVMConstInt(i64_ty, (unsigned long long)extra_arg_count, false);

            if (extra_arg_count > 0)
            {
                LLVMValueRef count_val = LLVMConstInt(i64_ty, (unsigned long long)extra_arg_count, false);

                // allocate vararg array in entry block to ensure it persists for the entire call
                LLVMBasicBlockRef entry       = LLVMGetEntryBasicBlock(ctx->current_function);
                LLVMBuilderRef    tmp_builder = LLVMCreateBuilderInContext(ctx->context);
                LLVMValueRef      first_inst  = LLVMGetFirstInstruction(entry);
                if (first_inst)
                {
                    LLVMPositionBuilderBefore(tmp_builder, first_inst);
                }
                else
                {
                    LLVMPositionBuilderAtEnd(tmp_builder, entry);
                }
                LLVMValueRef array_alloc = LLVMBuildArrayAlloca(tmp_builder, i8_ptr_ty, count_val, "mach_vararg_array");
                LLVMDisposeBuilder(tmp_builder);

                for (int j = 0; j < extra_arg_count; j++)
                {
                    AstNode     *arg_node = expr->call_expr.args->items[fixed_param_count + j];
                    Type        *arg_type = type_resolve_alias(arg_node->type);
                    LLVMValueRef value    = codegen_expr(ctx, arg_node);
                    if (!value)
                    {
                        free(args);
                        codegen_error(ctx, expr, "failed to generate variadic argument %d", fixed_param_count + j);
                        return NULL;
                    }

                    // For composite types (arrays, structs), we need the full value, not a loaded scalar
                    // For simple types, we need to load from allocas
                    if (arg_type && (arg_type->kind == TYPE_ARRAY || arg_type->kind == TYPE_STRUCT || arg_type->kind == TYPE_UNION || arg_type->kind == TYPE_STR))
                    {
                        // Composite types: if it's a pointer to the composite, load it
                        // This includes allocas, globals, and GEPs (e.g., array element access)
                        if (LLVMIsAAllocaInst(value) || LLVMIsAGlobalVariable(value) || LLVMIsAGetElementPtrInst(value))
                        {
                            LLVMTypeRef composite_ty = codegen_get_llvm_type(ctx, arg_type);
                            value                    = LLVMBuildLoad2(ctx->builder, composite_ty, value, "composite_load");
                        }
                        // else: already a composite value
                    }
                    else
                    {
                        // Simple types: load if needed
                        value = codegen_load_if_needed(ctx, value, arg_node->type, arg_node);
                    }

                    LLVMValueRef stored_value = value;
                    LLVMTypeRef  value_ty     = LLVMTypeOf(value);

                    if (arg_type && type_is_integer(arg_type) && arg_type->size < 8)
                    {
                        if (type_is_signed(arg_type))
                        {
                            stored_value = LLVMBuildSExt(ctx->builder, value, i64_ty, "vararg_promote_signed");
                            value_ty     = i64_ty;
                        }
                        else
                        {
                            stored_value = LLVMBuildZExt(ctx->builder, value, i64_ty, "vararg_promote_unsigned");
                            value_ty     = i64_ty;
                        }
                    }

                    char slot_name[32];
                    snprintf(slot_name, sizeof(slot_name), "mach_vararg_val_%d", j);
                    LLVMValueRef value_alloca = codegen_create_alloca(ctx, value_ty, slot_name);
                    LLVMBuildStore(ctx->builder, stored_value, value_alloca);
                    LLVMValueRef data_ptr = LLVMBuildBitCast(ctx->builder, value_alloca, i8_ptr_ty, "mach_vararg_cast");

                    LLVMValueRef idx_val = LLVMConstInt(i64_ty, (unsigned long long)j, false);
                    LLVMValueRef slot    = LLVMBuildInBoundsGEP2(ctx->builder, i8_ptr_ty, array_alloc, &idx_val, 1, "mach_vararg_slot");
                    LLVMBuildStore(ctx->builder, data_ptr, slot);
                }

                args[fixed_param_count + 1] = array_alloc;
            }
            else
            {
                args[fixed_param_count + 1] = LLVMConstNull(i8_ptr_ptr);
            }
        }
    }
    else
    {
        // C-style varargs: iterate through all variadic arguments
        // note: arg array index may differ from source index if AST_EXPR_VARARGS markers are present
        int arg_dst_index = (int)fixed_param_count;
        for (int i = (int)fixed_param_count; i < arg_expr_count; i++)
        {
            AstNode *arg_node = expr->call_expr.args->items[i];
            if (arg_node->kind == AST_EXPR_VARARGS)
            {
                // varargs forwarding not supported in C-style varargs
                continue;
            }
            LLVMValueRef value = codegen_expr(ctx, arg_node);
            if (!value)
            {
                free(args);
                codegen_error(ctx, expr, "failed to generate call argument %d", i);
                return NULL;
            }
            value               = codegen_load_if_needed(ctx, value, arg_node->type, arg_node);
            args[arg_dst_index] = value;
            Type *at            = type_resolve_alias(arg_node->type);

            if (at && at->kind == TYPE_ARRAY)
            {
                LLVMTypeRef elem_type = codegen_get_llvm_type(ctx, at->array.elem_type);
                if (!elem_type)
                    elem_type = LLVMInt8TypeInContext(ctx->context);
                LLVMTypeRef llvm_array_type = codegen_get_llvm_type(ctx, at);
                if (!llvm_array_type)
                    llvm_array_type = LLVMArrayType(elem_type, (unsigned)at->array.size);
                LLVMValueRef tmp = codegen_create_alloca(ctx, llvm_array_type, "vararg_array_tmp");
                LLVMBuildStore(ctx->builder, args[arg_dst_index], tmp);
                LLVMTypeRef  llvm_i32   = LLVMInt32TypeInContext(ctx->context);
                LLVMValueRef zero       = LLVMConstInt(llvm_i32, 0, false);
                LLVMValueRef indices[2] = {zero, zero};
                LLVMValueRef data_ptr   = LLVMBuildGEP2(ctx->builder, llvm_array_type, tmp, indices, 2, "vararg_array_data");
                LLVMTypeRef  elem_ptr   = LLVMPointerType(elem_type, 0);
                args[arg_dst_index]     = LLVMBuildBitCast(ctx->builder, data_ptr, elem_ptr, "vararg_array_decay");
            }
            else if (at && type_is_float(at) && at->size < 8)
            {
                args[arg_dst_index] = LLVMBuildFPExt(ctx->builder, args[arg_dst_index], LLVMDoubleTypeInContext(ctx->context), "vararg_fpromote");
            }
            else if (at && type_is_integer(at) && at->size < 4)
            {
                LLVMTypeRef i32ty = LLVMInt32TypeInContext(ctx->context);
                if (type_is_signed(at))
                    args[arg_dst_index] = LLVMBuildSExt(ctx->builder, args[arg_dst_index], i32ty, "vararg_ipromote");
                else
                    args[arg_dst_index] = LLVMBuildZExt(ctx->builder, args[arg_dst_index], i32ty, "vararg_ipromote");
            }
            arg_dst_index++;
        }
    }

    // build the actual function type for the call
    LLVMTypeRef return_type = func_type->function.return_type ? codegen_get_llvm_type(ctx, func_type->function.return_type) : LLVMVoidTypeInContext(ctx->context);

    size_t       llvm_param_count = fixed_param_count + (uses_mach_varargs ? 2 : 0);
    LLVMTypeRef *param_types      = NULL;
    if (llvm_param_count > 0)
    {
        param_types = malloc(sizeof(LLVMTypeRef) * llvm_param_count);
        for (size_t i = 0; i < fixed_param_count; i++)
        {
            param_types[i] = codegen_get_llvm_type(ctx, func_type->function.param_types[i]);
        }
        if (uses_mach_varargs)
        {
            LLVMTypeRef i64_ty                 = LLVMInt64TypeInContext(ctx->context);
            LLVMTypeRef i8_ptr                 = LLVMPointerType(LLVMInt8TypeInContext(ctx->context), 0);
            param_types[fixed_param_count]     = i64_ty;
            param_types[fixed_param_count + 1] = LLVMPointerType(i8_ptr, 0);
        }
    }

    LLVMTypeRef  llvm_func_type = LLVMFunctionType(return_type, param_types, llvm_param_count, uses_mach_varargs ? false : func_type->function.is_variadic);
    LLVMValueRef result         = LLVMBuildCall2(ctx->builder, llvm_func_type, func, args, total_args, "");

    free(param_types);
    free(args);
    return result;
}

LLVMValueRef codegen_expr_cast(CodegenContext *ctx, AstNode *expr)
{
    LLVMValueRef value = codegen_expr(ctx, expr->cast_expr.expr);
    if (!value)
    {
        codegen_error(ctx, expr, "failed to generate cast operand");
        return NULL;
    }

    // operate on the loaded value unless we're decaying arrays
    // loading before type resolution ensures correct scalar casts
    // underlying array-to-pointer cases are handled later

    Type *from_type = expr->cast_expr.expr->type;
    Type *to_type   = expr->type;

    // resolve type aliases for proper type checking
    Type *resolved_from_type = type_resolve_alias(from_type);
    Type *resolved_to_type   = type_resolve_alias(to_type);
    if (!resolved_from_type)
        resolved_from_type = from_type;
    if (!resolved_to_type)
        resolved_to_type = to_type;

    // for array to pointer decay, don't load the array value; otherwise load
    bool is_array_decay = (resolved_from_type->kind == TYPE_ARRAY && (resolved_to_type->kind == TYPE_POINTER || resolved_to_type->kind == TYPE_PTR));
    bool is_address_of  = expr->cast_expr.expr && expr->cast_expr.expr->kind == AST_EXPR_UNARY && expr->cast_expr.expr->unary_expr.op == TOKEN_QUESTION;

    if (!is_array_decay && !(is_address_of && type_is_pointer_like(resolved_from_type)))
    {
        value = codegen_load_if_needed(ctx, value, from_type, expr->cast_expr.expr);
    }

    LLVMTypeRef to_llvm_type = codegen_get_llvm_type(ctx, to_type);

    // same type cast (no-op, just type reinterpretation at compile time)
    // handles type aliases that resolve to the same underlying type
    if (resolved_from_type == resolved_to_type)
    {
        return value;
    }

    // same size, same kind cast (type aliases with matching structure)
    if (resolved_from_type->size == resolved_to_type->size && resolved_from_type->kind == resolved_to_type->kind)
    {
        return value;
    }

    // numeric casts
    if (type_is_numeric(resolved_from_type) && type_is_numeric(resolved_to_type))
    {
        if (type_is_float(resolved_from_type) && type_is_float(resolved_to_type))
        {
            // float to float
            if (resolved_from_type->size < resolved_to_type->size)
            {
                return LLVMBuildFPExt(ctx->builder, value, to_llvm_type, "fpext");
            }
            else if (resolved_from_type->size > resolved_to_type->size)
            {
                return LLVMBuildFPTrunc(ctx->builder, value, to_llvm_type, "fptrunc");
            }
            return value;
        }
        else if (type_is_float(resolved_from_type) && type_is_integer(resolved_to_type))
        {
            // float to int
            if (type_is_signed(resolved_to_type))
            {
                return LLVMBuildFPToSI(ctx->builder, value, to_llvm_type, "fptosi");
            }
            else
            {
                return LLVMBuildFPToUI(ctx->builder, value, to_llvm_type, "fptoui");
            }
        }
        else if (type_is_integer(resolved_from_type) && type_is_float(resolved_to_type))
        {
            // int to float
            if (type_is_signed(resolved_from_type))
            {
                return LLVMBuildSIToFP(ctx->builder, value, to_llvm_type, "sitofp");
            }
            else
            {
                return LLVMBuildUIToFP(ctx->builder, value, to_llvm_type, "uitofp");
            }
        }
        else
        {
            // int to int
            if (resolved_from_type->size < resolved_to_type->size)
            {
                if (type_is_signed(resolved_from_type))
                {
                    return LLVMBuildSExt(ctx->builder, value, to_llvm_type, "sext");
                }
                else
                {
                    return LLVMBuildZExt(ctx->builder, value, to_llvm_type, "zext");
                }
            }
            else if (resolved_from_type->size > resolved_to_type->size)
            {
                return LLVMBuildTrunc(ctx->builder, value, to_llvm_type, "trunc");
            }
            return value;
        }
    }

    // integer to pointer cast
    if (type_is_numeric(resolved_from_type) && type_is_pointer_like(resolved_to_type))
    {
        return LLVMBuildIntToPtr(ctx->builder, value, to_llvm_type, "inttoptr");
    }

    if (type_is_pointer_like(resolved_from_type) && type_is_integer(resolved_to_type))
    {
        return LLVMBuildPtrToInt(ctx->builder, value, to_llvm_type, "ptrtoint");
    }

    // pointer casts
    if (type_is_pointer_like(resolved_from_type) && type_is_pointer_like(resolved_to_type))
    {
        return value; // all pointers are opaque in LLVM
    }

    codegen_error(ctx, expr, "unimplemented cast");
    return NULL;
}

LLVMValueRef codegen_create_alloca(CodegenContext *ctx, LLVMTypeRef type, const char *name)
{
    // if we're not in a function, this is a global variable - handle differently
    if (!ctx->current_function)
    {
        // create a global variable instead of an alloca
        LLVMValueRef global = LLVMAddGlobal(ctx->module, type, name);
        LLVMSetInitializer(global, LLVMConstNull(type));
        LLVMSetLinkage(global, LLVMInternalLinkage);
        return global;
    }

    // create allocas in entry block for better optimization
    LLVMBasicBlockRef entry       = LLVMGetEntryBasicBlock(ctx->current_function);
    LLVMBuilderRef    tmp_builder = LLVMCreateBuilderInContext(ctx->context);

    // position at start of entry block
    LLVMValueRef first_inst = LLVMGetFirstInstruction(entry);
    if (first_inst)
    {
        LLVMPositionBuilderBefore(tmp_builder, first_inst);
    }
    else
    {
        LLVMPositionBuilderAtEnd(tmp_builder, entry);
    }

    LLVMValueRef alloca = LLVMBuildAlloca(tmp_builder, type, name);
    LLVMDisposeBuilder(tmp_builder);

    return alloca;
}

static LLVMValueRef codegen_strip_pointer_casts(LLVMValueRef value)
{
    while (value)
    {
        if (LLVMIsABitCastInst(value) || LLVMIsAPtrToIntInst(value) || LLVMIsAIntToPtrInst(value))
        {
            value = LLVMGetOperand(value, 0);
            continue;
        }

        if (LLVMIsAConstantExpr(value))
        {
            LLVMOpcode opcode = LLVMGetConstOpcode(value);
            if (opcode == LLVMBitCast || opcode == LLVMPtrToInt || opcode == LLVMIntToPtr)
            {
                value = LLVMGetOperand(value, 0);
                continue;
            }
        }

        break;
    }

    return value;
}

LLVMValueRef codegen_load_if_needed(CodegenContext *ctx, LLVMValueRef value, Type *type, AstNode *source_expr)
{
    // if no expected type, assume value is already in the right form
    if (!type)
    {
        return value;
    }

    // resolve any type aliases first
    type = type_resolve_alias(type);

    if (!type)
    {
        return value;
    }

    LLVMValueRef base_value = codegen_strip_pointer_casts(value);
    bool         is_alloca  = LLVMIsAAllocaInst(base_value);
    bool         is_global  = LLVMIsAGlobalVariable(base_value);
    bool         is_gep     = LLVMIsAGetElementPtrInst(base_value);
    if (!is_gep && LLVMIsAConstantExpr(base_value) && LLVMGetConstOpcode(base_value) == LLVMGetElementPtr)
        is_gep = true;
    bool value_is_memory = is_alloca || is_global || is_gep;

    if (type->kind == TYPE_PTR || type->kind == TYPE_POINTER)
    {
        bool pointer_from_memory = value_is_memory;
        if (is_gep && source_expr && !codegen_is_lvalue(source_expr))
        {
            pointer_from_memory = false;
        }

        if (!pointer_from_memory)
        {
            return value;
        }

        bool need_load = true;
        if (source_expr)
        {
            need_load = codegen_is_lvalue(source_expr);
        }

        if (!source_expr)
        {
            need_load = is_alloca || is_global;
        }

        if (!need_load)
        {
            return value;
        }

        LLVMTypeRef llvm_type = codegen_get_llvm_type(ctx, type);
        return LLVMBuildLoad2(ctx->builder, llvm_type, value, "");
    }

    if (!value_is_memory)
    {
        return value;
    }

    LLVMTypeRef llvm_type = codegen_get_llvm_type(ctx, type);
    return LLVMBuildLoad2(ctx->builder, llvm_type, value, "");
}

static LLVMValueRef codegen_load_rvalue(CodegenContext *ctx, LLVMValueRef value, Type *type, AstNode *source_expr)
{
    if (!type)
    {
        return value;
    }

    Type *resolved = type_resolve_alias(type);
    if (resolved && (resolved->kind == TYPE_PTR || resolved->kind == TYPE_POINTER))
    {
        return codegen_load_if_needed(ctx, value, resolved, source_expr);
    }

    return codegen_load_if_needed(ctx, value, type, source_expr);
}
bool codegen_is_lvalue(AstNode *expr)
{
    switch (expr->kind)
    {
    case AST_EXPR_IDENT:
        return true;
    case AST_EXPR_INDEX:
        return true;
    case AST_EXPR_FIELD:
        return codegen_is_lvalue(expr->field_expr.object);
    case AST_EXPR_UNARY:
        return expr->unary_expr.op == TOKEN_AT;
    default:
        return false;
    }
}

LLVMValueRef codegen_expr_field(CodegenContext *ctx, AstNode *expr)
{
    // module member access: object is an identifier bound to a module symbol
    if (expr->field_expr.object && expr->field_expr.object->kind == AST_EXPR_IDENT)
    {
        Symbol *obj_sym = expr->field_expr.object->symbol;
        if (obj_sym && obj_sym->kind == SYMBOL_MODULE)
        {
            // semantic analysis stored the resolved member symbol in expr->symbol
            if (expr->symbol)
            {
                // try constant folding first for val constants
                if (expr->symbol->has_const_i64)
                {
                    LLVMTypeRef llvm_type = codegen_get_llvm_type(ctx, expr->type);
                    if (llvm_type)
                    {
                        return LLVMConstInt(llvm_type, (unsigned long long)expr->symbol->const_i64, 1);
                    }
                }

                // try evaluating as constant expression
                int64_t const_val = 0;
                if (codegen_eval_const_i64(ctx, expr, &const_val))
                {
                    LLVMTypeRef llvm_type = codegen_get_llvm_type(ctx, expr->type);
                    if (llvm_type)
                    {
                        return LLVMConstInt(llvm_type, (unsigned long long)const_val, 1);
                    }
                }

                LLVMValueRef value = codegen_get_symbol_value(ctx, expr->symbol);
                if (!value)
                {
                    codegen_error(ctx, expr, "undefined symbol '%s'", expr->field_expr.field);
                    return NULL;
                }
                return value;
            }
        }
    }

    AstNode *original_object_expr = expr->field_expr.object;
    AstNode *object_expr          = original_object_expr;
    bool     forced_pointer       = false;

    if (object_expr && object_expr->kind == AST_EXPR_UNARY && object_expr->unary_expr.op == TOKEN_AT)
    {
        // treat (@ptr).field / ptr->field as pointer-based field access
        forced_pointer = true;
        object_expr    = object_expr->unary_expr.expr;
    }

    LLVMValueRef object = codegen_expr(ctx, object_expr);
    if (!object)
    {
        codegen_error(ctx, expr, "failed to generate object for field access");
        return NULL;
    }

    // get the object type
    Type *original_type     = type_resolve_alias(object_expr->type);
    Type *object_type       = original_type;
    bool  object_is_pointer = false;

    if (object_type && object_type->kind == TYPE_POINTER)
    {
        object_is_pointer = true;
        object_type       = type_resolve_alias(object_type->pointer.base);
    }
    else if (forced_pointer)
    {
        codegen_error(ctx, expr, "field access through dereference of non-pointer type");
        return NULL;
    }

    if (!object_type)
    {
        codegen_error(ctx, expr, "unable to resolve field base type");
        return NULL;
    }

    if (object_type->kind != TYPE_STRUCT && object_type->kind != TYPE_UNION && object_type->kind != TYPE_STR)
    {
        codegen_error(ctx, expr, "field access on non-composite type");
        return NULL;
    }

    // ensure we have an address to GEP from; structs may come back as values
    if (object_is_pointer)
    {
        LLVMTypeRef ptr_ty = codegen_get_llvm_type(ctx, original_type);
        if (LLVMIsAAllocaInst(object) || LLVMIsAGlobalVariable(object) || LLVMIsAGetElementPtrInst(object))
        {
            object = LLVMBuildLoad2(ctx->builder, ptr_ty, object, "struct_ptr");
        }
    }
    else if (!LLVMIsAAllocaInst(object) && !LLVMIsAGlobalVariable(object) && !LLVMIsAGetElementPtrInst(object))
    {
        LLVMTypeRef  llvm_struct_type = codegen_get_llvm_type(ctx, object_type);
        LLVMValueRef tmp              = codegen_create_alloca(ctx, llvm_struct_type, "tmp_struct");
        LLVMBuildStore(ctx->builder, object, tmp);
        object = tmp;
    }

    // find the field
    Symbol *field_symbol = NULL;
    int     field_index  = 0;
    for (Symbol *field = object_type->composite.fields; field; field = field->next)
    {
        if (field->name && strcmp(field->name, expr->field_expr.field) == 0)
        {
            field_symbol = field;
            break;
        }
        field_index++;
    }

    if (!field_symbol)
    {
        codegen_error(ctx, expr, "no field named '%s'", expr->field_expr.field);
        return NULL;
    }

    LLVMTypeRef llvm_i32 = LLVMInt32TypeInContext(ctx->context);

    if (object_type->kind == TYPE_UNION)
    {
        LLVMValueRef indices[] = {LLVMConstInt(llvm_i32, 0, false), LLVMConstInt(llvm_i32, 0, false)};

        LLVMTypeRef  union_type     = codegen_get_llvm_type(ctx, object_type);
        LLVMValueRef storage_ptr    = LLVMBuildGEP2(ctx->builder, union_type, object, indices, 2, "union_field_storage");
        LLVMTypeRef  field_llvm     = codegen_get_llvm_type(ctx, field_symbol->type);
        LLVMTypeRef  field_ptr_type = LLVMPointerType(field_llvm, 0);
        return LLVMBuildBitCast(ctx->builder, storage_ptr, field_ptr_type, "union_field");
    }

    LLVMValueRef indices[] = {LLVMConstInt(llvm_i32, 0, false), LLVMConstInt(llvm_i32, field_index, false)};

    LLVMTypeRef struct_type = codegen_get_llvm_type(ctx, object_type);
    return LLVMBuildGEP2(ctx->builder, struct_type, object, indices, 2, "field");
}

LLVMValueRef codegen_expr_index(CodegenContext *ctx, AstNode *expr)
{
    LLVMValueRef array = codegen_expr(ctx, expr->index_expr.array);
    LLVMValueRef index = codegen_expr(ctx, expr->index_expr.index);

    if (!array || !index)
    {
        codegen_error(ctx, expr, "failed to generate array indexing operands");
        return NULL;
    }

    index = codegen_load_if_needed(ctx, index, expr->index_expr.index->type, expr->index_expr.index);

    Type *array_type = type_resolve_alias(expr->index_expr.array->type);

    if (array_type->kind == TYPE_ARRAY)
    {
        LLVMTypeRef llvm_array_type = codegen_get_llvm_type(ctx, array_type);
        if (!LLVMIsAAllocaInst(array) && !LLVMIsAGlobalVariable(array) && !LLVMIsAGetElementPtrInst(array))
        {
            LLVMValueRef tmp = codegen_create_alloca(ctx, llvm_array_type, "tmp_array_index");
            LLVMBuildStore(ctx->builder, array, tmp);
            array = tmp;
        }

        LLVMTypeRef  llvm_i32   = LLVMInt32TypeInContext(ctx->context);
        LLVMValueRef zero       = LLVMConstInt(llvm_i32, 0, false);
        LLVMValueRef gep_index  = index;
        LLVMTypeRef  index_type = LLVMTypeOf(index);
        if (LLVMGetTypeKind(index_type) != LLVMIntegerTypeKind)
        {
            codegen_error(ctx, expr, "array index must be an integer value");
            return NULL;
        }
        unsigned width = LLVMGetIntTypeWidth(index_type);
        if (width > 32)
            gep_index = LLVMBuildTrunc(ctx->builder, index, llvm_i32, "index_i32");
        else if (width < 32)
            gep_index = LLVMBuildZExt(ctx->builder, index, llvm_i32, "index_i32");

        LLVMValueRef indices[2] = {zero, gep_index};
        return LLVMBuildGEP2(ctx->builder, llvm_array_type, array, indices, 2, "array_index");
    }

    if (array_type->kind == TYPE_POINTER || array_type->kind == TYPE_PTR)
    {
        if (LLVMIsAAllocaInst(array) || LLVMIsAGlobalVariable(array))
        {
            LLVMTypeRef llvm_type = codegen_get_llvm_type(ctx, array_type);
            array                 = LLVMBuildLoad2(ctx->builder, llvm_type, array, "ptr_val");
        }
        LLVMTypeRef elem_type = codegen_get_llvm_type(ctx, array_type->pointer.base);
        return LLVMBuildGEP2(ctx->builder, elem_type, array, &index, 1, "index");
    }

    codegen_error(ctx, expr, "indexing non-array type");
    return NULL;
}

LLVMValueRef codegen_expr_array(CodegenContext *ctx, AstNode *expr)
{
    Type *array_type = expr->type;

    // resolve type aliases to get the underlying array type
    Type *resolved_type = type_resolve_alias(array_type);
    if (!resolved_type)
        resolved_type = array_type;

    if (!resolved_type || resolved_type->kind != TYPE_ARRAY)
    {
        codegen_error(ctx, expr, "array literal has invalid type");
        return NULL;
    }

    Type       *elem_type       = resolved_type->array.elem_type;
    LLVMTypeRef llvm_elem_type  = codegen_get_llvm_type(ctx, elem_type);
    LLVMTypeRef llvm_array_type = codegen_get_llvm_type(ctx, resolved_type);
    if (!llvm_elem_type || !llvm_array_type)
    {
        return NULL;
    }

    size_t elem_count = expr->array_expr.elems ? (size_t)expr->array_expr.elems->count : 0;
    if (elem_count != resolved_type->array.size)
    {
        codegen_error(ctx, expr, "array literal element count mismatch");
        return NULL;
    }

    LLVMValueRef array_value = LLVMGetUndef(llvm_array_type);
    for (size_t i = 0; i < elem_count; i++)
    {
        AstNode     *elem_node  = expr->array_expr.elems->items[i];
        LLVMValueRef elem_value = codegen_expr(ctx, elem_node);
        if (!elem_value)
            return NULL;
        elem_value = codegen_load_if_needed(ctx, elem_value, elem_node->type, elem_node);

        LLVMTypeRef value_type = LLVMTypeOf(elem_value);
        if (value_type != llvm_elem_type)
        {
            LLVMTypeKind value_kind = LLVMGetTypeKind(value_type);
            LLVMTypeKind elem_kind  = LLVMGetTypeKind(llvm_elem_type);

            if (value_kind == LLVMIntegerTypeKind && elem_kind == LLVMIntegerTypeKind)
            {
                unsigned from_bits = LLVMGetIntTypeWidth(value_type);
                unsigned to_bits   = LLVMGetIntTypeWidth(llvm_elem_type);
                if (from_bits < to_bits)
                    elem_value = LLVMBuildZExt(ctx->builder, elem_value, llvm_elem_type, "array_elem_zext");
                else if (from_bits > to_bits)
                    elem_value = LLVMBuildTrunc(ctx->builder, elem_value, llvm_elem_type, "array_elem_trunc");
                else
                    elem_value = LLVMBuildBitCast(ctx->builder, elem_value, llvm_elem_type, "array_elem_bitcast");
            }
            else if (value_kind == LLVMPointerTypeKind && elem_kind == LLVMPointerTypeKind)
            {
                elem_value = LLVMBuildBitCast(ctx->builder, elem_value, llvm_elem_type, "array_elem_ptrcast");
            }
            else if ((value_kind == LLVMHalfTypeKind || value_kind == LLVMFloatTypeKind || value_kind == LLVMDoubleTypeKind || value_kind == LLVMFP128TypeKind) &&
                     (elem_kind == LLVMHalfTypeKind || elem_kind == LLVMFloatTypeKind || elem_kind == LLVMDoubleTypeKind || elem_kind == LLVMFP128TypeKind))
            {
                unsigned from_bits = (unsigned)LLVMSizeOfTypeInBits(ctx->data_layout, value_type);
                unsigned to_bits   = (unsigned)LLVMSizeOfTypeInBits(ctx->data_layout, llvm_elem_type);
                if (from_bits < to_bits)
                    elem_value = LLVMBuildFPExt(ctx->builder, elem_value, llvm_elem_type, "array_elem_fpext");
                else if (from_bits > to_bits)
                    elem_value = LLVMBuildFPTrunc(ctx->builder, elem_value, llvm_elem_type, "array_elem_fptrunc");
                else
                    elem_value = LLVMBuildBitCast(ctx->builder, elem_value, llvm_elem_type, "array_elem_fpcast");
            }
            else
            {
                codegen_error(ctx, elem_node, "array element has incompatible LLVM type");
                return NULL;
            }
        }

        array_value = LLVMBuildInsertValue(ctx->builder, array_value, elem_value, (unsigned)i, "array_insert");
    }

    return array_value;
}

LLVMValueRef codegen_expr_struct(CodegenContext *ctx, AstNode *expr)
{
    Type *struct_type = expr->type;
    struct_type       = type_resolve_alias(struct_type);

    if (struct_type->kind == TYPE_UNION)
    {
        LLVMTypeRef llvm_union_type = codegen_get_llvm_type(ctx, struct_type);
        if (!llvm_union_type)
        {
            codegen_error(ctx, expr, "failed to materialize union type");
            return NULL;
        }

        LLVMValueRef union_alloca = codegen_create_alloca(ctx, llvm_union_type, "union_lit");
        LLVMBuildStore(ctx->builder, LLVMConstNull(llvm_union_type), union_alloca);

        if (expr->struct_expr.fields && expr->struct_expr.fields->count > 0)
        {
            AstNode *field_init = expr->struct_expr.fields->items[0];
            if (!field_init || field_init->kind != AST_EXPR_FIELD)
            {
                codegen_error(ctx, expr, "invalid union field initializer");
                return NULL;
            }

            const char *field_name = field_init->field_expr.field;
            AstNode    *init_value = field_init->field_expr.object;

            Symbol *field_symbol = NULL;
            for (Symbol *field = struct_type->composite.fields; field; field = field->next)
            {
                if (field->name && strcmp(field->name, field_name) == 0)
                {
                    field_symbol = field;
                    break;
                }
            }

            if (!field_symbol)
            {
                codegen_error(ctx, field_init, "no field named '%s'", field_name);
                return NULL;
            }

            LLVMValueRef value = codegen_expr(ctx, init_value);
            if (!value)
            {
                codegen_error(ctx, field_init, "failed to generate union field initializer");
                return NULL;
            }
            value = codegen_load_if_needed(ctx, value, init_value->type, init_value);

            LLVMTypeRef  field_llvm_type = codegen_get_llvm_type(ctx, field_symbol->type);
            LLVMValueRef field_ptr       = LLVMBuildBitCast(ctx->builder, union_alloca, LLVMPointerType(field_llvm_type, 0), "union_field_ptr");
            LLVMBuildStore(ctx->builder, value, field_ptr);
        }

        return union_alloca;
    }

    if (struct_type->kind != TYPE_STRUCT)
    {
        codegen_error(ctx, expr, "record literal has non-record type");
        return NULL;
    }

    LLVMTypeRef llvm_struct_type = codegen_get_llvm_type(ctx, struct_type);

    // create a temporary alloca for the struct
    LLVMValueRef struct_alloca = codegen_create_alloca(ctx, llvm_struct_type, "struct_lit");

    // initialize to zero
    LLVMBuildStore(ctx->builder, LLVMConstNull(llvm_struct_type), struct_alloca);

    // process field initializers if any
    if (expr->struct_expr.fields && expr->struct_expr.fields->count > 0)
    {
        for (int i = 0; i < expr->struct_expr.fields->count; i++)
        {
            AstNode *field_init = expr->struct_expr.fields->items[i];

            // field initializers are stored as field expressions
            if (field_init->kind == AST_EXPR_FIELD)
            {
                const char *field_name = field_init->field_expr.field;
                AstNode    *init_value = field_init->field_expr.object;

                // find the field
                Symbol *field_symbol = NULL;
                int     field_index  = 0;
                for (Symbol *field = struct_type->composite.fields; field; field = field->next)
                {
                    if (field->name && strcmp(field->name, field_name) == 0)
                    {
                        field_symbol = field;
                        break;
                    }
                    field_index++;
                }

                if (!field_symbol)
                {
                    codegen_error(ctx, field_init, "no field named '%s'", field_name);
                    return NULL;
                }

                // generate the initializer value
                LLVMValueRef value = codegen_expr(ctx, init_value);
                if (!value)
                {
                    codegen_error(ctx, field_init, "failed to generate field initializer");
                    return NULL;
                }

                value = codegen_load_if_needed(ctx, value, init_value->type, init_value);

                // generate GEP for field and store
                LLVMValueRef indices[] = {LLVMConstInt(LLVMInt32TypeInContext(ctx->context), 0, false), LLVMConstInt(LLVMInt32TypeInContext(ctx->context), field_index, false)};

                LLVMValueRef field_ptr = LLVMBuildGEP2(ctx->builder, llvm_struct_type, struct_alloca, indices, 2, "field_ptr");
                LLVMBuildStore(ctx->builder, value, field_ptr);
            }
        }
    }

    return struct_alloca;
}
