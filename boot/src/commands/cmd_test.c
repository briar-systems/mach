#include "commands/cmd_test.h"
#include "compiler/lexer.h"
#include "compiler/masm/emit.h"
#include "compiler/masm/lower.h"
#include "compiler/parser.h"
#include "compiler/sema.h"
#include "compiler/token.h"
#include "config.h"
#include "filesystem.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

typedef struct TestInfo
{
    char    *name;
    char    *fn_name;
    AstNode *body;
} TestInfo;

// forward declarations
static char *build_single_test_output_path(const char *project_root, Config *config, ConfigTarget *target, const char *src_root, const char *file_path, const char *test_name, int test_index);

static int process_execute(const char *path)
{
#ifdef _WIN32
    const char *args[2]   = {path, NULL};
    int         exit_code = _spawnv(_P_WAIT, path, args);
    if (exit_code == -1)
    {
        fprintf(stderr, "error: failed to execute '%s'\n", path);
        return 1;
    }
    return exit_code;
#else
    pid_t pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "error: failed to fork process\n");
        return 1;
    }

    if (pid == 0)
    {
        char *args[] = {(char *)path, NULL};
        execv(path, args);
        fprintf(stderr, "error: failed to execute '%s'\n", path);
        exit(1);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1)
    {
        fprintf(stderr, "error: failed to wait for process\n");
        return 1;
    }

    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }
    return 1;
#endif
}

static Token *make_token(TokenKind kind)
{
    Token *token = malloc(sizeof(Token));
    if (!token)
    {
        return NULL;
    }
    token_init(token, kind, 0, 0);
    return token;
}

static AstNode *make_node(AstKind kind)
{
    AstNode *node = malloc(sizeof(AstNode));
    if (!node)
    {
        return NULL;
    }
    ast_node_init(node, kind);
    return node;
}

static AstList *make_list(void)
{
    AstList *list = malloc(sizeof(AstList));
    if (!list)
    {
        return NULL;
    }
    ast_list_init(list);
    return list;
}

static AstNode *make_type_name(const char *name)
{
    AstNode *node = make_node(AST_TYPE_NAME);
    if (!node)
    {
        return NULL;
    }
    node->type_name.module_alias = NULL;
    node->type_name.name         = strdup(name);
    node->type_name.generic_args = NULL;
    return node;
}

static AstNode *make_ptr_type(AstNode *base, bool is_read_only)
{
    AstNode *node = make_node(AST_TYPE_PTR);
    if (!node)
    {
        return NULL;
    }
    node->type_ptr.base         = base;
    node->type_ptr.is_read_only = is_read_only;
    return node;
}

static AstNode *make_ident(const char *name)
{
    AstNode *node = make_node(AST_EXPR_IDENT);
    if (!node)
    {
        return NULL;
    }
    node->ident_expr.name = strdup(name);
    return node;
}

static AstNode *make_lit_int(int64_t value)
{
    AstNode *node = make_node(AST_EXPR_LIT);
    if (!node)
    {
        return NULL;
    }
    node->token            = make_token(TOKEN_LIT_INT);
    node->lit_expr.kind    = TOKEN_LIT_INT;
    node->lit_expr.int_val = (unsigned long long)value;
    return node;
}

static AstNode *make_lit_string(const char *value)
{
    AstNode *node = make_node(AST_EXPR_LIT);
    if (!node)
    {
        return NULL;
    }
    node->token               = make_token(TOKEN_LIT_STRING);
    node->lit_expr.kind       = TOKEN_LIT_STRING;
    node->lit_expr.string_val = strdup(value ? value : "");
    return node;
}

static AstNode *make_binary(TokenKind op, AstNode *left, AstNode *right)
{
    AstNode *node = make_node(AST_EXPR_BINARY);
    if (!node)
    {
        return NULL;
    }
    node->binary_expr.left  = left;
    node->binary_expr.right = right;
    node->binary_expr.op    = op;
    return node;
}

