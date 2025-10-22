#include "preprocessor.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct PreprocessorBuffer
{
    char  *data;
    size_t length;
    size_t capacity;
} PreprocessorBuffer;

typedef struct PreprocessorFrame
{
    bool parent_active;
    bool branch_taken;
    bool current_active;
} PreprocessorFrame;

typedef struct PreprocessorFrameStack
{
    PreprocessorFrame *items;
    size_t             count;
    size_t             capacity;
} PreprocessorFrameStack;

typedef struct PreprocessorExprScanner
{
    const char *text;
    size_t      pos;
    size_t      len;
} PreprocessorExprScanner;

typedef enum PreprocessorExprTokenKind
{
    PREPROC_EXPR_TOKEN_END = 0,
    PREPROC_EXPR_TOKEN_NUMBER,
    PREPROC_EXPR_TOKEN_IDENTIFIER,
    PREPROC_EXPR_TOKEN_LPAREN,
    PREPROC_EXPR_TOKEN_RPAREN,
    PREPROC_EXPR_TOKEN_NOT,
    PREPROC_EXPR_TOKEN_AND,
    PREPROC_EXPR_TOKEN_OR,
    PREPROC_EXPR_TOKEN_EQ,
    PREPROC_EXPR_TOKEN_NEQ,
} PreprocessorExprTokenKind;

typedef struct PreprocessorExprToken
{
    PreprocessorExprTokenKind kind;
    char                     *text;
} PreprocessorExprToken;

static void preprocessor_buffer_init(PreprocessorBuffer *buffer)
{
    buffer->data     = NULL;
    buffer->length   = 0;
    buffer->capacity = 0;
}

static void preprocessor_buffer_dnit(PreprocessorBuffer *buffer)
{
    if (!buffer)
    {
        return;
    }

    free(buffer->data);
    buffer->data     = NULL;
    buffer->length   = 0;
    buffer->capacity = 0;
}

static bool preprocessor_buffer_reserve(PreprocessorBuffer *buffer, size_t extra)
{
    if (!buffer)
    {
        return false;
    }

    size_t required = buffer->length + extra + 1;
    if (required <= buffer->capacity)
    {
        return true;
    }

    size_t new_capacity = buffer->capacity == 0 ? 128 : buffer->capacity * 2;
    while (new_capacity < required)
    {
        new_capacity *= 2;
    }

    char *new_data = realloc(buffer->data, new_capacity);
    if (!new_data)
    {
        return false;
    }

    buffer->data     = new_data;
    buffer->capacity = new_capacity;
    return true;
}

static bool preprocessor_buffer_append_range(PreprocessorBuffer *buffer, const char *text, size_t len)
{
    if (!text || len == 0)
    {
        return true;
    }

    if (!preprocessor_buffer_reserve(buffer, len))
    {
        return false;
    }

    memcpy(buffer->data + buffer->length, text, len);
    buffer->length += len;
    buffer->data[buffer->length] = '\0';
    return true;
}

static bool preprocessor_buffer_append_char(PreprocessorBuffer *buffer, char c)
{
    if (!preprocessor_buffer_reserve(buffer, 1))
    {
        return false;
    }

    buffer->data[buffer->length++] = c;
    buffer->data[buffer->length]   = '\0';
    return true;
}

static char *preprocessor_buffer_take(PreprocessorBuffer *buffer)
{
    char *data       = buffer->data;
    buffer->data     = NULL;
    buffer->length   = 0;
    buffer->capacity = 0;
    return data;
}

static void preprocessor_frame_stack_init(PreprocessorFrameStack *stack)
{
    stack->items    = NULL;
    stack->count    = 0;
    stack->capacity = 0;
}

static void preprocessor_frame_stack_dnit(PreprocessorFrameStack *stack)
{
    if (!stack)
    {
        return;
    }

    free(stack->items);
    stack->items    = NULL;
    stack->count    = 0;
    stack->capacity = 0;
}

