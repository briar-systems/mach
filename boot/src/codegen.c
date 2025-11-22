// codegen.c - MIR-based code generation

#include "codegen.h"
#include "backend/backend.h"
#include "mir.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void codegen_context_init(CodegenContext *ctx, const char *module_name, bool no_pie)
{
    memset(ctx, 0, sizeof(CodegenContext));
    ctx->no_pie      = no_pie;
    ctx->opt_level   = 0;
    ctx->debug_info  = false;
    ctx->has_errors  = false;
    ctx->errors      = NULL;
    ctx->source_file = NULL;
    
    ctx->module = mir_module_create(module_name);
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
    
    if (ctx->module) {
        mir_module_destroy(ctx->module);
    }

    memset(ctx, 0, sizeof(CodegenContext));
}

bool codegen_generate(CodegenContext *ctx, AstNode *root, SymbolTable *symbols)
{
    (void)symbols;
    if (!root || root->kind != AST_PROGRAM) return false;
    
    AstList *stmts = root->program.stmts;
    if (!stmts) return true;
    
    for (int i = 0; i < stmts->count; i++) {
        AstNode *stmt = stmts->items[i];
        if (stmt->kind == AST_STMT_MIR) {
            // Append blocks to module
            MirBasicBlock *blocks = stmt->mir.blocks;
            if (blocks) {
                if (ctx->module->last_block) {
                    ctx->module->last_block->next = blocks;
                } else {
                    ctx->module->blocks = blocks;
                }
                
                // Update last_block
                MirBasicBlock *curr = blocks;
                while (curr->next) curr = curr->next;
                ctx->module->last_block = curr;
                
                // Detach from AST node to avoid double free
                stmt->mir.blocks = NULL; 
            }
        }
    }
    
    return !ctx->has_errors;
}

bool codegen_emit_object(CodegenContext *ctx, const char *filename)
{
    TargetDescriptor target = target_desc(TARGET_ARCH_KIND_X86_64, TARGET_OS_KIND_LINUX);
    return backend_emit_executable(target, ctx->module, filename);
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
