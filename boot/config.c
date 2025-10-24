#include "config.h"
#include <dirent.h>
#include <errno.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// simple toml parser for configuration
typedef struct TomlParser
{
    const char *input;
    size_t      pos;
    size_t      len;
} TomlParser;

static void toml_parser_init(TomlParser *parser, const char *input)
{
    parser->input = input;
    parser->pos   = 0;
    parser->len   = strlen(input);
}

static void toml_skip_whitespace(TomlParser *parser)
{
    while (parser->pos < parser->len)
    {
        char c = parser->input[parser->pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            parser->pos++;
        }
        else if (c == '#')
        {
            // skip comment line
            while (parser->pos < parser->len && parser->input[parser->pos] != '\n')
            {
                parser->pos++;
            }
        }
        else
        {
            break;
        }
    }
}

static char *toml_parse_string(TomlParser *parser)
{
    toml_skip_whitespace(parser);

    if (parser->pos >= parser->len)
        return NULL;

    char quote = parser->input[parser->pos];
    if (quote != '"' && quote != '\'')
        return NULL;

    parser->pos++; // skip opening quote

    size_t start = parser->pos;
    while (parser->pos < parser->len && parser->input[parser->pos] != quote)
    {
        parser->pos++;
    }

    if (parser->pos >= parser->len)
        return NULL;

    size_t len = parser->pos - start;
    parser->pos++; // skip closing quote

    char *result = malloc(len + 1);
    strncpy(result, parser->input + start, len);
    result[len] = '\0';

    return result;
}

static char *toml_parse_identifier(TomlParser *parser)
{
    toml_skip_whitespace(parser);

    if (parser->pos >= parser->len)
        return NULL;

    // handle quoted keys
    if (parser->input[parser->pos] == '"')
    {
        return toml_parse_string(parser);
    }

    size_t start = parser->pos;
    while (parser->pos < parser->len)
    {
        char c = parser->input[parser->pos];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')
        {
            parser->pos++;
        }
        else
        {
            break;
        }
    }

    if (parser->pos == start)
        return NULL;

    size_t len    = parser->pos - start;
    char  *result = malloc(len + 1);
    strncpy(result, parser->input + start, len);
    result[len] = '\0';

    return result;
}

static char *toml_parse_section_name(TomlParser *parser)
{
    toml_skip_whitespace(parser);

    if (parser->pos >= parser->len || parser->input[parser->pos] != '[')
        return NULL;

    parser->pos++; // skip '['

    size_t start = parser->pos;
    while (parser->pos < parser->len)
    {
        char c = parser->input[parser->pos];
        if (c == ']')
        {
            break;
        }
        else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')
        {
            parser->pos++;
        }
        else
        {
            return NULL; // invalid character in section name
        }
    }

    if (parser->pos >= parser->len || parser->input[parser->pos] != ']')
        return NULL;

    size_t len = parser->pos - start;
    parser->pos++; // skip ']'

    if (len == 0)
        return NULL;

    char *result = malloc(len + 1);
    strncpy(result, parser->input + start, len);
    result[len] = '\0';

    return result;
}

static void toml_skip_table_value(TomlParser *parser)
{
    toml_skip_whitespace(parser);

    if (parser->pos >= parser->len || parser->input[parser->pos] != '{')
    {
        // not a table, skip to end of line
        while (parser->pos < parser->len && parser->input[parser->pos] != '\n')
            parser->pos++;
        return;
    }

    parser->pos++; // skip '{'

    int brace_count = 1;
    while (parser->pos < parser->len && brace_count > 0)
    {
        if (parser->input[parser->pos] == '{')
            brace_count++;
        else if (parser->input[parser->pos] == '}')
            brace_count--;
        parser->pos++;
    }
}

static int toml_parse_number(TomlParser *parser)
{
    toml_skip_whitespace(parser);

    if (parser->pos >= parser->len)
        return 0;

    size_t start = parser->pos;
    while (parser->pos < parser->len)
    {
        char c = parser->input[parser->pos];
        if (c >= '0' && c <= '9')
        {
            parser->pos++;
        }
        else
        {
            break;
        }
    }

    if (parser->pos == start)
        return 0;

    char *number_str = malloc(parser->pos - start + 1);
    strncpy(number_str, parser->input + start, parser->pos - start);
    number_str[parser->pos - start] = '\0';

    int result = atoi(number_str);
    free(number_str);

    return result;
}

static bool toml_parse_bool(TomlParser *parser)
{
    toml_skip_whitespace(parser);

    if (parser->pos + 4 <= parser->len && strncmp(parser->input + parser->pos, "true", 4) == 0)
    {
        parser->pos += 4;
        return true;
    }
    else if (parser->pos + 5 <= parser->len && strncmp(parser->input + parser->pos, "false", 5) == 0)
    {
        parser->pos += 5;
        return false;
    }

    return false;
}

static bool toml_expect_char(TomlParser *parser, char expected)
{
    toml_skip_whitespace(parser);

    if (parser->pos >= parser->len || parser->input[parser->pos] != expected)
        return false;

    parser->pos++;
    return true;
}

static char *read_file_to_string(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    size_t read  = fread(buffer, 1, size, file);
    buffer[read] = '\0';
    fclose(file);

    return buffer;
}

