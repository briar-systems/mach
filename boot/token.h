#ifndef TOKEN_H
#define TOKEN_H

typedef enum TokenKind
{
    TOKEN_ERROR = -1,
    TOKEN_EOF   = 0,
    TOKEN_COMMENT,

    // literals
    TOKEN_LIT_INT,
    TOKEN_LIT_FLOAT,
    TOKEN_LIT_CHAR,
    TOKEN_LIT_STRING,

    TOKEN_IDENTIFIER,

    // keywords
    TOKEN_KW_USE, // use
    TOKEN_KW_EXT, // ext
    TOKEN_KW_DEF, // def
    TOKEN_KW_PUB, // pub
    TOKEN_KW_STR, // str
    TOKEN_KW_UNI, // uni
    TOKEN_KW_VAL, // val
    TOKEN_KW_VAR, // var
    TOKEN_KW_FUN, // fun
    TOKEN_KW_RET, // ret
    TOKEN_KW_IF,  // if
    TOKEN_KW_OR,  // or
    TOKEN_KW_FOR, // for
    TOKEN_KW_CNT, // cnt
    TOKEN_KW_BRK, // brk
    TOKEN_KW_ASM, // asm
    TOKEN_KW_NIL, // nil

    // type keywords
    // TOKEN_TYPE_PTR, // untyped pointer
    // TOKEN_TYPE_I8,  // i8
    // TOKEN_TYPE_I16, // i16
    // TOKEN_TYPE_I32, // i32
    // TOKEN_TYPE_I64, // i64
    // TOKEN_TYPE_U8,  // u8
    // TOKEN_TYPE_U16, // u16
    // TOKEN_TYPE_U32, // u32
    // TOKEN_TYPE_U64, // u64
    // TOKEN_TYPE_F16, // f16
    // TOKEN_TYPE_F32, // f32
    // TOKEN_TYPE_F64, // f64

    // punctuation
    TOKEN_L_PAREN,    // (
    TOKEN_R_PAREN,    // )
    TOKEN_L_BRACKET,  // [
    TOKEN_R_BRACKET,  // ]
    TOKEN_L_BRACE,    // {
    TOKEN_R_BRACE,    // }
    TOKEN_COLON,      // :
    TOKEN_SEMICOLON,  // ;
    TOKEN_QUESTION,   // ?
    TOKEN_AT,         // @
    TOKEN_DOT,        // .
    TOKEN_COMMA,      // ,
    TOKEN_UNDERSCORE, // _
    TOKEN_ELLIPSIS,   // ...

    // operators - single character
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
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

    // operators - multi-character
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

typedef struct Token
{
    TokenKind kind;
    int       pos;
    int       len;
} Token;

void token_init(Token *token, TokenKind kind, int pos, int len);
void token_dnit(Token *token);

void      token_copy(Token *src, Token *dst);
char     *token_kind_to_string(TokenKind kind);
TokenKind token_kind_from_identifier(const char *text, int len);

#endif
