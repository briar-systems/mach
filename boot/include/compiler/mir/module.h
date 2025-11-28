#ifndef MIR_MODULE_H
#define MIR_MODULE_H

#include "compiler/mir/function.h"
#include "compiler/mir/global.h"
#include <stddef.h>

// mir module (compilation unit)
typedef struct MIRModule
{
    char        *name;      // module name
    MIRFunction *functions; // function list
    MIRGlobal   *globals;   // global data list
    size_t       function_count;
    size_t       global_count;
} MIRModule;

// module management
MIRModule *mir_module_create(const char *name);
void       mir_module_destroy(MIRModule *module);

// add declarations
void mir_module_add_function(MIRModule *module, MIRFunction *func);
void mir_module_add_global(MIRModule *module, MIRGlobal *global);

// merge another module's functions and globals into this module
void mir_module_merge(MIRModule *dst, MIRModule *src);

// lookup
MIRFunction *mir_module_get_function(MIRModule *module, const char *name);
MIRGlobal   *mir_module_get_global(MIRModule *module, const char *name);

#endif // MIR_MODULE_H
