#ifndef OPERATOR_H
#define OPERATOR_H

#include "token.h"

#include <stdbool.h>

typedef enum Operator Operator;
typedef struct OpTokenKinds OpTokenKinds;

enum Operator
{
    OP_UNKNOWN,

    OP_ADD, // +
    OP_SUB, // -
    OP_MUL, // *
    OP_DIV, // /
    OP_MOD, // %

    OP_BITWISE_AND, // &
    OP_BITWISE_OR,  // |
    OP_BITWISE_XOR, // ^
    OP_BITWISE_NOT, // ~
    OP_BITWISE_SHL, // <<
    OP_BITWISE_SHR, // >>

    OP_LOGICAL_AND, // &&
    OP_LOGICAL_OR,  // ||
    OP_LOGICAL_NOT, // !

    OP_EQUAL,         // ==
    OP_NOT_EQUAL,     // !=
    OP_LESS,          // <
    OP_GREATER,       // >
    OP_LESS_EQUAL,    // <=
    OP_GREATER_EQUAL, // >=

    OP_ASSIGN,      // =
    OP_ADDRESS,     // ?
    OP_DEREFERENCE, // @

    OP_COUNT,
};

struct OpTokenKinds
{
    Operator op;
    TokenKind kind;
};

struct OpInfo
{
    Operator op;
    int precedence;
    bool right_associative;
    bool unary;
    bool binary;
    TokenKind token_kind;
};

static const struct OpInfo op_info[] = {
    {OP_ADD, 7, false, true, true, TOKEN_PLUS},
    {OP_SUB, 7, false, true, true, TOKEN_MINUS},
    {OP_MUL, 8, false, false, true, TOKEN_ASTERISK},
    {OP_DIV, 8, false, false, true, TOKEN_SLASH},
    {OP_MOD, 8, false, false, true, TOKEN_PERCENT},
    {OP_ASSIGN, 1, true, false, true, TOKEN_EQUAL},
    {OP_ADDRESS, 0, true, true, false, TOKEN_QUESTION},
    {OP_DEREFERENCE, 0, true, true, false, TOKEN_AT},
    {OP_LOGICAL_NOT, 12, false, true, false, TOKEN_BANG},
    {OP_BITWISE_NOT, 12, false, true, false, TOKEN_TILDE},
    {OP_BITWISE_AND, 9, false, false, true, TOKEN_AMPERSAND},
    {OP_BITWISE_OR, 10, false, false, true, TOKEN_PIPE},
    {OP_BITWISE_XOR, 11, false, false, true, TOKEN_CARET},
    {OP_BITWISE_SHL, 6, false, false, true, TOKEN_LESS_LESS},
    {OP_BITWISE_SHR, 6, false, false, true, TOKEN_GREATER_GREATER},
    {OP_LOGICAL_AND, 3, false, false, true, TOKEN_AMPERSAND_AMPERSAND},
    {OP_LOGICAL_OR, 2, false, false, true, TOKEN_PIPE_PIPE},
    {OP_EQUAL, 4, false, false, true, TOKEN_EQUAL_EQUAL},
    {OP_NOT_EQUAL, 4, false, false, true, TOKEN_BANG_EQUAL},
    {OP_LESS, 5, false, false, true, TOKEN_LESS},
    {OP_GREATER, 5, false, false, true, TOKEN_GREATER},
    {OP_LESS_EQUAL, 5, false, false, true, TOKEN_LESS_EQUAL},
    {OP_GREATER_EQUAL, 5, false, false, true, TOKEN_GREATER_EQUAL},
};

Operator op_from_token_kind(TokenKind kind);
TokenKind op_to_token_kind(Operator op);
bool op_is_unary(Operator op);
bool op_is_binary(Operator op);
int op_precedence(Operator op);
bool op_is_right_associative(Operator op);

const char *op_to_string(Operator op);

#endif