static AstNode *make_call(const char *name, AstNode **args, int arg_count)
{
    AstNode *node = make_node(AST_EXPR_CALL);
    if (!node)
    {
        return NULL;
    }

    node->call_expr.func           = make_ident(name);
    node->call_expr.args           = make_list();
    node->call_expr.type_args      = NULL;
    node->call_expr.is_method_call = false;

    if (!node->call_expr.func || !node->call_expr.args)
    {
        return node;
    }

    for (int i = 0; i < arg_count; i++)
    {
        ast_list_append(node->call_expr.args, args[i]);
    }

    return node;
}

static AstNode *make_var_stmt(const char *name, AstNode *type, AstNode *init, bool is_val)
{
    AstNode *node = make_node(is_val ? AST_STMT_VAL : AST_STMT_VAR);
    if (!node)
    {
        return NULL;
    }
    node->var_stmt.name      = strdup(name);
    node->var_stmt.type      = type;
    node->var_stmt.init      = init;
    node->var_stmt.is_val    = is_val;
    node->var_stmt.is_public = false;
    return node;
}

static AstNode *make_ret_stmt(AstNode *expr)
{
    AstNode *node = make_node(AST_STMT_RET);
    if (!node)
    {
        return NULL;
    }
    node->ret_stmt.expr = expr;
    return node;
}

static AstNode *make_block(void)
{
    AstNode *node = make_node(AST_STMT_BLOCK);
    if (!node)
    {
        return NULL;
    }
    node->block_stmt.stmts          = make_list();
    node->block_stmt.deferred_stmts = make_list();
    return node;
}

static AstNode *make_fun(const char *name, AstList *params, AstNode *return_type, AstNode *body)
{
    AstNode *node = make_node(AST_STMT_FUN);
    if (!node)
    {
        return NULL;
    }

    node->fun_stmt.name                 = strdup(name);
    node->fun_stmt.params               = params ? params : make_list();
    node->fun_stmt.generics             = NULL;
    node->fun_stmt.return_type          = return_type;
    node->fun_stmt.body                 = body;
    node->fun_stmt.is_variadic          = false;
    node->fun_stmt.is_public            = false;
    return node;
}

static AstNode *make_param(const char *name, AstNode *type)
{
    AstNode *node = make_node(AST_STMT_PARAM);
    if (!node)
    {
        return NULL;
    }
    node->param_stmt.name        = strdup(name);
    node->param_stmt.type        = type;
    node->param_stmt.is_variadic = false;
    return node;
}

static AstNode *make_if_stmt(AstNode *cond, AstNode *then_block, AstNode *else_block)
{
    AstNode *node = make_node(AST_STMT_IF);
    if (!node)
    {
        return NULL;
    }
    node->cond_stmt.cond    = cond;
    node->cond_stmt.body    = then_block;
    node->cond_stmt.stmt_or = NULL;

    if (else_block)
    {
        AstNode *or_node = make_node(AST_STMT_OR);
        if (!or_node)
        {
            return node;
        }
        or_node->cond_stmt.cond    = NULL;
        or_node->cond_stmt.body    = else_block;
        or_node->cond_stmt.stmt_or = NULL;
        node->cond_stmt.stmt_or    = or_node;
    }

    return node;
}

static AstNode *make_use_stmt(const char *module_path, const char *alias)
{
    AstNode *node = make_node(AST_STMT_USE);
    if (!node)
    {
        return NULL;
    }
    node->use_stmt.module_path = module_path ? strdup(module_path) : NULL;
    node->use_stmt.alias       = alias ? strdup(alias) : NULL;
    return node;
}

static char *sanitize_module_path(const char *module_path)
{
    if (!module_path)
    {
        return strdup("module");
    }

    size_t len = strlen(module_path);
    char  *out = malloc(len + 1);
    if (!out)
    {
        return NULL;
    }

    for (size_t i = 0; i < len; i++)
    {
        char c = module_path[i];
        if (c == '.' || c == '/' || c == '\\')
        {
            out[i] = '_';
        }
        else
        {
            out[i] = c;
        }
    }
    out[len] = '\0';
    return out;
}

