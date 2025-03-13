#ifndef SEMA_H
#define SEMA_H

#include "ast.h"
#include "scope.h"
#include "type.h"
#include "visitor.h"
#include <stdbool.h>

typedef struct SEMAAnalyzer SEMAAnalyzer;
typedef struct TypeCache TypeCache;

typedef struct
{
    Token token;
    const char *message;
} SEMAError;

typedef struct
{
    SEMAError *errors;
    int len;
    int cap;
} SEMAErrorList;

struct TypeCache
{
    Type *void_type;
    Type *u8_type;
    Type *u16_type;
    Type *u32_type;
    Type *u64_type;
    Type *i8_type;
    Type *i16_type;
    Type *i32_type;
    Type *i64_type;
    Type *f32_type;
    Type *f64_type;
};

typedef struct
{
    Scope *scope_current;

    NodeTable *type_table;
    NodeTable *const_table;

    SEMAErrorList errors;
    TypeCache type_cache;

    bool in_const_context;
    bool in_loop_context;
    bool in_function_context;
    Type *current_function_return_type;

    Target target;
} SEMAContext;

bool init_type_cache(TypeCache *cache);
void free_type_cache(TypeCache *cache);

bool sema_init(SEMAContext *sema, Target target);
void sema_free(SEMAContext *sema);

void sema_report_error(SEMAContext *sema, Token token, const char *format, ...);
void sema_print_errors(SEMAContext *sema, Lexer *lexer, const char *filename);

Type *sema_get_type(SEMAContext *sema, Node *node);
bool sema_set_type(SEMAContext *sema, Node *node, Type *type);
bool sema_is_constant(SEMAContext *sema, Node *node);
Node *sema_get_constant(SEMAContext *sema, Node *node);
bool sema_set_constant(SEMAContext *sema, Node *node, Node *constant_value);

bool sema_types_compatible(Type *a, Type *b);
bool sema_can_convert_type(Type *from, Type *to);
Type *sema_common_type(SEMAContext *sema, Type *a, Type *b);

SEMAAnalyzer *sema_analyzer_new(SEMAContext *context);
void sema_analyzer_free(SEMAAnalyzer *analyzer);
bool sema_analyze(SEMAContext *sema, Node *ast);

void sema_enter_scope(SEMAContext *sema);
void sema_exit_scope(SEMAContext *sema);

Type *sema_get_builtin_type(SEMAContext *sema, const char *name);

void sema_visit_module(Visitor *visitor, Node *node);
void sema_visit_identifier(Visitor *visitor, Node *node);
void sema_visit_lit_int(Visitor *visitor, Node *node);
void sema_visit_lit_float(Visitor *visitor, Node *node);
void sema_visit_lit_char(Visitor *visitor, Node *node);
void sema_visit_lit_string(Visitor *visitor, Node *node);
void sema_visit_expr_unary(Visitor *visitor, Node *node);
void sema_visit_expr_binary(Visitor *visitor, Node *node);
void sema_visit_post_member(Visitor *visitor, Node *node);
void sema_visit_post_call(Visitor *visitor, Node *node);
void sema_visit_post_idx_arr(Visitor *visitor, Node *node);
void sema_visit_post_cast(Visitor *visitor, Node *node);
void sema_visit_stmt_expr(Visitor *visitor, Node *node);
void sema_visit_stmt_block(Visitor *visitor, Node *node);
void sema_visit_stmt_val(Visitor *visitor, Node *node);
void sema_visit_stmt_var(Visitor *visitor, Node *node);
void sema_visit_stmt_vol(Visitor *visitor, Node *node);
void sema_visit_stmt_def(Visitor *visitor, Node *node);
void sema_visit_stmt_str(Visitor *visitor, Node *node);
void sema_visit_stmt_uni(Visitor *visitor, Node *node);
void sema_visit_stmt_fun(Visitor *visitor, Node *node);
void sema_visit_stmt_if(Visitor *visitor, Node *node);
void sema_visit_stmt_or(Visitor *visitor, Node *node);
void sema_visit_stmt_for(Visitor *visitor, Node *node);
void sema_visit_stmt_brk(Visitor *visitor, Node *node);
void sema_visit_stmt_cnt(Visitor *visitor, Node *node);
void sema_visit_stmt_ret(Visitor *visitor, Node *node);
void sema_visit_stmt_use(Visitor *visitor, Node *node);
void sema_visit_type_arr(Visitor *visitor, Node *node);
void sema_visit_type_ptr(Visitor *visitor, Node *node);
void sema_visit_type_fun(Visitor *visitor, Node *node);
void sema_visit_type_str(Visitor *visitor, Node *node);
void sema_visit_type_uni(Visitor *visitor, Node *node);
void sema_visit_field(Visitor *visitor, Node *node);

bool visitor_init_sema(Visitor *visitor, SEMAContext *context);

#endif // SEMA_H
