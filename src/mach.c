#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "lowering.h"
#include "parser.h"
#include "sema.h"
#include "target.h"
#include "visitor.h"

#define VERSION "0.0.1"

typedef struct Options Options;

struct Options
{
    Target target;
    
    char *path_in;
    char *path_out;

    bool dump_ast;
    bool dump_ir;
};

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);
    if (!buffer)
    {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read < file_size)
    {
        perror("error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);

    return buffer;
}

void build(Options opt)
{
    if (!opt.path_in)
    {
        fprintf(stderr, "error: no input path specified\n");
        return;
    }

    if (!opt.path_out)
    {
        fprintf(stderr, "error: no output path specified\n");
        return;
    }

    printf("building to target: %s\n", target_to_triple(opt.target));
    
    char *source = read_file(opt.path_in);
    if (!source)
    {
        printf("error: failed to read source file `%s`\n", opt.path_in);
        return;
    }

    Lexer *lexer = calloc(sizeof(Lexer), 1);
    if (!lexer_init(lexer, source))
    {
        fprintf(stderr, "error: failed to create lexer\n");
        free(source);
        return;
    }

    Parser *parser = calloc(sizeof(Parser), 1);
    if (!parser_init(parser, lexer))
    {
        fprintf(stderr, "error: failed to create parser\n");
        lexer_free(lexer);
        free(source);
        return;
    }

    printf("parsing...\n");
    Node *ast = parser_parse(parser);
    if (!ast)
    {
        fprintf(stderr, "error: parsing failed\n");
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return;
    }

    if (parser_has_errors(parser))
    {
        parser_print_errors(parser, opt.path_in);
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return;
    }

    printf("semantic analysis...\n");
    Visitor visitor_sema;
    SEMAContext *ctx_sema = calloc(sizeof(SEMAContext), 1);
    if (!sema_init(ctx_sema, opt.target))
    {
        fprintf(stderr, "error: failed to initialize semantic analysis context\n");
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return;
    }

    if (!visitor_init_sema(&visitor_sema, ctx_sema))
    {
        fprintf(stderr, "error: failed to create semantic analysis visitor\n");
        sema_free(ctx_sema);
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return;
    }
    visitor_visit(&visitor_sema, ast);

    if (ctx_sema->errors.len > 0)
    {
        sema_print_errors(ctx_sema, lexer, opt.path_in);
        sema_free(ctx_sema);
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return;
    }

    sema_free(ctx_sema);
    parser_free(parser);
    lexer_free(lexer);
    free(source);
}

void cmd_mach_help(int argc, char *argv[])
{
    printf("Usage: cmach <command> [options]\n");
    printf("Commands:\n");
    printf("  help       display this help message\n");
    printf("  version    display version information\n");
    printf("  build      build a project or file\n");
}

void cmd_mach_build(int argc, char *argv[])
{
    Options opt;

    if (argc > 1)
    {
        opt.path_in = argv[1];

        build(opt);

        return;
    }

    printf("Usage: cmach build <path> [options]\n");
    printf("Args:\n");
    printf("  <path>    path for build input which can be a source file or project config\n");
    printf("Options:\n");
    printf("  --target/-t    specify non-native target triple\n");
    printf("  --output/-o    specify output destination\n");
    printf("  --ast          dump generated AST\n");
    printf("  --ir           dump generated IR\n");
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        char *arg = argv[1];

        if (strcmp(arg, "help") == 0)
        {
            cmd_mach_help(argc - 1, &arg);
            return 0;
        }
        else if (strcmp(arg, "version") == 0)
        {
            printf("cmach compiler v%s\n", VERSION);
            return 0;
        }
        else if (strcmp(arg, "build") == 0)
        {
            cmd_mach_build(argc - 1, &arg);
            return 0;
        }
    }

    cmd_mach_help(argc, argv);

    return 1;
}