static char *make_test_fn_name(const char *module_path, int index)
{
    char *sanitized = sanitize_module_path(module_path);
    if (!sanitized)
    {
        return NULL;
    }

    char buf[512];
    snprintf(buf, sizeof(buf), "__mach_test_%s_%d", sanitized, index);
    free(sanitized);
    return strdup(buf);
}

// build main function for external test runner using std.runtime conventions
// main calls the test function and returns 0 (pass) or 1 (fail)
static AstNode *build_test_main_fn_portable(const char *test_fn_name)
{
    AstNode *body = make_block();
    if (!body)
    {
        return NULL;
    }

    // val result: i64 = __mach_test();
    AstNode *call_test   = make_call(test_fn_name, NULL, 0);
    AstNode *result_decl = make_var_stmt("__result", make_type_name("i64"), call_test, false);
    ast_list_append(body->block_stmt.stmts, result_decl);

    // if (result != 0) { ret 0; } else { ret 1; }
    // test returns non-zero for pass, so we invert for exit code
    AstNode *cond       = make_binary(TOKEN_BANG_EQUAL, make_ident("__result"), make_lit_int(0));
    AstNode *then_block = make_block();
    ast_list_append(then_block->block_stmt.stmts, make_ret_stmt(make_lit_int(0)));
    AstNode *else_block = make_block();
    ast_list_append(else_block->block_stmt.stmts, make_ret_stmt(make_lit_int(1)));
    AstNode *if_stmt = make_if_stmt(cond, then_block, else_block);
    ast_list_append(body->block_stmt.stmts, if_stmt);

    // create main(argc: i64, argv: &&u8) i64
    // note: we ignore argc/argv but need them for std.runtime compatibility
    AstList *params    = make_list();
    AstNode *argc_type = make_type_name("i64");
    AstNode *argv_type = make_ptr_type(make_ptr_type(make_type_name("u8"), false), false);
    ast_list_append(params, make_param("argc", argc_type));
    ast_list_append(params, make_param("argv", argv_type));

    AstNode *main_fn = make_fun("__mach_test_main", params, make_type_name("i64"), body);
    return main_fn;
}

// count tests in a program without allocating
static int count_tests(AstNode *program)
{
    if (!program || program->kind != AST_PROGRAM || !program->program.stmts)
    {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < program->program.stmts->count; i++)
    {
        AstNode *stmt = program->program.stmts->items[i];
        if (stmt && stmt->kind == AST_STMT_TEST)
        {
            count++;
        }
    }
    return count;
}

