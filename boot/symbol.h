#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"
#include <stdbool.h>
#include <stdint.h>

// forward declarations
typedef struct AstNode AstNode;
struct Symbol;
struct Scope;

typedef enum SymbolKind
{
    SYMBOL_VAR,    // variable
    SYMBOL_VAL,    // constant value
    SYMBOL_FUNC,   // function
    SYMBOL_TYPE,   // type definition
    SYMBOL_FIELD,  // struct/union field
    SYMBOL_PARAM,  // function parameter
    SYMBOL_MODULE, // module
} SymbolKind;

typedef struct GenericSpecialization GenericSpecialization;

struct GenericSpecialization
{
    size_t                 arg_count;
    Type                 **type_args;
    struct Symbol         *generic_symbol; // owning generic symbol
    struct Symbol         *symbol;
    GenericSpecialization *next;
};

typedef struct Symbol
{
    SymbolKind     kind;
    char          *name;
    Type          *type;
    AstNode       *decl;          // declaration node
    struct Scope  *home_scope;    // scope where the symbol is registered
    struct Symbol *next;          // for linked list in scope
    struct Symbol *method_next;   // linked list for type methods
    bool           is_imported;   // true if this symbol was imported from another module
    bool           is_public;     // true if symbol should be exported from module
    bool           has_const_i64; // semantic constant folding result (integer/bool)
    int64_t        const_i64;
    const char    *import_module; // source module for imported symbols
    char          *module_name;   // canonical module owning this symbol
    struct Symbol *import_origin; // original symbol when imported

    union
    {
        // SYMBOL_VAR, SYMBOL_VAL
        struct
        {
            bool  is_global;
            bool  is_const; // true for val
            char *mangled_name;
        } var;

        // SYMBOL_FUNC
        struct
        {
            bool                   is_external;
            bool                   is_defined;        // false for forward declarations
            bool                   uses_mach_varargs; // true when Mach compiler handles variadics (vs C ABI)
            char                  *extern_name;       // c-level symbol name for externs (defaults to mach name)
            char                  *convention;        // calling convention hint (e.g. "C")
            char                  *mangled_name;      // cached mangled name for codegen
            bool                   is_generic;
            size_t                 generic_param_count;
            char                 **generic_param_names;
            GenericSpecialization *generic_specializations;
            bool                   is_specialized_instance;
            bool                   is_method;
            struct Symbol         *method_owner;
            size_t                 method_forwarded_generic_count;
            bool                   method_receiver_is_pointer;
            char                  *method_receiver_name;
        } func;
        // SYMBOL_TYPE
        struct
        {
            bool                   is_alias;
            bool                   is_generic;
            size_t                 generic_param_count;
            char                 **generic_param_names;
            GenericSpecialization *generic_specializations;
            bool                   is_specialized_instance;
            struct Symbol         *methods;
        } type_def;

        // SYMBOL_FIELD
        struct
        {
            size_t offset; // offset within struct/union
        } field;

        // SYMBOL_PARAM
        struct
        {
            size_t index; // parameter index
        } param;

        // SYMBOL_MODULE
        struct
        {
            char         *path;  // module path
            struct Scope *scope; // module scope
        } module;
    };
} Symbol;

typedef struct Scope
{
    struct Scope *parent;    // parent scope
    Symbol       *symbols;   // linked list of symbols
    bool          is_module; // true for module scope
    char         *name;      // scope name (for debugging)
} Scope;

typedef struct SymbolTable
{
    Scope *current_scope;
    Scope *global_scope;
    Scope *module_scope; // current module scope
} SymbolTable;

// symbol table operations
void symbol_table_init(SymbolTable *table);
void symbol_table_dnit(SymbolTable *table);

// scope operations
Scope *scope_create(Scope *parent, const char *name);
void   scope_destroy(Scope *scope);
void   scope_enter(SymbolTable *table, Scope *scope);
void   scope_exit(SymbolTable *table);
Scope *scope_push(SymbolTable *table, const char *name);
void   scope_pop(SymbolTable *table);

// symbol operations
Symbol *symbol_create(SymbolKind kind, const char *name, Type *type, AstNode *decl);
void    symbol_destroy(Symbol *symbol);
void    symbol_add(Scope *scope, Symbol *symbol);
Symbol *symbol_lookup(SymbolTable *table, const char *name);
Symbol *symbol_lookup_scope(Scope *scope, const char *name);
Symbol *symbol_lookup_module(SymbolTable *table, const char *module, const char *name);

// field operations for structs/unions
Symbol *symbol_add_field(Symbol *composite_symbol, const char *field_name, Type *field_type, AstNode *decl);
Symbol *symbol_find_field(Symbol *composite_symbol, const char *field_name);
size_t  symbol_calculate_struct_layout(Symbol *struct_symbol);
size_t  symbol_calculate_union_layout(Symbol *union_symbol);

// method operations for types
void    type_add_method(Symbol *type_symbol, Symbol *method_symbol);
Symbol *type_find_method(Symbol *type_symbol, const char *method_name, bool receiver_is_pointer);

// module operations
Symbol *symbol_create_module(const char *name, const char *path);
void    symbol_add_module(SymbolTable *table, Symbol *module_symbol);
Symbol *symbol_find_module(SymbolTable *table, const char *name);

// debug
void symbol_print(Symbol *symbol, int indent);
void scope_print(Scope *scope, int indent);

#endif