static bool preprocessor_frame_stack_push(PreprocessorFrameStack *stack, PreprocessorFrame frame)
{
    if (stack->count >= stack->capacity)
    {
        size_t             new_capacity = stack->capacity == 0 ? 4 : stack->capacity * 2;
        PreprocessorFrame *items        = realloc(stack->items, new_capacity * sizeof(PreprocessorFrame));
        if (!items)
        {
            return false;
        }
        stack->items    = items;
        stack->capacity = new_capacity;
    }

    stack->items[stack->count++] = frame;
    return true;
}

static bool preprocessor_frame_stack_pop(PreprocessorFrameStack *stack, PreprocessorFrame *frame_out)
{
    if (!stack || stack->count == 0)
    {
        return false;
    }

    stack->count--;
    if (frame_out)
    {
        *frame_out = stack->items[stack->count];
    }
    return true;
}

static PreprocessorFrame *preprocessor_frame_stack_top(PreprocessorFrameStack *stack)
{
    if (!stack || stack->count == 0)
    {
        return NULL;
    }
    return &stack->items[stack->count - 1];
}

static long long preprocessor_constant_lookup(const PreprocessorConstant *constants, size_t constant_count, const char *name, bool *found)
{
    for (size_t i = 0; i < constant_count; i++)
    {
        if (strcmp(constants[i].name, name) == 0)
        {
            if (found)
            {
                *found = true;
            }
            return constants[i].value;
        }
    }

    if (found)
    {
        *found = false;
    }
    return 0;
}

static char *preprocessor_strdup_range(const char *start, size_t length)
{
    char *copy = malloc(length + 1);
    if (!copy)
    {
        return NULL;
    }
    memcpy(copy, start, length);
    copy[length] = '\0';
    return copy;
}

static void preprocessor_expr_scanner_init(PreprocessorExprScanner *scanner, const char *text)
{
    scanner->text = text ? text : "";
    scanner->pos  = 0;
    scanner->len  = strlen(scanner->text);
}

static void preprocessor_expr_skip_ws(PreprocessorExprScanner *scanner)
{
    while (scanner->pos < scanner->len && (scanner->text[scanner->pos] == ' ' || scanner->text[scanner->pos] == '\t'))
    {
        scanner->pos++;
    }
}

static void preprocessor_expr_token_init(PreprocessorExprToken *token)
{
    token->kind = PREPROC_EXPR_TOKEN_END;
    token->text = NULL;
}

static void preprocessor_expr_token_dnit(PreprocessorExprToken *token)
{
    if (!token)
    {
        return;
    }

    free(token->text);
    token->text = NULL;
    token->kind = PREPROC_EXPR_TOKEN_END;
}

