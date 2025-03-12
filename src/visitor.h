#ifndef VISITOR_H
#define VISITOR_H

#define MAX_VISITOR_DEPTH 256

#include "lexer.h"
#include "ast.h"

typedef struct Visitor Visitor;

struct Visitor
{
    void *context;

    void (*visit_unknown)(Visitor *visitor, Node *node);
    void (*visit_error)(Visitor *visitor, Node *node);
    void (*visit_module)(Visitor *visitor, Node *node);
    void (*visit_comment)(Visitor *visitor, Node *node);
    void (*visit_identifier)(Visitor *visitor, Node *node);
    void (*visit_lit_int)(Visitor *visitor, Node *node);
    void (*visit_lit_float)(Visitor *visitor, Node *node);
    void (*visit_lit_char)(Visitor *visitor, Node *node);
    void (*visit_lit_string)(Visitor *visitor, Node *node);
    void (*visit_post_member)(Visitor *visitor, Node *node);
    void (*visit_post_call)(Visitor *visitor, Node *node);
    void (*visit_post_idx_arr)(Visitor *visitor, Node *node);
    void (*visit_post_cast)(Visitor *visitor, Node *node);
    void (*visit_expr_unary)(Visitor *visitor, Node *node);
    void (*visit_expr_binary)(Visitor *visitor, Node *node);
    void (*visit_stmt_expr)(Visitor *visitor, Node *node);
    void (*visit_stmt_block)(Visitor *visitor, Node *node);
    void (*visit_stmt_val)(Visitor *visitor, Node *node);
    void (*visit_stmt_var)(Visitor *visitor, Node *node);
    void (*visit_stmt_vol)(Visitor *visitor, Node *node);
    void (*visit_stmt_def)(Visitor *visitor, Node *node);
    void (*visit_stmt_use)(Visitor *visitor, Node *node);
    void (*visit_stmt_str)(Visitor *visitor, Node *node);
    void (*visit_stmt_uni)(Visitor *visitor, Node *node);
    void (*visit_stmt_fun)(Visitor *visitor, Node *node);
    void (*visit_stmt_if)(Visitor *visitor, Node *node);
    void (*visit_stmt_or)(Visitor *visitor, Node *node);
    void (*visit_stmt_for)(Visitor *visitor, Node *node);
    void (*visit_stmt_brk)(Visitor *visitor, Node *node);
    void (*visit_stmt_cnt)(Visitor *visitor, Node *node);
    void (*visit_stmt_ret)(Visitor *visitor, Node *node);
    void (*visit_type_arr)(Visitor *visitor, Node *node);
    void (*visit_type_ptr)(Visitor *visitor, Node *node);
    void (*visit_type_fun)(Visitor *visitor, Node *node);
    void (*visit_type_str)(Visitor *visitor, Node *node);
    void (*visit_type_uni)(Visitor *visitor, Node *node);
    void (*visit_field)(Visitor *visitor, Node *node);

    int depth;
    Node *trace[MAX_VISITOR_DEPTH];
};

void visitor_visit(Visitor *visitor, Node *node);

bool visitor_init_printer(Visitor *visitor, Lexer *lexer);

#endif // VISITOR_H
