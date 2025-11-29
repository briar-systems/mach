#include "compiler/comptime.h"
#include "compiler/type.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper to set string result
static void set_string_result(AstNode *node, const char *value)
{
    node->comptime.value_kind = COMPTIME_STRING;
    node->comptime.string_value = strdup(value);
    node->type = type_get_primitive(TYPE_PTR); // &u8
}

// Helper to set int result
static void set_int_result(AstNode *node, int64_t value)
{
    node->comptime.value_kind = COMPTIME_INT;
    node->comptime.int_value = value;
    node->type = type_get_primitive(TYPE_I64);
}

// We need a way to flatten the AST field access chain into a list of strings
static int flatten_path(AstNode *expr, const char ***out_segments, int *out_count)
{
    int capacity = 4;
    int count = 0;
    const char **segments = malloc(sizeof(char*) * capacity);
    
    AstNode *curr = expr;
    while (curr->kind == AST_EXPR_FIELD)
    {
        if (count >= capacity)
        {
            capacity *= 2;
            segments = realloc(segments, sizeof(char*) * capacity);
        }
        segments[count++] = curr->field_expr.field;
        curr = curr->field_expr.object;
    }
    
    if (curr->kind == AST_EXPR_IDENT)
    {
        if (count >= capacity)
        {
            capacity *= 2;
            segments = realloc(segments, sizeof(char*) * capacity);
        }
        segments[count++] = curr->ident_expr.name;
    }
    else
    {
        free(segments);
        return -1; // Not a simple path
    }
    
    // Reverse the segments to get top-down order
    for (int i = 0; i < count / 2; i++)
    {
        const char *temp = segments[i];
        segments[i] = segments[count - 1 - i];
        segments[count - 1 - i] = temp;
    }
    
    *out_segments = segments;
    *out_count = count;
    return 0;
}

int comptime_lookup(Sema *sema, AstNode *node)
{
    (void)sema;
    AstNode *inner = node->comptime.inner;
    
    const char **segments = NULL;
    int count = 0;
    
    if (flatten_path(inner, &segments, &count) < 0)
    {
        return -1;
    }
    
    // Check for $mach prefix
    if (count < 1 || strcmp(segments[0], "mach") != 0)
    {
        free(segments);
        return -1; // Not a $mach constant
    }
    
    int result = 0;
    
    if (count >= 2 && strcmp(segments[1], "compiler") == 0)
    {
        if (count == 3)
        {
            if (strcmp(segments[2], "version") == 0)
            {
                set_string_result(node, "0.1.0");
            }
            else if (strcmp(segments[2], "name") == 0)
            {
                set_string_result(node, "mach");
            }
            else
            {
                result = -1;
            }
        }
        else
        {
            result = -1;
        }
    }
    else if (count >= 2 && strcmp(segments[1], "build") == 0)
    {
        if (count >= 3 && strcmp(segments[2], "target") == 0)
        {
            if (count == 4)
            {
                if (strcmp(segments[3], "os") == 0)
                {
                    // hardcoded for now, ideally comes from build config
                    set_string_result(node, "x86_64");
                }
                else if (strcmp(segments[3], "arch") == 0)
                {
                    set_string_result(node, "x86_64");
                }
                else
                {
                    result = -1;
                }
            }
            else if (count == 5)
            {
                 if (strcmp(segments[3], "os") == 0 && strcmp(segments[4], "id") == 0)
                 {
                     set_int_result(node, 1); // Linux ID
                 }
                 else if (strcmp(segments[3], "arch") == 0 && strcmp(segments[4], "id") == 0)
                 {
                     set_int_result(node, 1); // x86_64 ID
                 }
                 else
                 {
                     result = -1;
                 }
            }
            else
            {
                result = -1;
            }
        }
        else
        {
            result = -1;
        }
    }
    else if (count >= 2 && strcmp(segments[1], "os") == 0)
    {
        // $mach.os.linux.id
        if (count == 4 && strcmp(segments[2], "linux") == 0 && strcmp(segments[3], "id") == 0)
        {
            set_int_result(node, 1);
        }
        else if (count == 4 && strcmp(segments[2], "macos") == 0 && strcmp(segments[3], "id") == 0)
        {
            set_int_result(node, 2);
        }
        else if (count == 4 && strcmp(segments[2], "windows") == 0 && strcmp(segments[3], "id") == 0)
        {
            set_int_result(node, 3);
        }
        else
        {
            result = -1;
        }
    }
    else if (count >= 2 && strcmp(segments[1], "arch") == 0)
    {
        // $mach.arch.x86_64.id
        if (count == 4 && strcmp(segments[2], "x86_64") == 0 && strcmp(segments[3], "id") == 0)
        {
            set_int_result(node, 1);
        }
        else if (count == 4 && strcmp(segments[2], "arm64") == 0 && strcmp(segments[3], "id") == 0)
        {
            set_int_result(node, 2);
        }
        else
        {
            result = -1;
        }
    }
    else
    {
        result = -1;
    }
    
    free(segments);
    return result;
}