// build a program with just one test for external test runner
// uses std.runtime for portable entry point instead of inline assembly
static AstNode *build_single_test_program(AstNode *original, int test_index, const char *module_path, char **out_test_name)
{
    if (!original || original->kind != AST_PROGRAM || !original->program.stmts)
    {
        return NULL;
    }

    // find the test at test_index
    int      test_count  = 0;
    AstNode *target_test = NULL;
    for (int i = 0; i < original->program.stmts->count; i++)
    {
        AstNode *stmt = original->program.stmts->items[i];
        if (stmt && stmt->kind == AST_STMT_TEST)
        {
            if (test_count == test_index)
            {
                target_test = stmt;
                break;
            }
            test_count++;
        }
    }

    if (!target_test)
    {
        return NULL;
    }

    // create new program with cloned non-test statements
    AstNode *program = make_node(AST_PROGRAM);
    if (!program)
    {
        return NULL;
    }
    program->program.stmts = make_list();

    // add use statements for std.runtime at the beginning
    // std.runtime provides _start which calls main and exits
    AstNode *use_runtime = make_use_stmt("std.runtime", NULL);
    if (use_runtime)
    {
        ast_list_append(program->program.stmts, use_runtime);
    }

    for (int i = 0; i < original->program.stmts->count; i++)
    {
        AstNode *stmt = original->program.stmts->items[i];
        if (!stmt)
        {
            continue;
        }
        // skip all test statements
        if (stmt->kind == AST_STMT_TEST)
        {
            continue;
        }
        // skip _start if it exists (std.runtime provides it)
        if (stmt->kind == AST_STMT_FUN && stmt->fun_stmt.name && strcmp(stmt->fun_stmt.name, "_start") == 0)
        {
            continue;
        }
        // skip any existing main function
        if (stmt->kind == AST_STMT_FUN && stmt->fun_stmt.name && strcmp(stmt->fun_stmt.name, "main") == 0)
        {
            continue;
        }
        // skip main symbol directive if present
        if (stmt->kind == AST_COMPTIME)
        {
            // check if this is a $main.symbol directive - skip it
            // we'll add our own main function
            AstNode *inner = stmt->comptime.inner;
            if (inner && inner->kind == AST_EXPR_BINARY && inner->binary_expr.op == TOKEN_EQUAL)
            {
                AstNode *left_side = inner->binary_expr.left;
                if (left_side && left_side->kind == AST_EXPR_FIELD)
                {
                    AstNode *obj = left_side->field_expr.object;
                    if (obj && obj->kind == AST_EXPR_IDENT && obj->ident_expr.name && strcmp(obj->ident_expr.name, "main") == 0)
                    {
                        continue; // skip $main.* directives
                    }
                }
            }
        }
        AstNode *cloned = ast_clone(stmt);
        if (cloned)
        {
            ast_list_append(program->program.stmts, cloned);
        }
    }

    // generate test function name
    char *test_fn_name = make_test_fn_name(module_path, test_index);
    if (!test_fn_name)
    {
        return NULL;
    }

    // clone the test body and add implicit return 1 (pass)
    AstNode *test_body = ast_clone(target_test->test_stmt.body);
    if (test_body && test_body->kind == AST_STMT_BLOCK && test_body->block_stmt.stmts)
    {
        ast_list_append(test_body->block_stmt.stmts, make_ret_stmt(make_lit_int(1)));
    }

    // create the test function
    AstNode *test_fn = make_fun(test_fn_name, make_list(), make_type_name("i64"), test_body);
    if (!test_fn)
    {
        free(test_fn_name);
        return NULL;
    }
    ast_list_append(program->program.stmts, test_fn);

    // add main function that calls the test and returns exit code
    AstNode *main_fn = build_test_main_fn_portable(test_fn_name);
    if (!main_fn)
    {
        free(test_fn_name);
        return NULL;
    }
    ast_list_append(program->program.stmts, main_fn);

    // add comptime directive to set main symbol: $__mach_test_main.symbol = "main";
    // this makes std.runtime's _start call our main function
    AstNode *symbol_field = make_node(AST_EXPR_FIELD);
    if (symbol_field)
    {
        symbol_field->field_expr.object = make_ident("__mach_test_main");
        symbol_field->field_expr.field  = strdup("symbol");

        AstNode *assign = make_binary(TOKEN_EQUAL, symbol_field, make_lit_string("main"));
        if (assign)
        {
            AstNode *directive = make_node(AST_COMPTIME);
            if (directive)
            {
                directive->comptime.inner = assign;
                ast_list_append(program->program.stmts, directive);
            }
        }
    }

    // output test name
    if (out_test_name)
    {
        *out_test_name = target_test->test_stmt.name ? strdup(target_test->test_stmt.name) : strdup("");
    }

    free(test_fn_name);
    return program;
}

// get test name by index without allocating full TestInfo array
static char *get_test_name(AstNode *program, int test_index)
{
    if (!program || program->kind != AST_PROGRAM || !program->program.stmts)
    {
        return NULL;
    }

    int count = 0;
    for (int i = 0; i < program->program.stmts->count; i++)
    {
        AstNode *stmt = program->program.stmts->items[i];
        if (stmt && stmt->kind == AST_STMT_TEST)
        {
            if (count == test_index)
            {
                return stmt->test_stmt.name ? strdup(stmt->test_stmt.name) : strdup("");
            }
            count++;
        }
    }
    return NULL;
}

