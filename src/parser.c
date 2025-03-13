#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "operator.h"
#include "parser.h"
#include "token.h"

bool parser_init(Parser *parser, Lexer *lexer)
{
    if (!parser || !lexer)
    {
        return false;
    }

    parser->lexer = lexer;

    parser->tokens_cap = 8;
    parser->tokens_len = 0;
    parser->tokens = malloc(sizeof(Token) * parser->tokens_cap);
    if (!parser->tokens)
    {
        fprintf(stderr, "failed to allocate memory for parser tokens\n");
        free(parser);
        return false;
    }

    parser->tokens_cur = 0;

    parser->error_cap = 8;
    parser->error_len = 0;
    parser->errors = malloc(sizeof(ParseError) * parser->error_cap);
    if (!parser->errors)
    {
        fprintf(stderr, "failed to allocate memory for parser errors\n");
        free(parser);
        return false;
    }

    return true;
}

void parser_free(Parser *parser)
{
    if (!parser)
    {
        return;
    }

    free(parser->tokens);
    parser->tokens = NULL;

    free(parser->errors);
    parser->errors = NULL;

    free(parser);
}

void parser_print_error(Parser *parser, ParseError error, const char *file_path)
{
    int line_num = lexer_line_at(parser->lexer, error.token.start);
    int col_num = lexer_column_at(parser->lexer, error.token.start);

    char *line = lexer_get_line(parser->lexer, line_num);
    if (!line)
    {
        line = strdup("unable to retrieve source line");
    }

    printf("error: %s\n", error.message);
    printf("%s:%d:%d\n", file_path, line_num, col_num);
    printf("%5d | %s\n", line_num, line);
    printf("      | %*s^\n", col_num - 1, "");

    free(line);
}

void parser_print_errors(Parser *parser, const char *file_path)
{
    for (int i = 0; i < parser->error_len; i++)
    {
        parser_print_error(parser, parser->errors[i], file_path);
        if (i < parser->error_len - 1)
        {
            printf("\n");
        }
    }
}

void parser_error(Parser *parser, Token token, const char *message)
{
    if (parser->recovering)
    {
        return;
    }

    if (parser->error_len >= parser->error_cap)
    {
        parser->error_cap *= 2;
        parser->errors = realloc(parser->errors, sizeof(ParseError) * parser->error_cap);
        if (!parser->errors)
        {
            fprintf(stderr, "failed to reallocate memory for parser errors\n");
            return;
        }
    }

    parser->errors[parser->error_len].token = token;
    parser->errors[parser->error_len].message = message;
    parser->error_len++;

    parser->recovering = true;
}

bool parser_has_errors(Parser *parser) { return parser->error_len > 0; }

void parser_recovery_reset(Parser *parser)
{
    parser->recovering = false;
}

void parser_synchronize(Parser *parser)
{
    parser_advance(parser);

    while (!parser_is_at_end(parser))
    {
        if (parser_prev(parser).kind == TOKEN_SEMICOLON)
        {
            return;
        }

        switch (parser_curr(parser).kind)
        {
        case TOKEN_VAL:
        case TOKEN_VAR:
        case TOKEN_VOL:
        case TOKEN_DEF:
        case TOKEN_STR:
        case TOKEN_UNI:
        case TOKEN_FUN:
        case TOKEN_IF:
        case TOKEN_FOR:
            return;
        default:
            break;
        }

        parser_advance(parser);
    }
}

Token parser_curr(Parser *parser) { return parser->tokens[parser->tokens_cur]; }

Token parser_prev(Parser *parser)
{
    if (parser->tokens_cur == 0)
    {
        return parser->tokens[0];
    }

    return parser->tokens[parser->tokens_cur - 1];
}

Token parser_peek(Parser *parser, int offset)
{
    if (parser->tokens_cur + offset >= parser->tokens_len)
    {
        return parser->tokens[parser->tokens_len - 1];
    }

    return parser->tokens[parser->tokens_cur + offset];
}

bool parser_is_at_end(Parser *parser) { return parser->tokens_cur >= parser->tokens_len || parser_curr(parser).kind == TOKEN_EOF; }

Token parser_advance(Parser *parser)
{
    if (!parser_is_at_end(parser))
    {
        parser->tokens_cur++;
    }

    return parser_prev(parser);
}

bool parser_check(Parser *parser, TokenKind type) { return parser_curr(parser).kind == type; }

bool parser_match(Parser *parser, TokenKind type)
{
    if (parser_check(parser, type))
    {
        parser_advance(parser);
        return true;
    }

    return false;
}

