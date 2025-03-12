#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool lexer_init(Lexer *lexer, const char *source)
{
    if (!lexer)
    {
        return false;
    }

    lexer->source = source;
    lexer->pos = source;
    lexer->error = NULL;

    return true;
}

void lexer_free(Lexer *lexer) {
    lexer->source = NULL;
    lexer->pos = NULL;
    lexer->error = NULL;

    free(lexer);
}

bool lexer_at_end(Lexer *lexer) { return *lexer->pos == '\0'; }

char lexer_current(Lexer *lexer) { return *lexer->pos; }

char lexer_peek(Lexer *lexer, int offset) { return lexer->pos[offset]; }

void lexer_advance(Lexer *lexer) { lexer->pos++; }

int lexer_line_at(Lexer *lexer, const char *pos)
{
    int line = 1;
    for (const char *p = lexer->source; p < pos; p++)
    {
        if (*p == '\n')
        {
            line++;
        }
    }
    return line;
}

int lexer_column_at(Lexer *lexer, const char *pos)
{
    int column = 1;
    for (const char *p = lexer->source; p < pos; p++)
    {
        if (*p == '\n')
        {
            column = 1;
        }
        else
        {
            column++;
        }
    }
    return column;
}

char *lexer_get_line(Lexer *lexer, int line)
{
    int current_line = 1;
    const char *line_start = lexer->source;
    for (const char *p = lexer->source; *p != '\0'; p++)
    {
        if (*p == '\n')
        {
            if (current_line == line)
            {
                return strndup(line_start, p - line_start);
            }
            current_line++;
            line_start = p + 1;
        }
    }

    return NULL;
}

Token lexer_parse_comment(Lexer *lexer)
{
    const char *start = lexer->pos;

    while (!lexer_at_end(lexer) && lexer_current(lexer) != '\n')
    {
        lexer_advance(lexer);
    }

    return token_new(TOKEN_HASH, start, lexer->pos - start);
}

Token lexer_parse_identifier(Lexer *lexer)
{
    const char *start = lexer->pos;

    while (isalnum(lexer_current(lexer)) || lexer_current(lexer) == '_')
    {
        lexer_advance(lexer);
    }

    int len = lexer->pos - start;
    if (len == 3 && memcmp(start, "val", 3) == 0)
    {
        return token_new(TOKEN_VAL, start, len);
    }
    else if (len == 3 && memcmp(start, "var", 3) == 0)
    {
        return token_new(TOKEN_VAR, start, len);
    }
    else if (len == 3 && memcmp(start, "vol", 3) == 0)
    {
        return token_new(TOKEN_VOL, start, len);
    }
    else if (len == 3 && memcmp(start, "def", 3) == 0)
    {
        return token_new(TOKEN_DEF, start, len);
    }
    else if (len == 3 && memcmp(start, "use", 3) == 0)
    {
        return token_new(TOKEN_USE, start, len);
    }
    else if (len == 3 && memcmp(start, "str", 3) == 0)
    {
        return token_new(TOKEN_STR, start, len);
    }
    else if (len == 3 && memcmp(start, "uni", 3) == 0)
    {
        return token_new(TOKEN_UNI, start, len);
    }
    else if (len == 3 && memcmp(start, "fun", 3) == 0)
    {
        return token_new(TOKEN_FUN, start, len);
    }
    else if (len == 2 && memcmp(start, "if", 2) == 0)
    {
        return token_new(TOKEN_IF, start, len);
    }
    else if (len == 2 && memcmp(start, "or", 2) == 0)
    {
        return token_new(TOKEN_OR, start, len);
    }
    else if (len == 3 && memcmp(start, "for", 3) == 0)
    {
        return token_new(TOKEN_FOR, start, len);
    }
    else if (len == 3 && memcmp(start, "brk", 3) == 0)
    {
        return token_new(TOKEN_BRK, start, len);
    }
    else if (len == 3 && memcmp(start, "cnt", 3) == 0)
    {
        return token_new(TOKEN_CNT, start, len);
    }
    else if (len == 3 && memcmp(start, "ret", 3) == 0)
    {
        return token_new(TOKEN_RET, start, len);
    }

    return token_new(TOKEN_IDENTIFIER, start, lexer->pos - start);
}