// expand environment variables in format ${VAR_NAME}
static char *expand_env_vars(const char *input)
{
    if (!input)
        return NULL;

    size_t input_len = strlen(input);
    size_t buf_size  = input_len * 2; // initial buffer size
    char  *result    = malloc(buf_size);
    if (!result)
        return NULL;

    size_t      pos = 0;
    const char *p   = input;

    while (*p)
    {
        if (*p == '$' && *(p + 1) == '{')
        {
            // find closing brace
            const char *start = p + 2;
            const char *end   = strchr(start, '}');
            if (!end)
            {
                // malformed - just copy literal
                if (pos + 2 >= buf_size)
                {
                    buf_size *= 2;
                    result = realloc(result, buf_size);
                }
                result[pos++] = *p++;
                continue;
            }

            // extract variable name
            size_t var_len  = end - start;
            char  *var_name = malloc(var_len + 1);
            memcpy(var_name, start, var_len);
            var_name[var_len] = '\0';

            // get environment variable
            const char *value = getenv(var_name);
            if (!value)
            {
                fprintf(stderr, "error: undefined environment variable '${%s}'\n", var_name);
                free(var_name);
                free(result);
                return NULL;
            }

            // append value to result
            size_t value_len = strlen(value);
            while (pos + value_len >= buf_size)
            {
                buf_size *= 2;
                result = realloc(result, buf_size);
            }
            memcpy(result + pos, value, value_len);
            pos += value_len;

            free(var_name);
            p = end + 1; // skip past '}'
        }
        else
        {
            if (pos + 1 >= buf_size)
            {
                buf_size *= 2;
                result = realloc(result, buf_size);
            }
            result[pos++] = *p++;
        }
    }

    result[pos] = '\0';
    return result;
}

void target_config_init(TargetConfig *target)
{
    memset(target, 0, sizeof(TargetConfig));
    // No defaults - all fields must be explicitly set (mach explicitness philosophy)
}

void target_config_dnit(TargetConfig *target)
{
    free(target->name);
    free(target->target_triple);
    free(target->entrypoint);
    free(target->artifacts_dir);
    free(target->out);

    // Free link libraries
    for (int i = 0; i < target->link_lib_count; i++)
    {
        free(target->link_libraries[i]);
    }
    free(target->link_libraries);

    memset(target, 0, sizeof(TargetConfig));
}

TargetConfig *target_config_create(const char *name, const char *target_triple)
{
    TargetConfig *target = malloc(sizeof(TargetConfig));
    target_config_init(target);
    target->name          = strdup(name);
    target->target_triple = strdup(target_triple);
    return target;
}

// removed discovered dependency helpers (replaced by explicit DepSpec)

void config_init(ProjectConfig *config)
{
    memset(config, 0, sizeof(ProjectConfig));
}

void config_dnit(ProjectConfig *config)
{
    free(config->name);
    free(config->version);
    free(config->main_file);
    free(config->target);
    free(config->src_dir);

    // cleanup targets
    for (int i = 0; i < config->target_count; i++)
    {
        target_config_dnit(config->targets[i]);
        free(config->targets[i]);
    }
    free(config->targets);

    // cleanup dependencies
    for (int i = 0; i < config->dep_count; i++)
    {
        DepSpec *d = config->deps[i];
        if (d)
        {
            free(d->name);
            free(d->path);
            free(d->src_dir);
            free(d);
        }
    }
    free(config->deps);

    memset(config, 0, sizeof(ProjectConfig));
}

bool config_add_target(ProjectConfig *config, const char *name, const char *target_triple)
{
    // check if target already exists
    for (int i = 0; i < config->target_count; i++)
    {
        if (strcmp(config->targets[i]->name, name) == 0)
            return false; // already exists
    }

    // grow targets array if needed
    if (config->target_count == 0)
    {
        config->targets = malloc(sizeof(TargetConfig *));
    }
    else
    {
        config->targets = realloc(config->targets, (config->target_count + 1) * sizeof(TargetConfig *));
    }

    config->targets[config->target_count] = target_config_create(name, target_triple);
    config->target_count++;

    return true;
}

TargetConfig *config_get_target(ProjectConfig *config, const char *name)
{
    for (int i = 0; i < config->target_count; i++)
    {
        if (strcmp(config->targets[i]->name, name) == 0)
            return config->targets[i];
    }
    return NULL;
}

TargetConfig *config_get_target_by_triple(ProjectConfig *config, const char *target_triple)
{
    for (int i = 0; i < config->target_count; i++)
    {
        if (strcmp(config->targets[i]->target_triple, target_triple) == 0)
            return config->targets[i];
    }
    return NULL;
}

TargetConfig *config_get_default_target(ProjectConfig *config)
{
    if (config->target && strcmp(config->target, "all") != 0 && strcmp(config->target, "native") != 0)
    {
        return config_get_target(config, config->target);
    }
    else if (config->target && strcmp(config->target, "native") == 0)
    {
        return config_resolve_native_target(config);
    }
    else if (config->target_count > 0)
    {
        return config->targets[0]; // first target as fallback
    }
    return NULL;
}

