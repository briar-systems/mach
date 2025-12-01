#include "compiler/parser.h"
#include "compiler/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNode  *parser_alloc_node(Parser *parser, AstKind kind, Token *token);
static AstList  *parser_alloc_list(Parser *parser);
static void      parser_free_node(AstNode *node);
static void      parser_report_alloc_failure(Parser *parser, const char *context);
static char     *parser_strdup_checked(Parser *parser, const char *value, const char *context);
static AstNode  *parser_parse_directive(Parser *parser);
static TokenKind parser_peek_next_kind(Parser *parser);
static bool      parser_should_parse_type_args(Parser *parser, TokenKind *follow_kind);
static AstNode  *parser_expr_to_type(Parser *parser, AstNode *expr, AstList *generic_args);

static AstNode *parser_alloc_node(Parser *parser, AstKind kind, Token *token)
{
    AstNode *node = malloc(sizeof(AstNode));
    if (!node)
    {
        parser_error(parser, token ? token : parser->current, "out of memory allocating AST node");
        return NULL;
    }

    ast_node_init(node, kind);
    node->token = NULL;

    if (token)
    {
        Token *copy = malloc(sizeof(Token));
        if (!copy)
        {
            parser_error(parser, token, "out of memory allocating token copy");
            free(node);
            return NULL;
        }

        token_copy(token, copy);
        node->token = copy;
    }

    return node;
}

static void parser_free_node(AstNode *node)
{
    if (!node)
    {
        return;
    }

    ast_node_dnit(node);
    free(node);
}

static void parser_report_alloc_failure(Parser *parser, const char *context)
{
    if (!parser)
    {
        return;
    }

    Token *token = parser->current ? parser->current : parser->previous;
    parser_error(parser, token, context ? context : "out of memory");
}

static char *parser_strdup_checked(Parser *parser, const char *value, const char *context)
{
    if (!value)
    {
        return NULL;
    }

    char *copy = strdup(value);
    if (!copy)
    {
        parser_report_alloc_failure(parser, context);
    }
    return copy;
}
static AstList *parser_alloc_list(Parser *parser)
{
    AstList *list = malloc(sizeof(AstList));
    if (!list)
    {
        parser_error(parser, parser->current, "out of memory allocating AST list");
        return NULL;
    }

    ast_list_init(list);
    return list;
}

static bool parser_token_is_attr_category(TokenKind kind)
{
    switch (kind)
    {
    case TOKEN_KW_FUN:
    case TOKEN_KW_VAL:
    case TOKEN_KW_VAR:
    case TOKEN_KW_REC:
    case TOKEN_KW_UNI:
    case TOKEN_KW_EXT:
    case TOKEN_KW_DEF:
        return true;
    default:
        return false;
    }
}

static AstNode *parser_parse_stmt_comptime_if(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_IF, "expected 'if' after '$'"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_COMPTIME_IF, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_L_PAREN, "expected '(' after '$if'"))
    {
        parser_free_node(node);
        return NULL;
    }

    node->comptime_if_stmt.cond = parser_parse_expr(parser);
    if (!node->comptime_if_stmt.cond)
    {
        parser_error_at_current(parser, "expected condition expression");
        parser_free_node(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after condition"))
    {
        parser_free_node(node);
        return NULL;
    }

    node->comptime_if_stmt.body = parser_parse_stmt_block(parser);
    if (!node->comptime_if_stmt.body)
    {
        parser_error_at_current(parser, "expected block after '$if' condition");
        parser_free_node(node);
        return NULL;
    }

    // handle $or chains
    AstNode **tail = &node->comptime_if_stmt.stmt_or;
    while (parser_check(parser, TOKEN_DOLLAR) && parser_peek_next_kind(parser) == TOKEN_KW_OR)
    {
        parser_advance(parser); // consume $
        parser_advance(parser); // consume OR

        AstNode *or_node = parser_alloc_node(parser, AST_STMT_COMPTIME_OR, parser->previous);
        if (!or_node)
        {
            parser_free_node(node);
            return NULL;
        }

        // optional condition
        if (parser_match(parser, TOKEN_L_PAREN))
        {
            or_node->comptime_if_stmt.cond = parser_parse_expr(parser);
            if (!or_node->comptime_if_stmt.cond)
            {
                parser_error_at_current(parser, "expected condition expression");
                parser_free_node(or_node);
                parser_free_node(node);
                return NULL;
            }

            if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after '$or' condition"))
            {
                parser_free_node(or_node);
                parser_free_node(node);
                return NULL;
            }
        }

        or_node->comptime_if_stmt.body = parser_parse_stmt_block(parser);
        if (!or_node->comptime_if_stmt.body)
        {
            parser_error_at_current(parser, "expected block after '$or'");
            parser_free_node(or_node);
            parser_free_node(node);
            return NULL;
        }

        *tail = or_node;
        tail  = &or_node->comptime_if_stmt.stmt_or;
    }

    return node;
}

static AstNode *parser_parse_directive(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_DOLLAR, "expected '$'"))
    {
        return NULL;
    }

    if (parser_check(parser, TOKEN_KW_IF))
    {
        return parser_parse_stmt_comptime_if(parser);
    }

    Token *directive_token = parser->previous;

    AstNode *node = parser_alloc_node(parser, AST_COMPTIME, directive_token);
    if (!node)
    {
        return NULL;
    }

    if (parser_token_is_attr_category(parser->current->kind))
    {
        parser->current->kind = TOKEN_IDENTIFIER;
    }

    AstNode *inner = parser_parse_expr(parser);

    parser_match(parser, TOKEN_SEMICOLON);

    if (!inner)
    {
        parser_error_at_current(parser, "expected expression or statement after '$'");
        parser_free_node(node);
        return NULL;
    }

    node->comptime.inner = inner;
    return node;
}

static TokenKind parser_peek_next_kind(Parser *parser)
{
    if (!parser || !parser->lexer)
    {
        return TOKEN_EOF;
    }

    int saved_pos = parser->lexer->pos;

    while (true)
    {
        Token *tok = lexer_next(parser->lexer);
        if (!tok)
        {
            parser->lexer->pos = saved_pos;
            return TOKEN_EOF;
        }

        TokenKind kind = tok->kind;
        token_dnit(tok);
        free(tok);

        if (kind == TOKEN_COMMENT)
        {
            continue;
        }

        parser->lexer->pos = saved_pos;
        return kind;
    }
}

static bool     parser_should_parse_type_args(Parser *parser, TokenKind *follow_kind);
static AstList *parser_parse_type_arguments(Parser *parser);
static AstList *parser_parse_generic_param_list(Parser *parser);

static Token *parser_next_non_comment(Parser *parser, Token ***storage, size_t *count, size_t *capacity)
{
    if (!parser || !parser->lexer || !storage || !count || !capacity)
    {
        return NULL;
    }

    for (;;)
    {
        Token *tok = lexer_next(parser->lexer);
        if (!tok)
        {
            return NULL;
        }

        if (tok->kind == TOKEN_COMMENT)
        {
            token_dnit(tok);
            free(tok);
            continue;
        }

        if (*count >= *capacity)
        {
            size_t  new_capacity = (*capacity) ? (*capacity) * 2 : 8;
            Token **tmp          = realloc(*storage, new_capacity * sizeof(Token *));
            if (!tmp)
            {
                token_dnit(tok);
                free(tok);
                return NULL;
            }
            *storage  = tmp;
            *capacity = new_capacity;
        }

        (*storage)[(*count)++] = tok;
        return tok;
    }
}

// parser lifecycle
void parser_init(Parser *parser, Lexer *lexer)
{
    parser->lexer      = lexer;
    parser->current    = NULL;
    parser->previous   = NULL;
    parser->panic_mode = false;
    parser->had_error  = false;
    parser_error_list_init(&parser->errors);

    // prime the parser
    parser_advance(parser);
}

void parser_dnit(Parser *parser)
{
    if (parser->current)
    {
        token_dnit(parser->current);
        free(parser->current);
    }
    if (parser->previous)
    {
        token_dnit(parser->previous);
        free(parser->previous);
    }
    parser_error_list_dnit(&parser->errors);
}

// error list operations
void parser_error_list_init(ParserErrorList *list)
{
    list->errors   = NULL;
    list->count    = 0;
    list->capacity = 0;
}

void parser_error_list_dnit(ParserErrorList *list)
{
    for (int i = 0; i < list->count; i++)
    {
        free(list->errors[i].message);
        token_dnit(list->errors[i].token);
        free(list->errors[i].token);
    }
    free(list->errors);
}

void parser_error_list_add(ParserErrorList *list, Token *token, const char *message)
{
    if (list->count >= list->capacity)
    {
        int          new_capacity = list->capacity ? list->capacity * 2 : 8;
        ParserError *new_errors   = realloc(list->errors, sizeof(ParserError) * new_capacity);
        if (!new_errors)
        {
            fprintf(stderr, "error: memory allocation failed for parser error list\n");
            exit(EXIT_FAILURE);
        }
        list->errors   = new_errors;
        list->capacity = new_capacity;
    }

    Token *error_token = malloc(sizeof(Token));
    if (!error_token)
    {
        fprintf(stderr, "error: memory allocation failed for error token\n");
        exit(EXIT_FAILURE);
    }

    token_copy(token, error_token);

    list->errors[list->count].token   = error_token;
    list->errors[list->count].message = strdup(message);
    if (!list->errors[list->count].message)
    {
        fprintf(stderr, "error: memory allocation failed for error message\n");
        exit(EXIT_FAILURE);
    }

    list->count++;
}

