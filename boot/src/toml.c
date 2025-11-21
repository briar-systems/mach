#include "toml.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// lexer state
typedef struct
{
    const char *input;
    size_t      pos;
    size_t      line;
    size_t      col;
} lexer_t;

// parser state
typedef struct
{
    lexer_t lexer;
    char   *error;
} parser_t;

// forward declarations
static toml_value_t  parse_value(parser_t *p);
static toml_table_t *parse_table_inline(parser_t *p);
static toml_array_t *parse_array(parser_t *p);

// helper: skip whitespace and comments
static void skip_whitespace(lexer_t *l)
{
    while (l->input[l->pos])
    {
        char c = l->input[l->pos];
        if (c == ' ' || c == '\t' || c == '\r')
        {
            if (c == '\t')
            {
                l->col += 4;
            }
            else
            {
                l->col++;
            }
            l->pos++;
        }
        else if (c == '\n')
        {
            l->line++;
            l->col = 1;
            l->pos++;
        }
        else if (c == '#')
        {
            // skip comment until newline
            while (l->input[l->pos] && l->input[l->pos] != '\n')
            {
                l->pos++;
            }
        }
        else
        {
            break;
        }
    }
}

// helper: peek current character
static char peek(lexer_t *l)
{
    skip_whitespace(l);
    return l->input[l->pos];
}

// helper: consume and return current character
static char consume(lexer_t *l)
{
    skip_whitespace(l);
    char c = l->input[l->pos];
    if (c)
    {
        l->pos++;
        l->col++;
    }
    return c;
}

// helper: set error message
static void set_error(parser_t *p, const char *msg)
{
    if (p->error)
    {
        return; // keep first error
    }
    size_t len = strlen(msg) + 100;
    p->error   = malloc(len);
    snprintf(p->error, len, "line %zu, col %zu: %s", p->lexer.line, p->lexer.col, msg);
}

// parse string (basic or literal)
static char *parse_string(parser_t *p)
{
    lexer_t *l = &p->lexer;
    skip_whitespace(l);

    char quote = l->input[l->pos];
    if (quote != '"' && quote != '\'')
    {
        set_error(p, "expected string");
        return NULL;
    }

    l->pos++;
    l->col++;

    size_t start    = l->pos;
    size_t capacity = 64;
    size_t len      = 0;
    char  *result   = malloc(capacity);

    while (l->input[l->pos] && l->input[l->pos] != quote)
    {
        char c = l->input[l->pos];

        // handle escape sequences (only in double quotes)
        if (quote == '"' && c == '\\')
        {
            l->pos++;
            l->col++;
            if (!l->input[l->pos])
            {
                break;
            }

            char escaped = l->input[l->pos];
            switch (escaped)
            {
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case 'r':
                c = '\r';
                break;
            case '\\':
                c = '\\';
                break;
            case '"':
                c = '"';
                break;
            default:
                c = escaped;
                break;
            }
        }

        if (len + 1 >= capacity)
        {
            capacity *= 2;
            result = realloc(result, capacity);
        }

        result[len++] = c;
        l->pos++;
        l->col++;
    }

    if (l->input[l->pos] != quote)
    {
        free(result);
        set_error(p, "unterminated string");
        return NULL;
    }

    l->pos++;
    l->col++;
    result[len] = '\0';
    return result;
}

// parse key (bare, quoted)
static char *parse_key(parser_t *p)
{
    lexer_t *l = &p->lexer;
    skip_whitespace(l);

    // quoted key
    if (l->input[l->pos] == '"' || l->input[l->pos] == '\'')
    {
        return parse_string(p);
    }

    // bare key
    size_t start = l->pos;
    while (l->input[l->pos] && (isalnum(l->input[l->pos]) || l->input[l->pos] == '_' || l->input[l->pos] == '-'))
    {
        l->pos++;
        l->col++;
    }

    if (l->pos == start)
    {
        set_error(p, "expected key");
        return NULL;
    }

    size_t len = l->pos - start;
    char  *key = malloc(len + 1);
    memcpy(key, l->input + start, len);
    key[len] = '\0';
    return key;
}