TargetConfig *config_resolve_native_target(ProjectConfig *config)
{
    // Use LLVM to get the native host triple
    char *native_triple = LLVMGetDefaultTargetTriple();
    if (!native_triple)
    {
        fprintf(stderr, "error: 'native' target requested but could not determine host triple\n");
        return NULL;
    }

    // Try exact match first
    for (int i = 0; i < config->target_count; i++)
    {
        TargetConfig *target = config->targets[i];
        if (target->target_triple && strcmp(target->target_triple, native_triple) == 0)
        {
            LLVMDisposeMessage(native_triple);
            return target;
        }
    }

    // Try normalized triple match (LLVM normalizes triples)
    // Parse the native triple to extract arch-vendor-os
    char *arch_start   = native_triple;
    char *vendor_start = strchr(arch_start, '-');
    if (!vendor_start)
    {
        LLVMDisposeMessage(native_triple);
        fprintf(stderr, "error: 'native' target has malformed triple '%s'\n", native_triple);
        return NULL;
    }
    vendor_start++; // skip the dash

    char *os_start = strchr(vendor_start, '-');
    if (!os_start)
    {
        LLVMDisposeMessage(native_triple);
        fprintf(stderr, "error: 'native' target has malformed triple '%s'\n", native_triple);
        return NULL;
    }
    os_start++; // skip the dash

    // Extract components
    size_t arch_len = vendor_start - arch_start - 1;
    // vendor component not used in matching (can vary between platforms)

    // Now try fuzzy matching on architecture and OS
    for (int i = 0; i < config->target_count; i++)
    {
        TargetConfig *target = config->targets[i];
        if (!target->target_triple)
            continue;

        // Check if arch and OS match (vendor can vary)
        const char *t        = target->target_triple;
        char       *t_vendor = strchr(t, '-');
        if (!t_vendor)
            continue;

        size_t t_arch_len = t_vendor - t;
        if (t_arch_len != arch_len || strncmp(t, arch_start, arch_len) != 0)
            continue;

        t_vendor++; // skip dash
        char *t_os = strchr(t_vendor, '-');
        if (!t_os)
            continue;
        t_os++; // skip dash

        // Check if OS component matches (allow some flexibility)
        if (strncmp(os_start, t_os, strlen(t_os)) == 0 || strncmp(t_os, os_start, strlen(os_start)) == 0)
        {
            LLVMDisposeMessage(native_triple);
            return target;
        }
    }

    fprintf(stderr, "error: 'native' target requested but no matching target found for host triple '%s'\n", native_triple);
    fprintf(stderr, "available targets and triples:\n");
    for (int i = 0; i < config->target_count; i++)
    {
        fprintf(stderr, "  %s: %s\n", config->targets[i]->name, config->targets[i]->target_triple);
    }

    LLVMDisposeMessage(native_triple);
    return NULL;
}

bool config_is_build_all_targets(ProjectConfig *config)
{
    return !config->target || strcmp(config->target, "all") == 0;
}

