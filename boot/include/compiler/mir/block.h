#ifndef MIR_BLOCK_H
#define MIR_BLOCK_H

#include "compiler/mir/inst.h"
#include <stddef.h>
#include <stdint.h>

// mir basic block
typedef struct MIRBlock
{
    uint32_t    id;          // unique id within function
    char       *label;       // optional label name
    MIRInst    *first_inst;  // first instruction
    MIRInst    *last_inst;   // last instruction (for O(1) append)
    size_t      inst_count;
    struct MIRBlock *next;   // linked list within function
} MIRBlock;

// block management
MIRBlock *mir_block_create(uint32_t id, const char *label);
void      mir_block_destroy(MIRBlock *block);
void      mir_block_append_inst(MIRBlock *block, MIRInst *inst);
MIRInst  *mir_block_get_terminator(MIRBlock *block);
bool      mir_block_is_terminated(MIRBlock *block);

#endif // MIR_BLOCK_H