static PreprocessorExprToken preprocessor_expr_next_token(PreprocessorExprScanner *scanner)
{
    PreprocessorExprToken token;
    preprocessor_expr_token_init(&token);

    preprocessor_expr_skip_ws(scanner);

    if (scanner->pos >= scanner->len)
    {
        token.kind = PREPROC_EXPR_TOKEN_END;
        return token;
    }

    char c = scanner->text[scanner->pos];
    if (isdigit((unsigned char)c))
    {
        size_t start = scanner->pos;
        while (scanner->pos < scanner->len)
        {
            char ch = scanner->text[scanner->pos];
            if (isalnum((unsigned char)ch) || ch == '_' || ch == 'x' || ch == 'X')
            {
                scanner->pos++;
            }
            else
            {
                break;
            }
        }
        token.kind = PREPROC_EXPR_TOKEN_NUMBER;
        token.text = preprocessor_strdup_range(scanner->text + start, scanner->pos - start);
        return token;
    }

    if (isalpha((unsigned char)c) || c == '_')
    {
        size_t start = scanner->pos;
        while (scanner->pos < scanner->len && (isalnum((unsigned char)scanner->text[scanner->pos]) || scanner->text[scanner->pos] == '_'))
        {
            scanner->pos++;
        }
        token.kind = PREPROC_EXPR_TOKEN_IDENTIFIER;
        token.text = preprocessor_strdup_range(scanner->text + start, scanner->pos - start);
        return token;
    }

    scanner->pos++;
    switch (c)
    {
    case '(':
        token.kind = PREPROC_EXPR_TOKEN_LPAREN;
        break;
    case ')':
        token.kind = PREPROC_EXPR_TOKEN_RPAREN;
        break;
    case '!':
        if (scanner->pos < scanner->len && scanner->text[scanner->pos] == '=')
        {
            scanner->pos++;
            token.kind = PREPROC_EXPR_TOKEN_NEQ;
        }
        else
        {
            token.kind = PREPROC_EXPR_TOKEN_NOT;
        }
        break;
    case '&':
        if (scanner->pos < scanner->len && scanner->text[scanner->pos] == '&')
        {
            scanner->pos++;
            token.kind = PREPROC_EXPR_TOKEN_AND;
        }
        break;
    case '|':
        if (scanner->pos < scanner->len && scanner->text[scanner->pos] == '|')
        {
            scanner->pos++;
            token.kind = PREPROC_EXPR_TOKEN_OR;
        }
        break;
    case '=':
        if (scanner->pos < scanner->len && scanner->text[scanner->pos] == '=')
        {
            scanner->pos++;
            token.kind = PREPROC_EXPR_TOKEN_EQ;
        }
        break;
    default:
        token.kind = PREPROC_EXPR_TOKEN_END;
        break;
    }

    return token;
}

static long long preprocessor_parse_number(const char *text, bool *ok)
{
    if (!text)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    char *clean = malloc(strlen(text) + 1);
    if (!clean)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    size_t j = 0;
    for (size_t i = 0; text[i]; i++)
    {
        if (text[i] != '_')
        {
            clean[j++] = text[i];
        }
    }
    clean[j] = '\0';

    char     *endptr = NULL;
    long long value  = strtoll(clean, &endptr, 0);
    if (endptr == clean)
    {
        if (ok)
        {
            *ok = false;
        }
        free(clean);
        return 0;
    }

    free(clean);
    if (ok)
    {
        *ok = true;
    }
    return value;
}

static long long preprocessor_expr_parse_or(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok);
static long long preprocessor_expr_parse_and(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok);
static long long preprocessor_expr_parse_equality(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok);
static long long preprocessor_expr_parse_unary(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok);
static long long preprocessor_expr_parse_primary(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok);