ProjectConfig *config_load(const char *config_path)
{
    char *content = read_file_to_string(config_path);
    if (!content)
        return NULL;

    ProjectConfig *config = malloc(sizeof(ProjectConfig));
    config_init(config);

    TomlParser parser;
    toml_parser_init(&parser, content);

    char *current_section = NULL;

    // enhanced section-aware parsing
    while (parser.pos < parser.len)
    {
        toml_skip_whitespace(&parser);

        if (parser.pos >= parser.len)
            break;

        // check for section header
        if (parser.input[parser.pos] == '[')
        {
            free(current_section);
            current_section = toml_parse_section_name(&parser);
            continue;
        }

        // parse key-value pair
        char *key = toml_parse_identifier(&parser);
        if (!key)
        {
            toml_skip_whitespace(&parser);
            if (parser.pos < parser.len)
                parser.pos++; // skip unknown character
            continue;
        }

        if (!toml_expect_char(&parser, '='))
        {
            free(key);
            continue;
        }

        // parse value based on section and key
        if (!current_section || strcmp(current_section, "project") == 0)
        {
            // handle project section keys (merged with directories)
            if (strcmp(key, "name") == 0)
            {
                config->name = toml_parse_string(&parser);
            }
            else if (strcmp(key, "version") == 0)
            {
                config->version = toml_parse_string(&parser);
            }
            else if (strcmp(key, "src") == 0)
            {
                config->src_dir = toml_parse_string(&parser);
            }
            else if (strcmp(key, "target") == 0)
            {
                config->target = toml_parse_string(&parser);
            }
            else
            {
                // skip unknown keys in project section
                toml_skip_table_value(&parser);
            }
        }
        else if (strncmp(current_section, "targets.", 8) == 0)
        {
            // handle target-specific section - extract target name
            const char   *target_name = current_section + 8;
            TargetConfig *target      = config_get_target(config, target_name);

            // create target if it doesn't exist (on first key encountered)
            if (!target)
            {
                // Create target with empty triple, will be filled in when we parse triple key
                config_add_target(config, target_name, "");
                target = config_get_target(config, target_name);
            }

            if (target)
            {
                // parse target-specific options
                if (strcmp(key, "triple") == 0)
                {
                    char *target_triple = toml_parse_string(&parser);
                    if (target_triple)
                    {
                        free(target->target_triple);
                        target->target_triple = target_triple;
                    }
                }
                else if (strcmp(key, "entrypoint") == 0)
                {
                    target->entrypoint = toml_parse_string(&parser);
                }
                else if (strcmp(key, "artifacts") == 0)
                {
                    target->artifacts_dir = toml_parse_string(&parser);
                }
                else if (strcmp(key, "out") == 0)
                {
                    target->out = toml_parse_string(&parser);
                }
                else if (strcmp(key, "link") == 0)
                {
                    // Parse link library (can be called multiple times or as array)
                    char *lib_path = toml_parse_string(&parser);
                    if (lib_path)
                    {
                        target->link_libraries                           = realloc(target->link_libraries, (target->link_lib_count + 1) * sizeof(char *));
                        target->link_libraries[target->link_lib_count++] = lib_path;
                    }
                }
                else if (strcmp(key, "opt-level") == 0)
                {
                    target->opt_level = toml_parse_number(&parser);
                }
                else if (strcmp(key, "emit-ast") == 0)
                {
                    target->emit_ast = toml_parse_bool(&parser);
                }
                else if (strcmp(key, "emit-ir") == 0)
                {
                    target->emit_ir = toml_parse_bool(&parser);
                }
                else if (strcmp(key, "emit-asm") == 0)
                {
                    target->emit_asm = toml_parse_bool(&parser);
                }
                else if (strcmp(key, "emit-object") == 0)
                {
                    target->emit_object = toml_parse_bool(&parser);
                }
                else if (strcmp(key, "build-library") == 0)
                {
                    target->build_library = toml_parse_bool(&parser);
                }
                else if (strcmp(key, "shared") == 0)
                {
                    target->shared = toml_parse_bool(&parser);
                }
                else if (strcmp(key, "no-pie") == 0)
                {
                    target->no_pie = toml_parse_bool(&parser);
                }
                else
                {
                    // skip unknown target options
                    toml_skip_table_value(&parser);
                }
            }
            else
            {
                // skip unknown target options when target doesn't exist
                toml_skip_table_value(&parser);
            }
        }
        else if (strcmp(current_section, "dependencies") == 0)
        {
            // parse dependency: name = "path" with env var expansion
            char *dep_path_raw = toml_parse_string(&parser);
            if (dep_path_raw)
            {
                char *dep_path = expand_env_vars(dep_path_raw);
                free(dep_path_raw);

                if (!dep_path)
                {
                    fprintf(stderr, "error: failed to expand environment variables in dependency '%s'\n", key);
                    free(key);
                    free(current_section);
                    free(content);
                    config_dnit(config);
                    free(config);
                    return NULL;
                }

                DepSpec *dep = calloc(1, sizeof(DepSpec));
                dep->name    = strdup(key);
                dep->path    = dep_path;
                dep->src_dir = NULL; // will be resolved later

                DepSpec **new_arr = realloc(config->deps, (config->dep_count + 1) * sizeof(DepSpec *));
                if (new_arr)
                {
                    config->deps                      = new_arr;
                    config->deps[config->dep_count++] = dep;
                }
                else
                {
                    free(dep->name);
                    free(dep->path);
                    free(dep);
                }
            }
        }

        free(key);
    }

    free(current_section);
    free(content);
    return config;
}

ProjectConfig *config_load_from_dir(const char *dir_path)
{
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/mach.toml", dir_path);

    return config_load(config_path);
}

bool config_save(ProjectConfig *config, const char *config_path)
{
    FILE *file = fopen(config_path, "w");
    if (!file)
        return false;

    fprintf(file, "# mach project configuration\n");
    fprintf(file, "# all fields are required (mach explicitness philosophy)\n\n");

    fprintf(file, "[project]\n");
    if (config->name)
        fprintf(file, "name = \"%s\"\n", config->name);
    if (config->version)
        fprintf(file, "version = \"%s\"\n", config->version);
    if (config->src_dir)
        fprintf(file, "src = \"%s\"\n", config->src_dir);
    if (config->target)
        fprintf(file, "target = \"%s\"\n", config->target);

    // save targets
    for (int i = 0; i < config->target_count; i++)
    {
        TargetConfig *target = config->targets[i];
        fprintf(file, "\n[targets.%s]\n", target->name);
        if (target->target_triple)
            fprintf(file, "triple = \"%s\"\n", target->target_triple);
        if (target->entrypoint)
            fprintf(file, "entrypoint = \"%s\"\n", target->entrypoint);
        if (target->artifacts_dir)
            fprintf(file, "artifacts = \"%s\"\n", target->artifacts_dir);
        if (target->out)
            fprintf(file, "out = \"%s\"\n", target->out);
        fprintf(file, "opt-level = %d\n", target->opt_level);
        // always write booleans explicitly for transparency
        fprintf(file, "emit-ast = %s\n", target->emit_ast ? "true" : "false");
        fprintf(file, "emit-ir = %s\n", target->emit_ir ? "true" : "false");
        fprintf(file, "emit-asm = %s\n", target->emit_asm ? "true" : "false");
        fprintf(file, "emit-object = %s\n", target->emit_object ? "true" : "false");
        fprintf(file, "build-library = %s\n", target->build_library ? "true" : "false");
        fprintf(file, "shared = %s\n", target->shared ? "true" : "false");
        fprintf(file, "no-pie = %s\n", target->no_pie ? "true" : "false");

        // Write link libraries if any
        for (int j = 0; j < target->link_lib_count; j++)
        {
            fprintf(file, "link = \"%s\"\n", target->link_libraries[j]);
        }
    }

    // dependencies (also serve as module aliases)
    if (config->dep_count > 0)
    {
        fprintf(file, "\n[dependencies]\n");
        for (int i = 0; i < config->dep_count; i++)
        {
            DepSpec *d = config->deps[i];
            fprintf(file, "%s = \"%s\"\n", d->name, d->path);
        }
    }

    fclose(file);
    return true;
}