Token lexer_parse_lit_number(Lexer *lexer)
{
    const char *start = lexer->pos;

    if (lexer_current(lexer) == '0')
    {
        int base = 10;

        switch (lexer_peek(lexer, 1))
        {
        case 'x':
        case 'X':
            base = 16;
            lexer_advance(lexer);
            lexer_advance(lexer);
            break;
        case 'b':
        case 'B':
            base = 2;
            lexer_advance(lexer);
            lexer_advance(lexer);
            break;
        case 'o':
        case 'O':
            base = 8;
            lexer_advance(lexer);
            lexer_advance(lexer);
            break;
        }

        switch (base)
        {
        case 2:
            while (!lexer_at_end(lexer) && ((lexer_current(lexer) == '0' || lexer_current(lexer) == '1') || lexer_current(lexer) == '_'))
            {
                lexer_advance(lexer);
            }
            return token_new(TOKEN_LIT_INT, start, lexer->pos - start);
        case 8:
            while (!lexer_at_end(lexer) && ((lexer_current(lexer) >= '0' && lexer_current(lexer) <= '7') || lexer_current(lexer) == '_'))
            {
                lexer_advance(lexer);
            }
            return token_new(TOKEN_LIT_INT, start, lexer->pos - start);
        case 16:
            while (!lexer_at_end(lexer) && (isxdigit(lexer_current(lexer)) || lexer_current(lexer) == '_'))
            {
                lexer_advance(lexer);
            }
            return token_new(TOKEN_LIT_INT, start, lexer->pos - start);
        default:
            lexer_advance(lexer);
            break;
        }
    }

    while (!lexer_at_end(lexer) && (isdigit(lexer_current(lexer)) || lexer_current(lexer) == '_'))
    {
        lexer_advance(lexer);
    }

    if (lexer_current(lexer) == '.')
    {
        lexer_advance(lexer);

        while (!lexer_at_end(lexer) && (isdigit(lexer_current(lexer)) || lexer_current(lexer) == '_'))
        {
            lexer_advance(lexer);
        }

        return token_new(TOKEN_LIT_FLOAT, start, lexer->pos - start);
    }

    return token_new(TOKEN_LIT_INT, start, lexer->pos - start);
}

Token lexer_parse_lit_character(Lexer *lexer)
{
    const char *start = lexer->pos;

    if (lexer_current(lexer) != '\'')
    {
        return token_new(TOKEN_ERROR, start, 1);
    }

    lexer_advance(lexer);

    if (lexer_at_end(lexer))
    {
        return token_new(TOKEN_ERROR, start, 1);
    }

    if (lexer_current(lexer) == '\\')
    {
        lexer_advance(lexer);

        switch (lexer_current(lexer))
        {
        case '\'':
        case '\"':
        case '\\':
        case 'n':
        case 't':
        case 'r':
        case '0':
            lexer_advance(lexer);
            break;
        default:
            return token_new(TOKEN_ERROR, start, lexer->pos - start);
        }
    }
    else
    {
        lexer_advance(lexer);
    }

    if (lexer_at_end(lexer) || lexer_current(lexer) != '\'')
    {
        return token_new(TOKEN_ERROR, start, lexer->pos - start);
    }

    lexer_advance(lexer);

    return token_new(TOKEN_LIT_CHAR, start, lexer->pos - start);
}

Token lexer_parse_lit_string(Lexer *lexer)
{
    const char *start = lexer->pos;

    if (lexer_current(lexer) != '\"')
    {
        return token_new(TOKEN_ERROR, start, 1);
    }

    lexer_advance(lexer);

    while (!lexer_at_end(lexer) && lexer_current(lexer) != '\"')
    {
        if (lexer_current(lexer) == '\\')
        {
            lexer_advance(lexer);

            switch (lexer_current(lexer))
            {
            case '\'':
            case '\"':
            case '\\':
            case 'n':
            case 't':
            case 'r':
            case '0':
                lexer_advance(lexer);
                break;
            default:
                return token_new(TOKEN_ERROR, start, lexer->pos - start);
            }
        }
        else
        {
            lexer_advance(lexer);
        }
    }

    if (lexer_at_end(lexer) || lexer_current(lexer) != '\"')
    {
        return token_new(TOKEN_ERROR, start, lexer->pos - start);
    }

    lexer_advance(lexer);

    return token_new(TOKEN_LIT_STRING, start, lexer->pos - start);
}

Token lexer_emit(Lexer *lexer, TokenKind kind, int len)
{
    Token token;
    token.kind = kind;
    token.start = lexer->pos;
    token.len = len;

    for (int i = 0; i < len; i++)
    {
        lexer_advance(lexer);
    }

    return token;
}

long long lexer_eval_lit_int(const char *start, int len)
{
    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        return 0;
    }

    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (start[i] != '_')
        {
            buffer[j++] = start[i];
        }
    }
    buffer[j] = '\0';

    long long value = strtoll(buffer, NULL, 0);
    free(buffer);

    return value;
}

double lexer_eval_lit_float(const char *start, int len)
{
    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        return 0.0;
    }

    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (start[i] != '_')
        {
            buffer[j++] = start[i];
        }
    }
    buffer[j] = '\0';

    double value = strtod(buffer, NULL);
    free(buffer);

    return value;
}

char lexer_eval_lit_char(const char *start, int len)
{
    if (len < 3)
    {
        return '\0';
    }

    if (start[0] != '\'' || start[len - 1] != '\'')
    {
        return '\0';
    }

    if (start[1] == '\\')
    {
        switch (start[2])
        {
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '\\':
            return '\\';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '0':
            return '\0';
        default:
            return '\0';
        }
    }

    return start[1];
}

