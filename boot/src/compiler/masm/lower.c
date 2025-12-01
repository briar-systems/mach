#include "compiler/masm/lower.h"
#include <stdio.h>

Masm *masm_lower_module(AstNode *ast, SymbolTable *symbols)
{
    (void)ast;
    (void)symbols;

    // TODO: implement full lowering
    // for now, just create an empty masm module
    MasmTarget target = masm_target_native();
    Masm *masm = masm_create(target);
    
    // create a dummy text section
    MasmSection *text = masm_get_or_create_section(masm, ".text", MASM_SECTION_TEXT);
    (void)text;
    
    // add a dummy instruction (ret)
    // assuming generic RET opcode is available
    // masm_section_append_inst(text, masm_inst_0(MASM_OP_RET));
    
    return masm;
}