static long long preprocessor_expr_parse_primary(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok)
{
    preprocessor_expr_skip_ws(scanner);
    size_t                snapshot = scanner->pos;
    PreprocessorExprToken token    = preprocessor_expr_next_token(scanner);

    if (token.kind == PREPROC_EXPR_TOKEN_NUMBER)
    {
        bool      local_ok = false;
        long long value    = preprocessor_parse_number(token.text, &local_ok);
        preprocessor_expr_token_dnit(&token);
        if (!local_ok)
        {
            if (ok)
            {
                *ok = false;
            }
            return 0;
        }
        if (ok)
        {
            *ok = true;
        }
        return value;
    }

    if (token.kind == PREPROC_EXPR_TOKEN_IDENTIFIER)
    {
        bool      found = false;
        long long value = preprocessor_constant_lookup(constants, constant_count, token.text, &found);
        preprocessor_expr_token_dnit(&token);
        if (ok)
        {
            *ok = true;
        }
        return found ? value : 0;
    }

    if (token.kind == PREPROC_EXPR_TOKEN_LPAREN)
    {
        preprocessor_expr_token_dnit(&token);
        long long value = preprocessor_expr_parse_or(scanner, constants, constant_count, ok);
        if (!ok || !*ok)
        {
            return 0;
        }
        preprocessor_expr_skip_ws(scanner);
        if (scanner->pos < scanner->len && scanner->text[scanner->pos] == ')')
        {
            scanner->pos++;
            return value;
        }
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    preprocessor_expr_token_dnit(&token);
    scanner->pos = snapshot;
    if (ok)
    {
        *ok = false;
    }
    return 0;
}

static long long preprocessor_expr_parse_unary(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok)
{
    preprocessor_expr_skip_ws(scanner);
    if (scanner->pos < scanner->len && scanner->text[scanner->pos] == '!')
    {
        scanner->pos++;
        long long value = preprocessor_expr_parse_unary(scanner, constants, constant_count, ok);
        if (!ok || !*ok)
        {
            return 0;
        }
        return value ? 0 : 1;
    }

    return preprocessor_expr_parse_primary(scanner, constants, constant_count, ok);
}

static long long preprocessor_expr_parse_equality(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok)
{
    bool      local_ok = false;
    long long left     = preprocessor_expr_parse_unary(scanner, constants, constant_count, &local_ok);
    if (!local_ok)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    for (;;)
    {
        preprocessor_expr_skip_ws(scanner);
        size_t snapshot = scanner->pos;
        if (scanner->pos < scanner->len - 1 && scanner->text[scanner->pos] == '=' && scanner->text[scanner->pos + 1] == '=')
        {
            scanner->pos += 2;
            long long right = preprocessor_expr_parse_unary(scanner, constants, constant_count, &local_ok);
            if (!local_ok)
            {
                if (ok)
                {
                    *ok = false;
                }
                return 0;
            }
            left = (left == right) ? 1 : 0;
            continue;
        }
        if (scanner->pos < scanner->len - 1 && scanner->text[scanner->pos] == '!' && scanner->text[scanner->pos + 1] == '=')
        {
            scanner->pos += 2;
            long long right = preprocessor_expr_parse_unary(scanner, constants, constant_count, &local_ok);
            if (!local_ok)
            {
                if (ok)
                {
                    *ok = false;
                }
                return 0;
            }
            left = (left != right) ? 1 : 0;
            continue;
        }

        scanner->pos = snapshot;
        break;
    }

    if (ok)
    {
        *ok = true;
    }
    return left;
}

static long long preprocessor_expr_parse_and(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok)
{
    bool      local_ok = false;
    long long left     = preprocessor_expr_parse_equality(scanner, constants, constant_count, &local_ok);
    if (!local_ok)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    for (;;)
    {
        preprocessor_expr_skip_ws(scanner);
        size_t snapshot = scanner->pos;
        if (scanner->pos < scanner->len - 1 && scanner->text[scanner->pos] == '&' && scanner->text[scanner->pos + 1] == '&')
        {
            scanner->pos += 2;
            long long right = preprocessor_expr_parse_equality(scanner, constants, constant_count, &local_ok);
            if (!local_ok)
            {
                if (ok)
                {
                    *ok = false;
                }
                return 0;
            }
            left = (left && right) ? 1 : 0;
            continue;
        }
        scanner->pos = snapshot;
        break;
    }

    if (ok)
    {
        *ok = true;
    }
    return left;
}

static long long preprocessor_expr_parse_or(PreprocessorExprScanner *scanner, const PreprocessorConstant *constants, size_t constant_count, bool *ok)
{
    bool      local_ok = false;
    long long left     = preprocessor_expr_parse_and(scanner, constants, constant_count, &local_ok);
    if (!local_ok)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    for (;;)
    {
        preprocessor_expr_skip_ws(scanner);
        size_t snapshot = scanner->pos;
        if (scanner->pos < scanner->len - 1 && scanner->text[scanner->pos] == '|' && scanner->text[scanner->pos + 1] == '|')
        {
            scanner->pos += 2;
            long long right = preprocessor_expr_parse_and(scanner, constants, constant_count, &local_ok);
            if (!local_ok)
            {
                if (ok)
                {
                    *ok = false;
                }
                return 0;
            }
            left = (left || right) ? 1 : 0;
            continue;
        }
        scanner->pos = snapshot;
        break;
    }

    if (ok)
    {
        *ok = true;
    }
    return left;
}

static long long preprocessor_eval_condition(const char *expr, const PreprocessorConstant *constants, size_t constant_count, bool *ok)
{
    PreprocessorExprScanner scanner;
    preprocessor_expr_scanner_init(&scanner, expr);
    bool      local_ok = false;
    long long value    = preprocessor_expr_parse_or(&scanner, constants, constant_count, &local_ok);
    if (!local_ok)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    preprocessor_expr_skip_ws(&scanner);
    if (scanner.pos != scanner.len)
    {
        if (ok)
        {
            *ok = false;
        }
        return 0;
    }

    if (ok)
    {
        *ok = true;
    }
    return value ? 1 : 0;
}

void preprocessor_output_init(PreprocessorOutput *output)
{
    if (!output)
    {
        return;
    }

    output->success = false;
    output->source  = NULL;
    output->message = NULL;
    output->line    = 0;
}

void preprocessor_output_dnit(PreprocessorOutput *output)
{
    if (!output)
    {
        return;
    }

    free(output->source);
    free(output->message);
    output->success = false;
    output->source  = NULL;
    output->message = NULL;
    output->line    = 0;
}

static bool preprocessor_handle_if(const char *rest, const PreprocessorConstant *constants, size_t constant_count, PreprocessorFrameStack *frames, bool *global_active, PreprocessorOutput *output, int line_no)
{
    bool      parent_active = *global_active;
    bool      cond_ok       = false;
    long long cond          = preprocessor_eval_condition(rest, constants, constant_count, &cond_ok);
    if (!cond_ok)
    {
        output->message = strdup("invalid #@if expression");
        output->line    = line_no;
        return false;
    }

    bool branch_active = parent_active && (cond != 0);

    PreprocessorFrame frame;
    frame.parent_active  = parent_active;
    frame.branch_taken   = branch_active;
    frame.current_active = branch_active;

    if (!preprocessor_frame_stack_push(frames, frame))
    {
        output->message = strdup("out of memory");
        output->line    = line_no;
        return false;
    }

    *global_active = branch_active;
    return true;
}

static bool preprocessor_handle_or(const char *rest, const PreprocessorConstant *constants, size_t constant_count, PreprocessorFrameStack *frames, bool *global_active, PreprocessorOutput *output, int line_no)
{
    PreprocessorFrame *frame = preprocessor_frame_stack_top(frames);
    if (!frame)
    {
        output->message = strdup("#@or without matching #@if");
        output->line    = line_no;
        return false;
    }

    bool      cond_ok = false;
    long long cond    = preprocessor_eval_condition(rest, constants, constant_count, &cond_ok);
    if (!cond_ok)
    {
        output->message = strdup("invalid #@or expression");
        output->line    = line_no;
        return false;
    }

    bool branch_active = false;
    if (frame->parent_active && !frame->branch_taken && cond != 0)
    {
        branch_active       = true;
        frame->branch_taken = true;
    }

    frame->current_active = branch_active;
    *global_active        = branch_active;
    return true;
}

static bool preprocessor_handle_end(PreprocessorFrameStack *frames, bool *global_active, PreprocessorOutput *output, int line_no)
{
    PreprocessorFrame frame;
    if (!preprocessor_frame_stack_pop(frames, &frame))
    {
        output->message = strdup("#@end without matching #@if");
        output->line    = line_no;
        return false;
    }

    if (frames->count == 0)
    {
        *global_active = true;
    }
    else
    {
        *global_active = frames->items[frames->count - 1].current_active;
    }
    return true;
}

static bool preprocessor_is_directive(const char *trimmed)
{
    return trimmed && trimmed[0] == '#' && trimmed[1] == '@';
}

bool preprocessor_run(const char *source, const PreprocessorConstant *constants, size_t constant_count, PreprocessorOutput *output)
{
    if (!output)
    {
        return false;
    }

    preprocessor_output_init(output);

    const char *input = source ? source : "";
    size_t      len   = strlen(input);

    PreprocessorBuffer buffer;
    preprocessor_buffer_init(&buffer);

    PreprocessorFrameStack frames;
    preprocessor_frame_stack_init(&frames);

    bool   global_active = true;
    size_t offset        = 0;
    int    line_no       = 1;
    bool   ok            = true;

    while (offset <= len && ok)
    {
        size_t line_start = offset;
        while (offset < len && input[offset] != '\n')
        {
            offset++;
        }

        size_t line_length = offset - line_start;
        bool   has_newline = (offset < len && input[offset] == '\n');

        char *line = preprocessor_strdup_range(input + line_start, line_length);
        if (!line)
        {
            output->message = strdup("out of memory");
            output->line    = line_no;
            ok              = false;
        }

        if (ok)
        {
            char *trim = line;
            while (*trim == ' ' || *trim == '\t')
            {
                trim++;
            }

            if (preprocessor_is_directive(trim))
            {
                const char *directive = trim + 2;
                while (*directive == ' ' || *directive == '\t')
                {
                    directive++;
                }

                // Check if it's a preprocessor conditional directive
                if (strncmp(directive, "if", 2) == 0 && (directive[2] == '\0' || directive[2] == ' ' || directive[2] == '\t' || directive[2] == '('))
                {
                    const char *expr = directive + 2;
                    ok               = preprocessor_handle_if(expr, constants, constant_count, &frames, &global_active, output, line_no);
                    // preserve line number by inserting blank line where directive was
                    if (ok && has_newline)
                    {
                        ok = preprocessor_buffer_append_char(&buffer, '\n');
                    }
                }
                else if (strncmp(directive, "or", 2) == 0 && (directive[2] == '\0' || directive[2] == ' ' || directive[2] == '\t' || directive[2] == '('))
                {
                    const char *expr = directive + 2;
                    ok               = preprocessor_handle_or(expr, constants, constant_count, &frames, &global_active, output, line_no);
                    // preserve line number by inserting blank line where directive was
                    if (ok && has_newline)
                    {
                        ok = preprocessor_buffer_append_char(&buffer, '\n');
                    }
                }
                else if (strncmp(directive, "end", 3) == 0 && (directive[3] == '\0' || directive[3] == ' ' || directive[3] == '\t'))
                {
                    ok = preprocessor_handle_end(&frames, &global_active, output, line_no);
                    // preserve line number by inserting blank line where directive was
                    if (ok && has_newline)
                    {
                        ok = preprocessor_buffer_append_char(&buffer, '\n');
                    }
                }
                else
                {
                    // Not a preprocessor directive (e.g., #@symbol), pass through as regular line
                    if (global_active)
                    {
                        ok = preprocessor_buffer_append_range(&buffer, line, line_length);
                    }
                    else if (has_newline)
                    {
                        // preserve line numbers by inserting blank line
                        ok = preprocessor_buffer_append_char(&buffer, '\n');
                    }
                    if (has_newline && ok && global_active)
                    {
                        ok = preprocessor_buffer_append_char(&buffer, '\n');
                    }
                }
            }
            else
            {
                if (global_active)
                {
                    ok = preprocessor_buffer_append_range(&buffer, line, line_length);
                }
                else if (has_newline)
                {
                    // preserve line numbers by inserting blank line for inactive lines
                    ok = preprocessor_buffer_append_char(&buffer, '\n');
                }
                if (has_newline && ok && global_active)
                {
                    ok = preprocessor_buffer_append_char(&buffer, '\n');
                }
            }
        }

        free(line);

        if (!ok)
        {
            break;
        }

        if (has_newline)
        {
            offset++;
        }
        else
        {
            offset++;
            break;
        }

        line_no++;
    }

    if (ok && frames.count != 0)
    {
        output->message = strdup("unterminated #@if block");
        output->line    = line_no;
        ok              = false;
    }

    if (ok)
    {
        output->source  = preprocessor_buffer_take(&buffer);
        output->success = true;
    }

    preprocessor_buffer_dnit(&buffer);
    preprocessor_frame_stack_dnit(&frames);

    return output->success;
}