void parser_error_list_print(ParserErrorList *list, Lexer *lexer, const char *file_path)
{
    for (int i = 0; i < list->count; i++)
    {
        Token *token     = list->errors[i].token;
        int    line      = lexer_get_pos_line(lexer, token->pos);
        int    col       = lexer_get_pos_line_offset(lexer, token->pos);
        char  *line_text = lexer_get_line_text(lexer, line);
        if (!line_text)
        {
            line_text = strdup("unable to retrieve line text");
        }

        printf("error: %s\n", list->errors[i].message);
        printf("%s:%d:%d\n", file_path, line + 1, col);
        printf("%5d | %-*s\n", line + 1, col > 1 ? col - 1 : 0, line_text);
        printf("      | %*s^\n", col - 1, "");

        free(line_text);
    }
}

// token navigation
void parser_advance(Parser *parser)
{
    if (parser->previous)
    {
        token_dnit(parser->previous);
        free(parser->previous);
    }

    parser->previous = parser->current;

    for (;;)
    {
        parser->current = lexer_next(parser->lexer);

        // skip comments
        if (parser->current->kind == TOKEN_COMMENT)
        {
            token_dnit(parser->current);
            free(parser->current);
            continue;
        }

        // handle error tokens
        if (parser->current->kind == TOKEN_ERROR)
        {
            parser_error_at_current(parser, "unexpected character");
            token_dnit(parser->current);
            free(parser->current);
            continue;
        }

        break;
    }
}

bool parser_check(Parser *parser, TokenKind kind)
{
    return parser->current->kind == kind;
}

bool parser_match(Parser *parser, TokenKind kind)
{
    if (!parser_check(parser, kind))
    {
        return false;
    }
    parser_advance(parser);
    return true;
}

bool parser_consume(Parser *parser, TokenKind kind, const char *message)
{
    if (parser->current->kind == kind)
    {
        parser_advance(parser);
        return true;
    }
    parser_error_at_current(parser, message);
    return false;
}

void parser_synchronize(Parser *parser)
{
    if (!parser->panic_mode)
    {
        return;
    }

    parser->panic_mode = false;

    while (!parser_is_at_end(parser))
    {
        if (parser->current->kind == TOKEN_SEMICOLON)
        {
            parser_advance(parser);
            return;
        }

        switch (parser->current->kind)
        {
        case TOKEN_KW_USE:
        case TOKEN_KW_EXT:
        case TOKEN_KW_DEF:
        case TOKEN_KW_REC:
        case TOKEN_KW_UNI:
        case TOKEN_KW_VAL:
        case TOKEN_KW_VAR:
        case TOKEN_KW_FUN:
        case TOKEN_DOLLAR:
            return;
        default:
            parser_advance(parser);
        }
    }
}

bool parser_is_at_end(Parser *parser)
{
    return parser->current->kind == TOKEN_EOF;
}

// error handling
void parser_error(Parser *parser, Token *token, const char *message)
{
    if (!parser->panic_mode)
    {
        parser->panic_mode = true;
        parser->had_error  = true;
        parser_error_list_add(&parser->errors, token, message);
    }
}

void parser_error_at_current(Parser *parser, const char *message)
{
    parser_error(parser, parser->current, message);
}

void parser_error_at_previous(Parser *parser, const char *message)
{
    parser_error(parser, parser->previous, message);
}

char *parser_parse_identifier(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_IDENTIFIER, "expected identifier"))
    {
        return NULL;
    }
    return lexer_raw_value(parser->lexer, parser->previous);
}

static bool parser_should_parse_type_args(Parser *parser, TokenKind *follow_kind)
{
    if (follow_kind)
    {
        *follow_kind = TOKEN_ERROR;
    }

    if (!parser || !parser->current || parser->current->kind != TOKEN_L_BRACKET)
    {
        return false;
    }

    int     saved_pos     = parser->lexer->pos;
    Token **peeked_tokens = NULL;
    size_t  peek_count    = 0;
    size_t  peek_capacity = 0;
    bool    result        = false;
    bool    has_payload   = false;
    int     depth         = 1; // start at 1 to account for the opening '[' we're standing on

    // process tokens after the opening bracket
    while (true)
    {
        Token *tok = parser_next_non_comment(parser, &peeked_tokens, &peek_count, &peek_capacity);
        if (!tok || tok->kind == TOKEN_EOF || tok->kind == TOKEN_SEMICOLON)
        {
            break;
        }

        if (tok->kind == TOKEN_L_BRACKET)
        {
            depth++;
        }
        else if (tok->kind == TOKEN_R_BRACKET)
        {
            depth--;
            if (depth == 0)
            {
                break;
            }
        }
        else if (depth > 0)
        {
            has_payload = true;
        }
    }

    if (has_payload && depth == 0)
    {
        // check what follows the closing ']'
        Token *after = parser_next_non_comment(parser, &peeked_tokens, &peek_count, &peek_capacity);
        if (after && (after->kind == TOKEN_L_PAREN || after->kind == TOKEN_L_BRACE))
        {
            result = true;
            if (follow_kind)
            {
                *follow_kind = after->kind;
            }
        }
    }

    // restore lexer position
    parser->lexer->pos = saved_pos;

    // clean up peeked tokens
    for (size_t i = 0; i < peek_count; i++)
    {
        token_dnit(peeked_tokens[i]);
        free(peeked_tokens[i]);
    }
    free(peeked_tokens);

    return result;
}

static AstList *parser_parse_type_arguments(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_L_BRACKET, "expected '[' to start type arguments"))
    {
        return NULL;
    }

    AstList *args = parser_alloc_list(parser);
    if (!args)
    {
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_BRACKET))
    {
        do
        {
            AstNode *type_arg = parser_parse_type(parser);
            if (!type_arg)
            {
                ast_list_dnit(args);
                free(args);
                return NULL;
            }
            ast_list_append(args, type_arg);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_BRACKET, "expected ']' after type arguments"))
    {
        ast_list_dnit(args);
        free(args);
        return NULL;
    }

    return args;
}

static AstList *parser_parse_generic_param_list(Parser *parser)
{
    AstList *params = parser_alloc_list(parser);
    if (!params)
    {
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_BRACKET))
    {
        do
        {
            char *name = parser_parse_identifier(parser);
            if (!name)
            {
                ast_list_dnit(params);
                free(params);
                return NULL;
            }

            AstNode *param_node = parser_alloc_node(parser, AST_TYPE_PARAM, parser->previous);
            if (!param_node)
            {
                free(name);
                ast_list_dnit(params);
                free(params);
                return NULL;
            }

            param_node->type_param.name = name;
            ast_list_append(params, param_node);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_BRACKET, "expected ']' after generic parameters"))
    {
        ast_list_dnit(params);
        free(params);
        return NULL;
    }

    return params;
}

