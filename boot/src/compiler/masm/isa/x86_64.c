#include "compiler/masm/isa/x86_64.h"
#include <stdio.h>

MasmOperand masm_x86_reg(MasmX86Reg reg, uint8_t size)
{
    return masm_operand_register((uint32_t)reg, size);
}

int masm_x86_encode(MasmInstruction inst, uint8_t *buffer, size_t size)
{
    (void)inst;
    (void)buffer;
    (void)size;
    // TODO: implement x86_64 encoding
    return 0;
}
