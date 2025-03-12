#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#include <stdbool.h>

typedef struct
{
    const char *source;
    const char *pos;
    const char *error;
} Lexer;

bool lexer_init(Lexer *lexer, const char *source);
void lexer_free(Lexer *lexer);

bool lexer_at_end(Lexer *lexer);

char lexer_current(Lexer *lexer);
char lexer_peek(Lexer *lexer, int offset);
void lexer_advance(Lexer *lexer);

int lexer_line_at(Lexer *lexer, const char *pos);
int lexer_column_at(Lexer *lexer, const char *pos);

char* lexer_get_line(Lexer *lexer, int line);

Token lexer_parse_comment(Lexer *lexer);
Token lexer_parse_identifier(Lexer *lexer);
Token lexer_parse_lit_number(Lexer *lexer);
Token lexer_parse_lit_character(Lexer *lexer);
Token lexer_parse_lit_string(Lexer *lexer);

long long lexer_eval_lit_int(const char *start, int len);
double lexer_eval_lit_float(const char *start, int len);
char lexer_eval_lit_char(const char *start, int len);
char *lexer_eval_lit_string(const char *start, int len);

Token lexer_emit(Lexer *lexer, TokenKind kind, int len);
Token lexer_next(Lexer *lexer);

#endif // LEXER_H
