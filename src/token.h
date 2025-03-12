#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>

typedef enum
{
    TOKEN_UNKNOWN,

    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_VAL, // val
    TOKEN_VAR, // var
    TOKEN_VOL, // vol
    TOKEN_DEF, // def
    TOKEN_USE, // use
    TOKEN_STR, // str
    TOKEN_UNI, // uni
    TOKEN_FUN, // fun
    TOKEN_IF,  // if
    TOKEN_OR,  // or
    TOKEN_FOR, // for
    TOKEN_BRK, // brk
    TOKEN_CNT, // cnt
    TOKEN_RET, // ret

    TOKEN_IDENTIFIER,

    TOKEN_LIT_INT,
    TOKEN_LIT_FLOAT,
    TOKEN_LIT_STRING,
    TOKEN_LIT_CHAR,

    TOKEN_L_PAREN,   // (
    TOKEN_R_PAREN,   // )
    TOKEN_L_BRACKET, // [
    TOKEN_R_BRACKET, // ]
    TOKEN_L_BRACE,   // {
    TOKEN_R_BRACE,   // }
    TOKEN_COLON,     // :
    TOKEN_SEMICOLON, // ;
    TOKEN_QUESTION,  // ?
    TOKEN_AT,        // @
    TOKEN_HASH,      // #
    TOKEN_DOT,       // .
    TOKEN_COMMA,     // ,
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_ASTERISK,      // *
    TOKEN_PERCENT,   // %
    TOKEN_CARET,     // ^
    TOKEN_AMPERSAND, // &
    TOKEN_PIPE,      // |
    TOKEN_TILDE,     // ~
    TOKEN_LESS,      // <
    TOKEN_GREATER,   // >
    TOKEN_EQUAL,     // =
    TOKEN_BANG,      // !
    TOKEN_SLASH,     // /
    TOKEN_BACKSLASH,
    TOKEN_EQUAL_EQUAL,         // ==
    TOKEN_BANG_EQUAL,          // !=
    TOKEN_LESS_EQUAL,          // <=
    TOKEN_GREATER_EQUAL,       // >=
    TOKEN_LESS_LESS,           // <<
    TOKEN_GREATER_GREATER,     // >>
    TOKEN_AMPERSAND_AMPERSAND, // &&
    TOKEN_PIPE_PIPE,           // ||
    TOKEN_COLON_COLON,         // ::
} TokenKind;

typedef struct
{
    TokenKind kind;
    const char *start;
    int len;
} Token;

Token token_new(TokenKind kind, const char *start, int len);

const char *token_value(Token token);
const char *token_kind_to_string(TokenKind kind);

#endif // TOKEN_H