char *lexer_eval_lit_string(const char *start, int len)
{
    char *buffer = malloc(len + 1);
    if (!buffer)
    {
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (start[i] == '\\')
        {
            i++;
            switch (start[i])
            {
            case '\'':
                buffer[j++] = '\'';
                break;
            case '\"':
                buffer[j++] = '\"';
                break;
            case '\\':
                buffer[j++] = '\\';
                break;
            case 'n':
                buffer[j++] = '\n';
                break;
            case 't':
                buffer[j++] = '\t';
                break;
            case 'r':
                buffer[j++] = '\r';
                break;
            case '0':
                buffer[j++] = '\0';
                break;
            default:
                buffer[j++] = start[i];
                break;
            }
        }
        else
        {
            buffer[j++] = start[i];
        }
    }
    buffer[j] = '\0';

    return buffer;
}

Token lexer_next(Lexer *lexer)
{
    char c = lexer_current(lexer);

    // skip whitespace
    while (!lexer_at_end(lexer) && (c == ' ' || c == '\t' || c == '\r' || c == '\n'))
    {
        lexer_advance(lexer);
        c = lexer_current(lexer);
    }

    switch (c)
    {
    case '\0':
        return lexer_emit(lexer, TOKEN_EOF, 0);
    case 'a' ... 'z':
    case 'A' ... 'Z':
    case '_':
        return lexer_parse_identifier(lexer);
    case '0' ... '9':
        return lexer_parse_lit_number(lexer);
    case '\'':
        return lexer_parse_lit_character(lexer);
    case '\"':
        return lexer_parse_lit_string(lexer);
    case '(':
        return lexer_emit(lexer, TOKEN_L_PAREN, 1);
    case ')':
        return lexer_emit(lexer, TOKEN_R_PAREN, 1);
    case '[':
        return lexer_emit(lexer, TOKEN_L_BRACKET, 1);
    case ']':
        return lexer_emit(lexer, TOKEN_R_BRACKET, 1);
    case '{':
        return lexer_emit(lexer, TOKEN_L_BRACE, 1);
    case '}':
        return lexer_emit(lexer, TOKEN_R_BRACE, 1);
    case ':':
        switch (lexer_peek(lexer, 1))
        {
        case ':':
            return lexer_emit(lexer, TOKEN_COLON_COLON, 2);
        default:
            return lexer_emit(lexer, TOKEN_COLON, 1);
        }
        break;
    case ';':
        return lexer_emit(lexer, TOKEN_SEMICOLON, 1);
    case '?':
        return lexer_emit(lexer, TOKEN_QUESTION, 1);
    case '@':
        return lexer_emit(lexer, TOKEN_AT, 1);
    case '#':
        return lexer_parse_comment(lexer);
    case '.':
        return lexer_emit(lexer, TOKEN_DOT, 1);
    case ',':
        return lexer_emit(lexer, TOKEN_COMMA, 1);
    case '+':
        return lexer_emit(lexer, TOKEN_PLUS, 1);
    case '-':
        return lexer_emit(lexer, TOKEN_MINUS, 1);
    case '*':
        return lexer_emit(lexer, TOKEN_ASTERISK, 1);
    case '%':
        return lexer_emit(lexer, TOKEN_PERCENT, 1);
    case '^':
        return lexer_emit(lexer, TOKEN_CARET, 1);
    case '&':
        switch (lexer_peek(lexer, 1))
        {
        case '&':
            return lexer_emit(lexer, TOKEN_AMPERSAND_AMPERSAND, 2);
        default:
            return lexer_emit(lexer, TOKEN_AMPERSAND, 1);
        }
        break;
    case '|':
        switch (lexer_peek(lexer, 1))
        {
        case '|':
            return lexer_emit(lexer, TOKEN_PIPE_PIPE, 2);
        default:
            return lexer_emit(lexer, TOKEN_PIPE, 1);
        }
        break;
    case '~':
        return lexer_emit(lexer, TOKEN_TILDE, 1);
    case '<':
        switch (lexer_peek(lexer, 1))
        {
        case '<':
            return lexer_emit(lexer, TOKEN_LESS_LESS, 2);
        case '=':
            return lexer_emit(lexer, TOKEN_LESS_EQUAL, 2);
        default:
            return lexer_emit(lexer, TOKEN_LESS, 1);
        }
        break;
    case '>':
        switch (lexer_peek(lexer, 1))
        {
        case '>':
            return lexer_emit(lexer, TOKEN_GREATER_GREATER, 2);
        case '=':
            return lexer_emit(lexer, TOKEN_GREATER_EQUAL, 2);
        default:
            return lexer_emit(lexer, TOKEN_GREATER, 1);
        }
        break;
    case '=':
        switch (lexer_peek(lexer, 1))
        {
        case '=':
            return lexer_emit(lexer, TOKEN_EQUAL_EQUAL, 2);
        default:
            return lexer_emit(lexer, TOKEN_EQUAL, 1);
        }
        break;
    case '!':
        switch (lexer_peek(lexer, 1))
        {
        case '=':
            return lexer_emit(lexer, TOKEN_BANG_EQUAL, 2);
        default:
            return lexer_emit(lexer, TOKEN_BANG, 1);
        }
        break;
    case '/':
        return lexer_emit(lexer, TOKEN_SLASH, 1);
    case '\\':
        return lexer_emit(lexer, TOKEN_BACKSLASH, 1);
    default:
        return lexer_emit(lexer, TOKEN_ERROR, 1);
    }
}
