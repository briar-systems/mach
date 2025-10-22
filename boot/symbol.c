#include "symbol.h"
#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void symbol_table_init(SymbolTable *table)
{
    table->global_scope  = scope_create(NULL, "global");
    table->current_scope = table->global_scope;
    table->module_scope  = NULL;
}

void symbol_table_dnit(SymbolTable *table)
{
    scope_destroy(table->global_scope);
    table->global_scope  = NULL;
    table->current_scope = NULL;
    table->module_scope  = NULL;
}

Scope *scope_create(Scope *parent, const char *name)
{
    Scope *scope     = malloc(sizeof(Scope));
    scope->parent    = parent;
    scope->symbols   = NULL;
    scope->is_module = false;
    scope->name      = name ? strdup(name) : NULL;
    return scope;
}

void scope_destroy(Scope *scope)
{
    if (!scope)
        return;

    // destroy all symbols
    Symbol *symbol = scope->symbols;
    while (symbol)
    {
        Symbol *next = symbol->next;
        symbol_destroy(symbol);
        symbol = next;
    }

    free(scope->name);
    free(scope);
}

void scope_enter(SymbolTable *table, Scope *scope)
{
    table->current_scope = scope;
}

void scope_exit(SymbolTable *table)
{
    if (table->current_scope && table->current_scope->parent)
    {
        table->current_scope = table->current_scope->parent;
    }
}

Scope *scope_push(SymbolTable *table, const char *name)
{
    Scope *new_scope     = scope_create(table->current_scope, name);
    table->current_scope = new_scope;
    return new_scope;
}

void scope_pop(SymbolTable *table)
{
    if (table->current_scope && table->current_scope->parent)
    {
        Scope *old_scope     = table->current_scope;
        table->current_scope = old_scope->parent;
        scope_destroy(old_scope);
    }
}

Symbol *symbol_create(SymbolKind kind, const char *name, Type *type, AstNode *decl)
{
    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol)
        return NULL;

    memset(symbol, 0, sizeof(Symbol));

    symbol->kind          = kind;
    symbol->name          = name ? strdup(name) : NULL;
    symbol->type          = type;
    symbol->decl          = decl;
    symbol->home_scope    = NULL;
    symbol->next          = NULL;
    symbol->method_next   = NULL;
    symbol->is_imported   = false;
    symbol->is_public     = false;
    symbol->has_const_i64 = false;
    symbol->const_i64     = 0;
    symbol->import_module = NULL;
    symbol->module_name   = NULL;
    symbol->import_origin = NULL;

    // initialize kind-specific data
    switch (kind)
    {
    case SYMBOL_VAR:
    case SYMBOL_VAL:
        symbol->var.is_global    = false;
        symbol->var.is_const     = (kind == SYMBOL_VAL);
        symbol->var.mangled_name = NULL;
        break;

    case SYMBOL_FUNC:
        symbol->func.is_external                    = false;
        symbol->func.is_defined                     = false;
        symbol->func.uses_mach_varargs              = false;
        symbol->func.extern_name                    = NULL;
        symbol->func.convention                     = NULL;
        symbol->func.mangled_name                   = NULL;
        symbol->func.is_generic                     = false;
        symbol->func.generic_param_count            = 0;
        symbol->func.generic_param_names            = NULL;
        symbol->func.generic_specializations        = NULL;
        symbol->func.is_specialized_instance        = false;
        symbol->func.is_method                      = false;
        symbol->func.method_owner                   = NULL;
        symbol->func.method_forwarded_generic_count = 0;
        symbol->func.method_receiver_is_pointer     = false;
        symbol->func.method_receiver_name           = NULL;
        break;

    case SYMBOL_TYPE:
        symbol->type_def.is_alias = false;
        symbol->type_def.methods  = NULL;
        break;

    case SYMBOL_FIELD:
        symbol->field.offset = 0;
        break;

    case SYMBOL_PARAM:
        symbol->param.index = 0;
        break;

    case SYMBOL_MODULE:
        symbol->module.path  = NULL;
        symbol->module.scope = NULL;
        break;
    }

    return symbol;
}