static AstNode *parser_parse_anonymous_composite_literal(Parser *parser, bool is_union)
{
    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' to start composite literal"))
    {
        return NULL;
    }

    AstNode *literal = parser_alloc_node(parser, AST_EXPR_STRUCT, parser->previous);
    if (!literal)
    {
        return NULL;
    }

    literal->struct_expr.type                 = NULL;
    literal->struct_expr.fields               = parser_alloc_list(parser);
    literal->struct_expr.is_union_literal     = is_union;
    literal->struct_expr.is_anonymous_literal = true;
    if (!literal->struct_expr.fields)
    {
        ast_node_dnit(literal);
        free(literal);
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_BRACE))
    {
        do
        {
            char *field_name = parser_parse_identifier(parser);
            if (!field_name)
            {
                ast_node_dnit(literal);
                free(literal);
                return NULL;
            }

            if (!parser_consume(parser, TOKEN_COLON, "expected ':' after field name"))
            {
                free(field_name);
                ast_node_dnit(literal);
                free(literal);
                return NULL;
            }

            AstNode *init = parser_parse_expr(parser);
            if (!init)
            {
                free(field_name);
                ast_node_dnit(literal);
                free(literal);
                return NULL;
            }

            AstNode *field = parser_alloc_node(parser, AST_EXPR_FIELD, parser->previous);
            if (!field)
            {
                free(field_name);
                ast_node_dnit(init);
                free(init);
                ast_node_dnit(literal);
                free(literal);
                return NULL;
            }

            field->field_expr.field  = field_name;
            field->field_expr.object = init;
            ast_list_append(literal->struct_expr.fields, field);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after composite fields"))
    {
        ast_node_dnit(literal);
        free(literal);
        return NULL;
    }

    return literal;
}

static AstNode *parser_finish_call(Parser *parser, AstNode *callee, AstList *type_args)
{
    AstNode *call = parser_alloc_node(parser, AST_EXPR_CALL, parser->previous);
    if (!call)
    {
        if (type_args)
        {
            ast_list_dnit(type_args);
            free(type_args);
        }
        if (callee)
        {
            ast_node_dnit(callee);
            free(callee);
        }
        return NULL;
    }

    call->call_expr.func      = callee;
    call->call_expr.args      = parser_alloc_list(parser);
    call->call_expr.type_args = type_args;
    if (!call->call_expr.args)
    {
        if (type_args)
        {
            ast_list_dnit(type_args);
            free(type_args);
        }
        ast_node_dnit(call);
        free(call);
        return NULL;
    }

    // check if this is a type intrinsic that expects type arguments instead of expressions
    bool        is_type_intrinsic = false;
    const char *intrinsic_name    = NULL;
    if (callee->kind == AST_EXPR_IDENT)
    {
        intrinsic_name = callee->ident_expr.name;
        if (strcmp(intrinsic_name, "size_of") == 0 || strcmp(intrinsic_name, "align_of") == 0 || strcmp(intrinsic_name, "offset_of") == 0 || strcmp(intrinsic_name, "type_of") == 0)
        {
            is_type_intrinsic = true;
        }
    }

    if (!parser_check(parser, TOKEN_R_PAREN))
    {
        do
        {
            AstNode *arg       = NULL;
            size_t   arg_index = call->call_expr.args->count;

            if (is_type_intrinsic)
            {
                // offset_of expects the second argument to be a field name expression
                if (intrinsic_name && strcmp(intrinsic_name, "offset_of") == 0 && arg_index == 1)
                {
                    arg = parser_parse_expr(parser);
                }
                else
                {
                    // parse as type instead of expression
                    arg = parser_parse_type(parser);
                }
            }
            else
            {
                // normal expression argument
                arg = parser_parse_expr(parser);
            }

            if (!arg)
            {
                ast_node_dnit(call);
                free(call);
                return NULL;
            }
            ast_list_append(call->call_expr.args, arg);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after arguments"))
    {
        ast_node_dnit(call);
        free(call);
        return NULL;
    }

    return call;
}

// precedence climbing for expressions
static Precedence get_binary_precedence(TokenKind kind)
{
    switch (kind)
    {
    case TOKEN_EQUAL:
        return PREC_ASSIGNMENT;
    case TOKEN_PIPE_PIPE:
        return PREC_OR;
    case TOKEN_AMPERSAND_AMPERSAND:
        return PREC_AND;
    case TOKEN_PIPE:
        return PREC_BIT_OR;
    case TOKEN_CARET:
        return PREC_BIT_XOR;
    case TOKEN_AMPERSAND:
        return PREC_BIT_AND;
    case TOKEN_EQUAL_EQUAL:
    case TOKEN_BANG_EQUAL:
        return PREC_EQUALITY;
    case TOKEN_LESS:
    case TOKEN_GREATER:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER_EQUAL:
        return PREC_COMPARISON;
    case TOKEN_LESS_LESS:
    case TOKEN_GREATER_GREATER:
        return PREC_SHIFT;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return PREC_TERM;
    case TOKEN_STAR:
    case TOKEN_SLASH:
    case TOKEN_PERCENT:
        return PREC_FACTOR;
    default:
        return PREC_NONE;
    }
}

static bool is_unary_op(TokenKind kind)
{
    switch (kind)
    {
    case TOKEN_BANG:
    case TOKEN_MINUS:
    case TOKEN_PLUS:
    case TOKEN_TILDE:
    case TOKEN_QUESTION:
    case TOKEN_AT:
    case TOKEN_STAR:
    case TOKEN_AMPERSAND:
        return true;
    default:
        return false;
    }
}

// main parsing function
AstNode *parser_parse_program(Parser *parser)
{
    AstNode *program = parser_alloc_node(parser, AST_PROGRAM, NULL);
    if (!program)
    {
        return NULL;
    }

    program->program.stmts = parser_alloc_list(parser);
    if (!program->program.stmts)
    {
        ast_node_dnit(program);
        free(program);
        return NULL;
    }

    while (!parser_is_at_end(parser))
    {
        AstNode *stmt = parser_parse_stmt_top(parser);
        if (stmt)
        {
            ast_list_append(program->program.stmts, stmt);
        }
    }

    return program;
}

// parse top-level statements
AstNode *parser_parse_stmt_top(Parser *parser)
{
    bool     is_public = parser_match(parser, TOKEN_KW_PUB);
    AstNode *result    = NULL;

    switch (parser->current->kind)
    {
    case TOKEN_KW_NIL:
    {
        if (is_public)
        {
            parser_error_at_current(parser, "'pub' cannot precede 'nil'");
            parser_synchronize(parser);
            return NULL;
        }
        AstNode *nil_node = parser_alloc_node(parser, AST_EXPR_NULL, parser->current);
        if (!nil_node)
        {
            return NULL;
        }
        parser_advance(parser);
        return nil_node;
    }
    case TOKEN_KW_USE:
        if (is_public)
        {
            parser_error_at_current(parser, "'pub' cannot be applied to use statements");
            parser_synchronize(parser);
            return NULL;
        }
        result = parser_parse_stmt_use(parser);
        break;
    case TOKEN_KW_EXT:
        result = parser_parse_stmt_ext(parser, is_public);
        break;
    case TOKEN_KW_DEF:
        result = parser_parse_stmt_def(parser, is_public);
        break;
    case TOKEN_KW_VAL:
        result = parser_parse_stmt_val(parser, is_public);
        break;
    case TOKEN_KW_VAR:
        result = parser_parse_stmt_var(parser, is_public);
        break;
    case TOKEN_KW_FUN:
        result = parser_parse_stmt_fun(parser, is_public);
        break;
    case TOKEN_KW_REC:
        result = parser_parse_stmt_rec(parser, is_public);
        break;
    case TOKEN_KW_UNI:
        result = parser_parse_stmt_uni(parser, is_public);
        break;
    case TOKEN_KW_MIR:
        if (is_public)
        {
            parser_error_at_current(parser, "'pub' cannot precede 'mir' block");
        }
        result = parser_parse_stmt_mir(parser);
        break;
    case TOKEN_DOLLAR:
        if (is_public)
        {
            parser_error_at_current(parser, "'pub' cannot precede compile-time directives");
        }
        result = parser_parse_directive(parser);
        break;
    default:
        if (is_public)
        {
            parser_error_at_current(parser, "'pub' must precede a declaration");
            parser_synchronize(parser);
            return NULL;
        }
        parser_error_at_current(parser, "expected statement");
        break;
    }

    parser_synchronize(parser);
    return result;
}

// parse statements allowed at the function level
AstNode *parser_parse_stmt(Parser *parser)
{
    switch (parser->current->kind)
    {
    case TOKEN_KW_VAL:
        return parser_parse_stmt_val(parser, false);
    case TOKEN_KW_VAR:
        return parser_parse_stmt_var(parser, false);
    case TOKEN_KW_IF:
        return parser_parse_stmt_if(parser);
    case TOKEN_KW_FOR:
        return parser_parse_stmt_for(parser);
    case TOKEN_KW_BRK:
        return parser_parse_stmt_brk(parser);
    case TOKEN_KW_CNT:
        return parser_parse_stmt_cnt(parser);
    case TOKEN_KW_RET:
        return parser_parse_stmt_ret(parser);
    case TOKEN_L_BRACE:
        return parser_parse_stmt_block(parser);
    case TOKEN_KW_MIR:
        return parser_parse_stmt_mir(parser);
    case TOKEN_DOLLAR:
        return parser_parse_directive(parser);
    default:
        return parser_parse_stmt_expr(parser);
    }
}

AstNode *parser_parse_stmt_use(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_USE, "expected 'use' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_USE, parser->previous);
    if (!node)
    {
        return NULL;
    }

    // parse first identifier
    char *first = parser_parse_identifier(parser);
    if (!first)
    {
        parser_error_at_current(parser, "expected identifier after 'use'");
        parser_free_node(node);
        return NULL;
    }

    char *alias       = NULL;
    char *module_path = NULL;

    if (parser_match(parser, TOKEN_COLON))
    {
        alias      = first;
        char *head = parser_parse_identifier(parser);
        if (!head)
        {
            parser_error_at_current(parser, "expected module name after alias colon");
            free(alias);
            parser_free_node(node);
            return NULL;
        }
        module_path = head;
    }
    else
    {
        module_path = first;
    }

    // parse rest of module path
    while (parser_match(parser, TOKEN_DOT))
    {
        char *next = parser_parse_identifier(parser);
        if (!next)
        {
            parser_error_at_current(parser, "expected identifier after '.'");
            free(alias);
            free(module_path);
            parser_free_node(node);
            return NULL;
        }

        size_t len      = strlen(module_path) + strlen(next) + 2;
        char  *new_path = malloc(len);
        if (!new_path)
        {
            parser_report_alloc_failure(parser, "out of memory expanding module path");
            free(alias);
            free(module_path);
            free(next);
            parser_free_node(node);
            return NULL;
        }
        snprintf(new_path, len, "%s.%s", module_path, next);
        free(module_path);
        free(next);
        module_path = new_path;
    }

    node->use_stmt.module_path = module_path;
    node->use_stmt.alias       = alias;

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after use statement"))
    {
        parser_free_node(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_ext(Parser *parser, bool is_public)
{
    if (!parser_consume(parser, TOKEN_KW_EXT, "expected 'ext' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_EXT, parser->previous);
    if (!node)
    {
        return NULL;
    }

    // initialize fields
    node->ext_stmt.name       = NULL;
    node->ext_stmt.convention = NULL;
    node->ext_stmt.symbol     = NULL;
    node->ext_stmt.type       = NULL;
    node->ext_stmt.is_public  = is_public;

    // check for optional calling convention/symbol specification
    if (parser_match(parser, TOKEN_LIT_STRING))
    {
        char *raw = lexer_raw_value(parser->lexer, parser->previous);
        if (!raw)
        {
            parser_report_alloc_failure(parser, "out of memory reading extern metadata");
            parser_free_node(node);
            return NULL;
        }

        size_t raw_len   = strlen(raw);
        size_t copy_len  = raw_len >= 2 ? raw_len - 2 : 0; // remove quotes safely
        char  *conv_spec = malloc(copy_len + 1);
        if (!conv_spec)
        {
            free(raw);
            parser_report_alloc_failure(parser, "out of memory parsing extern metadata");
            parser_free_node(node);
            return NULL;
        }

        memcpy(conv_spec, raw + 1, copy_len);
        conv_spec[copy_len] = '\0';
        free(raw);

        // parse "convention:symbol" or just "convention"
        char *colon = strchr(conv_spec, ':');
        if (colon)
        {
            *colon                    = '\0';
            node->ext_stmt.convention = parser_strdup_checked(parser, conv_spec, "out of memory duplicating convention");
            if (!node->ext_stmt.convention)
            {
                free(conv_spec);
                parser_free_node(node);
                return NULL;
            }
            node->ext_stmt.symbol = parser_strdup_checked(parser, colon + 1, "out of memory duplicating external symbol");
            if (!node->ext_stmt.symbol)
            {
                free(conv_spec);
                parser_free_node(node);
                return NULL;
            }
        }
        else
        {
            node->ext_stmt.convention = parser_strdup_checked(parser, conv_spec, "out of memory duplicating convention");
            if (!node->ext_stmt.convention)
            {
                free(conv_spec);
                parser_free_node(node);
                return NULL;
            }
        }

        free(conv_spec);
    }

    node->ext_stmt.name = parser_parse_identifier(parser);
    if (!node->ext_stmt.name)
    {
        parser_error_at_current(parser, "expected identifier after 'ext'");
        parser_free_node(node);
        return NULL;
    }

    // if no symbol specified, use the function name
    if (!node->ext_stmt.symbol)
    {
        node->ext_stmt.symbol = parser_strdup_checked(parser, node->ext_stmt.name, "out of memory duplicating external symbol");
        if (!node->ext_stmt.symbol)
        {
            parser_free_node(node);
            return NULL;
        }
    }

    // if no convention specified, default to "C"
    if (!node->ext_stmt.convention)
    {
        node->ext_stmt.convention = parser_strdup_checked(parser, "C", "out of memory duplicating convention");
        if (!node->ext_stmt.convention)
        {
            parser_free_node(node);
            return NULL;
        }
    }

    if (!parser_consume(parser, TOKEN_COLON, "expected ':' after external name"))
    {
        parser_free_node(node);
        return NULL;
    }

    node->ext_stmt.type = parser_parse_type(parser);
    if (!node->ext_stmt.type)
    {
        parser_error_at_current(parser, "expected type after ':' in external statement");
        parser_free_node(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after external statement"))
    {
        parser_free_node(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_def(Parser *parser, bool is_public)
{
    if (!parser_consume(parser, TOKEN_KW_DEF, "expected 'def' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_DEF, parser->previous);
    if (!node)
    {
        return NULL;
    }

    node->def_stmt.name = parser_parse_identifier(parser);
    if (!node->def_stmt.name)
    {
        parser_error_at_current(parser, "expected identifier after 'def'");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_COLON, "expected ':' after definition name"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->def_stmt.type = parser_parse_type(parser);
    if (!node->def_stmt.type)
    {
        parser_error_at_current(parser, "expected type after ':' in definition statement");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after type definition"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->def_stmt.is_public = is_public;

    return node;
}

// helper for val/var parsing since they're so similar
static AstNode *parser_parse_var_decl(Parser *parser, bool is_val, bool is_public)
{
    TokenKind keyword = is_val ? TOKEN_KW_VAL : TOKEN_KW_VAR;

    if (!parser_consume(parser, keyword, "expected keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, is_val ? AST_STMT_VAL : AST_STMT_VAR, parser->previous);
    if (!node)
    {
        return NULL;
    }

    node->var_stmt.is_val    = is_val;
    node->var_stmt.is_public = is_public;
    node->var_stmt.name      = parser_parse_identifier(parser);
    if (!node->var_stmt.name)
    {
        parser_error_at_current(parser, "expected identifier after keyword");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_COLON, "expected ':' after name"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->var_stmt.type = parser_parse_type(parser);
    if (!node->var_stmt.type)
    {
        parser_error_at_current(parser, "expected type after ':'");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    // val requires initialization, var is optional
    if (is_val)
    {
        if (!parser_consume(parser, TOKEN_EQUAL, "expected '=' in val statement"))
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
        node->var_stmt.init = parser_parse_expr(parser);
        if (!node->var_stmt.init)
        {
            parser_error_at_current(parser, "expected expression after '='");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }
    else if (parser_match(parser, TOKEN_EQUAL))
    {
        node->var_stmt.init = parser_parse_expr(parser);
        if (!node->var_stmt.init)
        {
            parser_error_at_current(parser, "expected expression after '='");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after statement"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_val(Parser *parser, bool is_public)
{
    return parser_parse_var_decl(parser, true, is_public);
}

AstNode *parser_parse_stmt_var(Parser *parser, bool is_public)
{
    return parser_parse_var_decl(parser, false, is_public);
}

AstNode *parser_parse_stmt_fun(Parser *parser, bool is_public)
{
    if (!parser_consume(parser, TOKEN_KW_FUN, "expected 'fun' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_FUN, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (parser_check(parser, TOKEN_L_PAREN))
    {
        parser_advance(parser); // consume '('

        char *receiver_name = parser_parse_identifier(parser);
        if (!receiver_name)
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        if (!parser_consume(parser, TOKEN_COLON, "expected ':' after method receiver name"))
        {
            free(receiver_name);
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        AstNode *receiver_type = parser_parse_type(parser);
        if (!receiver_type)
        {
            free(receiver_name);
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after method receiver"))
        {
            free(receiver_name);
            ast_node_dnit(receiver_type);
            free(receiver_type);
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        node->fun_stmt.is_method            = true;
        node->fun_stmt.method_receiver      = receiver_type;
        node->fun_stmt.method_receiver_name = receiver_name;

        node->fun_stmt.name = parser_parse_identifier(parser);
        if (!node->fun_stmt.name)
        {
            parser_error_at_current(parser, "expected method name after receiver");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }
    else
    {
        node->fun_stmt.name = parser_parse_identifier(parser);
        if (!node->fun_stmt.name)
        {
            parser_error_at_current(parser, "expected identifier after 'fun'");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    if (parser_match(parser, TOKEN_L_BRACKET))
    {
        node->fun_stmt.generics = parser_parse_generic_param_list(parser);
        if (!node->fun_stmt.generics)
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    if (!parser_consume(parser, TOKEN_L_PAREN, "expected '(' after function name"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    // parse parameter list with optional variadic '...'
    node->fun_stmt.params      = parser_alloc_list(parser);
    node->fun_stmt.is_variadic = false;
    if (!node->fun_stmt.params)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (node->fun_stmt.is_method)
    {
        AstNode *receiver_param = parser_alloc_node(parser, AST_STMT_PARAM, parser->current);
        if (!receiver_param)
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        receiver_param->param_stmt.is_variadic = false;
        receiver_param->param_stmt.name        = strdup(node->fun_stmt.method_receiver_name ? node->fun_stmt.method_receiver_name : "");
        if (!receiver_param->param_stmt.name)
        {
            ast_node_dnit(receiver_param);
            free(receiver_param);
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        receiver_param->param_stmt.type = ast_clone(node->fun_stmt.method_receiver);
        if (!receiver_param->param_stmt.type)
        {
            ast_node_dnit(receiver_param);
            free(receiver_param);
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        ast_list_prepend(node->fun_stmt.params, receiver_param);
    }

    if (!parser_check(parser, TOKEN_R_PAREN))
    {
        do
        {
            if (parser_check(parser, TOKEN_ELLIPSIS))
            {
                if (node->fun_stmt.is_variadic)
                {
                    parser_error_at_current(parser, "multiple '...' in parameter list");
                }
                parser_advance(parser);
                node->fun_stmt.is_variadic = true;
                // ellipsis must be last
                if (!parser_check(parser, TOKEN_R_PAREN))
                {
                    parser_error_at_current(parser, "'...' must be last parameter");
                }
                break;
            }

            AstNode *param = parser_alloc_node(parser, AST_STMT_PARAM, parser->current);
            if (!param)
            {
                ast_node_dnit(node);
                free(node);
                return NULL;
            }

            param->param_stmt.is_variadic = false;
            param->param_stmt.name        = parser_parse_identifier(parser);
            if (!param->param_stmt.name)
            {
                ast_node_dnit(param);
                free(param);
                ast_node_dnit(node);
                free(node);
                return NULL;
            }

            if (!parser_consume(parser, TOKEN_COLON, "expected ':' after parameter name"))
            {
                ast_node_dnit(param);
                free(param);
                ast_node_dnit(node);
                free(node);
                return NULL;
            }

            param->param_stmt.type = parser_parse_type(parser);
            if (!param->param_stmt.type)
            {
                ast_node_dnit(param);
                free(param);
                ast_node_dnit(node);
                free(node);
                return NULL;
            }

            ast_list_append(node->fun_stmt.params, param);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after parameters"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->fun_stmt.is_public = is_public;

    // optional return type
    if (!parser_check(parser, TOKEN_L_BRACE))
    {
        node->fun_stmt.return_type = parser_parse_type(parser);
        if (!node->fun_stmt.return_type)
        {
            parser_error_at_current(parser, "expected return type or '{' after function parameters");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    node->fun_stmt.body = parser_parse_stmt_block(parser);
    if (!node->fun_stmt.body)
    {
        parser_error_at_current(parser, "expected function body");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_rec(Parser *parser, bool is_public)
{
    if (!parser_consume(parser, TOKEN_KW_REC, "expected 'rec' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_REC, parser->previous);
    if (!node)
    {
        return NULL;
    }

    node->rec_stmt.name = parser_parse_identifier(parser);
    if (!node->rec_stmt.name)
    {
        parser_error_at_current(parser, "expected identifier after 'rec'");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    // parse optional generic parameters
    node->rec_stmt.generics = NULL;
    if (parser_match(parser, TOKEN_L_BRACKET))
    {
        node->rec_stmt.generics = parser_parse_generic_param_list(parser);
        if (!node->rec_stmt.generics)
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' after record name"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->rec_stmt.fields = parser_parse_field_list(parser);
    if (!node->rec_stmt.fields)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after record fields"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->rec_stmt.is_public = is_public;

    return node;
}

AstNode *parser_parse_stmt_uni(Parser *parser, bool is_public)
{
    if (!parser_consume(parser, TOKEN_KW_UNI, "expected 'uni' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_UNI, parser->previous);
    if (!node)
    {
        return NULL;
    }

    node->uni_stmt.name = parser_parse_identifier(parser);
    if (!node->uni_stmt.name)
    {
        parser_error_at_current(parser, "expected identifier after 'uni'");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    // parse optional generic parameters
    node->uni_stmt.generics = NULL;
    if (parser_match(parser, TOKEN_L_BRACKET))
    {
        node->uni_stmt.generics = parser_parse_generic_param_list(parser);
        if (!node->uni_stmt.generics)
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' after union name"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->uni_stmt.fields = parser_parse_field_list(parser);
    if (!node->uni_stmt.fields)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after union fields"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->uni_stmt.is_public = is_public;

    return node;
}

AstNode *parser_parse_stmt_if(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_IF, "expected 'if' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_IF, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_L_PAREN, "expected '(' after 'if'"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->cond_stmt.cond = parser_parse_expr(parser);
    if (!node->cond_stmt.cond)
    {
        parser_error_at_current(parser, "expected condition expression");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after condition"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->cond_stmt.body = parser_parse_stmt_block(parser);
    if (!node->cond_stmt.body)
    {
        parser_error_at_current(parser, "expected block after 'if' condition");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    // handle or chains
    AstNode **tail = &node->cond_stmt.stmt_or;
    while (parser_match(parser, TOKEN_KW_OR))
    {
        AstNode *or_node = parser_alloc_node(parser, AST_STMT_OR, parser->previous);
        if (!or_node)
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        // optional condition
        if (parser_match(parser, TOKEN_L_PAREN))
        {
            or_node->cond_stmt.cond = parser_parse_expr(parser);
            if (!or_node->cond_stmt.cond)
            {
                parser_error_at_current(parser, "expected condition expression");
                ast_node_dnit(or_node);
                free(or_node);
                ast_node_dnit(node);
                free(node);
                return NULL;
            }

            if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after 'or' condition"))
            {
                ast_node_dnit(or_node);
                free(or_node);
                ast_node_dnit(node);
                free(node);
                return NULL;
            }
        }

        or_node->cond_stmt.body = parser_parse_stmt_block(parser);
        if (!or_node->cond_stmt.body)
        {
            parser_error_at_current(parser, "expected block after 'or'");
            ast_node_dnit(or_node);
            free(or_node);
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        *tail = or_node;
        tail  = &or_node->cond_stmt.stmt_or;
    }

    return node;
}

AstNode *parser_parse_stmt_for(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_FOR, "expected 'for' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_FOR, parser->previous);
    if (!node)
    {
        return NULL;
    }

    // optional condition
    if (parser_match(parser, TOKEN_L_PAREN))
    {
        node->for_stmt.cond = parser_parse_expr(parser);
        if (!node->for_stmt.cond)
        {
            parser_error_at_current(parser, "expected loop condition");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }

        if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after loop condition"))
        {
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    node->for_stmt.body = parser_parse_stmt_block(parser);
    if (!node->for_stmt.body)
    {
        parser_error_at_current(parser, "expected block after 'for'");
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_brk(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_BRK, "expected 'brk' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_BRK, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after 'brk'"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_cnt(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_CNT, "expected 'cnt' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_CNT, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after 'cnt'"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_ret(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_RET, "expected 'ret' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_RET, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (!parser_check(parser, TOKEN_SEMICOLON))
    {
        node->ret_stmt.expr = parser_parse_expr(parser);
        if (!node->ret_stmt.expr)
        {
            parser_error_at_current(parser, "expected return value");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after return"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_mir(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_KW_MIR, "expected 'mir' keyword"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_MIR, parser->previous);
    if (!node)
    {
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' after 'mir'"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    // capture starting position after '{' using character position in source
    int start_pos   = parser->current->pos;
    int brace_depth = 1;

    // scan until matching closing brace
    while (brace_depth > 0 && !parser_is_at_end(parser))
    {
        if (parser_check(parser, TOKEN_L_BRACE))
        {
            brace_depth++;
        }
        else if (parser_check(parser, TOKEN_R_BRACE))
        {
            brace_depth--;
            if (brace_depth == 0)
            {
                break;
            }
        }
        parser_advance(parser);
    }

    // calculate length of content between braces
    int end_pos = parser->current->pos;
    int len     = end_pos - start_pos;

    // extract raw content
    node->mir_stmt.content = (char *)malloc(len + 1);
    if (!node->mir_stmt.content)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }
    memcpy(node->mir_stmt.content, parser->lexer->source + start_pos, len);
    node->mir_stmt.content[len] = '\0';

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after mir block"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_block(Parser *parser)
{
    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{'"))
    {
        return NULL;
    }

    AstNode *node = parser_alloc_node(parser, AST_STMT_BLOCK, parser->previous);
    if (!node)
    {
        return NULL;
    }

    node->block_stmt.stmts = parser_alloc_list(parser);
    if (!node->block_stmt.stmts)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    node->block_stmt.deferred_stmts = parser_alloc_list(parser);
    if (!node->block_stmt.deferred_stmts)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        if (parser_match(parser, TOKEN_KW_FIN))
        {
            AstNode *stmt = parser_parse_stmt(parser);
            if (!stmt)
            {
                parser_error_at_current(parser, "expected statement after 'fin'");
                ast_node_dnit(node);
                free(node);
                return NULL;
            }
            ast_list_append(node->block_stmt.deferred_stmts, stmt);
            continue;
        }

        AstNode *stmt = parser_parse_stmt(parser);
        if (!stmt)
        {
            parser_error_at_current(parser, "expected statement in block");
            ast_node_dnit(node);
            free(node);
            return NULL;
        }
        // attach trailing 'or' branches if present and not already consumed
        if (stmt->kind == AST_STMT_IF)
        {
            AstNode **tail = &stmt->cond_stmt.stmt_or;
            while (parser_match(parser, TOKEN_KW_OR))
            {
                AstNode *or_node = parser_alloc_node(parser, AST_STMT_OR, parser->previous);
                if (!or_node)
                {
                    ast_node_dnit(stmt);
                    free(stmt);
                    ast_node_dnit(node);
                    free(node);
                    return NULL;
                }

                // optional condition
                if (parser_match(parser, TOKEN_L_PAREN))
                {
                    or_node->cond_stmt.cond = parser_parse_expr(parser);
                    if (!or_node->cond_stmt.cond)
                    {
                        parser_error_at_current(parser, "expected condition expression");
                        ast_node_dnit(or_node);
                        free(or_node);
                        ast_node_dnit(stmt);
                        free(stmt);
                        ast_node_dnit(node);
                        free(node);
                        return NULL;
                    }

                    if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after 'or' condition"))
                    {
                        ast_node_dnit(or_node);
                        free(or_node);
                        ast_node_dnit(stmt);
                        free(stmt);
                        ast_node_dnit(node);
                        free(node);
                        return NULL;
                    }
                }

                or_node->cond_stmt.body = parser_parse_stmt_block(parser);
                if (!or_node->cond_stmt.body)
                {
                    parser_error_at_current(parser, "expected block after 'or'");
                    ast_node_dnit(or_node);
                    free(or_node);
                    ast_node_dnit(stmt);
                    free(stmt);
                    ast_node_dnit(node);
                    free(node);
                    return NULL;
                }

                *tail = or_node;
                tail  = &or_node->cond_stmt.stmt_or;
            }
        }
        ast_list_append(node->block_stmt.stmts, stmt);
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after block"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

AstNode *parser_parse_stmt_expr(Parser *parser)
{
    AstNode *node = parser_alloc_node(parser, AST_STMT_EXPR, parser->current);
    if (!node)
    {
        return NULL;
    }

    node->expr_stmt.expr = parser_parse_expr(parser);
    if (!node->expr_stmt.expr)
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_SEMICOLON, "expected ';' after expression"))
    {
        ast_node_dnit(node);
        free(node);
        return NULL;
    }

    return node;
}

// expression parsing
AstNode *parser_parse_expr(Parser *parser)
{
    return parser_parse_expr_prec(parser, PREC_ASSIGNMENT);
}

AstNode *parser_parse_expr_prec(Parser *parser, Precedence min_prec)
{
    AstNode *left = parser_parse_expr_prefix(parser);
    if (!left)
    {
        return NULL;
    }

    while (!parser_is_at_end(parser))
    {
        Precedence prec = get_binary_precedence(parser->current->kind);
        if (prec < min_prec)
        {
            break;
        }

        Token op = *parser->current;
        parser_advance(parser);

        AstNode *right = parser_parse_expr_prec(parser, prec + 1);
        if (!right)
        {
            ast_node_dnit(left);
            free(left);
            return NULL;
        }

        AstNode *binary = parser_alloc_node(parser, AST_EXPR_BINARY, &op);
        if (!binary)
        {
            ast_node_dnit(left);
            free(left);
            ast_node_dnit(right);
            free(right);
            return NULL;
        }

        binary->binary_expr.left  = left;
        binary->binary_expr.right = right;
        binary->binary_expr.op    = op.kind;
        left                      = binary;
    }

    return left;
}

AstNode *parser_parse_expr_prefix(Parser *parser)
{
    if (is_unary_op(parser->current->kind))
    {
        Token op = *parser->current;
        parser_advance(parser);

        AstNode *expr = parser_parse_expr_prec(parser, PREC_UNARY);
        if (!expr)
        {
            return NULL;
        }

        AstNode *unary = parser_alloc_node(parser, AST_EXPR_UNARY, &op);
        if (!unary)
        {
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        unary->unary_expr.expr = expr;
        unary->unary_expr.op   = op.kind;
        return unary;
    }

    return parser_parse_expr_postfix(parser);
}

static AstNode *parser_expr_to_type(Parser *parser, AstNode *expr, AstList *generic_args)
{
    if (!expr)
    {
        if (generic_args)
        {
            ast_list_dnit(generic_args);
            free(generic_args);
        }
        return NULL;
    }

    AstNode *type = NULL;

    switch (expr->kind)
    {
    case AST_EXPR_IDENT:
    {
        type = parser_alloc_node(parser, AST_TYPE_NAME, expr->token);
        if (!type)
        {
            if (generic_args)
            {
                ast_list_dnit(generic_args);
                free(generic_args);
            }
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        type->type_name.module_alias = NULL;
        type->type_name.name         = expr->ident_expr.name;
        type->type_name.generic_args = generic_args;
        expr->ident_expr.name        = NULL;
        break;
    }
    case AST_EXPR_FIELD:
    {
        AstNode *object = expr->field_expr.object;
        if (!object || object->kind != AST_EXPR_IDENT)
        {
            parser_error(parser, expr->token ? expr->token : parser->previous, "invalid type name");
            if (generic_args)
            {
                ast_list_dnit(generic_args);
                free(generic_args);
            }
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        type = parser_alloc_node(parser, AST_TYPE_NAME, expr->token);
        if (!type)
        {
            if (generic_args)
            {
                ast_list_dnit(generic_args);
                free(generic_args);
            }
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        type->type_name.module_alias = object->ident_expr.name;
        type->type_name.name         = expr->field_expr.field;
        type->type_name.generic_args = generic_args;

        object->ident_expr.name = NULL;
        expr->field_expr.field  = NULL;

        ast_node_dnit(object);
        free(object);
        expr->field_expr.object = NULL;
        break;
    }
    case AST_EXPR_INDEX:
    {
        if (generic_args)
        {
            parser_error(parser, expr->token ? expr->token : parser->previous, "unexpected nested type arguments");
            ast_list_dnit(generic_args);
            free(generic_args);
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        AstNode *base_expr     = expr->index_expr.array;
        AstNode *index_expr    = expr->index_expr.index;
        expr->index_expr.array = NULL;
        expr->index_expr.index = NULL;

        AstNode *base_type = parser_expr_to_type(parser, base_expr, NULL);
        if (!base_type)
        {
            if (index_expr)
            {
                ast_node_dnit(index_expr);
                free(index_expr);
            }
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        AstList *type_args = parser_alloc_list(parser);
        if (!type_args)
        {
            if (index_expr)
            {
                ast_node_dnit(index_expr);
                free(index_expr);
            }
            ast_node_dnit(base_type);
            free(base_type);
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        AstNode *type_arg = parser_expr_to_type(parser, index_expr, NULL);
        if (!type_arg)
        {
            ast_list_dnit(type_args);
            free(type_args);
            ast_node_dnit(base_type);
            free(base_type);
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        ast_list_append(type_args, type_arg);

        if (base_type->kind != AST_TYPE_NAME)
        {
            parser_error(parser, expr->token ? expr->token : parser->previous, "invalid type expression");
            ast_list_dnit(type_args);
            free(type_args);
            ast_node_dnit(base_type);
            free(base_type);
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }

        if (base_type->type_name.generic_args)
        {
            AstList *existing = base_type->type_name.generic_args;
            for (int i = 0; i < type_args->count; i++)
            {
                ast_list_append(existing, type_args->items[i]);
                type_args->items[i] = NULL;
            }
            free(type_args->items);
            free(type_args);
        }
        else
        {
            base_type->type_name.generic_args = type_args;
        }

        type = base_type;
        break;
    }
    default:
        parser_error(parser, expr->token ? expr->token : parser->previous, "expected type name");
        if (generic_args)
        {
            ast_list_dnit(generic_args);
            free(generic_args);
        }
        ast_node_dnit(expr);
        free(expr);
        return NULL;
    }

    ast_node_dnit(expr);
    free(expr);
    return type;
}

AstNode *parser_parse_expr_postfix(Parser *parser)
{
    AstNode *expr = parser_parse_expr_atom(parser);
    if (!expr)
    {
        return NULL;
    }

    for (;;)
    {
        TokenKind follow_kind = TOKEN_ERROR;
        if (parser_check(parser, TOKEN_L_BRACKET) && parser_should_parse_type_args(parser, &follow_kind))
        {
            AstList *type_args = parser_parse_type_arguments(parser);
            if (!type_args)
            {
                ast_node_dnit(expr);
                free(expr);
                return NULL;
            }

            if (follow_kind == TOKEN_L_PAREN)
            {
                if (!parser_consume(parser, TOKEN_L_PAREN, "expected '(' after type arguments"))
                {
                    ast_list_dnit(type_args);
                    free(type_args);
                    ast_node_dnit(expr);
                    free(expr);
                    return NULL;
                }

                AstNode *call = parser_finish_call(parser, expr, type_args);
                if (!call)
                {
                    return NULL;
                }

                expr = call;
                continue;
            }
            else if (follow_kind == TOKEN_L_BRACE)
            {
                AstNode *type = parser_expr_to_type(parser, expr, type_args);
                if (!type)
                {
                    return NULL;
                }

                expr = parser_parse_typed_literal(parser, type);
                if (!expr)
                {
                    return NULL;
                }

                continue;
            }
            else
            {
                parser_error_at_current(parser, "unexpected token after type arguments");
                ast_list_dnit(type_args);
                free(type_args);
                ast_node_dnit(expr);
                free(expr);
                return NULL;
            }
        }
        else if (parser_match(parser, TOKEN_L_PAREN))
        {
            AstNode *call = parser_finish_call(parser, expr, NULL);
            if (!call)
            {
                return NULL;
            }

            expr = call;
        }
        else if (parser_match(parser, TOKEN_L_BRACKET))
        {
            // array index
            AstNode *index_expr = parser_parse_expr(parser);
            if (!index_expr)
            {
                ast_node_dnit(expr);
                free(expr);
                return NULL;
            }

            if (!parser_consume(parser, TOKEN_R_BRACKET, "expected ']' after index"))
            {
                ast_node_dnit(expr);
                free(expr);
                ast_node_dnit(index_expr);
                free(index_expr);
                return NULL;
            }

            AstNode *index = parser_alloc_node(parser, AST_EXPR_INDEX, parser->previous);
            if (!index)
            {
                ast_node_dnit(expr);
                free(expr);
                ast_node_dnit(index_expr);
                free(index_expr);
                return NULL;
            }

            index->index_expr.array = expr;
            index->index_expr.index = index_expr;
            expr                    = index;
        }
        else if (parser_match(parser, TOKEN_DOT))
        {
            // field access
            char *field = parser_parse_identifier(parser);
            if (!field)
            {
                ast_node_dnit(expr);
                free(expr);
                return NULL;
            }

            AstNode *field_expr = parser_alloc_node(parser, AST_EXPR_FIELD, parser->previous);
            if (!field_expr)
            {
                free(field);
                ast_node_dnit(expr);
                free(expr);
                return NULL;
            }

            field_expr->field_expr.object = expr;
            field_expr->field_expr.field  = field;
            expr                          = field_expr;
        }
        else if (parser_match(parser, TOKEN_COLON_COLON))
        {
            // type cast
            AstNode *type = parser_parse_type(parser);
            if (!type)
            {
                ast_node_dnit(expr);
                free(expr);
                return NULL;
            }

            AstNode *cast = parser_alloc_node(parser, AST_EXPR_CAST, parser->previous);
            if (!cast)
            {
                ast_node_dnit(expr);
                free(expr);
                ast_node_dnit(type);
                free(type);
                return NULL;
            }

            cast->cast_expr.expr = expr;
            cast->cast_expr.type = type;
            expr                 = cast;
        }
        else if (parser_check(parser, TOKEN_L_BRACE))
        {
            AstNode *type = parser_expr_to_type(parser, expr, NULL);
            if (!type)
            {
                return NULL;
            }

            expr = parser_parse_typed_literal(parser, type);
            if (!expr)
            {
                return NULL;
            }
        }
        else
        {
            break;
        }
    }

    return expr;
}

AstNode *parser_parse_expr_atom(Parser *parser)
{
    switch (parser->current->kind)
    {
    case TOKEN_LIT_INT:
    {
        AstNode *lit = parser_alloc_node(parser, AST_EXPR_LIT, parser->current);
        if (!lit)
        {
            return NULL;
        }
        lit->lit_expr.kind    = TOKEN_LIT_INT;
        char *raw             = lexer_raw_value(parser->lexer, parser->current);
        lit->lit_expr.int_val = strtoull(raw, NULL, 0);
        free(raw);
        parser_advance(parser);
        return lit;
    }

    case TOKEN_LIT_FLOAT:
    {
        AstNode *lit = parser_alloc_node(parser, AST_EXPR_LIT, parser->current);
        if (!lit)
        {
            return NULL;
        }
        lit->lit_expr.kind      = TOKEN_LIT_FLOAT;
        char *raw               = lexer_raw_value(parser->lexer, parser->current);
        lit->lit_expr.float_val = strtod(raw, NULL);
        free(raw);
        parser_advance(parser);
        return lit;
    }

    case TOKEN_LIT_CHAR:
    {
        AstNode *lit = parser_alloc_node(parser, AST_EXPR_LIT, parser->current);
        if (!lit)
        {
            return NULL;
        }
        lit->lit_expr.kind     = TOKEN_LIT_CHAR;
        lit->lit_expr.char_val = lexer_eval_lit_char(parser->lexer, parser->current);
        parser_advance(parser);
        return lit;
    }

    case TOKEN_LIT_STRING:
    {
        AstNode *lit = parser_alloc_node(parser, AST_EXPR_LIT, parser->current);
        if (!lit)
        {
            return NULL;
        }
        lit->lit_expr.kind       = TOKEN_LIT_STRING;
        lit->lit_expr.string_val = lexer_eval_lit_string(parser->lexer, parser->current);
        parser_advance(parser);
        return lit;
    }

    case TOKEN_L_PAREN:
    {
        parser_advance(parser);
        AstNode *expr = parser_parse_expr(parser);
        if (!expr)
        {
            return NULL;
        }
        if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after expression"))
        {
            ast_node_dnit(expr);
            free(expr);
            return NULL;
        }
        return expr;
    }

    case TOKEN_L_BRACKET:
    {
        parser_advance(parser);
        return parser_parse_array_literal(parser);
    }

    case TOKEN_KW_NIL:
    {
        AstNode *lit = parser_alloc_node(parser, AST_EXPR_NULL, parser->current);
        if (!lit)
        {
            return NULL;
        }
        parser_advance(parser);
        return lit;
    }

    case TOKEN_ELLIPSIS:
    {
        AstNode *pack = parser_alloc_node(parser, AST_EXPR_VARARGS, parser->current);
        if (!pack)
        {
            return NULL;
        }
        parser_advance(parser);
        return pack;
    }

    case TOKEN_KW_REC:
        parser_advance(parser);
        return parser_parse_anonymous_composite_literal(parser, false);

    case TOKEN_KW_UNI:
        parser_advance(parser);
        return parser_parse_anonymous_composite_literal(parser, true);

    case TOKEN_DOLLAR:
    {
        // parse compile-time expression: $size_of(T), $mach.os.linux, $mach.build.target.os, etc.
        Token dollar_token = *parser->current;
        parser_advance(parser);

        // parse the prefix expression after $ (not full expression with binary ops)
        AstNode *inner = parser_parse_expr_prefix(parser);
        if (!inner)
        {
            return NULL;
        }

        // wrap it in AST_COMPTIME
        AstNode *comptime = parser_alloc_node(parser, AST_COMPTIME, &dollar_token);
        if (!comptime)
        {
            ast_node_dnit(inner);
            free(inner);
            return NULL;
        }

        comptime->comptime.inner = inner;
        return comptime;
    }

    case TOKEN_IDENTIFIER:
    {
        AstNode *ident = parser_alloc_node(parser, AST_EXPR_IDENT, parser->current);
        if (!ident)
        {
            return NULL;
        }
        ident->ident_expr.name = lexer_raw_value(parser->lexer, parser->current);
        parser_advance(parser);
        return ident;
    }

    default:
        parser_error_at_current(parser, "expected expression");
        return NULL;
    }
}

AstNode *parser_parse_array_literal(Parser *parser)
{
    AstNode *array = parser_alloc_node(parser, AST_EXPR_ARRAY, parser->previous);
    if (!array)
    {
        return NULL;
    }

    // parse array type
    array->array_expr.type = parser_alloc_node(parser, AST_TYPE_ARRAY, parser->previous);
    if (!array->array_expr.type)
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    // require explicit length
    if (parser_check(parser, TOKEN_R_BRACKET))
    {
        parser_error_at_current(parser, "array literal requires explicit length (use [N]T)");
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    array->array_expr.type->type_array.size = parser_parse_expr(parser);
    if (!array->array_expr.type->type_array.size)
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_R_BRACKET, "expected ']' in array type"))
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    array->array_expr.type->type_array.elem_type = parser_parse_type(parser);
    if (!array->array_expr.type->type_array.elem_type)
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' to start array literal"))
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    array->array_expr.elems = parser_alloc_list(parser);
    if (!array->array_expr.elems)
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_BRACE))
    {
        do
        {
            AstNode *elem = parser_parse_expr(parser);
            if (!elem)
            {
                ast_node_dnit(array);
                free(array);
                return NULL;
            }
            ast_list_append(array->array_expr.elems, elem);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after array elements"))
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    return array;
}

AstNode *parser_parse_struct_literal(Parser *parser, AstNode *type)
{
    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' to start record literal"))
    {
        ast_node_dnit(type);
        free(type);
        return NULL;
    }

    AstNode *struct_lit = parser_alloc_node(parser, AST_EXPR_STRUCT, parser->previous);
    if (!struct_lit)
    {
        ast_node_dnit(type);
        free(type);
        return NULL;
    }

    struct_lit->struct_expr.type   = type;
    struct_lit->struct_expr.fields = parser_alloc_list(parser);
    if (!struct_lit->struct_expr.fields)
    {
        ast_node_dnit(struct_lit);
        free(struct_lit);
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_BRACE))
    {
        do
        {
            // field initializer: name = expr
            char *field_name = parser_parse_identifier(parser);
            if (!field_name)
            {
                ast_node_dnit(struct_lit);
                free(struct_lit);
                return NULL;
            }

            if (!parser_consume(parser, TOKEN_COLON, "expected ':' after field name"))
            {
                free(field_name);
                ast_node_dnit(struct_lit);
                free(struct_lit);
                return NULL;
            }

            AstNode *init = parser_parse_expr(parser);
            if (!init)
            {
                free(field_name);
                ast_node_dnit(struct_lit);
                free(struct_lit);
                return NULL;
            }

            // store as field access node for easier processing
            AstNode *field = parser_alloc_node(parser, AST_EXPR_FIELD, parser->previous);
            if (!field)
            {
                free(field_name);
                ast_node_dnit(init);
                free(init);
                ast_node_dnit(struct_lit);
                free(struct_lit);
                return NULL;
            }

            field->field_expr.field  = field_name;
            field->field_expr.object = init;
            ast_list_append(struct_lit->struct_expr.fields, field);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after record fields"))
    {
        ast_node_dnit(struct_lit);
        free(struct_lit);
        return NULL;
    }

    return struct_lit;
}

// type parsing
AstNode *parser_parse_type(Parser *parser)
{
    switch (parser->current->kind)
    {
    case TOKEN_KW_FUN:
        parser_advance(parser);
        return parser_parse_type_fun(parser);

    case TOKEN_KW_REC:
        parser_advance(parser);
        return parser_parse_type_rec(parser);

    case TOKEN_KW_UNI:
        parser_advance(parser);
        return parser_parse_type_uni(parser);

    case TOKEN_STAR:
    case TOKEN_AMPERSAND:
        parser_advance(parser);
        return parser_parse_type_ptr(parser);

    case TOKEN_AMPERSAND_AMPERSAND:
    {
        parser_advance(parser);
        AstNode *outer = parser_alloc_node(parser, AST_TYPE_PTR, parser->previous);
        if (!outer)
        {
            return NULL;
        }
        outer->type_ptr.is_read_only = true;

        AstNode *inner = parser_alloc_node(parser, AST_TYPE_PTR, parser->previous);
        if (!inner)
        {
            ast_node_dnit(outer);
            free(outer);
            return NULL;
        }
        inner->type_ptr.is_read_only = true;

        outer->type_ptr.base = inner;
        inner->type_ptr.base = parser_parse_type(parser);

        if (!inner->type_ptr.base)
        {
            ast_node_dnit(outer);
            free(outer);
            return NULL;
        }

        return outer;
    }

    case TOKEN_L_BRACKET:
        parser_advance(parser);
        return parser_parse_type_array(parser);

    case TOKEN_IDENTIFIER:
        return parser_parse_type_name(parser);

    default:
        parser_error_at_current(parser, "expected type");
        return NULL;
    }
}

AstNode *parser_parse_type_name(Parser *parser)
{
    AstNode *type = parser_alloc_node(parser, AST_TYPE_NAME, parser->current);
    if (!type)
    {
        return NULL;
    }

    type->type_name.module_alias = NULL;
    type->type_name.name         = parser_parse_identifier(parser);
    if (!type->type_name.name)
    {
        ast_node_dnit(type);
        free(type);
        return NULL;
    }

    if (parser_match(parser, TOKEN_DOT))
    {
        type->type_name.module_alias = type->type_name.name;
        type->type_name.name         = parser_parse_identifier(parser);
        if (!type->type_name.name)
        {
            ast_node_dnit(type);
            free(type);
            return NULL;
        }

        if (parser_check(parser, TOKEN_DOT))
        {
            parser_error_at_current(parser, "unexpected '.' in type name");
            ast_node_dnit(type);
            free(type);
            return NULL;
        }
    }

    if (parser_check(parser, TOKEN_L_BRACKET))
    {
        AstList *generic_args = parser_parse_type_arguments(parser);
        if (!generic_args)
        {
            ast_node_dnit(type);
            free(type);
            return NULL;
        }
        type->type_name.generic_args = generic_args;
    }

    return type;
}

AstNode *parser_parse_type_ptr(Parser *parser)
{
    AstNode *ptr = parser_alloc_node(parser, AST_TYPE_PTR, parser->previous);
    if (!ptr)
    {
        return NULL;
    }

    ptr->type_ptr.is_read_only = (parser->previous->kind == TOKEN_AMPERSAND);

    ptr->type_ptr.base = parser_parse_type(parser);
    if (!ptr->type_ptr.base)
    {
        ast_node_dnit(ptr);
        free(ptr);
        return NULL;
    }

    return ptr;
}

AstNode *parser_parse_type_array(Parser *parser)
{
    AstNode *array = parser_alloc_node(parser, AST_TYPE_ARRAY, parser->previous);
    if (!array)
    {
        return NULL;
    }

    // require explicit length
    if (parser_check(parser, TOKEN_R_BRACKET))
    {
        parser_error_at_current(parser, "array type requires explicit length (use [N]T)");
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    array->type_array.size = parser_parse_expr(parser);
    if (!array->type_array.size)
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_R_BRACKET, "expected ']' after array size"))
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    array->type_array.elem_type = parser_parse_type(parser);
    if (!array->type_array.elem_type)
    {
        ast_node_dnit(array);
        free(array);
        return NULL;
    }

    return array;
}

AstNode *parser_parse_type_fun(Parser *parser)
{
    AstNode *fun = parser_alloc_node(parser, AST_TYPE_FUN, parser->previous);
    if (!fun)
    {
        return NULL;
    }

    if (!parser_consume(parser, TOKEN_L_PAREN, "expected '(' after 'fun'"))
    {
        ast_node_dnit(fun);
        free(fun);
        return NULL;
    }

    fun->type_fun.params = parser_alloc_list(parser);
    if (!fun->type_fun.params)
    {
        ast_node_dnit(fun);
        free(fun);
        return NULL;
    }

    fun->type_fun.is_variadic = false;
    if (!parser_check(parser, TOKEN_R_PAREN))
    {
        do
        {
            if (parser_check(parser, TOKEN_ELLIPSIS))
            {
                if (fun->type_fun.is_variadic)
                {
                    parser_error_at_current(parser, "multiple '...' in function type");
                }
                parser_advance(parser);
                fun->type_fun.is_variadic = true;
                if (!parser_check(parser, TOKEN_R_PAREN))
                {
                    parser_error_at_current(parser, "'...' must be last parameter in function type");
                }
                break;
            }
            AstNode *param_type = parser_parse_type(parser);
            if (!param_type)
            {
                ast_node_dnit(fun);
                free(fun);
                return NULL;
            }
            ast_list_append(fun->type_fun.params, param_type);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_PAREN, "expected ')' after function type parameters"))
    {
        ast_node_dnit(fun);
        free(fun);
        return NULL;
    }

    // optional return type
    if (!parser_is_at_end(parser) && !parser_check(parser, TOKEN_SEMICOLON) && !parser_check(parser, TOKEN_COMMA) && !parser_check(parser, TOKEN_R_PAREN) && !parser_check(parser, TOKEN_R_BRACKET) && !parser_check(parser, TOKEN_R_BRACE) &&
        !parser_check(parser, TOKEN_EQUAL))
    {
        fun->type_fun.return_type = parser_parse_type(parser);
        if (!fun->type_fun.return_type)
        {
            ast_node_dnit(fun);
            free(fun);
            return NULL;
        }
    }

    return fun;
}

AstNode *parser_parse_type_rec(Parser *parser)
{
    AstNode *rec = parser_alloc_node(parser, AST_TYPE_REC, parser->previous);
    if (!rec)
    {
        return NULL;
    }

    if (parser_check(parser, TOKEN_IDENTIFIER))
    {
        rec->type_rec.name = parser_parse_identifier(parser);
    }
    else if (parser_check(parser, TOKEN_L_BRACE))
    {
        parser_advance(parser);
        rec->type_rec.fields = parser_parse_field_list(parser);
        if (!rec->type_rec.fields)
        {
            ast_node_dnit(rec);
            free(rec);
            return NULL;
        }
        if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after record fields"))
        {
            ast_node_dnit(rec);
            free(rec);
            return NULL;
        }
    }
    else
    {
        parser_error_at_current(parser, "expected record name or anonymous record definition");
        ast_node_dnit(rec);
        free(rec);
        return NULL;
    }

    return rec;
}

AstNode *parser_parse_type_uni(Parser *parser)
{
    AstNode *uni = parser_alloc_node(parser, AST_TYPE_UNI, parser->previous);
    if (!uni)
    {
        return NULL;
    }

    if (parser_check(parser, TOKEN_IDENTIFIER))
    {
        uni->type_uni.name = parser_parse_identifier(parser);
    }
    else if (parser_check(parser, TOKEN_L_BRACE))
    {
        parser_advance(parser);
        uni->type_uni.fields = parser_parse_field_list(parser);
        if (!uni->type_uni.fields)
        {
            ast_node_dnit(uni);
            free(uni);
            return NULL;
        }
        if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after union fields"))
        {
            ast_node_dnit(uni);
            free(uni);
            return NULL;
        }
    }
    else
    {
        parser_error_at_current(parser, "expected union name or anonymous union definition");
        ast_node_dnit(uni);
        free(uni);
        return NULL;
    }

    return uni;
}

AstList *parser_parse_field_list(Parser *parser)
{
    AstList *list = parser_alloc_list(parser);
    if (!list)
    {
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        if (parser->current->kind == TOKEN_DOLLAR)
        {
            parser_error_at_current(parser, "'$if' is not allowed inside record or union declarations");
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        AstNode *field = parser_alloc_node(parser, AST_STMT_FIELD, parser->current);
        if (!field)
        {
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        field->field_stmt.name = parser_parse_identifier(parser);
        if (!field->field_stmt.name)
        {
            ast_node_dnit(field);
            free(field);
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        if (!parser_consume(parser, TOKEN_COLON, "expected ':' after field name"))
        {
            ast_node_dnit(field);
            free(field);
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        field->field_stmt.type = parser_parse_type(parser);
        if (!field->field_stmt.type)
        {
            ast_node_dnit(field);
            free(field);
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        ast_list_append(list, field);

        if (!parser_match(parser, TOKEN_SEMICOLON))
        {
            break;
        }
    }

    return list;
}

AstList *parser_parse_parameter_list(Parser *parser)
{
    AstList *list = parser_alloc_list(parser);
    if (!list)
    {
        return NULL;
    }

    if (parser_check(parser, TOKEN_R_PAREN))
    {
        return list;
    }

    do
    {
        if (parser_check(parser, TOKEN_ELLIPSIS))
        {
            // caller should handle variadic; here treat as error to avoid misuse
            parser_error_at_current(parser, "'...' not allowed here");
            return list;
        }
        AstNode *param = parser_alloc_node(parser, AST_STMT_PARAM, parser->current);
        if (!param)
        {
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        param->param_stmt.name = parser_parse_identifier(parser);
        if (!param->param_stmt.name)
        {
            ast_node_dnit(param);
            free(param);
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        if (!parser_consume(parser, TOKEN_COLON, "expected ':' after parameter name"))
        {
            ast_node_dnit(param);
            free(param);
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        param->param_stmt.type        = parser_parse_type(parser);
        param->param_stmt.is_variadic = false;
        if (!param->param_stmt.type)
        {
            ast_node_dnit(param);
            free(param);
            ast_list_dnit(list);
            free(list);
            return NULL;
        }

        ast_list_append(list, param);
    } while (parser_match(parser, TOKEN_COMMA));

    return list;
}

AstNode *parser_parse_typed_literal(Parser *parser, AstNode *type)
{
    if (!parser_consume(parser, TOKEN_L_BRACE, "expected '{' to start literal"))
    {
        ast_node_dnit(type);
        free(type);
        return NULL;
    }

    bool struct_syntax = false;
    if (!parser_check(parser, TOKEN_R_BRACE) && parser_check(parser, TOKEN_IDENTIFIER))
    {
        TokenKind next_kind = parser_peek_next_kind(parser);
        if (next_kind == TOKEN_COLON)
        {
            struct_syntax = true;
        }
    }

    if (struct_syntax)
    {
        AstNode *literal = parser_alloc_node(parser, AST_EXPR_STRUCT, parser->previous);
        if (!literal)
        {
            ast_node_dnit(type);
            free(type);
            return NULL;
        }

        literal->struct_expr.type   = type;
        literal->struct_expr.fields = parser_alloc_list(parser);
        if (!literal->struct_expr.fields)
        {
            ast_node_dnit(literal);
            free(literal);
            return NULL;
        }

        if (!parser_check(parser, TOKEN_R_BRACE))
        {
            do
            {
                char *field_name = parser_parse_identifier(parser);
                if (!field_name)
                {
                    ast_node_dnit(literal);
                    free(literal);
                    return NULL;
                }

                if (!parser_consume(parser, TOKEN_COLON, "expected ':' after field name"))
                {
                    free(field_name);
                    ast_node_dnit(literal);
                    free(literal);
                    return NULL;
                }

                AstNode *value = parser_parse_expr(parser);
                if (!value)
                {
                    free(field_name);
                    ast_node_dnit(literal);
                    free(literal);
                    return NULL;
                }

                AstNode *field_node = parser_alloc_node(parser, AST_EXPR_FIELD, parser->previous);
                if (!field_node)
                {
                    free(field_name);
                    ast_node_dnit(value);
                    free(value);
                    ast_node_dnit(literal);
                    free(literal);
                    return NULL;
                }

                field_node->field_expr.field  = field_name;
                field_node->field_expr.object = value;
                ast_list_append(literal->struct_expr.fields, field_node);
            } while (parser_match(parser, TOKEN_COMMA));
        }

        if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after literal fields"))
        {
            ast_node_dnit(literal);
            free(literal);
            return NULL;
        }

        return literal;
    }

    AstNode *literal = parser_alloc_node(parser, AST_EXPR_ARRAY, parser->previous);
    if (!literal)
    {
        ast_node_dnit(type);
        free(type);
        return NULL;
    }

    literal->array_expr.type  = type;
    literal->array_expr.elems = parser_alloc_list(parser);
    if (!literal->array_expr.elems)
    {
        ast_node_dnit(literal);
        free(literal);
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_BRACE))
    {
        do
        {
            AstNode *elem = parser_parse_expr(parser);
            if (!elem)
            {
                ast_node_dnit(literal);
                free(literal);
                return NULL;
            }
            ast_list_append(literal->array_expr.elems, elem);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_consume(parser, TOKEN_R_BRACE, "expected '}' after literal elements"))
    {
        ast_node_dnit(literal);
        free(literal);
        return NULL;
    }

    return literal;
}