// result codes for compile_and_run_single_test
typedef enum
{
    TEST_RESULT_PASS    = 0,
    TEST_RESULT_FAIL    = 1,
    TEST_RESULT_CRASH   = 2,
    TEST_RESULT_COMPILE = 3
} TestResult;

// compile and run a single test, returning the result
static TestResult
compile_and_run_single_test(AstNode *original_ast, int test_index, const char *module_path, const char *project_root, Config *config, ConfigTarget *target, const char *src_root, const char *dep_root, const char *file_path, char **out_test_name)
{
    // build single-test program
    char    *test_name = NULL;
    AstNode *test_ast  = build_single_test_program(original_ast, test_index, module_path, &test_name);
    if (!test_ast)
    {
        return TEST_RESULT_COMPILE;
    }

    if (out_test_name)
    {
        *out_test_name = test_name;
    }
    else if (test_name)
    {
        free(test_name);
    }

    // semantic analysis
    Sema *sema = sema_create(module_path);
    if (!sema)
    {
        return TEST_RESULT_COMPILE;
    }

    char *abs_src_root = absolutize_path(src_root);
    if (abs_src_root && config->id)
    {
        sema_set_module_roots(sema, config->id, abs_src_root, dep_root, config->deps, config->dep_count);
    }
    free(abs_src_root);

    sema_set_file_context(sema, file_path, NULL);

    if (sema_analyze(sema, test_ast) < 0)
    {
        sema_print_errors(sema);
        sema_destroy(sema);
        return TEST_RESULT_COMPILE;
    }

    // lowering
    Masm *masm = masm_lower_module(test_ast, sema_get_main_module_table(sema));
    if (!masm)
    {
        sema_destroy(sema);
        return TEST_RESULT_COMPILE;
    }

    // merge imported modules (including std.runtime)
    SemaLoadedModule loaded[256];
    int              loaded_count = sema_get_loaded_modules(sema, loaded, 256);
    for (int m = 0; m < loaded_count; m++)
    {
        Masm *imported_masm = masm_lower_module(loaded[m].ast, loaded[m].table);
        if (imported_masm)
        {
            masm_merge(masm, imported_masm);
            masm_destroy(imported_masm);
        }
    }

    // build output path for this specific test (use test_name for readable filename)
    char *output_path = build_single_test_output_path(project_root, config, target, src_root, file_path, test_name, test_index);
    if (!output_path)
    {
        masm_destroy(masm);
        sema_destroy(sema);
        return TEST_RESULT_COMPILE;
    }

    char *out_dir = path_dirname(output_path);
    if (out_dir)
    {
        ensure_dir_recursive(out_dir);
        free(out_dir);
    }

    // emit object file
    char obj_path[2048];
    snprintf(obj_path, sizeof(obj_path), "%s.o", output_path);

    if (masm_emit_object(masm, obj_path) < 0)
    {
        free(output_path);
        masm_destroy(masm);
        sema_destroy(sema);
        return TEST_RESULT_COMPILE;
    }

    // link - std.runtime provides _start which calls main
    // no need for custom entry point since we export main symbol
    char link_cmd[4096];
    snprintf(link_cmd, sizeof(link_cmd), "cc -nostdlib -no-pie -o %s %s", output_path, obj_path);
    int rc = system(link_cmd);
    if (rc != 0)
    {
        free(output_path);
        masm_destroy(masm);
        sema_destroy(sema);
        return TEST_RESULT_COMPILE;
    }

    // make executable
    char chmod_cmd[4096];
    snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x %s 2>/dev/null", output_path);
    (void)system(chmod_cmd);

    // run the test
    int exit_code = process_execute(output_path);

    // cleanup
    free(output_path);
    masm_destroy(masm);
    sema_destroy(sema);

    // interpret exit code
    if (exit_code > 128)
    {
        return TEST_RESULT_CRASH;
    }
    else if (exit_code == 0)
    {
        return TEST_RESULT_PASS;
    }
    else
    {
        return TEST_RESULT_FAIL;
    }
}

