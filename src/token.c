#include "token.h"

Token token_new(TokenKind kind, const char *start, int len)
{
    Token token;
    token.kind = kind;
    token.start = start;
    token.len = len;
    return token;
}

const char *token_kind_to_string(TokenKind kind)
{
    switch (kind)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_ERROR:
        return "ERROR";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_LIT_INT:
        return "LIT_INT";
    case TOKEN_LIT_FLOAT:
        return "LIT_FLOAT";
    case TOKEN_LIT_STRING:
        return "LIT_STRING";
    case TOKEN_LIT_CHAR:
        return "LIT_CHARACTER";
    case TOKEN_L_PAREN:
        return "L_PAREN";
    case TOKEN_R_PAREN:
        return "R_PAREN";
    case TOKEN_L_BRACKET:
        return "L_BRACKET";
    case TOKEN_R_BRACKET:
        return "R_BRACKET";
    case TOKEN_L_BRACE:
        return "L_BRACE";
    case TOKEN_R_BRACE:
        return "R_BRACE";
    case TOKEN_COLON:
        return "COLON";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_QUESTION:
        return "QUESTION";
    case TOKEN_AT:
        return "AT";
    case TOKEN_HASH:
        return "HASH";
    case TOKEN_DOT:
        return "DOT";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_ASTERISK:
        return "STAR";
    case TOKEN_PERCENT:
        return "PERCENT";
    case TOKEN_CARET:
        return "CARET";
    case TOKEN_AMPERSAND:
        return "AMPERSAND";
    case TOKEN_PIPE:
        return "PIPE";
    case TOKEN_TILDE:
        return "TILDE";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_EQUAL:
        return "EQUAL";
    case TOKEN_BANG:
        return "BANG";
    case TOKEN_SLASH:
        return "SLASH";
    case TOKEN_BACKSLASH:
        return "BACKSLASH";
    case TOKEN_EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case TOKEN_BANG_EQUAL:
        return "BANG_EQUAL";
    case TOKEN_LESS_EQUAL:
        return "LESS_EQUAL";
    case TOKEN_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TOKEN_LESS_LESS:
        return "LESS_LESS";
    case TOKEN_GREATER_GREATER:
        return "GREATER_GREATER";
    case TOKEN_AMPERSAND_AMPERSAND:
        return "AMPERSAND_AMPERSAND";
    case TOKEN_PIPE_PIPE:
        return "PIPE_PIPE";
    case TOKEN_COLON_COLON:
        return "COLON_COLON";
    default:
        return "INVALID";
    }
}