ProjectConfig *config_create_default(const char *project_name)
{
    ProjectConfig *config = malloc(sizeof(ProjectConfig));
    config_init(config);

    config->name    = strdup(project_name);
    config->version = strdup("0.1.0");
    config->src_dir = strdup("src");
    config->target  = strdup("native");
    // note: no default entrypoint or out_dir - they are per-target now

    return config;
}

bool config_has_main_file(ProjectConfig *config)
{
    // Check if at least one target has an entrypoint
    if (!config)
        return false;
    for (int i = 0; i < config->target_count; i++)
    {
        if (config->targets[i]->entrypoint && strlen(config->targets[i]->entrypoint) > 0)
            return true;
    }
    return false;
}

bool config_should_emit_ast(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return false;
    TargetConfig *target = config_get_target(config, target_name);
    return target && target->emit_ast;
}

bool config_should_emit_ir(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return false;
    TargetConfig *target = config_get_target(config, target_name);
    return target && target->emit_ir;
}

bool config_should_emit_asm(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return false;
    TargetConfig *target = config_get_target(config, target_name);
    return target && target->emit_asm;
}

bool config_should_emit_object(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return false;
    TargetConfig *target = config_get_target(config, target_name);
    return target && target->emit_object;
}

bool config_should_build_library(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return false;
    TargetConfig *target = config_get_target(config, target_name);
    return target && target->build_library;
}

bool config_should_link_executable(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return true;
    TargetConfig *target = config_get_target(config, target_name);
    return target && !target->build_library;
}

bool config_is_shared_library(ProjectConfig *config, const char *target_name)
{
    if (!config || !target_name)
        return true;
    TargetConfig *target = config_get_target(config, target_name);
    return target && target->shared;
}

char *config_default_executable_name(ProjectConfig *config)
{
    // Deprecated - use per-target final_name instead
    const char *name = config && config->name ? config->name : "a.out";
    return strdup(name);
}

char *config_default_library_name(ProjectConfig *config, bool shared)
{
    // Deprecated - use per-target final_name instead
    const char *base = config && config->name ? config->name : "libmach";
    size_t      nlen = strlen(base);
    const char *ext  = shared ? ".so" : ".a";
    size_t      len  = 3 + nlen + strlen(ext) + 1;
    char       *out  = malloc(len);
    snprintf(out, len, "lib%s%s", base, ext);
    return out;
}

char *config_resolve_final_output_path(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    if (!config || !target_name)
        return NULL;

    TargetConfig *target = config_get_target(config, target_name);
    if (!target)
        return NULL;

    // out field is required and must be a path specification
    const char *out_spec = target->out;
    if (!out_spec || strlen(out_spec) == 0)
    {
        fprintf(stderr, "error: target '%s' missing required 'out' field\n", target_name);
        return NULL;
    }

    // if out is absolute path, use as-is
    if (out_spec[0] == '/')
        return strdup(out_spec);

    // check if out_spec contains path separators (relative path from project root)
    if (strchr(out_spec, '/'))
    {
        // out_spec is a relative path - resolve it relative to project_dir
        size_t len  = strlen(project_dir) + strlen(out_spec) + 2;
        char  *path = malloc(len);
        snprintf(path, len, "%s/%s", project_dir, out_spec);
        return path;
    }

    // out_spec is a simple name - place in bin_dir (artifacts_dir/bin/)
    char *bin_dir = config_resolve_bin_dir(config, project_dir, target_name);
    if (!bin_dir)
    {
        fprintf(stderr, "error: could not resolve bin directory for target '%s'\n", target_name);
        return NULL;
    }

    size_t len  = strlen(bin_dir) + strlen(out_spec) + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/%s", bin_dir, out_spec);

    free(bin_dir);
    return path;
}

char *config_resolve_artifacts_dir(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    if (!config || !target_name)
        return NULL;

    TargetConfig *target = config_get_target(config, target_name);
    if (!target || !target->artifacts_dir)
        return NULL;

    // if artifacts dir is absolute path, return as is
    if (target->artifacts_dir[0] == '/')
        return strdup(target->artifacts_dir);

    // resolve relative to project directory
    size_t len  = strlen(project_dir) + strlen(target->artifacts_dir) + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/%s", project_dir, target->artifacts_dir);

    return path;
}

