#include "compiler/mir/lower.h"
#include <stdlib.h>

// lowering from ast to mir is a complex multi-step process
// this provides the interface stubs for now

MIRModule *mir_lower_module(AstNode *ast_module)
{
    if (!ast_module)
    {
        return NULL;
    }

    // TODO: implement full ast -> mir lowering
    // for now, create empty module
    MIRModule *module = mir_module_create("module");
    return module;
}

MIRFunction *mir_lower_function(AstNode *ast_function)
{
    if (!ast_function)
    {
        return NULL;
    }

    // TODO: implement function lowering
    return NULL;
}

MIRGlobal *mir_lower_global(AstNode *ast_var)
{
    if (!ast_var)
    {
        return NULL;
    }

    // TODO: implement global lowering
    return NULL;
}

int mir_parse_inline_block(MIRFunction *func, MIRBlock *current_block, const char *mir_text)
{
    if (!func || !current_block || !mir_text)
    {
        return -1;
    }

    // TODO: implement inline mir parsing
    // this will parse raw mir syntax and inject instructions into the function
    return 0;
}
