#include "commands/cmd_build.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/sema.h"
#include "compiler/mir/lower.h"
#include "compiler/mir/emit.h"
#include "compiler/mir/target.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cmd_build_help(FILE *stream)
{
    fprintf(stream, "usage: mach build <project|file> [options]\n");
    fprintf(stream, "\n");
    fprintf(stream, "build a Mach project from the specified directory or compile a single Mach source file\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  --target <name>      select target from mach.toml (required for projects)\n");
    fprintf(stream, "  -o <file>            output file (executable or object)\n");
    fprintf(stream, "  -I n=dir             map module prefix 'n' to base directory 'dir'\n");
}

int cmd_build_handle(int argc, char **argv)
{
    // argv[0] = "cmach", argv[1] = "build", argv[2] = input file
    if (argc < 3)
    {
        fprintf(stderr, "error: no input file specified\n");
        cmd_build_help(stderr);
        return 1;
    }

    const char *input_file = argv[2];
    const char *output_file = NULL;

    // parse options
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            output_file = argv[++i];
        }
    }

    // default output name
    if (!output_file)
    {
        output_file = "output";
    }

    printf("Compiling %s...\n", input_file);

    // read source file
    FILE *f = fopen(input_file, "r");
    if (!f)
    {
        fprintf(stderr, "error: could not open file '%s'\n", input_file);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *source = malloc(size + 1);
    if (!source)
    {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return 1;
    }

    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    // lex
    Lexer lexer;
    lexer_init(&lexer, source);

    // parse
    Parser parser;
    parser_init(&parser, &lexer);

    AstNode *ast = parser_parse_program(&parser);
    if (!ast || parser.had_error)
    {
        fprintf(stderr, "error: parsing failed\n");
        parser_error_list_print(&parser.errors, &lexer, input_file);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // semantic analysis
    Sema *sema = sema_create();
    if (!sema)
    {
        fprintf(stderr, "error: failed to create semantic analyzer\n");
        // ast cleanup would go here
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    printf("  Running semantic analysis...\n");
    if (sema_analyze(sema, ast) < 0)
    {
        fprintf(stderr, "error: semantic analysis failed\n");
        sema_print_errors(sema);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // lower to MIR
    printf("  Lowering to MIR...\n");
    MIRModule *mir = mir_lower_module(ast);
    if (!mir)
    {
        fprintf(stderr, "error: lowering to MIR failed\n");
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // generate object file
    printf("  Generating object file...\n");
    char obj_file[512];
    snprintf(obj_file, sizeof(obj_file), "%s.o", output_file);

    MIRTarget target = mir_target_native();
    if (mir_emit_object(mir, &target, obj_file) < 0)
    {
        fprintf(stderr, "error: failed to emit object file\n");
        mir_module_destroy(mir);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    // link into executable
    printf("  Linking executable...\n");
    char link_cmd[1024];
    snprintf(link_cmd, sizeof(link_cmd), "ld -o %s %s 2>&1", output_file, obj_file);

    int link_result = system(link_cmd);
    if (link_result != 0)
    {
        fprintf(stderr, "error: linking failed\n");
        mir_module_destroy(mir);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        return 1;
    }

    printf("✓ Successfully built '%s'\n", output_file);

    // cleanup
    mir_module_destroy(mir);
    sema_destroy(sema);
    parser_dnit(&parser);
    lexer_dnit(&lexer);
    free(source);

    return 0;
}
