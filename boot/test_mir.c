// mir system test - demonstrates building a simple mir module and emitting it

#include "compiler/mir/module.h"
#include "compiler/mir/function.h"
#include "compiler/mir/block.h"
#include "compiler/mir/inst.h"
#include "compiler/mir/global.h"
#include "compiler/mir/emit.h"
#include "compiler/mir/target.h"
#include "compiler/type.h"
#include <stdio.h>

// simple test: create a function that returns 42
static int test_simple_function()
{
    printf("test: simple function\n");

    // create module
    MIRModule *module = mir_module_create("test");
    if (!module)
    {
        printf("  failed to create module\n");
        return -1;
    }

    // create i64 type
    Type *i64_type = type_create_int(false, 64);

    // create function: fun answer() i64
    MIRFunction *func = mir_function_create("answer", i64_type, true);
    if (!func)
    {
        printf("  failed to create function\n");
        mir_module_destroy(module);
        return -1;
    }

    // add entry block
    MIRBlock *entry = mir_function_add_block(func, "entry");
    if (!entry)
    {
        printf("  failed to create block\n");
        mir_function_destroy(func);
        mir_module_destroy(module);
        return -1;
    }

    // allocate result value
    MIRValue *result = mir_function_alloc_value(func, i64_type, "result");
    if (!result)
    {
        printf("  failed to allocate value\n");
        mir_function_destroy(func);
        mir_module_destroy(module);
        return -1;
    }

    // create const instruction: result = const 42
    MIRInst *const_inst = mir_inst_const(i64_type, 42);
    if (!const_inst)
    {
        printf("  failed to create const instruction\n");
        mir_function_destroy(func);
        mir_module_destroy(module);
        return -1;
    }
    mir_inst_set_result(const_inst, result);
    mir_block_append_inst(entry, const_inst);

    // create return instruction: ret result
    MIRInst *ret_inst = mir_inst_ret(i64_type, mir_operand_value(result->id));
    if (!ret_inst)
    {
        printf("  failed to create ret instruction\n");
        mir_function_destroy(func);
        mir_module_destroy(module);
        return -1;
    }
    mir_block_append_inst(entry, ret_inst);

    // add function to module
    mir_module_add_function(module, func);

    // emit to object file
    MIRTarget target = mir_target_native();
    int emit_result = mir_emit_object(module, &target, "test.o");
    if (emit_result < 0)
    {
        printf("  failed to emit object file\n");
        mir_module_destroy(module);
        return -1;
    }

    printf("  success! created test.o\n");

    // cleanup
    mir_module_destroy(module);
    type_destroy(i64_type);

    return 0;
}

int main()
{
    printf("mir system tests\n");
    printf("================\n\n");

    if (test_simple_function() < 0)
    {
        printf("\ntests failed\n");
        return 1;
    }

    printf("\nall tests passed\n");
    return 0;
}
