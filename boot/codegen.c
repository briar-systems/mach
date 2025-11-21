// codegen.c - stub implementation
// this will be replaced with MIR-based code generation

#include "codegen.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void codegen_context_init(CodegenContext *ctx, const char *module_name, bool no_pie)
{
    (void)module_name; // unused in stub

    memset(ctx, 0, sizeof(CodegenContext));
    ctx->no_pie      = no_pie;
    ctx->opt_level   = 0;
    ctx->debug_info  = false;
    ctx->has_errors  = false;
    ctx->errors      = NULL;
    ctx->source_file = NULL;
}

void codegen_context_dnit(CodegenContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    // free error list
    CodegenError *err = ctx->errors;
    while (err)
    {
        CodegenError *next = err->next;
        free(err->message);
        free(err);
        err = next;
    }

    memset(ctx, 0, sizeof(CodegenContext));
}

bool codegen_generate(CodegenContext *ctx, AstNode *root, SymbolTable *symbols)
{
    (void)root;
    (void)symbols;

    // stub: just report that codegen is not implemented yet
    fprintf(stderr, "codegen: MIR code generation not yet implemented\n");
    fprintf(stderr, "codegen: bootstrap compiler can parse but not generate code\n");

    ctx->has_errors = true;
    return false;
}

bool codegen_emit_object(CodegenContext *ctx, const char *filename)
{
    (void)ctx;
    (void)filename;

    fprintf(stderr, "codegen: object file emission not yet implemented\n");
    return false;
}

void codegen_error(CodegenContext *ctx, AstNode *node, const char *fmt, ...)
{
    CodegenError *err = malloc(sizeof(CodegenError));
    if (!err)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);

    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    err->message = strdup(buffer);
    err->node    = node;
    err->next    = ctx->errors;
    ctx->errors  = err;

    ctx->has_errors = true;
}

void codegen_print_errors(CodegenContext *ctx)
{
    if (!ctx || !ctx->has_errors)
    {
        return;
    }

    CodegenError *err = ctx->errors;
    while (err)
    {
        fprintf(stderr, "codegen error: %s\n", err->message);
        err = err->next;
    }
}