static char *derive_module_path(const char *project_root, Config *config, const char *file_path)
{
    if (!project_root || !config || !config->id || !config->dir_src || !file_path)
    {
        return NULL;
    }

    char *src_dir   = path_join(project_root, config->dir_src);
    char *abs_input = absolutize_path(file_path);
    if (!src_dir || !abs_input)
    {
        free(src_dir);
        free(abs_input);
        return NULL;
    }

    char *module_path = NULL;
    if (strncmp(abs_input, src_dir, strlen(src_dir)) == 0)
    {
        char *rel_path = abs_input + strlen(src_dir);
        if (is_sep(*rel_path))
        {
            rel_path++;
        }

        char *rel_no_ext = strdup(rel_path);
        char *dot        = strrchr(rel_no_ext, '.');
        if (dot)
        {
            *dot = '\0';
        }

        for (char *p = rel_no_ext; *p; p++)
        {
            if (is_sep(*p))
            {
                *p = '.';
            }
        }

        size_t len  = strlen(config->id) + 1 + strlen(rel_no_ext) + 1;
        module_path = malloc(len);
        snprintf(module_path, len, "%s.%s", config->id, rel_no_ext);
        free(rel_no_ext);
    }

    free(src_dir);
    free(abs_input);
    return module_path;
}

// sanitize a test name for use as a filename
// replaces spaces and special chars with underscores
static char *sanitize_test_name(const char *name, int test_index)
{
    if (!name || *name == '\0')
    {
        // fallback to index-based name
        char *buf = malloc(32);
        if (buf)
        {
            snprintf(buf, 32, "test_%d", test_index);
        }
        return buf;
    }

    size_t len       = strlen(name);
    char  *sanitized = malloc(len + 1);
    if (!sanitized)
    {
        return NULL;
    }

    for (size_t i = 0; i < len; i++)
    {
        char c = name[i];
        // keep alphanumeric and some safe chars
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_')
        {
            sanitized[i] = c;
        }
        else
        {
            sanitized[i] = '_';
        }
    }
    sanitized[len] = '\0';
    return sanitized;
}

// build output path for a single test executable
// format: <tests_root>/<rel_path>/<sanitized_test_name>
static char *build_single_test_output_path(const char *project_root, Config *config, ConfigTarget *target, const char *src_root, const char *file_path, const char *test_name, int test_index)
{
    if (!project_root || !config || !target || !src_root || !file_path)
    {
        return NULL;
    }

    char *rel_path = NULL;
    if (strncmp(file_path, src_root, strlen(src_root)) == 0)
    {
        rel_path = strdup(file_path + strlen(src_root));
        if (rel_path && is_sep(rel_path[0]))
        {
            memmove(rel_path, rel_path + 1, strlen(rel_path));
        }
    }

    if (!rel_path)
    {
        return NULL;
    }

    char *dot = strrchr(rel_path, '.');
    if (dot)
    {
        *dot = '\0';
    }

    char *out_root   = path_join(project_root, config->dir_out);
    char *out_target = path_join(out_root, target->artifacts);
    char *tests_root = path_join(out_target, ".tests");
    char *module_dir = path_join(tests_root, rel_path);

    // create output path with sanitized test name
    char *sanitized = sanitize_test_name(test_name, test_index);
    char *output    = path_join(module_dir, sanitized ? sanitized : "test");
    free(sanitized);

    free(rel_path);
    free(out_root);
    free(out_target);
    free(tests_root);
    free(module_dir);

    return output;
}

void cmd_test_help(FILE *stream)
{
    fprintf(stream, "usage: mach test [options] [path]\n");
    fprintf(stream, "\n");
    fprintf(stream, "build and run all tests in a Mach project\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  --target <name>      select target from mach.toml (default: project target)\n");
    fprintf(stream, "  --filter <pattern>   only run tests matching pattern (substring match)\n");
    fprintf(stream, "  -v, --verbose        show all test results (pass and fail)\n");
    fprintf(stream, "  -m, --modules        show module-level progress\n");
    fprintf(stream, "  path                 project directory (default: current directory)\n");
}