Node *parse_comment(Parser *parser)
{
    if (!parser_match(parser, TOKEN_HASH))
    {
        parser_error(parser, parser_curr(parser), "expected comment");
        return NULL;
    }

    Node *comment = calloc(sizeof(Node), 1);
    if (!node_init(comment, NODE_COMMENT, parser_prev(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    return comment;
}


Node *parse_identifier(Parser *parser)
{
    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier");
        return NULL;
    }

    Token token = parser_advance(parser);

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    return ident;
}

Node *parse_lit_int(Parser *parser)
{
    if (!parser_check(parser, TOKEN_LIT_INT))
    {
        parser_error(parser, parser_curr(parser), "expected integer literal");
        return NULL;
    }

    Token token = parser_advance(parser);

    Node *lit_int = calloc(sizeof(Node), 1);
    if (!node_init(lit_int, NODE_LIT_INT, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    lit_int->lit_int.value = lexer_eval_lit_int(token.start, token.len);

    return lit_int;
}

Node *parse_lit_float(Parser *parser)
{
    if (!parser_check(parser, TOKEN_LIT_FLOAT))
    {
        parser_error(parser, parser_curr(parser), "expected float literal");
        return NULL;
    }

    Token token = parser_advance(parser);

    Node *lit_float = calloc(sizeof(Node), 1);
    if (!node_init(lit_float, NODE_LIT_FLOAT, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    lit_float->lit_float.value = lexer_eval_lit_float(token.start, token.len);

    return lit_float;
}

Node *parse_lit_char(Parser *parser)
{
    if (!parser_check(parser, TOKEN_LIT_CHAR))
    {
        parser_error(parser, parser_curr(parser), "expected character literal");
        return NULL;
    }

    Token token = parser_advance(parser);

    Node *lit_char = calloc(sizeof(Node), 1);
    if (!node_init(lit_char, NODE_LIT_CHAR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    lit_char->lit_char.value = lexer_eval_lit_char(token.start, token.len);

    return lit_char;
}

Node *parse_lit_string(Parser *parser)
{
    if (!parser_check(parser, TOKEN_LIT_STRING))
    {
        parser_error(parser, parser_curr(parser), "expected string literal");
        return NULL;
    }

    Token token = parser_advance(parser);

    Node *lit_string = calloc(sizeof(Node), 1);
    if (!node_init(lit_string, NODE_LIT_STRING, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    lit_string->lit_string.value = lexer_eval_lit_string(token.start, token.len);

    return lit_string;
}

Node *parse_stmt_val(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'val'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    Node *type = NULL;
    if (parser_match(parser, TOKEN_COLON))
    {
        type = parse_type(parser);
        if (!type)
        {
            parser_error(parser, parser_curr(parser), "expected type after ':'");
            node_free(ident);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected ':' after identifier");
        node_free(ident);
        return NULL;
    }

    Node *initializer = NULL;
    if (parser_match(parser, TOKEN_EQUAL))
    {
        initializer = parse_expr(parser);
        if (!initializer)
        {
            parser_error(parser, parser_curr(parser), "expected expression after '='");
            node_free(ident);
            node_free(type);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected '=' after type");
        node_free(ident);
        node_free(type);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(ident);
        node_free(type);
        node_free(initializer);
        return NULL;
    }

    Node *val_decl = calloc(sizeof(Node), 1);
    if (!node_init(val_decl, NODE_STMT_VAL, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_free(type);
        node_free(initializer);
        return NULL;
    }

    val_decl->stmt_val.ident = ident;
    val_decl->stmt_val.type_node = type;
    val_decl->stmt_val.expr = initializer;

    return val_decl;
}

Node *parse_stmt_var(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *ident = parse_identifier(parser);
    if (!ident)
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'var'");
        return NULL;
    }

    Node *type = NULL;
    if (parser_match(parser, TOKEN_COLON))
    {
        type = parse_type(parser);
        if (!type)
        {
            parser_error(parser, parser_curr(parser), "expected type after ':'");
            node_free(ident);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected ':' after identifier");
        node_free(ident);
        return NULL;
    }

    Node *initializer = NULL;
    if (parser_match(parser, TOKEN_EQUAL))
    {
        initializer = parse_expr(parser);
        if (!initializer)
        {
            parser_error(parser, parser_curr(parser), "expected expression after '='");
            node_free(ident);
            node_free(type);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(ident);
        node_free(type);
        node_free(initializer);
        return NULL;
    }

    Node *var_decl = calloc(sizeof(Node), 1);
    if (!node_init(var_decl, NODE_STMT_VAR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_free(type);
        node_free(initializer);
        return NULL;
    }

    var_decl->stmt_var.ident = ident;
    var_decl->stmt_var.type_node = type;
    var_decl->stmt_var.expr = initializer;

    return var_decl;
}

Node *parse_stmt_vol(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'vol'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    Node *type = NULL;
    if (parser_match(parser, TOKEN_COLON))
    {
        type = parse_type(parser);
        if (!type)
        {
            parser_error(parser, parser_curr(parser), "expected type after ':'");
            node_free(ident);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected ':' after identifier");
        node_free(ident);
        return NULL;
    }

    Node *initializer = NULL;
    if (parser_match(parser, TOKEN_EQUAL))
    {
        initializer = parse_expr(parser);
        if (!initializer)
        {
            parser_error(parser, parser_curr(parser), "expected expression after '='");
            node_free(ident);
            node_free(type);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(ident);
        node_free(type);
        node_free(initializer);
        return NULL;
    }

    Node *vol_decl = calloc(sizeof(Node), 1);
    if (!node_init(vol_decl, NODE_STMT_VOL, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_free(type);
        node_free(initializer);
        return NULL;
    }

    vol_decl->stmt_vol.ident = ident;
    vol_decl->stmt_vol.type_node = type;
    vol_decl->stmt_vol.expr = initializer;

    return vol_decl;
}

Node *parse_stmt_def(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'def'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    Node *type = NULL;
    if (parser_match(parser, TOKEN_COLON))
    {
        type = parse_type(parser);
        if (!type)
        {
            parser_error(parser, parser_curr(parser), "expected type after ':'");
            node_free(ident);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected ':' after identifier");
        node_free(ident);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(ident);
        node_free(type);
        return NULL;
    }

    Node *def_decl = calloc(sizeof(Node), 1);
    if (!node_init(def_decl, NODE_STMT_DEF, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_free(type);
        return NULL;
    }
    def_decl->stmt_def.ident = ident;
    def_decl->stmt_def.type_node = type;

    return def_decl;
}

Node *parse_stmt_use(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *ident = NULL;
    if (parser_match(parser, TOKEN_IDENTIFIER))
    {
        ident = calloc(sizeof(Node), 1);
        if (!node_init(ident, NODE_IDENTIFIER, parser_prev(parser)))
        {
            fprintf(stderr, "failed to initialize node\n");
            return NULL;
        }

        if (!parser_match(parser, TOKEN_COLON))
        {
            parser_error(parser, parser_curr(parser), "expected ':' after identifier");
            node_free(ident);
            return NULL;
        }
    }

    // node path is a chain of identifiers with member expressions, e.g:
    // `use foo.bar.baz;`
    Node *path = NULL;
    if (parser_match(parser, TOKEN_IDENTIFIER))
    {
        path = calloc(sizeof(Node), 1);
        if (!node_init(path, NODE_IDENTIFIER, parser_prev(parser)))
        {
            fprintf(stderr, "failed to initialize node\n");
            node_free(ident);
            return NULL;
        }

        while (parser_match(parser, TOKEN_DOT) && !parser_is_at_end(parser))
        {
            if (!parser_match(parser, TOKEN_IDENTIFIER))
            {
                parser_error(parser, parser_curr(parser), "expected identifier after '.'");
                node_free(ident);
                node_free(path);
                return NULL;
            }

            Node *member = calloc(sizeof(Node), 1);
            if (!node_init(member, NODE_IDENTIFIER, parser_prev(parser)))
            {
                fprintf(stderr, "failed to initialize node\n");
                node_free(ident);
                node_free(path);
                return NULL;
            }

            Node *member_expr = calloc(sizeof(Node), 1);
            if (!node_init(member_expr, NODE_POST_MEMBER, token))
            {
                fprintf(stderr, "failed to initialize node\n");
                node_free(ident);
                node_free(path);
                node_free(member);
                return NULL;
            }

            member_expr->post_member.target = path;
            member_expr->post_member.member = member;
            path = member_expr;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected identifier");
        node_free(ident);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(ident);
        node_free(path);
        return NULL;
    }

    Node *use_decl = calloc(sizeof(Node), 1);
    if (!node_init(use_decl, NODE_STMT_USE, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_free(path);
        return NULL;
    }
    use_decl->stmt_use.alias = ident;

    return use_decl;
}

Node *parse_field(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'field'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    if (!parser_match(parser, TOKEN_COLON))
    {
        parser_error(parser, parser_curr(parser), "expected ':' after identifier");
        node_free(ident);
        return NULL;
    }

    Node *type = parse_type(parser);
    if (!type)
    {
        parser_error(parser, parser_curr(parser), "expected type after ':'");
        node_free(ident);
        return NULL;
    }

    Node *field = calloc(sizeof(Node), 1);
    if (!node_init(field, NODE_FIELD, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_free(type);
        return NULL;
    }
    field->field.ident = ident;
    field->field.type_node = type;

    return field;
}

Node *parse_stmt_str(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'str'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    if (!parser_match(parser, TOKEN_L_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '{' after identifier");
        node_free(ident);
        return NULL;
    }

    NodeList *fields = calloc(sizeof(NodeList), 1);
    if (!node_list_init(fields))
    {
        fprintf(stderr, "failed to initialize node list\n");
        node_free(ident);
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        Node *field = parse_field(parser);
        if (!field)
        {
            parser_error(parser, parser_curr(parser), "expected field declaration");
            node_free(ident);
            node_list_free(fields);
            return NULL;
        }

        node_list_add(fields, field);

        if (!parser_match(parser, TOKEN_SEMICOLON))
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_R_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '}' after struct fields");
        node_free(ident);
        node_list_free(fields);
        return NULL;
    }

    Node *str_decl = calloc(sizeof(Node), 1);
    if (!node_init(str_decl, NODE_STMT_STR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_list_free(fields);
        return NULL;
    }
    str_decl->stmt_str.ident = ident;
    str_decl->stmt_str.fields = fields;

    return str_decl;
}

Node *parse_stmt_uni(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'uni'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    if (!parser_match(parser, TOKEN_L_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '{' after identifier");
        node_free(ident);
        return NULL;
    }

    NodeList *fields = calloc(sizeof(NodeList), 1);
    if (!node_list_init(fields))
    {
        fprintf(stderr, "failed to initialize node list\n");
        node_free(ident);
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        Node *field = parse_field(parser);
        if (!field)
        {
            parser_error(parser, parser_curr(parser), "expected field declaration");
            node_free(ident);
            node_list_free(fields);
            return NULL;
        }

        node_list_add(fields, field);

        if (!parser_match(parser, TOKEN_SEMICOLON))
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_R_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '}' after union fields");
        node_free(ident);
        node_list_free(fields);
        return NULL;
    }

    Node *uni_decl = calloc(sizeof(Node), 1);
    if (!node_init(uni_decl, NODE_STMT_UNI, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_list_free(fields);
        return NULL;
    }
    uni_decl->stmt_uni.ident = ident;
    uni_decl->stmt_uni.fields = fields;

    return uni_decl;
}

Node *parse_stmt_fun(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after 'fun'");
        return NULL;
    }

    Node *ident = calloc(sizeof(Node), 1);
    if (!node_init(ident, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    if (!parser_match(parser, TOKEN_L_PAREN))
    {
        parser_error(parser, parser_curr(parser), "expected '(' after identifier");
        node_free(ident);
        return NULL;
    }

    NodeList *params = calloc(sizeof(NodeList), 1);
    if (!node_list_init(params))
    {
        fprintf(stderr, "failed to initialize node list\n");
        node_free(ident);
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_PAREN) && !parser_is_at_end(parser))
    {
        Node *param = parse_field(parser);
        if (!param)
        {
            parser_error(parser, parser_curr(parser), "expected parameter declaration");
            node_free(ident);
            node_list_free(params);
            return NULL;
        }

        node_list_add(params, param);

        if (!parser_match(parser, TOKEN_COMMA))
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_R_PAREN))
    {
        parser_error(parser, parser_curr(parser), "expected ')' after function parameters");
        node_free(ident);
        node_list_free(params);
        return NULL;
    }

    Node *type = NULL;
    if (parser_match(parser, TOKEN_COLON))
    {
        type = parse_type(parser);
        if (!type)
        {
            parser_error(parser, parser_curr(parser), "expected type after ':'");
            node_free(ident);
            node_list_free(params);
            return NULL;
        }
    }

    Node *body = NULL;
    if (parser_match(parser, TOKEN_L_BRACE))
    {
        body = parse_stmt_block(parser);
        if (!body)
        {
            parser_error(parser, parser_curr(parser), "expected statement after '{'");
            node_free(ident);
            node_list_free(params);
            node_free(type);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected '{' after function definition");
        node_free(ident);
        node_list_free(params);
        node_free(type);
        return NULL;
    }

    Node *fun_decl = calloc(sizeof(Node), 1);
    if (!node_init(fun_decl, NODE_STMT_FUN, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(ident);
        node_list_free(params);
        node_free(type);
        node_free(body);
        return NULL;
    }
    fun_decl->stmt_fun.ident = ident;
    fun_decl->stmt_fun.params = params;
    fun_decl->stmt_fun.type_node = type;
    fun_decl->stmt_fun.body = body;

    return fun_decl;
}

Node *parse_stmt_block(Parser *parser)
{
    Token token = parser_prev(parser);

    NodeList *stmts = calloc(sizeof(NodeList), 1);
    if (!node_list_init(stmts))
    {
        fprintf(stderr, "failed to initialize node list\n");
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        Node *stmt = parse_stmt(parser);
        if (!stmt)
        {
            parser_error(parser, parser_curr(parser), "expected statement");
            node_list_free(stmts);
            return NULL;
        }

        node_list_add(stmts, stmt);
    }

    if (!parser_match(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        parser_error(parser, parser_curr(parser), "expected '}' after block");
        node_list_free(stmts);
        return NULL;
    }

    Node *block = calloc(sizeof(Node), 1);
    if (!node_init(block, NODE_STMT_BLOCK, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_list_free(stmts);
        return NULL;
    }
    block->stmt_block.stmts = stmts;

    return block;
}

Node *parse_stmt_if(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *cond = parse_expr(parser);
    if (!cond)
    {
        parser_error(parser, parser_curr(parser), "expected expression after '('");
        return NULL;
    }

    Node *body = NULL;
    if (parser_match(parser, TOKEN_L_BRACE))
    {
        body = parse_stmt_block(parser);
        if (!body)
        {
            parser_error(parser, parser_curr(parser), "expected statement after '{'");
            node_free(cond);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected '{' after if condition");
        node_free(cond);
        return NULL;
    }

    Node *branch = NULL;
    if (parser_match(parser, TOKEN_OR))
    {
        branch = parse_stmt_or(parser);
        if (!branch)
        {
            parser_error(parser, parser_curr(parser), "expected 'or' statement after 'if'");
            node_free(cond);
            node_free(body);
            return NULL;
        }
    }

    Node *if_stmt = calloc(sizeof(Node), 1);
    if (!node_init(if_stmt, NODE_STMT_IF, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(cond);
        node_free(body);
        return NULL;
    }
    if_stmt->stmt_if.cond = cond;
    if_stmt->stmt_if.body = body;

    if (branch)
    {
        branch->stmt_or.parent = if_stmt;
        if_stmt->stmt_if.branch = branch;
    }

    return if_stmt;
}

Node *parse_stmt_or(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *cond = NULL;
    if (!parser_check(parser, TOKEN_L_BRACE))
    {
        cond = parse_expr(parser);
        if (!cond)
        {
            parser_error(parser, parser_curr(parser), "expected expression after 'or'");
            return NULL;
        }
    }

    Node *body = NULL;
    if (parser_match(parser, TOKEN_L_BRACE))
    {
        body = parse_stmt_block(parser);
        if (!body)
        {
            parser_error(parser, parser_curr(parser), "expected statement after '{'");
            node_free(cond);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected '{' after or condition");
        node_free(cond);
        return NULL;
    }

    Node *branch = NULL;
    if (parser_match(parser, TOKEN_OR))
    {
        branch = parse_stmt_or(parser);
        if (!branch)
        {
            parser_error(parser, parser_curr(parser), "expected 'or' statement after 'or'");
            node_free(cond);
            node_free(body);
            return NULL;
        }
    }

    Node *or_stmt = calloc(sizeof(Node), 1);
    if (!node_init(or_stmt, NODE_STMT_OR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(cond);
        node_free(body);
        return NULL;
    }
    or_stmt->stmt_or.cond = cond;
    or_stmt->stmt_or.body = body;

    if (branch)
    {
        branch->stmt_or.parent = or_stmt;
        or_stmt->stmt_or.branch = branch;
    }

    return or_stmt;
}

Node *parse_stmt_for(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *cond = parse_expr(parser);
    if (!cond)
    {
        parser_error(parser, parser_curr(parser), "expected expression after 'for'");
        return NULL;
    }

    Node *body = NULL;
    if (parser_match(parser, TOKEN_L_BRACE))
    {
        body = parse_stmt_block(parser);
        if (!body)
        {
            parser_error(parser, parser_curr(parser), "expected statement after '{'");
            node_free(cond);
            return NULL;
        }
    }
    else
    {
        parser_error(parser, parser_curr(parser), "expected '{' after for condition");
        node_free(cond);
        return NULL;
    }

    Node *for_stmt = calloc(sizeof(Node), 1);
    if (!node_init(for_stmt, NODE_STMT_FOR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(cond);
        node_free(body);
        return NULL;
    }
    for_stmt->stmt_for.cond = cond;
    for_stmt->stmt_for.body = body;

    return for_stmt;
}

Node *parse_stmt_brk(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        return NULL;
    }

    Node *brk_stmt = calloc(sizeof(Node), 1);
    if (!node_init(brk_stmt, NODE_STMT_BRK, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    return brk_stmt;
}

Node *parse_stmt_cnt(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        return NULL;
    }

    Node *cnt_stmt = calloc(sizeof(Node), 1);
    if (!node_init(cnt_stmt, NODE_STMT_CNT, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }

    return cnt_stmt;
}

Node *parse_stmt_ret(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *expr = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON))
    {
        expr = parse_expr(parser);
        if (!expr)
        {
            parser_error(parser, parser_curr(parser), "expected expression after 'ret'");
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(expr);
        return NULL;
    }

    Node *ret_stmt = calloc(sizeof(Node), 1);
    if (!node_init(ret_stmt, NODE_STMT_RET, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(expr);
        return NULL;
    }
    ret_stmt->stmt_ret.expr = expr;

    return ret_stmt;
}

Node *parse_stmt_expr(Parser *parser)
{
    Node *expr = parse_expr(parser);
    if (!expr)
    {
        parser_error(parser, parser_curr(parser), "expected statement expression");
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, parser_curr(parser), "expected ';'");
        node_free(expr);
        return NULL;
    }

    return expr;
}

Node *parse_stmt(Parser *parser)
{
    parser_recovery_reset(parser);

    Node *result;

    switch (parser_curr(parser).kind)
    {
    case TOKEN_VAL:
        parser_advance(parser);
        result = parse_stmt_val(parser);
        break;
    case TOKEN_VAR:
        parser_advance(parser);
        result = parse_stmt_var(parser);
        break;
    case TOKEN_VOL:
        parser_advance(parser);
        result = parse_stmt_vol(parser);
        break;
    case TOKEN_DEF:
        parser_advance(parser);
        result = parse_stmt_def(parser);
        break;
    case TOKEN_USE:
        parser_advance(parser);
        result = parse_stmt_use(parser);
        break;
    case TOKEN_STR:
        parser_advance(parser);
        result = parse_stmt_str(parser);
        break;
    case TOKEN_UNI:
        parser_advance(parser);
        result = parse_stmt_uni(parser);
        break;
    case TOKEN_FUN:
        parser_advance(parser);
        result = parse_stmt_fun(parser);
        break;
    case TOKEN_IF:
        parser_advance(parser);
        result = parse_stmt_if(parser);
        break;
    case TOKEN_OR:
        parser_advance(parser);
        result = parse_stmt_or(parser);
        break;
    case TOKEN_FOR:
        parser_advance(parser);
        result = parse_stmt_for(parser);
        break;
    case TOKEN_BRK:
        parser_advance(parser);
        result = parse_stmt_brk(parser);
        break;
    case TOKEN_CNT:
        parser_advance(parser);
        result = parse_stmt_cnt(parser);
        break;
    case TOKEN_RET:
        parser_advance(parser);
        result = parse_stmt_ret(parser);
        break;
    case TOKEN_HASH:
        result = parse_comment(parser);
        break;
    default:
        result = parse_stmt_expr(parser);
        break;
    }

    if (result == NULL && parser->recovering)
    {
        parser_synchronize(parser);
    }

    return result;
}

Node *parse_post_member(Parser *parser, Node *target)
{
    Token token = parser_prev(parser);

    if (!parser_check(parser, TOKEN_IDENTIFIER))
    {
        parser_error(parser, parser_curr(parser), "expected identifier after '.'");
        node_free(target);
        return NULL;
    }

    Node *member = calloc(sizeof(Node), 1);
    if (!node_init(member, NODE_IDENTIFIER, parser_advance(parser)))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(target);
        return NULL;
    }

    Node *post_member = calloc(sizeof(Node), 1);
    if (!node_init(post_member, NODE_POST_MEMBER, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(target);
        node_free(member);
        return NULL;
    }
    post_member->post_member.target = target;
    post_member->post_member.member = member;

    return post_member;
}

Node *parse_post_call(Parser *parser, Node *target)
{
    Token token = parser_prev(parser);

    NodeList *args = calloc(sizeof(NodeList), 1);
    if (!node_list_init(args))
    {
        fprintf(stderr, "failed to initialize node list\n");
        node_free(target);
        return NULL;
    }

    if (!parser_check(parser, TOKEN_R_PAREN))
    {
        do
        {
            Node *arg = parse_expr(parser);
            if (!arg)
            {
                parser_error(parser, parser_curr(parser), "expected expression in function call");
                node_free(target);
                node_list_free(args);
                return NULL;
            }

            node_list_add(args, arg);
        } while (parser_match(parser, TOKEN_COMMA));
    }

    if (!parser_match(parser, TOKEN_R_PAREN))
    {
        parser_error(parser, parser_curr(parser), "expected ')' after function arguments");
        node_free(target);
        node_list_free(args);
        return NULL;
    }

    Node *post_call = calloc(sizeof(Node), 1);
    if (!node_init(post_call, NODE_POST_CALL, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(target);
        node_list_free(args);
        return NULL;
    }
    post_call->post_call.target = target;
    post_call->post_call.args = args;

    return post_call;
}

Node *parse_post_idx_arr(Parser *parser, Node *target)
{
    Token token = parser_prev(parser);

    Node *index = parse_expr(parser);
    if (!index)
    {
        parser_error(parser, parser_curr(parser), "expected expression in array index");
        node_free(target);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_R_BRACKET))
    {
        parser_error(parser, parser_curr(parser), "expected ']' after array index");
        node_free(target);
        node_free(index);
        return NULL;
    }

    Node *post_idx_arr = calloc(sizeof(Node), 1);
    if (!node_init(post_idx_arr, NODE_POST_IDX_ARR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(target);
        node_free(index);
        return NULL;
    }
    post_idx_arr->post_idx_arr.target = target;
    post_idx_arr->post_idx_arr.index = index;

    return post_idx_arr;
}

Node *parse_post_cast(Parser *parser, Node *target)
{
    Token token = parser_prev(parser);

    Node *type = parse_type(parser);
    if (!type)
    {
        parser_error(parser, parser_curr(parser), "expected type after ':'");
        node_free(target);
        return NULL;
    }

    Node *post_cast = calloc(sizeof(Node), 1);
    if (!node_init(post_cast, NODE_POST_CAST, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(target);
        node_free(type);
        return NULL;
    }
    post_cast->post_cast.target = target;
    post_cast->post_cast.type_node = type;

    return post_cast;
}

Node *parse_post(Parser *parser)
{
    Node *node = parse_expr_unary(parser);
    if (!node)
    {
        return NULL;
    }

    while (true)
    {
        switch (parser_curr(parser).kind)
        {
        case TOKEN_DOT:
            parser_advance(parser);
            node = parse_post_member(parser, node);
            break;
        case TOKEN_L_PAREN:
            parser_advance(parser);
            node = parse_post_call(parser, node);
            break;
        case TOKEN_L_BRACKET:
            parser_advance(parser);
            node = parse_post_idx_arr(parser, node);
            break;
        case TOKEN_COLON_COLON:
            parser_advance(parser);
            node = parse_post_cast(parser, node);
            break;
        default:
            return node;
        }
    }

    return node;
}

Node *parse_expr_unary(Parser *parser)
{
    Token op_token = parser_curr(parser);
    Operator op = op_from_token_kind(op_token.kind);

    if (op_is_unary(op))
    {
        parser_advance(parser);

        Node *expr = parse_expr_unary(parser);
        if (!expr)
        {
            parser_error(parser, parser_curr(parser), "expected expression after unary operator");
            return NULL;
        }

        Node *unary = calloc(sizeof(Node), 1);
        if (!node_init(unary, NODE_EXPR_UNARY, op_token))
        {
            fprintf(stderr, "failed to initialize node\n");
            node_free(expr);
            return NULL;
        }
        unary->expr_unary.op = op;
        unary->expr_unary.expr = expr;

        return unary;
    }

    return parse_expr_primary(parser);
}

Node *parse_expr_binary(Parser *parser, int min_prec)
{
    Node *left = parse_post(parser);
    if (!left)
    {
        return NULL;
    }

    while (true)
    {
        TokenKind token_kind = parser_curr(parser).kind;
        Operator op = op_from_token_kind(token_kind);

        if (op == OP_UNKNOWN || !op_is_binary(op))
            break;

        int prec = op_precedence(op);
        if (prec < min_prec)
            break;

        parser_advance(parser);

        int next_min_prec = op_is_right_associative(op) ? prec : prec + 1;

        Node *right = parse_expr_binary(parser, next_min_prec);
        if (!right)
        {
            parser_error(parser, parser_curr(parser), "expected expression after binary operator");
            node_free(left);
            return NULL;
        }

        Node *binary = calloc(sizeof(Node), 1);
        if (!node_init(binary, NODE_EXPR_BINARY, parser_prev(parser)))
        {
            fprintf(stderr, "failed to initialize node\n");
            node_free(left);
            node_free(right);
            return NULL;
        }
        binary->expr_binary.op = op;
        binary->expr_binary.left = left;
        binary->expr_binary.right = right;

        left = binary;
    }

    return left;
}

Node *parse_expr_grouping(Parser *parser)
{
    Node *expr = parse_expr(parser);
    if (!expr)
        return NULL;

    if (!parser_match(parser, TOKEN_R_PAREN))
    {
        parser_error(parser, parser_curr(parser), "expected ')' after expression");
        node_free(expr);
        return NULL;
    }

    return expr;
}

Node *parse_expr_primary(Parser *parser)
{
    switch (parser_curr(parser).kind)
    {
    case TOKEN_IDENTIFIER:
        return parse_identifier(parser);
    case TOKEN_LIT_INT:
        return parse_lit_int(parser);
    case TOKEN_LIT_FLOAT:
        return parse_lit_float(parser);
    case TOKEN_LIT_CHAR:
        return parse_lit_char(parser);
    case TOKEN_LIT_STRING:
        return parse_lit_string(parser);
    case TOKEN_L_PAREN:
        return parse_expr_grouping(parser);
    default:
        parser_error(parser, parser_advance(parser), "expected primary expression");
        return NULL;
    }
}

Node *parse_expr(Parser *parser) { return parse_expr_binary(parser, 0); }

Node *parse_type_arr(Parser *parser)
{
    Token token = parser_prev(parser);
    
    Node *size = NULL;
    // check for unbound array syntax, e.g `[]u8`
    if (!parser_check(parser, TOKEN_R_BRACKET))
    {
        size = parse_expr(parser);
        if (!size)
        {
            parser_error(parser, parser_curr(parser), "expected expression after '['");
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_R_BRACKET))
    {
        parser_error(parser, parser_curr(parser), "expected ']' after array size");
        node_free(size);
        return NULL;
    }

    Node *type = parse_type(parser);
    if (!type)
    {
        parser_error(parser, parser_curr(parser), "expected type after ']'");
        node_free(size);
        return NULL;
    }

    Node *arr_type = calloc(sizeof(Node), 1);
    if (!node_init(arr_type, NODE_TYPE_ARR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(size);
        node_free(type);
        return NULL;
    }
    arr_type->type_arr.size = size;
    arr_type->type_arr.type_node = type;

    return arr_type;
}

Node *parse_type_ptr(Parser *parser)
{
    Token token = parser_prev(parser);

    Node *type = parse_type(parser);
    if (!type)
    {
        parser_error(parser, parser_curr(parser), "expected type after '#'");
        return NULL;
    }

    Node *type_ref = calloc(sizeof(Node), 1);
    if (!node_init(type_ref, NODE_TYPE_PTR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_free(type);
        return NULL;
    }
    type_ref->type_ptr.type_node = type;

    return type_ref;
}

Node *parse_type_fun(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_match(parser, TOKEN_L_PAREN))
    {
        parser_error(parser, parser_curr(parser), "expected '(' after 'fun'");
        return NULL;
    }

    NodeList *params = calloc(sizeof(NodeList), 1);
    if (!node_list_init(params))
    {
        fprintf(stderr, "failed to initialize node list\n");
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_PAREN) && !parser_is_at_end(parser))
    {
        Node *param = parse_type(parser);
        if (!param)
        {
            parser_error(parser, parser_curr(parser), "expected parameter type");
            node_list_free(params);
            return NULL;
        }

        node_list_add(params, param);

        if (!parser_match(parser, TOKEN_COMMA))
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_R_PAREN))
    {
        parser_error(parser, parser_curr(parser), "expected ')' after function parameters");
        node_list_free(params);
        return NULL;
    }

    Node *type = NULL;
    if (parser_match(parser, TOKEN_COLON))
    {
        type = parse_type(parser);
        if (!type)
        {
            parser_error(parser, parser_curr(parser), "expected type after ':'");
            node_list_free(params);
            return NULL;
        }
    }

    Node *fun_type = calloc(sizeof(Node), 1);
    if (!node_init(fun_type, NODE_TYPE_FUN, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_list_free(params);
        node_free(type);
        return NULL;
    }
    fun_type->type_fun.params = params;
    fun_type->type_fun.type_node = type;

    return fun_type;
}

Node *parse_type_str(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_match(parser, TOKEN_L_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '{' after 'str'");
        return NULL;
    }

    NodeList *fields = calloc(sizeof(NodeList), 1);
    if (!node_list_init(fields))
    {
        fprintf(stderr, "failed to initialize node list\n");
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        Node *field = parse_field(parser);
        if (!field)
        {
            parser_error(parser, parser_curr(parser), "expected field declaration");
            node_list_free(fields);
            return NULL;
        }

        node_list_add(fields, field);

        if (!parser_match(parser, TOKEN_SEMICOLON))
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_R_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '}' after struct fields");
        node_list_free(fields);
        return NULL;
    }

    Node *str_type = calloc(sizeof(Node), 1);
    if (!node_init(str_type, NODE_TYPE_STR, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_list_free(fields);
        return NULL;
    }
    str_type->type_str.fields = fields;

    return str_type;
}

Node *parse_type_uni(Parser *parser)
{
    Token token = parser_prev(parser);

    if (!parser_match(parser, TOKEN_L_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '{' after 'uni'");
        return NULL;
    }

    NodeList *fields = calloc(sizeof(NodeList), 1);
    if (!node_list_init(fields))
    {
        fprintf(stderr, "failed to initialize node list\n");
        return NULL;
    }

    while (!parser_check(parser, TOKEN_R_BRACE) && !parser_is_at_end(parser))
    {
        Node *field = parse_field(parser);
        if (!field)
        {
            parser_error(parser, parser_curr(parser), "expected field declaration");
            node_list_free(fields);
            return NULL;
        }

        node_list_add(fields, field);

        if (!parser_match(parser, TOKEN_SEMICOLON))
        {
            break;
        }
    }

    if (!parser_match(parser, TOKEN_R_BRACE))
    {
        parser_error(parser, parser_curr(parser), "expected '}' after union fields");
        node_list_free(fields);
        return NULL;
    }

    Node *uni_type = calloc(sizeof(Node), 1);
    if (!node_init(uni_type, NODE_TYPE_UNI, token))
    {
        fprintf(stderr, "failed to initialize node\n");
        node_list_free(fields);
        return NULL;
    }
    uni_type->type_uni.fields = fields;

    return uni_type;
}

Node *parse_type(Parser *parser)
{
    switch (parser_curr(parser).kind)
    {
    case TOKEN_ASTERISK:
        parser_advance(parser);
        return parse_type_ptr(parser);
    case TOKEN_L_BRACKET:
        parser_advance(parser);
        return parse_type_arr(parser);
    case TOKEN_FUN:
        parser_advance(parser);
        return parse_type_fun(parser);
    case TOKEN_STR:
        parser_advance(parser);
        return parse_type_str(parser);
    case TOKEN_UNI:
        parser_advance(parser);
        return parse_type_uni(parser);
    default:
        return parse_identifier(parser);
    }
}

Node *parse_module(Parser *parser)
{
    Node *module = calloc(sizeof(Node), 1);
    if (!node_init(module, NODE_MODULE, (Token){0}))
    {
        fprintf(stderr, "failed to initialize node\n");
        return NULL;
    }
    module->module.stmts = calloc(sizeof(NodeList), 1);
    if (!node_list_init(module->module.stmts))
    {
        fprintf(stderr, "failed to initialize node list\n");
        node_free(module);
        return NULL;
    }

    while (!parser_is_at_end(parser))
    {
        Node *stmt = parse_stmt(parser);
        if (stmt != NULL)
        {
            node_list_add(module->module.stmts, stmt);
        }
    }

    return module;
}

Node *parser_parse(Parser *parser)
{
    if (!parser_build_token_list(parser))
    {
        fprintf(stderr, "failed to build token list\n");
        return NULL;
    }

    return parse_module(parser);
}

bool parser_build_token_list(Parser *parser)
{
    while (!lexer_at_end(parser->lexer))
    {
        Token next = lexer_next(parser->lexer);
        if (next.kind == TOKEN_ERROR)
        {
            int line = lexer_line_at(parser->lexer, next.start);
            int col = lexer_column_at(parser->lexer, next.start);
            fprintf(stderr, "failed to parse token at %d:%d (`%.*s`)\n", line, col, next.len, next.start);
            return false;
        }

        if (parser->tokens_len >= parser->tokens_cap)
        {
            int new_cap = parser->tokens_cap * 2;
            Token *new_tokens = realloc(parser->tokens, sizeof(Token) * new_cap);
            if (!new_tokens)
            {
                fprintf(stderr, "failed to grow tokens array\n");
                return false;
            }

            parser->tokens = new_tokens;
            parser->tokens_cap = new_cap;
        }

        parser->tokens[parser->tokens_len] = next;
        parser->tokens_len++;
    }

    return true;
}