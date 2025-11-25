#include "compiler/mir/opcode.h"

const char *mir_op_to_string(MIROp op)
{
    switch (op)
    {
    case MIR_OP_CONST: return "const";
    case MIR_OP_MOV: return "mov";
    case MIR_OP_LOAD: return "load";
    case MIR_OP_STORE: return "store";
    case MIR_OP_PHI: return "phi";
    case MIR_OP_ADD: return "add";
    case MIR_OP_SUB: return "sub";
    case MIR_OP_MUL: return "mul";
    case MIR_OP_DIV: return "div";
    case MIR_OP_UDIV: return "udiv";
    case MIR_OP_MOD: return "mod";
    case MIR_OP_UMOD: return "umod";
    case MIR_OP_NEG: return "neg";
    case MIR_OP_AND: return "and";
    case MIR_OP_OR: return "or";
    case MIR_OP_XOR: return "xor";
    case MIR_OP_NOT: return "not";
    case MIR_OP_SHL: return "shl";
    case MIR_OP_SHR: return "shr";
    case MIR_OP_SAR: return "sar";
    case MIR_OP_EQ: return "eq";
    case MIR_OP_NE: return "ne";
    case MIR_OP_LT: return "lt";
    case MIR_OP_LE: return "le";
    case MIR_OP_GT: return "gt";
    case MIR_OP_GE: return "ge";
    case MIR_OP_ULT: return "ult";
    case MIR_OP_ULE: return "ule";
    case MIR_OP_UGT: return "ugt";
    case MIR_OP_UGE: return "uge";
    case MIR_OP_ZEXT: return "zext";
    case MIR_OP_SEXT: return "sext";
    case MIR_OP_TRUNC: return "trunc";
    case MIR_OP_CAST: return "cast";
    case MIR_OP_GEP: return "gep";
    case MIR_OP_RET: return "ret";
    case MIR_OP_BR: return "br";
    case MIR_OP_BRCOND: return "brcond";
    case MIR_OP_CALL: return "call";
    case MIR_OP_UNREACHABLE: return "unreachable";
    case MIR_OP_SYSCALL: return "syscall";
    default: return "unknown";
    }
}

bool mir_op_is_terminator(MIROp op)
{
    return op == MIR_OP_RET || op == MIR_OP_BR || op == MIR_OP_BRCOND || op == MIR_OP_UNREACHABLE;
}

bool mir_op_is_comparison(MIROp op)
{
    return op >= MIR_OP_EQ && op <= MIR_OP_UGE;
}

bool mir_op_is_binary(MIROp op)
{
    return (op >= MIR_OP_ADD && op <= MIR_OP_SAR) || mir_op_is_comparison(op);
}

bool mir_op_is_unary(MIROp op)
{
    return op == MIR_OP_NEG || op == MIR_OP_NOT || (op >= MIR_OP_ZEXT && op <= MIR_OP_CAST);
}
