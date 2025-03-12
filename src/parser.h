#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "token.h"

#include <stdbool.h>

typedef struct Parser Parser;
typedef struct ParseError ParseError;

struct ParseError
{
    Token token;
    const char *message;
};

struct Parser
{
    Lexer *lexer;

    Token *tokens;
    int tokens_len;
    int tokens_cap;
    int tokens_cur;

    ParseError *errors;
    int error_len;
    int error_cap;

    bool recovering;
};

bool parser_init(Parser *parser, Lexer *lexer);
void parser_free(Parser *parser);

void parser_print_error(Parser *parser, ParseError error, const char *file_path);
void parser_print_errors(Parser *parser, const char *file_path);
void parser_error(Parser *parser, Token token, const char *message);
bool parser_has_errors(Parser *parser);

void parser_recovery_reset(Parser *parser);
void parser_synchronize(Parser *parser);

Token parser_curr(Parser *parser);
Token parser_prev(Parser *parser);

bool parser_is_at_end(Parser *parser);
Token parser_advance(Parser *parser);
bool parser_check(Parser *parser, TokenKind type);
bool parser_match(Parser *parser, TokenKind type);

Node *parse_comment(Parser *parser);

Node *parse_identifier(Parser *parser);
Node *parse_lit_int(Parser *parser);
Node *parse_lit_float(Parser *parser);
Node *parse_lit_char(Parser *parser);
Node *parse_lit_string(Parser *parser);

Node *parse_stmt_val(Parser *parser);
Node *parse_stmt_var(Parser *parser);
Node *parse_stmt_vol(Parser *parser);
Node *parse_stmt_def(Parser *parser);
Node *parse_stmt_use(Parser *parser);

Node *parse_field(Parser *parser);
Node *parse_stmt_str(Parser *parser);
Node *parse_stmt_uni(Parser *parser);
Node *parse_stmt_fun(Parser *parser);

Node *parse_stmt_block(Parser *parser);
Node *parse_stmt_if(Parser *parser);
Node *parse_stmt_or(Parser *parser);
Node *parse_stmt_for(Parser *parser);
Node *parse_stmt_brk(Parser *parser);
Node *parse_stmt_cnt(Parser *parser);
Node *parse_stmt_ret(Parser *parser);
Node *parse_stmt_expr(Parser *parser);
Node *parse_stmt(Parser *parser);

Node *parse_post_member(Parser *parser, Node *target);
Node *parse_post_call(Parser *parser, Node *target);
Node *parse_post_idx_arr(Parser *parser, Node *target);
Node *parse_post_cast(Parser *parser, Node *target);
Node *parse_post(Parser *parser);

Node *parse_expr_unary(Parser *parser);
Node *parse_expr_binary(Parser *parser, int min_prec);
Node *parse_expr_grouping(Parser *parser);
Node *parse_expr_primary(Parser *parser);
Node *parse_expr(Parser *parser);

Node *parse_type_arr(Parser *parser);
Node *parse_type_ptr(Parser *parser);
Node *parse_type_fun(Parser *parser);
Node *parse_type_str(Parser *parser);
Node *parse_type_uni(Parser *parser);
Node *parse_type(Parser *parser);

Node *parse_module(Parser *parser);

Node *parser_parse(Parser *parser);

bool parser_build_token_list(Parser *parser);

#endif // PARSER_H
