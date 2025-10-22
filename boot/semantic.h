#ifndef SEMANTIC_NEW_H
#define SEMANTIC_NEW_H

#include "ast.h"
#include "module.h"
#include "symbol.h"
#include "type.h"
#include <stdbool.h>
#include <stddef.h>

// forward declarations
typedef struct SemanticDriver      SemanticDriver;
typedef struct AnalysisContext     AnalysisContext;
typedef struct DiagnosticSink      DiagnosticSink;
typedef struct GenericBindingCtx   GenericBindingCtx;
typedef struct SpecializationKey   SpecializationKey;
typedef struct SpecializationCache SpecializationCache;
typedef struct InstantiationQueue  InstantiationQueue;

// diagnostic severity
typedef enum
{
    DIAG_ERROR,
    DIAG_WARNING,
    DIAG_NOTE
} DiagnosticLevel;

// diagnostic entry
typedef struct Diagnostic
{
    DiagnosticLevel level;
    char           *message;
    char           *file_path;
    Token          *token;
    int             line;
    int             column;
} Diagnostic;

// source cache entry for diagnostic printing
typedef struct SourceCacheEntry
{
    char                    *file_path;
    char                    *source;
    struct SourceCacheEntry *next;
} SourceCacheEntry;

// diagnostic sink: collects errors/warnings
struct DiagnosticSink
{
    Diagnostic        *entries;
    size_t             count;
    size_t             capacity;
    bool               has_errors;
    bool               has_fatal;
    SourceCacheEntry **source_cache;
    size_t             cache_size;
};

// generic binding: immutable type parameter -> concrete type mapping
typedef struct GenericBinding
{
    const char *param_name;
    Type       *concrete_type;
} GenericBinding;

// generic binding context: stack of bindings for nested generics
struct GenericBindingCtx
{
    GenericBinding *bindings;
    size_t          count;
    size_t          capacity;
};

// analysis context: immutable snapshot of scope + bindings
struct AnalysisContext
{
    Scope            *current_scope;
    Scope            *module_scope;
    Scope            *global_scope;
    GenericBindingCtx bindings;
    const char       *module_name;
    const char       *file_path;
    Symbol           *current_function;
};

// specialization key: identifies unique type argument tuple
struct SpecializationKey
{
    Symbol *generic_symbol;
    Type  **type_args;
    size_t  type_arg_count;
};

// specialization cache entry
typedef struct SpecializationEntry
{
    SpecializationKey           key;
    Symbol                     *specialized_symbol;
    struct SpecializationEntry *next;
} SpecializationEntry;

// specialization cache: hash table of instantiated generics
struct SpecializationCache
{
    SpecializationEntry **buckets;
    size_t                bucket_count;
    size_t                entry_count;
};

// instantiation request kinds
typedef enum
{
    INST_FUNCTION,
    INST_STRUCT,
    INST_UNION
} InstantiationKind;

// instantiation request: deferred generic specialization
typedef struct InstantiationRequest
{
    InstantiationKind            kind;
    Symbol                      *generic_symbol;
    Type                       **type_args;
    size_t                       type_arg_count;
    AstNode                     *call_site;
    struct InstantiationRequest *next;
} InstantiationRequest;

// instantiation queue: work list for monomorphization
struct InstantiationQueue
{
    InstantiationRequest *head;
    InstantiationRequest *tail;
    size_t                count;
};

// semantic driver: orchestrates multi-pass analysis
struct SemanticDriver
{
    ModuleManager       module_manager;
    SymbolTable         symbol_table; // for codegen compatibility
    SpecializationCache spec_cache;
    InstantiationQueue  inst_queue;
    DiagnosticSink      diagnostics;
    AstNode            *program_root;
    const char         *entry_module_name;
};

// driver lifecycle
SemanticDriver *semantic_driver_create(void);
void            semantic_driver_destroy(SemanticDriver *driver);

// main entry point
bool semantic_driver_analyze(SemanticDriver *driver, AstNode *root, const char *module_name, const char *module_path);

// diagnostics
void diagnostic_sink_init(DiagnosticSink *sink);
void diagnostic_sink_dnit(DiagnosticSink *sink);
void diagnostic_emit(DiagnosticSink *sink, DiagnosticLevel level, AstNode *node, const char *file_path, const char *fmt, ...);
void diagnostic_print_all(DiagnosticSink *sink, ModuleManager *module_manager);

// generic bindings
GenericBindingCtx generic_binding_ctx_create(void);
void              generic_binding_ctx_destroy(GenericBindingCtx *ctx);
GenericBindingCtx generic_binding_ctx_push(GenericBindingCtx *parent, const char *param_name, Type *concrete_type);
Type             *generic_binding_ctx_lookup(const GenericBindingCtx *ctx, const char *param_name);

// specialization cache
void    specialization_cache_init(SpecializationCache *cache);
void    specialization_cache_dnit(SpecializationCache *cache);
Symbol *specialization_cache_find(SpecializationCache *cache, Symbol *generic_symbol, Type **type_args, size_t type_arg_count);
void    specialization_cache_insert(SpecializationCache *cache, Symbol *generic_symbol, Type **type_args, size_t type_arg_count, Symbol *specialized);
void    specialization_cache_foreach(SpecializationCache *cache, void (*callback)(Symbol *specialized, void *user_data), void *user_data);

// instantiation queue
void                  instantiation_queue_init(InstantiationQueue *queue);
void                  instantiation_queue_dnit(InstantiationQueue *queue);
void                  instantiation_queue_push(InstantiationQueue *queue, InstantiationKind kind, Symbol *generic_symbol, Type **type_args, size_t type_arg_count, AstNode *call_site);
InstantiationRequest *instantiation_queue_pop(InstantiationQueue *queue);

// name mangling
char *mangle_generic_type(const char *module_name, const char *base_name, Type **type_args, size_t type_arg_count);
char *mangle_generic_function(const char *module_name, const char *base_name, Type **type_args, size_t type_arg_count);
char *mangle_method(const char *module_name, const char *owner_name, const char *method_name, bool receiver_is_pointer);
char *mangle_global_symbol(const char *module_name, const char *symbol_name);

// analysis context helpers
AnalysisContext analysis_context_create(Scope *global_scope, Scope *module_scope, const char *module_name, const char *module_path);
AnalysisContext analysis_context_with_scope(const AnalysisContext *parent, Scope *new_scope);
AnalysisContext analysis_context_with_bindings(const AnalysisContext *parent, GenericBindingCtx new_bindings);
AnalysisContext analysis_context_with_function(const AnalysisContext *parent, Symbol *function);

// main analysis entry point
bool semantic_analyze_new(SemanticDriver *driver, AstNode *root, const char *module_name, const char *module_path);

#endif // SEMANTIC_NEW_H
