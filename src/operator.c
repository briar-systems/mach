#include "operator.h"

Operator op_from_token_kind(TokenKind kind)
{
    for (int i = 0; i < OP_COUNT; i++)
    {
        if (op_info[i].token_kind == kind)
        {
            return op_info[i].op;
        }
    }

    return OP_UNKNOWN;
}

TokenKind op_to_token_kind(Operator op)
{
    for (int i = 0; i < OP_COUNT; i++)
    {
        if (op_info[i].op == op)
        {
            return op_info[i].token_kind;
        }
    }

    return TOKEN_ERROR;
}

bool op_is_unary(Operator op)
{
    if (op == OP_UNKNOWN)
    {
        return false;
    }

    for (int i = 0; i < OP_COUNT; i++)
    {
        if (op_info[i].op == op)
        {
            return op_info[i].unary;
        }
    }

    return false;
}

bool op_is_binary(Operator op)
{
    if (op == OP_UNKNOWN)
    {
        return false;
    }

    for (int i = 0; i < OP_COUNT; i++)
    {
        if (op_info[i].op == op)
        {
            return op_info[i].binary;
        }
    }

    return false;
}

int op_precedence(Operator op)
{
    for (int i = 0; i < OP_COUNT; i++)
    {
        if (op_info[i].op == op)
        {
            return op_info[i].precedence;
        }
    }

    return -1;
}

bool op_is_right_associative(Operator op)
{
    for (int i = 0; i < OP_COUNT; i++)
    {
        if (op_info[i].op == op)
        {
            return op_info[i].right_associative;
        }
    }

    return false;
}

const char *op_to_string(Operator op)
{
    switch (op)
    {
    case OP_ADD:
        return "ADD";
    case OP_SUB:
        return "SUB";
    case OP_MUL:
        return "MUL";
    case OP_DIV:
        return "DIV";
    case OP_MOD:
        return "MOD";
    case OP_BITWISE_AND:
        return "B_AND";
    case OP_BITWISE_OR:
        return "B_OR";
    case OP_BITWISE_XOR:
        return "B_XOR";
    case OP_BITWISE_NOT:
        return "B_NOT";
    case OP_BITWISE_SHL:
        return "B_SHL";
    case OP_BITWISE_SHR:
        return "B_SHR";
    case OP_LOGICAL_AND:
        return "L_AND";
    case OP_LOGICAL_OR:
        return "L_OR";
    case OP_LOGICAL_NOT:
        return "L_NOT";
    case OP_EQUAL:
        return "EQ";
    case OP_NOT_EQUAL:
        return "NEQ";
    case OP_LESS:
        return "LT";
    case OP_GREATER:
        return "GT";
    case OP_LESS_EQUAL:
        return "LE";
    case OP_GREATER_EQUAL:
        return "GE";
    case OP_ASSIGN:
        return "ASS";
    case OP_ADDRESS:
        return "REF";
    case OP_DEREFERENCE:
        return "DEREF";
    default:
        return "UNK";
    }
}
