#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#include <stdbool.h>

typedef struct Lexer
{
    int   pos;
    char *source;
} Lexer;

void lexer_init(Lexer *lexer, char *source);
void lexer_dnit(Lexer *lexer);

bool lexer_at_end(Lexer *lexer);
char lexer_current(Lexer *lexer);
char lexer_peek(Lexer *lexer, int offset);
char lexer_advance(Lexer *lexer);

int   lexer_get_pos_line(Lexer *lexer, int pos);
int   lexer_get_pos_line_offset(Lexer *lexer, int pos);
char *lexer_get_line_text(Lexer *lexer, int line);

void lexer_skip_whitespace(Lexer *lexer);

Token *lexer_parse_identifier(Lexer *lexer);
Token *lexer_parse_lit_number(Lexer *lexer);
Token *lexer_parse_lit_char(Lexer *lexer);
Token *lexer_parse_lit_string(Lexer *lexer);

unsigned long long lexer_eval_lit_int(Lexer *lexer, Token *token);
double             lexer_eval_lit_float(Lexer *lexer, Token *token);
char               lexer_eval_lit_char(Lexer *lexer, Token *token);
char              *lexer_eval_lit_string(Lexer *lexer, Token *token);
char              *lexer_raw_value(Lexer *lexer, Token *token);

Token *lexer_emit(Lexer *lexer, TokenKind kind, int len);

Token *lexer_next(Lexer *lexer);

#endif