void symbol_destroy(Symbol *symbol)
{
    if (!symbol)
        return;

    free(symbol->name);
    free(symbol->module_name);
    symbol->module_name = NULL;

    if (symbol->kind == SYMBOL_MODULE)
    {
        if (symbol->module.scope && symbol->is_imported)
        {
            scope_destroy(symbol->module.scope);
            symbol->module.scope = NULL;
        }
        free(symbol->module.path);
        symbol->module.path = NULL;
        // non-imported module scopes are managed separately
    }
    else if (symbol->kind == SYMBOL_FUNC)
    {
        free(symbol->func.extern_name);
        free(symbol->func.convention);
        free(symbol->func.mangled_name);
        symbol->func.extern_name  = NULL;
        symbol->func.convention   = NULL;
        symbol->func.mangled_name = NULL;
        free(symbol->func.method_receiver_name);
        symbol->func.method_receiver_name = NULL;

        if (symbol->func.generic_param_names)
        {
            for (size_t i = 0; i < symbol->func.generic_param_count; i++)
            {
                free(symbol->func.generic_param_names[i]);
            }
            free(symbol->func.generic_param_names);
            symbol->func.generic_param_names = NULL;
        }
        GenericSpecialization *spec = symbol->func.generic_specializations;
        if (!symbol->import_origin || symbol->import_origin == symbol)
        {
            while (spec)
            {
                GenericSpecialization *next = spec->next;
                free(spec->type_args);
                free(spec);
                spec = next;
            }
        }
        symbol->func.generic_specializations = NULL;
    }
    else if (symbol->kind == SYMBOL_VAR || symbol->kind == SYMBOL_VAL)
    {
        free(symbol->var.mangled_name);
        symbol->var.mangled_name = NULL;
    }
    else if (symbol->kind == SYMBOL_TYPE)
    {
        if (symbol->type_def.generic_param_names)
        {
            for (size_t i = 0; i < symbol->type_def.generic_param_count; i++)
            {
                free(symbol->type_def.generic_param_names[i]);
            }
            free(symbol->type_def.generic_param_names);
            symbol->type_def.generic_param_names = NULL;
        }
        if (!symbol->is_imported && symbol->type_def.methods)
        {
            Symbol *method = symbol->type_def.methods;
            while (method)
            {
                Symbol *next_method = method->method_next;
                method->method_next = NULL;
                symbol_destroy(method);
                method = next_method;
            }
        }
        symbol->type_def.methods = NULL;
    }

    free(symbol);
}

void symbol_add(Scope *scope, Symbol *symbol)
{
    if (!scope || !symbol)
        return;

    if (!symbol->home_scope)
        symbol->home_scope = scope;

    // add to front of list
    symbol->next   = scope->symbols;
    scope->symbols = symbol;
}

Symbol *symbol_lookup(SymbolTable *table, const char *name)
{
    if (!table || !name)
        return NULL;

    // search from current scope up to global
    for (Scope *scope = table->current_scope; scope; scope = scope->parent)
    {
        Symbol *symbol = symbol_lookup_scope(scope, name);
        if (symbol)
            return symbol;
    }

    return NULL;
}

Symbol *symbol_lookup_scope(Scope *scope, const char *name)
{
    if (!scope || !name)
        return NULL;

    for (Symbol *symbol = scope->symbols; symbol; symbol = symbol->next)
    {
        if (symbol->name && strcmp(symbol->name, name) == 0)
        {
            return symbol;
        }
    }

    return NULL;
}

Symbol *symbol_lookup_module(SymbolTable *table, const char *module, const char *name)
{
    if (!table || !module || !name)
        return NULL;

    // find module symbol first
    Symbol *module_symbol = symbol_lookup(table, module);
    if (!module_symbol || module_symbol->kind != SYMBOL_MODULE)
    {
        return NULL;
    }

    // search in module scope
    Symbol *symbol = symbol_lookup_scope(module_symbol->module.scope, name);
    if (symbol && !symbol->is_public)
    {
        return NULL;
    }
    return symbol;
}

Symbol *symbol_add_field(Symbol *composite_symbol, const char *field_name, Type *field_type, AstNode *decl)
{
    if (!composite_symbol || !field_name || !field_type)
        return NULL;
    if (composite_symbol->type->kind != TYPE_STRUCT && composite_symbol->type->kind != TYPE_UNION)
    {
        return NULL;
    }

    Symbol *field_symbol = symbol_create(SYMBOL_FIELD, field_name, field_type, decl);

    // add to composite type's field list
    if (!composite_symbol->type->composite.fields)
    {
        composite_symbol->type->composite.fields = field_symbol;
    }
    else
    {
        // find last field and append
        Symbol *last = composite_symbol->type->composite.fields;
        while (last->next)
            last = last->next;
        last->next = field_symbol;
    }

    composite_symbol->type->composite.field_count++;

    return field_symbol;
}

Symbol *symbol_find_field(Symbol *composite_symbol, const char *field_name)
{
    if (!composite_symbol || !field_name)
        return NULL;
    if (composite_symbol->type->kind != TYPE_STRUCT && composite_symbol->type->kind != TYPE_UNION)
    {
        return NULL;
    }

    for (Symbol *field = composite_symbol->type->composite.fields; field; field = field->next)
    {
        if (field->name && strcmp(field->name, field_name) == 0)
        {
            return field;
        }
    }

    return NULL;
}

void type_add_method(Symbol *type_symbol, Symbol *method_symbol)
{
    if (!type_symbol || !method_symbol)
        return;
    if (type_symbol->kind != SYMBOL_TYPE || method_symbol->kind != SYMBOL_FUNC)
        return;

    method_symbol->method_next       = type_symbol->type_def.methods;
    type_symbol->type_def.methods    = method_symbol;
    method_symbol->func.is_method    = true;
    method_symbol->func.method_owner = type_symbol;

    if (!method_symbol->home_scope && type_symbol->home_scope)
    {
        method_symbol->home_scope = type_symbol->home_scope;
    }
}