// parse integer or float
static toml_value_t parse_number(parser_t *p)
{
    lexer_t *l = &p->lexer;
    skip_whitespace(l);

    toml_value_t value    = {0};
    size_t       start    = l->pos;
    bool         is_float = false;

    // handle sign
    if (l->input[l->pos] == '+' || l->input[l->pos] == '-')
    {
        l->pos++;
        l->col++;
    }

    // parse digits
    while (l->input[l->pos])
    {
        char c = l->input[l->pos];
        if (isdigit(c))
        {
            l->pos++;
            l->col++;
        }
        else if (c == '_')
        {
            l->pos++;
            l->col++; // skip underscores
        }
        else if (c == '.' || c == 'e' || c == 'E')
        {
            is_float = true;
            l->pos++;
            l->col++;
        }
        else
        {
            break;
        }
    }

    size_t len       = l->pos - start;
    char  *num_str   = malloc(len + 1);
    size_t write_pos = 0;

    // copy without underscores
    for (size_t i = start; i < l->pos; i++)
    {
        if (l->input[i] != '_')
        {
            num_str[write_pos++] = l->input[i];
        }
    }
    num_str[write_pos] = '\0';

    if (is_float)
    {
        value.type        = TOML_FLOAT;
        value.as.floating = atof(num_str);
    }
    else
    {
        value.type       = TOML_INTEGER;
        value.as.integer = atoll(num_str);
    }

    free(num_str);
    return value;
}

// parse boolean
static toml_value_t parse_boolean(parser_t *p)
{
    lexer_t *l = &p->lexer;
    skip_whitespace(l);

    toml_value_t value = {0};
    value.type         = TOML_BOOLEAN;

    if (strncmp(l->input + l->pos, "true", 4) == 0)
    {
        value.as.boolean = true;
        l->pos += 4;
        l->col += 4;
    }
    else if (strncmp(l->input + l->pos, "false", 5) == 0)
    {
        value.as.boolean = false;
        l->pos += 5;
        l->col += 5;
    }
    else
    {
        set_error(p, "expected boolean");
    }

    return value;
}

// parse array
static toml_array_t *parse_array(parser_t *p)
{
    lexer_t *l = &p->lexer;

    if (consume(l) != '[')
    {
        set_error(p, "expected '['");
        return NULL;
    }

    toml_array_t *array = malloc(sizeof(toml_array_t));
    array->capacity     = 8;
    array->count        = 0;
    array->values       = malloc(array->capacity * sizeof(toml_value_t));

    while (peek(l) && peek(l) != ']')
    {
        if (array->count > 0)
        {
            if (peek(l) == ',')
            {
                consume(l);
            }
            else if (peek(l) != ']')
            {
                set_error(p, "expected ',' or ']'");
                free(array->values);
                free(array);
                return NULL;
            }
        }

        if (peek(l) == ']')
        {
            break;
        }

        if (array->count >= array->capacity)
        {
            array->capacity *= 2;
            array->values = realloc(array->values, array->capacity * sizeof(toml_value_t));
        }

        array->values[array->count++] = parse_value(p);
        if (p->error)
        {
            free(array->values);
            free(array);
            return NULL;
        }
    }

    if (consume(l) != ']')
    {
        set_error(p, "expected ']'");
        free(array->values);
        free(array);
        return NULL;
    }

    return array;
}

