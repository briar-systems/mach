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

static AstNode *make_stmt_expr(AstNode *expr)
{
    AstNode *node = make_node(AST_STMT_EXPR);
    if (!node)
    {
        return NULL;
    }
    node->expr_stmt.expr = expr;
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
    node->fun_stmt.is_method            = false;
    node->fun_stmt.method_receiver      = NULL;
    node->fun_stmt.method_receiver_name = NULL;
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

static AstNode *make_masm_stmt(const char *content)
{
    AstNode *node = make_node(AST_STMT_MASM);
    if (!node)
    {
        return NULL;
    }
    node->masm_stmt.content = strdup(content ? content : "");
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

static char *build_mangled_symbol(const char *module_path, const char *name)
{
    const char *path = module_path ? module_path : "main";
    if (!name)
    {
        return NULL;
    }

    size_t encoded_len = 0;
    char  *path_copy   = strdup(path);
    char  *saveptr     = NULL;
    char  *token       = strtok_r(path_copy, ".", &saveptr);
    while (token)
    {
        char len_str[32];
        snprintf(len_str, sizeof(len_str), "%zu", strlen(token));
        encoded_len += strlen(len_str) + strlen(token);
        token = strtok_r(NULL, ".", &saveptr);
    }
    free(path_copy);

    size_t name_len = strlen(name);
    char   name_len_str[32];
    snprintf(name_len_str, sizeof(name_len_str), "%zu", name_len);

    size_t total_len = 2 + encoded_len + 1 + strlen(name_len_str) + name_len + 1;
    char  *mangled   = malloc(total_len);
    if (!mangled)
    {
        return NULL;
    }

    char *ptr = mangled;
    ptr += sprintf(ptr, "_M");

    path_copy = strdup(path);
    token     = strtok_r(path_copy, ".", &saveptr);
    while (token)
    {
        ptr += sprintf(ptr, "%zu%s", strlen(token), token);
        token = strtok_r(NULL, ".", &saveptr);
    }
    free(path_copy);

    sprintf(ptr, "N%zu%s", name_len, name);
    return mangled;
}

static bool program_has_fun_name(AstNode *program, const char *name)
{
    if (!program || program->kind != AST_PROGRAM || !program->program.stmts || !name)
    {
        return false;
    }

    for (int i = 0; i < program->program.stmts->count; i++)
    {
        AstNode *stmt = program->program.stmts->items[i];
        if (stmt && stmt->kind == AST_STMT_FUN && stmt->fun_stmt.name && strcmp(stmt->fun_stmt.name, name) == 0)
        {
            return true;
        }
    }

    return false;
}

static AstNode *build_test_write_fn(void)
{
    AstList *params = make_list();
    if (!params)
    {
        return NULL;
    }

    AstNode *u8_type   = make_type_name("u8");
    AstNode *ptr_type  = make_ptr_type(u8_type, false);
    AstNode *len_type  = make_type_name("i64");
    AstNode *param_ptr = make_param("ptr", ptr_type);
    AstNode *param_len = make_param("len", len_type);

    if (!param_ptr || !param_len)
    {
        return NULL;
    }

    ast_list_append(params, param_ptr);
    ast_list_append(params, param_len);

    AstNode *body = make_block();
    if (!body)
    {
        return NULL;
    }

    AstNode *masm = make_masm_stmt("mov rax, 1\n"
                                   "mov rdi, 1\n"
                                   "mov rsi, ptr\n"
                                   "mov rdx, len\n"
                                   "syscall\n");
    if (!masm)
    {
        return NULL;
    }

    ast_list_append(body->block_stmt.stmts, masm);

    return make_fun("__mach_test_write", params, NULL, body);
}

static AstNode *build_test_main_fn(TestInfo *tests, int test_count)
{
    AstNode *body = make_block();
    if (!body)
    {
        return NULL;
    }

    AstNode *failures_decl = make_var_stmt("failures", make_type_name("i64"), make_lit_int(0), false);
    if (!failures_decl)
    {
        return NULL;
    }
    ast_list_append(body->block_stmt.stmts, failures_decl);

    for (int i = 0; i < test_count; i++)
    {
        TestInfo *info = &tests[i];
        char      result_name[128];
        snprintf(result_name, sizeof(result_name), "__mach_test_result_%d", i);

        AstNode *call_test   = make_call(info->fn_name, NULL, 0);
        AstNode *result_decl = make_var_stmt(result_name, make_type_name("i64"), call_test, false);
        ast_list_append(body->block_stmt.stmts, result_decl);

        AstNode *cond = make_binary(TOKEN_EQUAL_EQUAL, make_ident(result_name), make_lit_int(0));

        // then block: ok prefix
        AstNode *then_block = make_block();
        AstNode *ok_args[]  = {make_lit_string("ok: "), make_lit_int(4)};
        AstNode *ok_call    = make_call("__mach_test_write", ok_args, 2);
        ast_list_append(then_block->block_stmt.stmts, make_stmt_expr(ok_call));

        // else block: fail prefix + failures++
        AstNode *else_block  = make_block();
        AstNode *fail_args[] = {make_lit_string("fail: "), make_lit_int(6)};
        AstNode *fail_call   = make_call("__mach_test_write", fail_args, 2);
        ast_list_append(else_block->block_stmt.stmts, make_stmt_expr(fail_call));

        AstNode *fail_plus   = make_binary(TOKEN_PLUS, make_ident("failures"), make_lit_int(1));
        AstNode *fail_assign = make_binary(TOKEN_EQUAL, make_ident("failures"), fail_plus);
        ast_list_append(else_block->block_stmt.stmts, make_stmt_expr(fail_assign));

        AstNode *if_stmt = make_if_stmt(cond, then_block, else_block);
        ast_list_append(body->block_stmt.stmts, if_stmt);

        // print test name and newline
        size_t   name_len    = info->name ? strlen(info->name) : 0;
        AstNode *name_args[] = {make_lit_string(info->name ? info->name : ""), make_lit_int((int64_t)name_len)};
        AstNode *name_call   = make_call("__mach_test_write", name_args, 2);
        ast_list_append(body->block_stmt.stmts, make_stmt_expr(name_call));

        AstNode *nl_args[] = {make_lit_string("\n"), make_lit_int(1)};
        AstNode *nl_call   = make_call("__mach_test_write", nl_args, 2);
        ast_list_append(body->block_stmt.stmts, make_stmt_expr(nl_call));
    }

    AstNode *ret_stmt = make_ret_stmt(make_ident("failures"));
    ast_list_append(body->block_stmt.stmts, ret_stmt);

    return make_fun("__mach_test_main", make_list(), make_type_name("i64"), body);
}

static AstNode *build_test_start_fn(void)
{
    AstNode *body = make_block();
    if (!body)
    {
        return NULL;
    }

    AstNode *call_main = make_call("__mach_test_main", NULL, 0);
    ast_list_append(body->block_stmt.stmts, make_stmt_expr(call_main));

    AstNode *exit_masm = make_masm_stmt("mov rdi, rax\n"
                                        "mov rax, 60\n"
                                        "syscall\n");
    ast_list_append(body->block_stmt.stmts, exit_masm);

    AstNode *start_fn = make_fun("_start", make_list(), NULL, body);
    if (start_fn)
    {
        start_fn->fun_stmt.is_public = true;
    }
    return start_fn;
}

static int collect_tests(AstNode *program, TestInfo **out_tests)
{
    if (!program || program->kind != AST_PROGRAM || !program->program.stmts)
    {
        return 0;
    }

    int       count    = 0;
    int       capacity = 8;
    TestInfo *tests    = calloc((size_t)capacity, sizeof(TestInfo));
    if (!tests)
    {
        return 0;
    }

    for (int i = 0; i < program->program.stmts->count; i++)
    {
        AstNode *stmt = program->program.stmts->items[i];
        if (!stmt || stmt->kind != AST_STMT_TEST)
        {
            continue;
        }

        if (count >= capacity)
        {
            capacity *= 2;
            TestInfo *next = realloc(tests, sizeof(TestInfo) * (size_t)capacity);
            if (!next)
            {
                break;
            }
            tests = next;
        }

        tests[count].name    = stmt->test_stmt.name ? strdup(stmt->test_stmt.name) : strdup("");
        tests[count].fn_name = NULL;
        tests[count].body    = stmt->test_stmt.body;
        count++;
    }

    if (count == 0)
    {
        free(tests);
        return 0;
    }

    *out_tests = tests;
    return count;
}

static int transform_tests(AstNode *program, const char *module_path, TestInfo **out_tests)
{
    if (!program || program->kind != AST_PROGRAM || !program->program.stmts)
    {
        return 0;
    }

    if (program_has_fun_name(program, "_start") || program_has_fun_name(program, "__mach_test_main") || program_has_fun_name(program, "__mach_test_write"))
    {
        fprintf(stderr, "error: test harness symbol name conflicts with existing function\n");
        return -1;
    }

    TestInfo *tests      = NULL;
    int       test_count = collect_tests(program, &tests);
    if (test_count <= 0)
    {
        return 0;
    }

    AstList *new_stmts = make_list();
    if (!new_stmts)
    {
        return -1;
    }

    // keep non-test statements
    for (int i = 0; i < program->program.stmts->count; i++)
    {
        AstNode *stmt = program->program.stmts->items[i];
        if (!stmt || stmt->kind == AST_STMT_TEST)
        {
            continue;
        }
        ast_list_append(new_stmts, stmt);
    }

    // add test functions
    for (int i = 0; i < test_count; i++)
    {
        tests[i].fn_name = make_test_fn_name(module_path, i);
        if (!tests[i].fn_name)
        {
            continue;
        }

        AstNode *body = tests[i].body;
        if (body && body->kind == AST_STMT_BLOCK && body->block_stmt.stmts)
        {
            ast_list_append(body->block_stmt.stmts, make_ret_stmt(make_lit_int(0)));
        }

        AstNode *test_fn = make_fun(tests[i].fn_name, make_list(), make_type_name("i64"), body);
        ast_list_append(new_stmts, test_fn);
    }

    AstNode *write_fn = build_test_write_fn();
    AstNode *main_fn  = build_test_main_fn(tests, test_count);
    AstNode *start_fn = build_test_start_fn();

    if (!write_fn || !main_fn || !start_fn)
    {
        fprintf(stderr, "error: failed to build test harness\n");
        return -1;
    }

    ast_list_append(new_stmts, write_fn);
    ast_list_append(new_stmts, main_fn);
    ast_list_append(new_stmts, start_fn);

    program->program.stmts = new_stmts;
    *out_tests             = tests;
    return test_count;
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

static char *build_test_output_path(const char *project_root, Config *config, ConfigTarget *target, const char *src_root, const char *file_path)
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
    char *tests_root = path_join(out_target, target->dir_tests);
    char *output     = path_join(tests_root, rel_path);

    free(rel_path);
    free(out_root);
    free(out_target);
    free(tests_root);

    return output;
}

void cmd_test_help(FILE *stream)
{
    fprintf(stream, "usage: mach test [--target <name>] [path]\n");
    fprintf(stream, "\n");
    fprintf(stream, "build and run all tests in a Mach project\n");
    fprintf(stream, "\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  --target <name>      select target from mach.toml (default: project target)\n");
    fprintf(stream, "  path                 project directory (default: current directory)\n");
}

int cmd_test_handle(int argc, char **argv)
{
    const char *target_name  = NULL;
    const char *project_path = NULL;

    for (int i = 2; i < argc; i++)
    {
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

    if (!target->dir_tests)
    {
        fprintf(stderr, "error: missing 'dir_tests' for target in mach.toml\n");
        config_dnit(config);
        free(config);
        free(project_root);
        return 1;
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

    int  total_tests    = 0;
    int  total_failures = 0;
    int  total_modules  = 0;
    bool had_error      = false;

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
            continue;
        }

        TestInfo *tests      = NULL;
        int       test_count = transform_tests(ast, module_path, &tests);
        if (test_count == 0)
        {
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            continue;
        }
        if (test_count < 0)
        {
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        total_modules++;
        total_tests += test_count;

        Sema *sema = sema_create(module_path);
        if (!sema)
        {
            fprintf(stderr, "error: failed to create semantic analyzer\n");
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        char *abs_src_root = absolutize_path(src_root);
        if (abs_src_root && config->id)
        {
            sema_set_module_roots(sema, config->id, abs_src_root, dep_root, config->deps, config->dep_count);
        }
        free(abs_src_root);

        sema_set_file_context(sema, files[i], source);

        if (sema_analyze(sema, ast) < 0)
        {
            sema_print_errors(sema);
            sema_destroy(sema);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        Masm *masm = masm_lower_module(ast, sema_get_main_module_table(sema));
        if (!masm)
        {
            fprintf(stderr, "error: lowering failed for '%s'\n", files[i]);
            sema_destroy(sema);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        SemaLoadedModule loaded[64];
        int              loaded_count = sema_get_loaded_modules(sema, loaded, 64);
        for (int m = 0; m < loaded_count; m++)
        {
            Masm *imported_masm = masm_lower_module(loaded[m].ast, loaded[m].table);
            if (imported_masm)
            {
                masm_merge(masm, imported_masm);
                masm_destroy(imported_masm);
            }
        }

        char *output_path = build_test_output_path(project_root, config, target, src_root, files[i]);
        if (!output_path)
        {
            fprintf(stderr, "error: failed to compute output path for '%s'\n", files[i]);
            masm_destroy(masm);
            sema_destroy(sema);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        char *out_dir = path_dirname(output_path);
        if (out_dir)
        {
            ensure_dir_recursive(out_dir);
            free(out_dir);
        }

        char obj_path[2048];
        snprintf(obj_path, sizeof(obj_path), "%s.o", output_path);

        if (masm_emit_object(masm, obj_path) < 0)
        {
            fprintf(stderr, "error: failed to emit object for '%s'\n", files[i]);
            free(output_path);
            masm_destroy(masm);
            sema_destroy(sema);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        char *entry_symbol = build_mangled_symbol(module_path, "_start");
        if (!entry_symbol)
        {
            fprintf(stderr, "error: failed to build entry symbol for '%s'\n", files[i]);
            free(output_path);
            masm_destroy(masm);
            sema_destroy(sema);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        char link_cmd[4096];
        snprintf(link_cmd, sizeof(link_cmd), "cc -nostdlib -no-pie -Wl,-e,%s -o %s %s", entry_symbol, output_path, obj_path);
        int rc = system(link_cmd);
        free(entry_symbol);
        if (rc != 0)
        {
            fprintf(stderr, "error: linking failed for '%s' (%d)\n", files[i], rc);
            free(output_path);
            masm_destroy(masm);
            sema_destroy(sema);
            parser_dnit(&parser);
            lexer_dnit(&lexer);
            free(source);
            free(module_path);
            had_error = true;
            continue;
        }

        char chmod_cmd[4096];
        snprintf(chmod_cmd, sizeof(chmod_cmd), "chmod +x %s 2>/dev/null", output_path);
        (void)system(chmod_cmd);

        printf("[test] %s (%d)\n", module_path, test_count);
        int exit_code = process_execute(output_path);
        if (exit_code != 0)
        {
            printf("[fail] %s (%d)\n", module_path, exit_code);
            total_failures += exit_code > 0 ? exit_code : 1;
        }
        else
        {
            printf("[ok] %s\n", module_path);
        }

        free(output_path);
        masm_destroy(masm);
        sema_destroy(sema);
        parser_dnit(&parser);
        lexer_dnit(&lexer);
        free(source);

        if (tests)
        {
            for (int t = 0; t < test_count; t++)
            {
                free(tests[t].name);
                free(tests[t].fn_name);
            }
            free(tests);
        }

        free(module_path);
    }

    free_string_array(files);
    free(src_root);
    free(dep_root);
    config_dnit(config);
    free(config);
    free(project_root);

    if (total_modules == 0)
    {
        if (had_error)
        {
            return 1;
        }
        printf("no tests found\n");
        return 0;
    }

    printf("tests: %d, failures: %d\n", total_tests, total_failures);
    if (total_failures != 0)
    {
        return 1;
    }
    return had_error ? 1 : 0;
}