Symbol *type_find_method(Symbol *type_symbol, const char *method_name, bool receiver_is_pointer)
{
    if (!type_symbol || type_symbol->kind != SYMBOL_TYPE || !method_name)
        return NULL;

    for (Symbol *method = type_symbol->type_def.methods; method; method = method->method_next)
    {
        if (!method->name || strcmp(method->name, method_name) != 0)
            continue;
        if (method->func.method_receiver_is_pointer != receiver_is_pointer)
            continue;
        return method;
    }

    return NULL;
}

static size_t align_offset(size_t offset, size_t alignment)
{
    return (offset + alignment - 1) & ~(alignment - 1);
}

size_t symbol_calculate_struct_layout(Symbol *struct_symbol)
{
    if (!struct_symbol || struct_symbol->type->kind != TYPE_STRUCT)
        return 0;

    size_t offset        = 0;
    size_t max_alignment = 1;

    for (Symbol *field = struct_symbol->type->composite.fields; field; field = field->next)
    {
        size_t field_alignment = type_alignof(field->type);
        offset                 = align_offset(offset, field_alignment);

        field->field.offset = offset;
        offset += type_sizeof(field->type);

        if (field_alignment > max_alignment)
        {
            max_alignment = field_alignment;
        }
    }

    // align struct size to largest member alignment
    offset = align_offset(offset, max_alignment);

    struct_symbol->type->size      = offset;
    struct_symbol->type->alignment = max_alignment;

    return offset;
}

size_t symbol_calculate_union_layout(Symbol *union_symbol)
{
    if (!union_symbol || union_symbol->type->kind != TYPE_UNION)
        return 0;

    size_t max_size      = 0;
    size_t max_alignment = 1;

    for (Symbol *field = union_symbol->type->composite.fields; field; field = field->next)
    {
        field->field.offset = 0; // all fields start at offset 0 in union

        size_t field_size      = type_sizeof(field->type);
        size_t field_alignment = type_alignof(field->type);

        if (field_size > max_size)
        {
            max_size = field_size;
        }
        if (field_alignment > max_alignment)
        {
            max_alignment = field_alignment;
        }
    }

    // align union size to largest member alignment
    max_size = align_offset(max_size, max_alignment);

    union_symbol->type->size      = max_size;
    union_symbol->type->alignment = max_alignment;

    return max_size;
}

Symbol *symbol_create_module(const char *name, const char *path)
{
    Symbol *module_symbol                  = symbol_create(SYMBOL_MODULE, name, NULL, NULL);
    module_symbol->module.path             = path ? strdup(path) : NULL;
    module_symbol->module.scope            = scope_create(NULL, name);
    module_symbol->module.scope->is_module = true;
    return module_symbol;
}

void symbol_add_module(SymbolTable *table, Symbol *module_symbol)
{
    if (!table || !module_symbol || module_symbol->kind != SYMBOL_MODULE)
        return;

    symbol_add(table->global_scope, module_symbol);
}

Symbol *symbol_find_module(SymbolTable *table, const char *name)
{
    if (!table || !name)
        return NULL;

    Symbol *symbol = symbol_lookup_scope(table->global_scope, name);
    if (symbol && symbol->kind == SYMBOL_MODULE)
    {
        return symbol;
    }

    return NULL;
}

void symbol_print(Symbol *symbol, int indent)
{
    if (!symbol)
        return;

    for (int i = 0; i < indent; i++)
        printf("  ");

    const char *kind_name;
    switch (symbol->kind)
    {
    case SYMBOL_VAR:
        kind_name = "var";
        break;
    case SYMBOL_VAL:
        kind_name = "val";
        break;
    case SYMBOL_FUNC:
        kind_name = "fun";
        break;
    case SYMBOL_TYPE:
        kind_name = "type";
        break;
    case SYMBOL_FIELD:
        kind_name = "field";
        break;
    case SYMBOL_PARAM:
        kind_name = "param";
        break;
    case SYMBOL_MODULE:
        kind_name = "module";
        break;
    default:
        kind_name = "unknown";
        break;
    }

    printf("%s %s: ", kind_name, symbol->name ? symbol->name : "(anonymous)");
    type_print(symbol->type);
    printf("\n");
}

void scope_print(Scope *scope, int indent)
{
    if (!scope)
        return;

    for (int i = 0; i < indent; i++)
        printf("  ");
    printf("scope %s {\n", scope->name ? scope->name : "(anonymous)");

    for (Symbol *symbol = scope->symbols; symbol; symbol = symbol->next)
    {
        symbol_print(symbol, indent + 1);
    }

    for (int i = 0; i < indent; i++)
        printf("  ");
    printf("}\n");
}
