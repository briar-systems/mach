#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "parser.h"
#include "preprocessor.h"
#include <stdbool.h>

typedef struct Module              Module;
typedef struct ModuleManager       ModuleManager;
typedef struct SpecializationCache SpecializationCache;

// forward statement
typedef struct SymbolTable SymbolTable;

// represents a loaded module
struct Module
{
    char        *name;          // module name
    char        *file_path;     // absolute file path
    char        *object_path;   // compiled object file path
    char        *source;        // cached source content
    AstNode     *ast;           // parsed AST
    SymbolTable *symbols;       // module's symbol table
    bool         is_parsed;     // parsing complete
    bool         is_analyzed;   // semantic analysis complete
    bool         is_compiled;   // object file compilation complete
    bool         needs_linking; // true if this module should be linked
    Module      *next;          // linked list for dependencies
};

// error tracking for modules
typedef struct ModuleError
{
    char *module_path;
    char *file_path;
    char *message;
} ModuleError;

typedef struct ModuleErrorList
{
    ModuleError **errors;
    int           count;
    int           capacity;
} ModuleErrorList;

// manages module loading and dependency resolution
struct ModuleManager
{
    Module **modules;      // hash table of loaded modules
    int      capacity;     // hash table size
    int      count;        // number of loaded modules
    char   **search_paths; // directories to search for modules
    int      search_count; // number of search paths
    // simple alias map for config-less resolution: name -> base directory
    char          **alias_names;
    char          **alias_paths;
    int             alias_count;
    ModuleErrorList errors;    // accumulated errors during loading
    bool            had_error; // true if any module failed to load/parse

    // configuration for dependency resolution
    void       *config;      // ProjectConfig* (void* to avoid circular includes)
    const char *project_dir; // project directory for resolving paths

    char *target_triple; // cached target triple (from config or host)
    char *target_os;     // normalized os name (linux/windows/darwin/...) for platform suffix resolution
    char *target_arch;   // normalized arch name (x86_64/aarch64/...)

    // cached preprocessor constants
    PreprocessorConstant *cached_constants;
    size_t                cached_constants_count;
};

// module manager lifecycle
void module_manager_init(ModuleManager *manager);
void module_manager_dnit(ModuleManager *manager);

// search path management
void   module_manager_add_search_path(ModuleManager *manager, const char *path);
void   module_manager_add_alias(ModuleManager *manager, const char *name, const char *base_dir);
void   module_manager_set_config(ModuleManager *manager, void *config, const char *project_dir);
size_t module_manager_collect_constants(ModuleManager *manager, PreprocessorConstant *out, size_t max_count);

// module loading
Module *module_manager_load_module(ModuleManager *manager, const char *module_path);
Module *module_manager_find_module(ModuleManager *manager, const char *name);
Module *module_manager_find_by_file_path(ModuleManager *manager, const char *file_path);

// dependency compilation and linking
bool module_manager_compile_dependencies(ModuleManager *manager, const char *output_dir, int opt_level, bool no_pie, bool debug_info, SpecializationCache *spec_cache);
bool module_manager_get_link_objects(ModuleManager *manager, char ***object_files, int *count);

// utility helpers
char *module_make_object_path(const char *output_dir, const char *module_name);

// error handling
void module_error_list_init(ModuleErrorList *list);
void module_error_list_dnit(ModuleErrorList *list);
void module_error_list_add(ModuleErrorList *list, const char *module_path, const char *file_path, const char *message);
void module_error_list_print(ModuleErrorList *list);

// module operations
void module_init(Module *module, const char *name, const char *file_path);
void module_dnit(Module *module);

// utility functions
char *module_path_to_file_path(ModuleManager *manager, const char *module_path);
bool  module_has_circular_dependency(ModuleManager *manager, Module *module, const char *target);

#endif