// parse inline table
static toml_table_t *parse_table_inline(parser_t *p)
{
    lexer_t *l = &p->lexer;

    if (consume(l) != '{')
    {
        set_error(p, "expected '{'");
        return NULL;
    }

    toml_table_t *table = malloc(sizeof(toml_table_t));
    table->capacity     = 8;
    table->count        = 0;
    table->entries      = malloc(table->capacity * sizeof(toml_entry_t));

    while (peek(l) && peek(l) != '}')
    {
        if (table->count > 0)
        {
            if (peek(l) == ',')
            {
                consume(l);
            }
            else
            {
                set_error(p, "expected ',' or '}'");
                free(table->entries);
                free(table);
                return NULL;
            }
        }

        if (peek(l) == '}')
        {
            break;
        }

        if (table->count >= table->capacity)
        {
            table->capacity *= 2;
            table->entries = realloc(table->entries, table->capacity * sizeof(toml_entry_t));
        }

        char *key = parse_key(p);
        if (!key || p->error)
        {
            free(table->entries);
            free(table);
            return NULL;
        }

        skip_whitespace(l);
        if (l->input[l->pos] != '=')
        {
            set_error(p, "expected '='");
            free(key);
            free(table->entries);
            free(table);
            return NULL;
        }
        l->pos++;
        l->col++;

        table->entries[table->count].key   = key;
        table->entries[table->count].value = parse_value(p);
        if (p->error)
        {
            free(table->entries);
            free(table);
            return NULL;
        }
        table->count++;
    }

    if (consume(l) != '}')
    {
        set_error(p, "expected '}'");
        free(table->entries);
        free(table);
        return NULL;
    }

    return table;
}

// parse value (dispatch based on first character)
static toml_value_t parse_value(parser_t *p)
{
    lexer_t     *l     = &p->lexer;
    toml_value_t value = {0};

    char c = peek(l);

    if (c == '"' || c == '\'')
    {
        value.type      = TOML_STRING;
        value.as.string = parse_string(p);
    }
    else if (c == '[')
    {
        value.type     = TOML_ARRAY;
        value.as.array = parse_array(p);
    }
    else if (c == '{')
    {
        value.type     = TOML_TABLE;
        value.as.table = parse_table_inline(p);
    }
    else if (c == 't' || c == 'f')
    {
        value = parse_boolean(p);
    }
    else if (isdigit(c) || c == '+' || c == '-')
    {
        value = parse_number(p);
    }
    else
    {
        set_error(p, "unexpected value");
    }

    return value;
}

// parse entire toml document
toml_table_t *toml_parse(const char *input, char **error)
{
    parser_t parser    = {0};
    parser.lexer.input = input;
    parser.lexer.pos   = 0;
    parser.lexer.line  = 1;
    parser.lexer.col   = 1;
    parser.error       = NULL;

    toml_table_t *root = malloc(sizeof(toml_table_t));
    root->capacity     = 16;
    root->count        = 0;
    root->entries      = malloc(root->capacity * sizeof(toml_entry_t));

    toml_table_t *current_table = root;

    while (peek(&parser.lexer))
    {
        char c = peek(&parser.lexer);

        if (c == '[')
        {
            consume(&parser.lexer);

            // parse table header
            char *table_name = parse_key(&parser);
            if (!table_name || parser.error)
            {
                break;
            }

            if (consume(&parser.lexer) != ']')
            {
                set_error(&parser, "expected ']'");
                free(table_name);
                break;
            }

            // find or create nested table
            toml_value_t *existing = toml_table_get(root, table_name);
            if (existing && existing->type == TOML_TABLE)
            {
                current_table = existing->as.table;
            }
            else
            {
                if (root->count >= root->capacity)
                {
                    root->capacity *= 2;
                    root->entries = realloc(root->entries, root->capacity * sizeof(toml_entry_t));
                }

                toml_table_t *new_table = malloc(sizeof(toml_table_t));
                new_table->capacity     = 8;
                new_table->count        = 0;
                new_table->entries      = malloc(new_table->capacity * sizeof(toml_entry_t));

                root->entries[root->count].key            = table_name;
                root->entries[root->count].value.type     = TOML_TABLE;
                root->entries[root->count].value.as.table = new_table;
                root->count++;

                current_table = new_table;
            }
            continue;
        }

        // parse key-value pair
        char *key = parse_key(&parser);
        if (!key || parser.error)
        {
            break;
        }

        skip_whitespace(&parser.lexer);
        if (parser.lexer.input[parser.lexer.pos] != '=')
        {
            set_error(&parser, "expected '='");
            free(key);
            break;
        }
        parser.lexer.pos++;
        parser.lexer.col++;

        if (current_table->count >= current_table->capacity)
        {
            current_table->capacity *= 2;
            current_table->entries = realloc(current_table->entries, current_table->capacity * sizeof(toml_entry_t));
        }

        current_table->entries[current_table->count].key   = key;
        current_table->entries[current_table->count].value = parse_value(&parser);
        if (parser.error)
        {
            break;
        }
        current_table->count++;
    }

    if (parser.error)
    {
        if (error)
        {
            *error = parser.error;
        }
        else
        {
            free(parser.error);
        }
        toml_table_free(root);
        return NULL;
    }

    if (error)
    {
        *error = NULL;
    }

    return root;
}

