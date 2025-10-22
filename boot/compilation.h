#ifndef COMPILATION_H
#define COMPILATION_H

#include "ast.h"
#include "codegen.h"
#include "config.h"
#include "module.h"
#include "semantic.h"
#include <stdbool.h>

typedef struct
{
    char **items;
    int    count;
    int    cap;
} StringVec;

typedef struct
{
    char **names;
    char **dirs;
    int    count;
    int    cap;
} AliasVec;

typedef struct
{
    const char *input_file;
    const char *output_file;
    int         opt_level;
    int         link_exe;
    int         no_pie;
    int         debug_info;
    int         emit_ast;
    int         emit_ir;
    int         emit_asm;
    const char *emit_ast_path;
    const char *emit_ir_path;
    const char *emit_asm_path;
    StringVec   include_paths;
    StringVec   link_objects;
    AliasVec    aliases;
} BuildOptions;

typedef struct
{
    BuildOptions   *options;
    SemanticDriver *driver;
    ProjectConfig  *config;
    char           *project_root;
    char           *source;
    AstNode        *ast;
    Parser          parser;
    Lexer           lexer;
    CodegenContext  codegen;
    char           *module_name;
    char          **dep_objects;
    int             dep_count;
    bool            had_error;
    bool            lexer_initialized;
    bool            parser_initialized;
    bool            codegen_initialized;
} CompilationContext;

// build options
void build_options_init(BuildOptions *opts);
void build_options_dnit(BuildOptions *opts);
void build_options_add_include(BuildOptions *opts, const char *path);
void build_options_add_link_object(BuildOptions *opts, const char *obj);
void build_options_add_alias(BuildOptions *opts, const char *name, const char *dir);

// compilation context lifecycle
bool compilation_context_init(CompilationContext *ctx, BuildOptions *opts);
void compilation_context_dnit(CompilationContext *ctx);

// compilation pipeline
bool compilation_load_and_preprocess(CompilationContext *ctx);
bool compilation_parse(CompilationContext *ctx);
bool compilation_analyze(CompilationContext *ctx);
bool compilation_codegen(CompilationContext *ctx);
bool compilation_emit_artifacts(CompilationContext *ctx);
bool compilation_compile_dependencies(CompilationContext *ctx);
bool compilation_link(CompilationContext *ctx);

// high-level orchestration
bool compilation_run(CompilationContext *ctx);

// helper: derive module name from file path and aliases
char *derive_module_name(const char *filename, const AliasVec *aliases);

#endif