char *config_resolve_main_file(ProjectConfig *config, const char *project_dir)
{
    // Deprecated: main_file is now per-target (entrypoint field)
    // This function kept for backward compatibility but should not be used
    if (!config || !config->main_file)
        return NULL;

    // if main file is absolute path, return as is
    if (config->main_file[0] == '/')
        return strdup(config->main_file);

    // resolve relative to src directory
    char *src_dir = config_resolve_src_dir(config, project_dir);
    if (!src_dir)
        return NULL;

    size_t len  = strlen(src_dir) + strlen(config->main_file) + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/%s", src_dir, config->main_file);

    free(src_dir);
    return path;
}

char *config_resolve_target_entrypoint(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    if (!config || !target_name)
        return NULL;

    TargetConfig *target = config_get_target(config, target_name);
    if (!target || !target->entrypoint)
        return NULL;

    // if entrypoint is absolute path, return as is
    if (target->entrypoint[0] == '/')
        return strdup(target->entrypoint);

    // resolve relative to src directory
    char *src_dir = config_resolve_src_dir(config, project_dir);
    if (!src_dir)
        return NULL;

    size_t len  = strlen(src_dir) + strlen(target->entrypoint) + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/%s", src_dir, target->entrypoint);

    free(src_dir);
    return path;
}

char *config_resolve_src_dir(ProjectConfig *config, const char *project_dir)
{
    if (!config || !config->src_dir)
        return NULL;

    // if src dir is absolute path, return as is
    if (config->src_dir[0] == '/')
        return strdup(config->src_dir);

    // resolve relative to project directory
    size_t len  = strlen(project_dir) + strlen(config->src_dir) + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/%s", project_dir, config->src_dir);

    return path;
}

char *config_resolve_bin_dir(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    char *artifacts_dir = config_resolve_artifacts_dir(config, project_dir, target_name);
    if (!artifacts_dir)
        return NULL;

    // bin directory is directly under artifacts_dir
    size_t len  = strlen(artifacts_dir) + strlen("/bin") + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/bin", artifacts_dir);

    free(artifacts_dir);
    return path;
}

char *config_resolve_obj_dir(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    char *artifacts_dir = config_resolve_artifacts_dir(config, project_dir, target_name);
    if (!artifacts_dir)
        return NULL;

    // obj directory is directly under artifacts_dir
    size_t len  = strlen(artifacts_dir) + strlen("/obj") + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/obj", artifacts_dir);

    free(artifacts_dir);
    return path;
}

char *config_resolve_asm_dir(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    char *artifacts_dir = config_resolve_artifacts_dir(config, project_dir, target_name);
    if (!artifacts_dir)
        return NULL;

    // asm directory is directly under artifacts_dir
    size_t len  = strlen(artifacts_dir) + strlen("/asm") + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/asm", artifacts_dir);

    free(artifacts_dir);
    return path;
}

char *config_resolve_ir_dir(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    char *artifacts_dir = config_resolve_artifacts_dir(config, project_dir, target_name);
    if (!artifacts_dir)
        return NULL;

    // ir directory is directly under artifacts_dir
    size_t len  = strlen(artifacts_dir) + strlen("/ir") + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/ir", artifacts_dir);

    free(artifacts_dir);
    return path;
}

char *config_resolve_ast_dir(ProjectConfig *config, const char *project_dir, const char *target_name)
{
    char *artifacts_dir = config_resolve_artifacts_dir(config, project_dir, target_name);
    if (!artifacts_dir)
        return NULL;

    // ast directory is directly under artifacts_dir
    size_t len  = strlen(artifacts_dir) + strlen("/ast") + 2;
    char  *path = malloc(len);
    snprintf(path, len, "%s/ast", artifacts_dir);

    free(artifacts_dir);
    return path;
}

static bool ensure_directory_exists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return S_ISDIR(st.st_mode);

    // create directory with mkdir -p equivalent
    char *path_copy = strdup(path);
    char *p         = path_copy;

    // skip leading slash
    if (*p == '/')
        p++;

    while (*p)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(path_copy, 0755);
            *p = '/';
        }
        p++;
    }

    int result = mkdir(path_copy, 0755);
    free(path_copy);

    return result == 0 || errno == EEXIST;
}

bool config_ensure_directories(ProjectConfig *config, const char *project_dir)
{
    if (!config)
        return false;

    // ensure all configured directories exist
    char *src_dir = config_resolve_src_dir(config, project_dir);
    if (src_dir)
    {
        if (!ensure_directory_exists(src_dir))
        {
            free(src_dir);
            return false;
        }
        free(src_dir);
    }

    // create target-specific directories for each target
    for (int i = 0; i < config->target_count; i++)
    {
        const char *target_name = config->targets[i]->name;

        // ensure base artifacts_dir for this target
        char *artifacts_dir = config_resolve_artifacts_dir(config, project_dir, target_name);
        if (artifacts_dir)
        {
            if (!ensure_directory_exists(artifacts_dir))
            {
                free(artifacts_dir);
                return false;
            }
            free(artifacts_dir);
        }

        char *bin_dir = config_resolve_bin_dir(config, project_dir, target_name);
        if (bin_dir)
        {
            if (!ensure_directory_exists(bin_dir))
            {
                free(bin_dir);
                return false;
            }
            free(bin_dir);
        }

        char *obj_dir = config_resolve_obj_dir(config, project_dir, target_name);
        if (obj_dir)
        {
            if (!ensure_directory_exists(obj_dir))
            {
                free(obj_dir);
                return false;
            }
            free(obj_dir);
        }

        char *asm_dir = config_resolve_asm_dir(config, project_dir, target_name);
        if (asm_dir)
        {
            if (!ensure_directory_exists(asm_dir))
            {
                free(asm_dir);
                return false;
            }
            free(asm_dir);
        }

        char *ir_dir = config_resolve_ir_dir(config, project_dir, target_name);
        if (ir_dir)
        {
            if (!ensure_directory_exists(ir_dir))
            {
                free(ir_dir);
                return false;
            }
            free(ir_dir);
        }

        char *ast_dir = config_resolve_ast_dir(config, project_dir, target_name);
        if (ast_dir)
        {
            if (!ensure_directory_exists(ast_dir))
            {
                free(ast_dir);
                return false;
            }
            free(ast_dir);
        }
    }

    return true;
}