int cmd_test_handle(int argc, char **argv)
{
    const char *target_name    = NULL;
    const char *project_path   = NULL;
    const char *filter_pattern = NULL;
    bool        verbose        = false;
    bool        show_modules   = false;

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            cmd_test_help(stdout);
            return 0;
        }
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0)
        {
            verbose = true;
            continue;
        }
        if (strcmp(argv[i], "--modules") == 0 || strcmp(argv[i], "-m") == 0)
        {
            show_modules = true;
            continue;
        }
        if (strcmp(argv[i], "--filter") == 0)
        {
            if (i + 1 < argc)
            {
                filter_pattern = argv[++i];
                continue;
            }
            fprintf(stderr, "error: --filter requires a pattern\n");
            return 1;
        }
        else if (strncmp(argv[i], "--filter=", 9) == 0)
        {
            filter_pattern = argv[i] + 9;
            continue;
        }
        if (strcmp(argv[i], "--target") == 0)
        {
            if (i + 1 < argc)
            {
                target_name = argv[++i];
                continue;
            }
            fprintf(stderr, "error: --target requires a target name\n");
            return 1;
        }
        else if (strncmp(argv[i], "--target=", 9) == 0)
        {
            target_name = argv[i] + 9;
            continue;
        }

        if (!project_path)
        {
            project_path = argv[i];
        }
    }

    if (!project_path)
    {
        project_path = ".";
    }

    char *project_root = find_project_root(project_path);
    if (!project_root)
    {
        fprintf(stderr, "error: failed to find project root\n");
        return 1;
    }

    char *config_path = path_join(project_root, "mach.toml");
    if (!config_path || !file_exists(config_path))
    {
        fprintf(stderr, "error: directory '%s' does not contain mach.toml\n", project_root);
        free(config_path);
        free(project_root);
        return 1;
    }

    Config *config = config_load(config_path);
    free(config_path);
    if (!config)
    {
        free(project_root);
        return 1;
    }

    ConfigTarget *target = NULL;
    if (target_name)
    {
        target = config_get_target(config, target_name);
        if (!target)
        {
            fprintf(stderr, "error: target '%s' not found in mach.toml\n", target_name);
            config_dnit(config);
            free(config);
            free(project_root);
            return 1;
        }
    }
    else
    {
        target = config_get_target(config, config->target);
        if (!target)
        {
            fprintf(stderr, "error: default target '%s' not found in mach.toml\n", config->target);
            config_dnit(config);
            free(config);
            free(project_root);
            return 1;
        }
    }

    char *src_root = path_join(project_root, config->dir_src);
    if (!src_root)
    {
        fprintf(stderr, "error: failed to resolve source root\n");
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
    }

    char *dep_root = NULL;
    if (config->dir_dep)
    {
        char *dep_dir_path = path_join(project_root, config->dir_dep);
        dep_root           = absolutize_path(dep_dir_path);
        free(dep_dir_path);
    }

    char **files = list_files_recursive(src_root);
    if (!files)
    {
        fprintf(stderr, "error: failed to list source files\n");
        free(src_root);
        free(dep_root);
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
    }

    int  total_modules  = 0;
    int  total_tests    = 0;
    int  total_passed   = 0;
    int  total_failures = 0;
    int  total_crashes  = 0;
    int  compile_errors = 0;
    bool had_error      = false;

    (void)total_modules; // used for future reporting

    // external test runner: compile and run each test individually
    for (int i = 0; files[i]; i++)
    {
        if (!is_mach_file(files[i]))
        {
            continue;
        }

        char *module_path = derive_module_path(project_root, config, files[i]);
        if (!module_path)
        {
            continue;
        }

        FILE *f = fopen(files[i], "r");
        if (!f)
        {
            free(module_path);
            continue;
        }

        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *source = malloc((size_t)size + 1);
        if (!source)
        {
            fclose(f);
            free(module_path);
            continue;
        }

        fread(source, 1, (size_t)size, f);
        source[size] = '\0';
        fclose(f);

        Lexer lexer;
        lexer_init(&lexer, source);

        Parser parser;
        parser_init(&parser, &lexer);

        AstNode *ast = parser_parse_program(&parser);
        if (!ast || parser.had_error)
        {
            fprintf(stderr, "error: parsing failed for '%s'\n", files[i]);
            parser_error_list_print(&parser.errors, &lexer, files[i]);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            compile_errors++;
            continue;
        }

        // count tests in this module without transforming
        int test_count = count_tests(ast);
        if (test_count == 0)
        {
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            continue;
        }

        // check filter at module level first
        bool module_matches = filter_pattern ? (strstr(module_path, filter_pattern) != NULL) : true;

        // track per-module results for reporting
        int module_passed  = 0;
        int module_failed  = 0;
        int module_crashed = 0;
        int module_compile = 0;
        int module_tested  = 0;

        if (show_modules)
        {
            printf("[module] %s (%d tests)\n", module_path, test_count);
        }

        total_modules++;

        // run each test individually
        for (int t = 0; t < test_count; t++)
        {
            char *test_name = get_test_name(ast, t);

            // apply filter: skip if neither module nor test name matches
            if (filter_pattern && !module_matches)
            {
                if (!test_name || strstr(test_name, filter_pattern) == NULL)
                {
                    free(test_name);
                    continue;
                }
            }

            total_tests++;
            module_tested++;

            // compile and run this single test
            char      *result_test_name = NULL;
            TestResult result           = compile_and_run_single_test(ast, t, module_path, project_root, config, target, src_root, dep_root, files[i], &result_test_name);

            const char *display_name = result_test_name ? result_test_name : (test_name ? test_name : "(unnamed)");

            switch (result)
            {
            case TEST_RESULT_PASS:
                total_passed++;
                module_passed++;
                if (verbose)
                {
                    printf("pass: %s\n", display_name);
                }
                break;

            case TEST_RESULT_FAIL:
                total_failures++;
                module_failed++;
                printf("fail: %s\n", display_name);
                break;

            case TEST_RESULT_CRASH:
                total_crashes++;
                module_crashed++;
                printf("crash: %s\n", display_name);
                break;

            case TEST_RESULT_COMPILE:
                compile_errors++;
                module_compile++;
                had_error = true;
                fprintf(stderr, "error: compile failed for test '%s'\n", display_name);
                break;
            }

            free(result_test_name);
            free(test_name);
        }

        // module summary if showing modules
        if (show_modules && module_tested > 0)
        {
            if (module_failed == 0 && module_crashed == 0 && module_compile == 0)
            {
                printf("  [pass] %d/%d\n", module_passed, module_tested);
            }
            else
            {
                printf("  [done] %d passed, %d failed, %d crashed, %d compile errors\n", module_passed, module_failed, module_crashed, module_compile);
            }
        }

        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);
        free(module_path);
    }

    free_string_array(files);
    free(src_root);
    free(dep_root);
    config_dnit(config);
    free(config);
    free(project_root);

    if (total_tests == 0)
    {
        if (had_error)
        {
            return 1;
        }
        printf("no tests found\n");
        return 0;
    }

    printf("\n--- results ---\n");
    printf("passed:  %d\n", total_passed);
    if (total_failures > 0 || total_crashes > 0 || compile_errors > 0)
    {
        printf("failed:  %d\n", total_failures);
        if (total_crashes > 0)
        {
            printf("crashed: %d\n", total_crashes);
        }
        if (compile_errors > 0)
        {
            printf("compile: %d\n", compile_errors);
        }
    }
    printf("total:   %d\n", total_tests);

    if (total_failures != 0 || total_crashes != 0 || compile_errors != 0)
    {
        return 1;
    }
    return had_error ? 1 : 0;
}
