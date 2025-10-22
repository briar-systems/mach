#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include <stdbool.h>

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_BIT_OR,
    PREC_BIT_XOR,
    PREC_BIT_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_SHIFT,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_POSTFIX,
    PREC_PRIMARY
} Precedence;

typedef struct ParserError
{
    Token *token;
    char  *message;
} ParserError;

typedef struct ParserErrorList
{
    ParserError *errors;
    int          count;
    int          capacity;
} ParserErrorList;

typedef struct Parser
{
    Lexer          *lexer;
    Token          *current;
    Token          *previous;
    bool            panic_mode;
    bool            had_error;
    ParserErrorList errors;
    char           *pending_mangle;
} Parser;

// parser lifecycle
void parser_init(Parser *parser, Lexer *lexer);
void parser_dnit(Parser *parser);

// error list operations
void parser_error_list_init(ParserErrorList *list);
void parser_error_list_dnit(ParserErrorList *list);
void parser_error_list_add(ParserErrorList *list, Token *token, const char *message);
void parser_error_list_print(ParserErrorList *list, Lexer *lexer, const char *file_path);

// token navigation
void parser_advance(Parser *parser);
bool parser_check(Parser *parser, TokenKind kind);
bool parser_match(Parser *parser, TokenKind kind);
bool parser_consume(Parser *parser, TokenKind kind, const char *message);
void parser_synchronize(Parser *parser);
bool parser_is_at_end(Parser *parser);

// error handling
void parser_error(Parser *parser, Token *token, const char *message);
void parser_error_at_current(Parser *parser, const char *message);
void parser_error_at_previous(Parser *parser, const char *message);

// parsing entry point
AstNode *parser_parse_program(Parser *parser);

// statement parsing
AstNode *parser_parse_stmt_top(Parser *parser);
AstNode *parser_parse_stmt(Parser *parser);
AstNode *parser_parse_stmt_use(Parser *parser);
AstNode *parser_parse_stmt_ext(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_def(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_val(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_var(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_fun(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_str(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_uni(Parser *parser, bool is_public);
AstNode *parser_parse_stmt_if(Parser *parser);
AstNode *parser_parse_stmt_for(Parser *parser);
AstNode *parser_parse_stmt_brk(Parser *parser);
AstNode *parser_parse_stmt_cnt(Parser *parser);
AstNode *parser_parse_stmt_ret(Parser *parser);
AstNode *parser_parse_stmt_block(Parser *parser);
AstNode *parser_parse_stmt_expr(Parser *parser);

AstNode *parser_parse_expr(Parser *parser);
AstNode *parser_parse_expr_prec(Parser *parser, Precedence min_prec);
AstNode *parser_parse_expr_prefix(Parser *parser);
AstNode *parser_parse_expr_postfix(Parser *parser);
AstNode *parser_parse_expr_atom(Parser *parser);
AstNode *parser_parse_array_literal(Parser *parser);
AstNode *parser_parse_struct_literal(Parser *parser, AstNode *type);
AstNode *parser_parse_typed_literal(Parser *parser, AstNode *type);

// type parsing
AstNode *parser_parse_type(Parser *parser);
AstNode *parser_parse_type_name(Parser *parser);
AstNode *parser_parse_type_ptr(Parser *parser);
AstNode *parser_parse_type_array(Parser *parser);
AstNode *parser_parse_type_fun(Parser *parser);
AstNode *parser_parse_type_str(Parser *parser);
AstNode *parser_parse_type_uni(Parser *parser);

// list parsing
AstList *parser_parse_field_list(Parser *parser);
AstList *parser_parse_parameter_list(Parser *parser);

// utilities
char *parser_parse_identifier(Parser *parser);

#endif
