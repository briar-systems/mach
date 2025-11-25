#include "compiler/mir/block.h"
#include <stdlib.h>
#include <string.h>

MIRBlock *mir_block_create(uint32_t id, const char *label)
{
    MIRBlock *block = malloc(sizeof(MIRBlock));
    if (!block)
    {
        return NULL;
    }

    block->id = id;
    block->label = label ? strdup(label) : NULL;
    block->first_inst = NULL;
    block->last_inst = NULL;
    block->inst_count = 0;
    block->next = NULL;

    return block;
}

void mir_block_destroy(MIRBlock *block)
{
    if (!block)
    {
        return;
    }

    // free all instructions
    MIRInst *inst = block->first_inst;
    while (inst)
    {
        MIRInst *next = inst->next;
        mir_inst_destroy(inst);
        inst = next;
    }

    if (block->label)
    {
        free(block->label);
    }

    free(block);
}

void mir_block_append_inst(MIRBlock *block, MIRInst *inst)
{
    if (!block || !inst)
    {
        return;
    }

    inst->next = NULL;

    if (!block->first_inst)
    {
        block->first_inst = inst;
        block->last_inst = inst;
    }
    else
    {
        block->last_inst->next = inst;
        block->last_inst = inst;
    }

    block->inst_count++;
}

MIRInst *mir_block_get_terminator(MIRBlock *block)
{
    if (!block || !block->last_inst)
    {
        return NULL;
    }

    if (mir_op_is_terminator(block->last_inst->op))
    {
        return block->last_inst;
    }

    return NULL;
}

bool mir_block_is_terminated(MIRBlock *block)
{
    return mir_block_get_terminator(block) != NULL;
}