// get value from table by key
toml_value_t *toml_table_get(toml_table_t *table, const char *key)
{
    if (!table || !key)
    {
        return NULL;
    }

    for (size_t i = 0; i < table->count; i++)
    {
        if (strcmp(table->entries[i].key, key) == 0)
        {
            return &table->entries[i].value;
        }
    }

    return NULL;
}

// get nested value using dot notation
toml_value_t *toml_table_get_path(toml_table_t *table, const char *path)
{
    if (!table || !path)
    {
        return NULL;
    }

    char         *path_copy     = strdup(path);
    char         *token         = strtok(path_copy, ".");
    toml_value_t *current       = NULL;
    toml_table_t *current_table = table;

    while (token)
    {
        current = toml_table_get(current_table, token);
        if (!current)
        {
            free(path_copy);
            return NULL;
        }

        token = strtok(NULL, ".");
        if (token)
        {
            if (current->type != TOML_TABLE)
            {
                free(path_copy);
                return NULL;
            }
            current_table = current->as.table;
        }
    }

    free(path_copy);
    return current;
}

// type check helpers
bool toml_value_is_string(toml_value_t *value)
{
    return value && value->type == TOML_STRING;
}

bool toml_value_is_integer(toml_value_t *value)
{
    return value && value->type == TOML_INTEGER;
}

bool toml_value_is_float(toml_value_t *value)
{
    return value && value->type == TOML_FLOAT;
}

bool toml_value_is_boolean(toml_value_t *value)
{
    return value && value->type == TOML_BOOLEAN;
}

bool toml_value_is_array(toml_value_t *value)
{
    return value && value->type == TOML_ARRAY;
}

bool toml_value_is_table(toml_value_t *value)
{
    return value && value->type == TOML_TABLE;
}

// array accessors
size_t toml_array_length(toml_array_t *array)
{
    return array ? array->count : 0;
}

toml_value_t *toml_array_get(toml_array_t *array, size_t index)
{
    if (!array || index >= array->count)
    {
        return NULL;
    }
    return &array->values[index];
}

// cleanup - recursively free value
static void free_value(toml_value_t *value)
{
    if (!value)
    {
        return;
    }

    switch (value->type)
    {
    case TOML_STRING:
        free(value->as.string);
        break;
    case TOML_ARRAY:
        if (value->as.array)
        {
            for (size_t i = 0; i < value->as.array->count; i++)
            {
                free_value(&value->as.array->values[i]);
            }
            free(value->as.array->values);
            free(value->as.array);
        }
        break;
    case TOML_TABLE:
        toml_table_free(value->as.table);
        break;
    default:
        break;
    }
}

void toml_table_free(toml_table_t *table)
{
    if (!table)
    {
        return;
    }

    for (size_t i = 0; i < table->count; i++)
    {
        free(table->entries[i].key);
        free_value(&table->entries[i].value);
    }

    free(table->entries);
    free(table);
}