bool config_validate(ProjectConfig *config)
{
    if (!config)
    {
        fprintf(stderr, "error: config is NULL\n");
        return false;
    }

    // Validate [project] section - all fields required
    if (!config->name || strlen(config->name) == 0)
    {
        fprintf(stderr, "error: [project] name is required\n");
        return false;
    }

    if (!config->version || strlen(config->version) == 0)
    {
        fprintf(stderr, "error: [project] version is required\n");
        return false;
    }

    if (!config->src_dir || strlen(config->src_dir) == 0)
    {
        fprintf(stderr, "error: [project] src is required\n");
        return false;
    }

    // Validate at least one target exists
    if (config->target_count == 0)
    {
        fprintf(stderr, "error: at least one [targets.<name>] section is required\n");
        return false;
    }

    // Validate each target - all fields required
    for (int i = 0; i < config->target_count; i++)
    {
        TargetConfig *target = config->targets[i];

        if (!target->name || strlen(target->name) == 0)
        {
            fprintf(stderr, "error: target name is missing\n");
            return false;
        }

        if (!target->target_triple || strlen(target->target_triple) == 0)
        {
            fprintf(stderr, "error: [targets.%s] triple is required\n", target->name);
            return false;
        }

        if (!target->artifacts_dir || strlen(target->artifacts_dir) == 0)
        {
            fprintf(stderr, "error: [targets.%s] artifacts is required\n", target->name);
            return false;
        }

        if (!target->out || strlen(target->out) == 0)
        {
            fprintf(stderr, "error: [targets.%s] out is required\n", target->name);
            return false;
        }

        if (!target->entrypoint || strlen(target->entrypoint) == 0)
        {
            fprintf(stderr, "error: [targets.%s] entrypoint is required\n", target->name);
            return false;
        }

        if (target->opt_level < 0 || target->opt_level > 3)
        {
            fprintf(stderr, "error: [targets.%s] opt-level must be 0-3 (got %d)\n", target->name, target->opt_level);
            return false;
        }

        // Note: boolean fields (emit-ast, emit-ir, emit-asm, emit-object, build-library, no-pie)
        // are implicitly validated as they default to false if not specified in parsing
    }

    return true;
}

// dependency discovery functions
// new dependency management implementation
DepSpec *config_get_dep(ProjectConfig *config, const char *name)
{
    if (!config || !name)
        return NULL;
    for (int i = 0; i < config->dep_count; i++)
    {
        if (strcmp(config->deps[i]->name, name) == 0)
            return config->deps[i];
    }
    return NULL;
}

bool config_has_dep(ProjectConfig *config, const char *name)
{
    return config_get_dep(config, name) != NULL;
}

// deps now serve as module aliases automatically
static const char *config_get_dep_alias(ProjectConfig *config, const char *alias)
{
    if (!config || !alias)
        return NULL;

    // check if alias matches a dependency name
    DepSpec *dep = config_get_dep(config, alias);
    if (dep)
        return dep->name; // dependency name IS the module prefix

    return NULL;
}

static bool is_self_alias(const char *alias)
{
    return alias && strcmp(alias, "self") == 0;
}

char *config_expand_module_path(ProjectConfig *config, const char *module_path)
{
    if (!module_path)
        return NULL;

    if (!config)
        return strdup(module_path);

    // special-case builtin target module to avoid prefixing with project or dep
    if (strcmp(module_path, "target") == 0)
        return strdup("target");

    // legacy 'dep.' prefix removed

    size_t project_name_len = config->name ? strlen(config->name) : 0;
    if (project_name_len > 0 && strncmp(module_path, config->name, project_name_len) == 0)
    {
        char next = module_path[project_name_len];
        if (next == '\0' || next == '.')
            return strdup(module_path);
    }

    const char *dot      = strchr(module_path, '.');
    size_t      head_len = dot ? (size_t)(dot - module_path) : strlen(module_path);

    char *head = malloc(head_len + 1);
    if (!head)
        return NULL;
    memcpy(head, module_path, head_len);
    head[head_len] = '\0';

    const char *alias_target = NULL;
    if (is_self_alias(head) && project_name_len > 0)
    {
        alias_target = config->name;
    }
    else
    {
        alias_target = config_get_dep_alias(config, head);
    }

    char *result = NULL;
    if (alias_target)
    {
        size_t alias_len = strlen(alias_target);
        if (dot)
        {
            size_t tail_len = strlen(dot + 1);
            size_t total    = alias_len + 1 + tail_len + 1;
            result          = malloc(total);
            if (result)
                snprintf(result, total, "%s.%s", alias_target, dot + 1);
        }
        else
        {
            result = strdup(alias_target);
        }
        free(head);
        return result;
    }

    if (!dot)
    {
        if (project_name_len > 0)
        {
            size_t tail_len = strlen(module_path);
            size_t total    = project_name_len + 1 + tail_len + 1;
            result          = malloc(total);
            if (result)
                snprintf(result, total, "%s.%s", config->name, module_path);
        }
        else
        {
            result = strdup(module_path);
        }

        free(head);
        return result;
    }

    // no dependency prefixing; assume already fully qualified - return original path
    free(head);
    return strdup(module_path);
}

