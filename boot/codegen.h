#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "lexer.h"
#include "semantic.h"
#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <stdbool.h>

typedef struct CodegenContext CodegenContext;
typedef struct CodegenError   CodegenError;

struct CodegenError
{
    char         *message;
    AstNode      *node;
    CodegenError *next;
};

struct CodegenContext
{
    // llvm core
    LLVMContextRef       context;
    LLVMModuleRef        module;
    LLVMBuilderRef       builder;
    LLVMTargetMachineRef target_machine;
    LLVMTargetDataRef    data_layout;
    LLVMDIBuilderRef     di_builder;
    LLVMMetadataRef      di_compile_unit;
    LLVMMetadataRef      di_file;
    LLVMMetadataRef      current_di_scope;
    LLVMMetadataRef      current_di_subprogram;
    LLVMMetadataRef      di_unknown_type;

    // symbol mapping
    struct
    {
        Symbol      **symbols;
        LLVMValueRef *values;
        int           count;
        int           capacity;
    } symbol_map;

    // type cache
    struct
    {
        Type       **types;
        LLVMTypeRef *llvm_types;
        int          count;
        int          capacity;
    } type_cache;

    // current function context
    LLVMValueRef      current_function;
    Type             *current_function_type; // mach type for current function
    LLVMBasicBlockRef break_block;
    LLVMBasicBlockRef continue_block;

    // initialization context
    bool generating_mutable_init; // true when generating initializer for var (not val)

    // module-level assembly aggregation
    char  *module_inline_asm;
    size_t module_inline_asm_len;

    // error tracking
    CodegenError *errors;
    bool          has_errors;

    // options
    int   opt_level;
    bool  debug_info;
    bool  debug_finalized;
    bool  no_pie; // disable position independent executable
    char *debug_full_path;
    char *debug_dir;
    char *debug_file;

    // source context for diagnostics
    const char *source_file;  // current file being compiled
    Lexer      *source_lexer; // lexer for mapping pos->line/column

    // variadic function support (Mach ABI)
    LLVMValueRef current_vararg_count_value; // u64 count parameter passed to current function (null if none)
    LLVMValueRef current_vararg_array;       // i8** pointing to packed variadic argument slots
    size_t       current_fixed_param_count;  // number of fixed parameters in current function

    // specialization cache for cross-module generic instantiation
    struct SpecializationCache *spec_cache;
};

// context lifecycle
void codegen_context_init(CodegenContext *ctx, const char *module_name, bool no_pie);
void codegen_context_dnit(CodegenContext *ctx);

// main entry point
bool codegen_generate(CodegenContext *ctx, AstNode *root, SymbolTable *symbols);

// output generation
bool codegen_emit_object(CodegenContext *ctx, const char *filename);
bool codegen_emit_llvm_ir(CodegenContext *ctx, const char *filename);
bool codegen_emit_assembly(CodegenContext *ctx, const char *filename);

// error handling
void codegen_error(CodegenContext *ctx, AstNode *node, const char *fmt, ...);
void codegen_print_errors(CodegenContext *ctx);

// type conversion
LLVMTypeRef codegen_get_llvm_type(CodegenContext *ctx, Type *type);

// value lookup
LLVMValueRef codegen_get_symbol_value(CodegenContext *ctx, Symbol *symbol);
void         codegen_set_symbol_value(CodegenContext *ctx, Symbol *symbol, LLVMValueRef value);

// statement generation
LLVMValueRef codegen_stmt(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_use(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_ext(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_block(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_var(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_fun(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_ret(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_if(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_for(CodegenContext *ctx, AstNode *stmt);
LLVMValueRef codegen_stmt_expr(CodegenContext *ctx, AstNode *stmt);

// expression generation
LLVMValueRef codegen_expr(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_lit(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_null(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_ident(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_binary(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_unary(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_call(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_cast(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_field(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_index(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_array(CodegenContext *ctx, AstNode *expr);
LLVMValueRef codegen_expr_struct(CodegenContext *ctx, AstNode *expr);

// utility functions
LLVMValueRef codegen_create_alloca(CodegenContext *ctx, LLVMTypeRef type, const char *name);
LLVMValueRef codegen_load_if_needed(CodegenContext *ctx, LLVMValueRef value, Type *type, AstNode *source_expr);
bool         codegen_is_lvalue(AstNode *expr);

#endif
