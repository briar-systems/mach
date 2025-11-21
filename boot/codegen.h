#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "lexer.h"
#include "semantic.h"
#include <stdbool.h>

typedef struct CodegenContext CodegenContext;
typedef struct CodegenError   CodegenError;

struct CodegenError
{
    char         *message;
    AstNode      *node;
    CodegenError *next;
};

// minimal stub structure - to be replaced with MIR implementation
struct CodegenContext
{
    // error tracking
    CodegenError *errors;
    bool          has_errors;

    // source context for diagnostics
    const char *source_file;  // current file being compiled
    Lexer      *source_lexer; // lexer for mapping pos->line/column

    // options
    int  opt_level;
    bool debug_info;
    bool no_pie;

    // compile-time intrinsics state
    unsigned long iota_counter; // for $iota() - incrementing compile-time counter
};

// context lifecycle
void codegen_context_init(CodegenContext *ctx, const char *module_name, bool no_pie);
void codegen_context_dnit(CodegenContext *ctx);

// main entry point (stubbed - will be replaced with MIR implementation)
bool codegen_generate(CodegenContext *ctx, AstNode *root, SymbolTable *symbols);

// output generation (stubbed)
bool codegen_emit_object(CodegenContext *ctx, const char *filename);

// error handling
void codegen_error(CodegenContext *ctx, AstNode *node, const char *fmt, ...);
void codegen_print_errors(CodegenContext *ctx);

#endif