bool config_ensure_dep_loaded(ProjectConfig *config, const char *project_dir, DepSpec *dep)
{
    (void)project_dir;
    (void)config;
    (void)dep;
    // no-op: deps no longer have nested config or src_dir defaults
    return true;
}

char *config_resolve_package_root(ProjectConfig *config, const char *project_dir, const char *package_name)
{
    if (!config || !project_dir || !package_name)
        return NULL;
    // root project
    if (strcmp(package_name, config->name) == 0)
    {
        return strdup(project_dir);
    }
    // external
    DepSpec *dep = config_get_dep(config, package_name);
    if (!dep)
        return NULL;
    // path is relative to project_dir if not absolute
    if (dep->path[0] == '/')
        return strdup(dep->path);
    size_t len = strlen(project_dir) + 1 + strlen(dep->path) + 1;
    char  *buf = malloc(len);
    snprintf(buf, len, "%s/%s", project_dir, dep->path);
    return buf;
}

char *config_get_package_src_dir(ProjectConfig *config, const char *project_dir, const char *package_name)
{
    char *root = config_resolve_package_root(config, project_dir, package_name);
    if (!root)
        return NULL;

    // for self, use project's src-dir
    if (strcmp(package_name, config->name) == 0)
    {
        const char *src_rel = config->src_dir ? config->src_dir : "src";
        size_t      len     = strlen(root) + 1 + strlen(src_rel) + 1;
        char       *path    = malloc(len);
        snprintf(path, len, "%s/%s", root, src_rel);
        free(root);
        return path;
    }

    // for dependencies, check if root/mach.toml exists
    size_t toml_path_len = strlen(root) + strlen("/mach.toml") + 1;
    char  *toml_path     = malloc(toml_path_len);
    snprintf(toml_path, toml_path_len, "%s/mach.toml", root);

    struct stat st;
    if (stat(toml_path, &st) == 0 && S_ISREG(st.st_mode))
    {
        // mach.toml exists - load it to find src directory
        ProjectConfig *dep_config = config_load(toml_path);
        free(toml_path);

        if (dep_config && dep_config->src_dir)
        {
            // build path: root/src_dir
            size_t len  = strlen(root) + 1 + strlen(dep_config->src_dir) + 1;
            char  *path = malloc(len);
            snprintf(path, len, "%s/%s", root, dep_config->src_dir);

            config_dnit(dep_config);
            free(dep_config);
            free(root);
            return path;
        }

        if (dep_config)
        {
            config_dnit(dep_config);
            free(dep_config);
        }

        // if config load failed or no src_dir, fall through to assume root is src
    }
    else
    {
        free(toml_path);
    }

    // no mach.toml or couldn't read it - assume path IS the source directory
    return root;
}

static char *duplicate_range(const char *start, size_t len)
{
    char *s = malloc(len + 1);
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

char *config_resolve_module_fqn(ProjectConfig *config, const char *project_dir, const char *fqn)
{
    if (!fqn)
        return NULL;

    char *normalized = config_expand_module_path(config, fqn);
    if (!normalized)
        return NULL;

    const char *cursor = normalized;
    const char *dot    = strchr(cursor, '.');
    if (!dot)
    {
        free(normalized);
        return NULL; // need pkg.segment
    }
    char *pkg     = duplicate_range(cursor, (size_t)(dot - cursor));
    char *src_dir = config_get_package_src_dir(config, project_dir, pkg);
    if (!src_dir)
    {
        free(pkg);
        free(normalized);
        return NULL;
    }
    const char *rest = dot + 1;
    // convert rest '.' to '/'
    size_t rest_len = strlen(rest);
    char  *rel      = malloc(rest_len + 1);
    for (size_t i = 0; i < rest_len; i++)
        rel[i] = (rest[i] == '.') ? '/' : rest[i];
    rel[rest_len]   = '\0';
    size_t full_len = strlen(src_dir) + 1 + strlen(rel) + 6; // '/' + name + '.mach' + '\0'
    char  *full     = malloc(full_len);
    snprintf(full, full_len, "%s/%s.mach", src_dir, rel);
    free(pkg);
    free(src_dir);
    free(rel);
    free(normalized);
    return full;
}

// simple placeholder lock writing (hashing not yet implemented)
// lockfile and lib-dependency APIs removed
