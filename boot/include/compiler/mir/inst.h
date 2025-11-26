#ifndef MIR_INST_H
#define MIR_INST_H

#include "compiler/mir/opcode.h"
#include "compiler/mir/operand.h"
#include "compiler/mir/value.h"
#include "compiler/type.h"
#include <stddef.h>

// mir instruction (ssa instruction)
typedef struct MIRInst
{
    MIROp       op;
    Type       *type;              // operation type
    MIRValue   *result;            // result value (null for terminators without return)
    MIROperand *operands;          // operand array
    size_t      operand_count;
    size_t      operand_capacity;
    struct MIRInst *next;          // linked list within block
} MIRInst;

// instruction management
MIRInst *mir_inst_create(MIROp op, Type *type);
void     mir_inst_destroy(MIRInst *inst);
void     mir_inst_add_operand(MIRInst *inst, MIROperand operand);
void     mir_inst_set_result(MIRInst *inst, MIRValue *result);

// instruction builders for common patterns
MIRInst *mir_inst_const(Type *type, int64_t value);
MIRInst *mir_inst_const_float(Type *type, double value);
MIRInst *mir_inst_binary(MIROp op, Type *type, MIROperand left, MIROperand right);
MIRInst *mir_inst_unary(MIROp op, Type *type, MIROperand operand);
MIRInst *mir_inst_load(Type *type, MIROperand ptr);
MIRInst *mir_inst_store(Type *type, MIROperand ptr, MIROperand value);
MIRInst *mir_inst_gep(Type *type, MIROperand ptr, MIROperand offset);
MIRInst *mir_inst_call(Type *return_type, const char *func_name, MIROperand *args, size_t arg_count);
MIRInst *mir_inst_ret(Type *type, MIROperand value);
MIRInst *mir_inst_ret_void();
MIRInst *mir_inst_br(uint32_t target_block);
MIRInst *mir_inst_brcond(MIROperand cond, uint32_t true_block, uint32_t false_block);

#endif // MIR_INST_H
