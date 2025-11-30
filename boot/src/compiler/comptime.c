#include "compiler/comptime.h"
#include "compiler/type.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    const char *name;
    int64_t     id;
} TargetDescriptor;

static TargetDescriptor detect_host_os(void);
static TargetDescriptor detect_host_arch(void);
static int64_t          os_id_from_name(const char *name);
static int64_t          arch_id_from_name(const char *name);
static int              ensure_segment_capacity(const char ***segments_ptr, int *capacity, int count);
static int              flatten_path(AstNode *expr, const char ***out_segments, int *out_count);
static void             set_int_result(AstNode *node, int64_t value);
static void             set_string_result(AstNode *node, const char *value);

static void set_string_result(AstNode *node, const char *value)
{
    if (!node)
    {
        return;
    }

    const char *text = value ? value : "";
    node->comptime.value_kind   = COMPTIME_STRING;
    node->comptime.string_value = strdup(text);
    node->type                  = type_get_primitive(TYPE_PTR);
}

static void set_int_result(AstNode *node, int64_t value)
{
    if (!node)
    {
        return;
    }

    node->comptime.value_kind = COMPTIME_INT;
    node->comptime.int_value  = value;
    node->type                = type_get_primitive(TYPE_I64);
}

static TargetDescriptor detect_host_os(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return (TargetDescriptor){"windows", 3};
#elif defined(__APPLE__)
    return (TargetDescriptor){"macos", 2};
#else
    return (TargetDescriptor){"linux", 1};
#endif
}

static TargetDescriptor detect_host_arch(void)
{
#if defined(__aarch64__) || defined(_M_ARM64)
    return (TargetDescriptor){"arm64", 2};
#elif defined(__x86_64__) || defined(_M_X64)
    return (TargetDescriptor){"x86_64", 1};
#else
    return (TargetDescriptor){"unknown", 0};
#endif
}

static int64_t os_id_from_name(const char *name)
{
    if (!name)
    {
        return 0;
    }

    if (strcmp(name, "linux") == 0)
    {
        return 1;
    }
    if (strcmp(name, "macos") == 0)
    {
        return 2;
    }
    if (strcmp(name, "windows") == 0)
    {
        return 3;
    }

    return 0;
}

static int64_t arch_id_from_name(const char *name)
{
    if (!name)
    {
        return 0;
    }

    if (strcmp(name, "x86_64") == 0)
    {
        return 1;
    }
    if (strcmp(name, "arm64") == 0)
    {
        return 2;
    }

    return 0;
}

static int ensure_segment_capacity(const char ***segments_ptr, int *capacity, int count)
{
    if (count < *capacity)
    {
        return 0;
    }

    int new_capacity        = (*capacity) * 2;
    const char **new_buffer = realloc(*segments_ptr, sizeof(*new_buffer) * new_capacity);
    if (!new_buffer)
    {
        return -1;
    }

    *segments_ptr = new_buffer;
    *capacity     = new_capacity;
    return 0;
}

static int flatten_path(AstNode *expr, const char ***out_segments, int *out_count)
{
    if (!expr || !out_segments || !out_count)
    {
        return -1;
    }

    int            capacity = 4;
    const char   **segments = malloc(sizeof(*segments) * capacity);
    int            count    = 0;
    AstNode       *curr     = expr;

    if (!segments)
    {
        return -1;
    }

    while (curr && curr->kind == AST_EXPR_FIELD)
    {
        if (ensure_segment_capacity(&segments, &capacity, count) < 0)
        {
            free(segments);
            return -1;
        }

        segments[count++] = curr->field_expr.field;
        curr              = curr->field_expr.object;
    }

    if (!curr || curr->kind != AST_EXPR_IDENT)
    {
        free(segments);
        return -1;
    }

    if (ensure_segment_capacity(&segments, &capacity, count) < 0)
    {
        free(segments);
        return -1;
    }

    segments[count++] = curr->ident_expr.name;

    for (int i = 0; i < count / 2; i++)
    {
        const char *tmp           = segments[i];
        segments[i]               = segments[count - 1 - i];
        segments[count - 1 - i]   = tmp;
    }

    *out_segments = segments;
    *out_count    = count;
    return 0;
}

int comptime_lookup(Sema *sema, AstNode *node)
{
    (void)sema;

    if (!node || !node->comptime.inner)
    {
        return -1;
    }

    const char **segments = NULL;
    int          count    = 0;

    if (flatten_path(node->comptime.inner, &segments, &count) < 0)
    {
        return -1;
    }

    if (count < 1 || strcmp(segments[0], "mach") != 0)
    {
        free(segments);
        return -1;
    }

    TargetDescriptor host_os   = detect_host_os();
    TargetDescriptor host_arch = detect_host_arch();
    int              result    = -1;

    if (count >= 2 && strcmp(segments[1], "compiler") == 0)
    {
        if (count == 3)
        {
            if (strcmp(segments[2], "version") == 0)
            {
                set_string_result(node, "0.1.0");
                result = 0;
            }
            else if (strcmp(segments[2], "name") == 0)
            {
                set_string_result(node, "mach");
                result = 0;
            }
        }
    }
    else if (count >= 3 && strcmp(segments[1], "build") == 0 && strcmp(segments[2], "target") == 0)
    {
        if (count == 4)
        {
            if (strcmp(segments[3], "os") == 0)
            {
                set_string_result(node, host_os.name);
                result = 0;
            }
            else if (strcmp(segments[3], "arch") == 0)
            {
                set_string_result(node, host_arch.name);
                result = 0;
            }
        }
        else if (count == 5 && strcmp(segments[4], "id") == 0)
        {
            if (strcmp(segments[3], "os") == 0)
            {
                set_int_result(node, host_os.id);
                result = 0;
            }
            else if (strcmp(segments[3], "arch") == 0)
            {
                set_int_result(node, host_arch.id);
                result = 0;
            }
        }
    }
    else if (count == 4 && strcmp(segments[1], "os") == 0 && strcmp(segments[3], "id") == 0)
    {
        int64_t requested = os_id_from_name(segments[2]);
        if (requested != 0)
        {
            set_int_result(node, requested);
            result = 0;
        }
    }
    else if (count == 4 && strcmp(segments[1], "arch") == 0 && strcmp(segments[3], "id") == 0)
    {
        int64_t requested = arch_id_from_name(segments[2]);
        if (requested != 0)
        {
            set_int_result(node, requested);
            result = 0;
        }
    }

    free(segments);
    return result;
}
