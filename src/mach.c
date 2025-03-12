#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "lowering.h"
#include "parser.h"
#include "target.h"
#include "visitor.h"

void print_usage(const char *program_name)
{
    printf("Usage: %s [options] input_file\n", program_name);
    printf("Options:\n");
    printf("  --output/-o <file>          Output file name (default: a.out)\n");
    printf("  --target/-t <target_triple> Target triple for compilation\n");
    printf("  --ir                        Output LLVM IR (.ll file)\n");
    printf("  --bc                        Output LLVM bitcode (.bc file)\n");
    printf("  --asm                       Output assembly code (.s file)\n");
    printf("  --obj                       Output object file (.o file)\n");
    printf("  --bin                       Output executable binary (default)\n");
    printf("  --help/-h                   Display this help message\n");
}

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

typedef enum OutputType
{
    OUTPUT_BINARY,
    OUTPUT_IR,
    OUTPUT_BITCODE,
    OUTPUT_ASSEMBLY,
    OUTPUT_OBJECT
} OutputType;

int main(int argc, char *argv[])
{
    const char *input_file = NULL;
    const char *output_file = "a.out";
    OutputType output_type = OUTPUT_BINARY;
    char *target_triple = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
            {
                output_file = argv[++i];
            }
            else
            {
                fprintf(stderr, "error: Output file name missing\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--target") == 0 || strcmp(argv[i], "-t") == 0)
        {
            if (i + 1 < argc)
            {
                target_triple = argv[++i];
            }
            else
            {
                fprintf(stderr, "error: Target triple missing\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--ir") == 0)
        {
            output_type = OUTPUT_IR;
        }
        else if (strcmp(argv[i], "--bc") == 0)
        {
            output_type = OUTPUT_BITCODE;
        }
        else if (strcmp(argv[i], "--asm") == 0)
        {
            output_type = OUTPUT_ASSEMBLY;
        }
        else if (strcmp(argv[i], "--obj") == 0)
        {
            output_type = OUTPUT_OBJECT;
        }
        else if (strcmp(argv[i], "--bin") == 0)
        {
            output_type = OUTPUT_BINARY;
        }
        else if (argv[i][0] == '-')
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
        else
        {
            input_file = argv[i];
        }
    }

    Target target = {0};
    if (target_triple)
    {
        target = target_from_string(target_triple);
        if (!valid_target(target))
        {
            fprintf(stderr, "error: invalid target triple '%s'\n", target_triple);
            return 1;
        }
    }
    else
    {
        target = target_current();
    }

    printf("Target: %s\n", target_to_string(target));

    if (!input_file)
    {
        fprintf(stderr, "error: input file not specified\n");
        print_usage(argv[0]);
        return 1;
    }

    char *source = read_file(input_file);
    if (!source)
    {
        return 1;
    }

    Lexer *lexer = calloc(sizeof(Lexer), 1);
    if (!lexer_init(lexer, source))
    {
        fprintf(stderr, "error: failed to create lexer\n");
        free(source);
        return 1;
    }

    Parser *parser = calloc(sizeof(Parser), 1);
    if (!parser_init(parser, lexer))
    {
        fprintf(stderr, "error: failed to create parser\n");
        lexer_free(lexer);
        free(source);
        return 1;
    }

    Node *ast = parser_parse(parser);
    if (!ast)
    {
        fprintf(stderr, "error: parsing failed\n");
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return 1;
    }

    if (parser_has_errors(parser))
    {
        printf("Parse errors found in `%s`\n", input_file);
        parser_print_errors(parser, input_file);
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return 1;
    }
    else {
        printf("No parse errors found in `%s`\n", input_file);
    }

    Visitor *printer = calloc(sizeof(Visitor), 1);
    if (!visitor_init_printer(printer, lexer))
    {
        fprintf(stderr, "error: failed to create printer visitor\n");
        parser_free(parser);
        lexer_free(lexer);
        free(source);
        return 1;
    }

    visitor_visit(printer, ast);
    printf("\n");

    // Lower AST to MIR
    // MIRModule *module = lower_ast_to_mir(ast, target, "main");
    // if (!module)
    // {
    //     fprintf(stderr, "error: Lowering to MIR failed\n");
    //     node_free(ast);
    //     free(source);
    //     return 1;
    // }

    // Optimize MIR
    // mir_optimize(module);

    // printf("Successfully compiled %s to %s (%d)\n", input_file, output_file, output_type);

    // mir_module_free(module);
    parser_free(parser);
    lexer_free(lexer);
    free(source);

    return 0;
}
