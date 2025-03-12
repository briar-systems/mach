#include "visitor.h"
#include <stdio.h>
#include <stdlib.h>

void visitor_visit(Visitor *visitor, Node *node)
{
    if (!node)
    {
        return;
    }

    if (visitor->depth >= MAX_VISITOR_DEPTH)
    {
        fprintf(stderr, "visitor depth exceeded\n");
        return;
    }

    visitor->trace[visitor->depth++] = node;

    switch (node->kind)
    {
    case NODE_UNKNOWN:
        visitor->visit_unknown(visitor, node);
        break;
    case NODE_ERROR:
        visitor->visit_error(visitor, node);
        break;
    case NODE_MODULE:
        visitor->visit_module(visitor, node);
        break;
    case NODE_COMMENT:
        visitor->visit_comment(visitor, node);
        break;
    case NODE_IDENTIFIER:
        visitor->visit_identifier(visitor, node);
        break;
    case NODE_LIT_INT:
        visitor->visit_lit_int(visitor, node);
        break;
    case NODE_LIT_FLOAT:
        visitor->visit_lit_float(visitor, node);
        break;
    case NODE_LIT_CHAR:
        visitor->visit_lit_char(visitor, node);
        break;
    case NODE_LIT_STRING:
        visitor->visit_lit_string(visitor, node);
        break;
    case NODE_POST_MEMBER:
        visitor->visit_post_member(visitor, node);
        break;
    case NODE_POST_CALL:
        visitor->visit_post_call(visitor, node);
        break;
    case NODE_POST_IDX_ARR:
        visitor->visit_post_idx_arr(visitor, node);
        break;
    case NODE_POST_CAST:
        visitor->visit_post_cast(visitor, node);
        break;
    case NODE_EXPR_UNARY:
        visitor->visit_expr_unary(visitor, node);
        break;
    case NODE_EXPR_BINARY:
        visitor->visit_expr_binary(visitor, node);
        break;
    case NODE_STMT_EXPR:
        visitor->visit_stmt_expr(visitor, node);
        break;
    case NODE_STMT_BLOCK:
        visitor->visit_stmt_block(visitor, node);
        break;
    case NODE_STMT_VAL:
        visitor->visit_stmt_val(visitor, node);
        break;
    case NODE_STMT_VAR:
        visitor->visit_stmt_var(visitor, node);
        break;
    case NODE_STMT_VOL:
        visitor->visit_stmt_vol(visitor, node);
        break;
    case NODE_STMT_DEF:
        visitor->visit_stmt_def(visitor, node);
        break;
    case NODE_STMT_USE:
        visitor->visit_stmt_use(visitor, node);
        break;
    case NODE_STMT_STR:
        visitor->visit_stmt_str(visitor, node);
        break;
    case NODE_STMT_UNI:
        visitor->visit_stmt_uni(visitor, node);
        break;
    case NODE_STMT_FUN:
        visitor->visit_stmt_fun(visitor, node);
        break;
    case NODE_STMT_IF:
        visitor->visit_stmt_if(visitor, node);
        break;
    case NODE_STMT_OR:
        visitor->visit_stmt_or(visitor, node);
        break;
    case NODE_STMT_FOR:
        visitor->visit_stmt_for(visitor, node);
        break;
    case NODE_STMT_BRK:
        visitor->visit_stmt_brk(visitor, node);
        break;
    case NODE_STMT_CNT:
        visitor->visit_stmt_cnt(visitor, node);
        break;
    case NODE_STMT_RET:
        visitor->visit_stmt_ret(visitor, node);
        break;
    case NODE_TYPE_ARR:
        visitor->visit_type_arr(visitor, node);
        break;
    case NODE_TYPE_PTR:
        visitor->visit_type_ptr(visitor, node);
        break;
    case NODE_TYPE_FUN:
        visitor->visit_type_fun(visitor, node);
        break;
    case NODE_TYPE_STR:
        visitor->visit_type_str(visitor, node);
        break;
    case NODE_TYPE_UNI:
        visitor->visit_type_uni(visitor, node);
        break;
    case NODE_FIELD:
        visitor->visit_field(visitor, node);
        break;
    }

    visitor->trace[visitor->depth] = NULL;
    visitor->depth--;
}

typedef struct {
    Lexer *lexer;
} ContextPrinter;

static void print_indent(Visitor *visitor) {
    for (int i = 0; i < visitor->depth - 1; i++) {
        printf("  ");
    }
}

static void print_location(Lexer *lexer, Token token) {
    int line = lexer_line_at(lexer, token.start);
    int column = lexer_column_at(lexer, token.start);

    printf("[%d:%d] ", line, column);
}

static void print_node_list(Visitor *visitor, NodeList *list, const char *prefix) {
    if (!list || list->len == 0) {
        printf("[]");
        return;
    }
    
    printf("[\n");
    for (int i = 0; i < list->len; i++) {
        print_indent(visitor);
        printf("  %s %d: ", prefix, i);
        visitor_visit(visitor, list->nodes[i]);
        if (i < list->len - 1) {
            printf(",");
        }
        printf("\n");
    }
    print_indent(visitor);
    printf("  ]");
}

void printer_unknown(Visitor *visitor, Node *_) {
    print_indent(visitor);
    printf("Unknown Node");
}

void printer_error(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Error: %s", node->error.message);
}

void printer_module(Visitor *visitor, Node *node) {
    print_indent(visitor);
    printf("Module:");
    if (node->module.stmts) {
        printf("\n");
        print_indent(visitor);
        printf("  Statements: ");
        print_node_list(visitor, node->module.stmts, "Stmt");
    }
}

void printer_comment(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Comment: %.*s", node->token.len, node->token.start);
}

void printer_identifier(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Identifier: %.*s", node->token.len, node->token.start);
}

void printer_lit_int(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Int: %ld", node->lit_int.value);
}

void printer_lit_float(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Float: %f", node->lit_float.value);
}

void printer_lit_char(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Char: '%c'", node->lit_char.value);
}

void printer_lit_string(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("String: \"%s\"", node->lit_string.value);
}

void printer_post_member(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Member Access:\n");
    
    print_indent(visitor);
    printf("  Target: ");
    visitor_visit(visitor, node->post_member.target);
    printf("\n");
    
    print_indent(visitor);
    printf("  Member: ");
    visitor_visit(visitor, node->post_member.member);
}

void printer_post_call(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Function Call:\n");
    
    print_indent(visitor);
    printf("  Target: ");
    visitor_visit(visitor, node->post_call.target);
    printf("\n");
    
    print_indent(visitor);
    printf("  Arguments: ");
    print_node_list(visitor, node->post_call.args, "Arg");
}

void printer_post_idx_arr(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Array Index:\n");
    
    print_indent(visitor);
    printf("  Target: ");
    visitor_visit(visitor, node->post_idx_arr.target);
    printf("\n");
    
    print_indent(visitor);
    printf("  Index: ");
    visitor_visit(visitor, node->post_idx_arr.index);
}

void printer_post_cast(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Type Cast:\n");
    
    print_indent(visitor);
    printf("  Target: ");
    visitor_visit(visitor, node->post_cast.target);
    printf("\n");
    
    print_indent(visitor);
    printf("  Type: ");
    visitor_visit(visitor, node->post_cast.type_node);
}

void printer_expr_unary(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Unary Expression: %s\n", op_to_string(node->expr_unary.op));
    
    print_indent(visitor);
    printf("  Operand: ");
    visitor_visit(visitor, node->expr_unary.right);
}

void printer_expr_binary(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Binary Expression: %s\n", op_to_string(node->expr_binary.op));
    
    print_indent(visitor);
    printf("  Left: ");
    visitor_visit(visitor, node->expr_binary.left);
    printf("\n");
    
    print_indent(visitor);
    printf("  Right: ");
    visitor_visit(visitor, node->expr_binary.right);
}

void printer_stmt_expr(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Expression Statement:\n");
    
    print_indent(visitor);
    printf("  Expression: ");
    visitor_visit(visitor, node->stmt_expr.expr);
}

void printer_stmt_block(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Block Statement:\n");
    
    print_indent(visitor);
    printf("  Statements: ");
    print_node_list(visitor, node->stmt_block.stmts, "Stmt");
}

void printer_stmt_val(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Val Declaration:\n");
    
    print_indent(visitor);
    printf("  Identifier: ");
    visitor_visit(visitor, node->stmt_val.ident);
    printf("\n");
    
    if (node->stmt_val.type_node) {
        print_indent(visitor);
        printf("  Type: ");
        visitor_visit(visitor, node->stmt_val.type_node);
        printf("\n");
    }
    
    if (node->stmt_val.expr) {
        print_indent(visitor);
        printf("  Initializer: ");
        visitor_visit(visitor, node->stmt_val.expr);
    }
}

void printer_stmt_var(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Var Declaration:\n");
    
    print_indent(visitor);
    printf("  Identifier: ");
    visitor_visit(visitor, node->stmt_var.ident);
    printf("\n");
    
    if (node->stmt_var.type_node) {
        print_indent(visitor);
        printf("  Type: ");
        visitor_visit(visitor, node->stmt_var.type_node);
        printf("\n");
    }
    
    if (node->stmt_var.expr) {
        print_indent(visitor);
        printf("  Initializer: ");
        visitor_visit(visitor, node->stmt_var.expr);
    }
}

void printer_stmt_vol(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Vol Declaration:\n");
    
    print_indent(visitor);
    printf("  Identifier: ");
    visitor_visit(visitor, node->stmt_vol.ident);
    printf("\n");
    
    if (node->stmt_vol.type_node) {
        print_indent(visitor);
        printf("  Type: ");
        visitor_visit(visitor, node->stmt_vol.type_node);
        printf("\n");
    }
    
    if (node->stmt_vol.expr) {
        print_indent(visitor);
        printf("  Initializer: ");
        visitor_visit(visitor, node->stmt_vol.expr);
    }
}

void printer_stmt_def(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Type Definition:\n");
    
    print_indent(visitor);
    printf("  Identifier: ");
    visitor_visit(visitor, node->stmt_def.ident);
    printf("\n");
    
    print_indent(visitor);
    printf("  Type: ");
    visitor_visit(visitor, node->stmt_def.type_node);
}

void printer_stmt_use(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Use Statement:\n");
    
    print_indent(visitor);
    printf("  Path: ");
    visitor_visit(visitor, node->stmt_use.path);
    
    if (node->stmt_use.alias) {
        printf("\n");
        print_indent(visitor);
        printf("  Alias: ");
        visitor_visit(visitor, node->stmt_use.alias);
    }
}

void printer_stmt_str(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Struct Declaration:\n");
    
    print_indent(visitor);
    printf("  Name: ");
    visitor_visit(visitor, node->stmt_str.ident);
    printf("\n");
    
    print_indent(visitor);
    printf("  Fields: ");
    print_node_list(visitor, node->stmt_str.fields, "Field");
}

void printer_stmt_uni(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Union Declaration:\n");
    
    print_indent(visitor);
    printf("  Name: ");
    visitor_visit(visitor, node->stmt_uni.ident);
    printf("\n");
    
    print_indent(visitor);
    printf("  Fields: ");
    print_node_list(visitor, node->stmt_uni.fields, "Field");
}

void printer_stmt_fun(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Function Declaration:\n");
    
    print_indent(visitor);
    printf("  Name: ");
    visitor_visit(visitor, node->stmt_fun.ident);
    printf("\n");
    
    print_indent(visitor);
    printf("  Parameters: ");
    print_node_list(visitor, node->stmt_fun.params, "Param");
    printf("\n");
    
    if (node->stmt_fun.type_node) {
        print_indent(visitor);
        printf("  Return Type: ");
        visitor_visit(visitor, node->stmt_fun.type_node);
        printf("\n");
    }
    
    if (node->stmt_fun.body) {
        print_indent(visitor);
        printf("  Body: ");
        visitor_visit(visitor, node->stmt_fun.body);
    }
}

void printer_stmt_if(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("If Statement:\n");
    
    print_indent(visitor);
    printf("  Condition: ");
    visitor_visit(visitor, node->stmt_if.cond);
    printf("\n");
    
    print_indent(visitor);
    printf("  Then: ");
    visitor_visit(visitor, node->stmt_if.body);
    
    if (node->stmt_if.branch) {
        printf("\n");
        print_indent(visitor);
        printf("  Else: ");
        visitor_visit(visitor, node->stmt_if.branch);
    }
}

void printer_stmt_or(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("OR Statement:\n");
    
    print_indent(visitor);
    printf("  Condition: ");
    visitor_visit(visitor, node->stmt_or.cond);
    printf("\n");
    
    print_indent(visitor);
    printf("  Body: ");
    visitor_visit(visitor, node->stmt_or.body);
}

void printer_stmt_for(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("For Loop:\n");
    
    print_indent(visitor);
    printf("  Condition: ");
    visitor_visit(visitor, node->stmt_for.cond);
    printf("\n");
    
    print_indent(visitor);
    printf("  Body: ");
    visitor_visit(visitor, node->stmt_for.body);
}

void printer_stmt_brk(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Break Statement");
}

void printer_stmt_cnt(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Continue Statement");
}

void printer_stmt_ret(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Return Statement");
    
    if (node->stmt_ret.expr) {
        printf(":\n");
        print_indent(visitor);
        printf("  Value: ");
        visitor_visit(visitor, node->stmt_ret.expr);
    }
}

void printer_type_arr(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Array Type:\n");
    
    print_indent(visitor);
    printf("  Element Type: ");
    visitor_visit(visitor, node->type_arr.type_node);
    
    if (node->type_arr.size) {
        printf("\n");
        print_indent(visitor);
        printf("  Size: ");
        visitor_visit(visitor, node->type_arr.size);
    }
}

void printer_type_ptr(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Pointer Type:\n");
    
    print_indent(visitor);
    printf("  Target Type: ");
    visitor_visit(visitor, node->type_ptr.type_node);
}

void printer_type_fun(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Function Type:\n");
    
    if (node->type_fun.params) {
        print_indent(visitor);
        printf("  Parameters: ");
        print_node_list(visitor, node->type_fun.params, "Param");
        printf("\n");
    }
    
    print_indent(visitor);
    printf("  Return Type: ");
    visitor_visit(visitor, node->type_fun.type_node);
}

void printer_type_str(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Struct Type:\n");
    
    print_indent(visitor);
    printf("  Fields: ");
    print_node_list(visitor, node->type_str.fields, "Field");
}

void printer_type_uni(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Union Type:\n");
    
    print_indent(visitor);
    printf("  Fields: ");
    print_node_list(visitor, node->type_uni.fields, "Field");
}

void printer_field(Visitor *visitor, Node *node) {
    ContextPrinter *ctx = (ContextPrinter*)visitor->context;

    print_location(ctx->lexer, node->token);
    printf("Field:\n");
    
    if (node->field.ident) {
        print_indent(visitor);
        printf("  Name: ");
        visitor_visit(visitor, node->field.ident);
        printf("\n");
    }
    
    print_indent(visitor);
    printf("  Type: ");
    visitor_visit(visitor, node->field.type_node);
}

bool visitor_init_printer(Visitor *visitor, Lexer *lexer)
{
    ContextPrinter *context = calloc(sizeof(ContextPrinter), 1);
    if (!context)
    {
        fprintf(stderr, "failed to initialize context\n");
        return false;
    }
    context->lexer = lexer;
    visitor->context = context;

    visitor->visit_unknown = printer_unknown;
    visitor->visit_error = printer_error;
    visitor->visit_module = printer_module;
    visitor->visit_comment = printer_comment;
    visitor->visit_identifier = printer_identifier;
    visitor->visit_lit_int = printer_lit_int;
    visitor->visit_lit_float = printer_lit_float;
    visitor->visit_lit_char = printer_lit_char;
    visitor->visit_lit_string = printer_lit_string;
    visitor->visit_post_member = printer_post_member;
    visitor->visit_post_call = printer_post_call;
    visitor->visit_post_idx_arr = printer_post_idx_arr;
    visitor->visit_post_cast = printer_post_cast;
    visitor->visit_expr_unary = printer_expr_unary;
    visitor->visit_expr_binary = printer_expr_binary;
    visitor->visit_stmt_expr = printer_stmt_expr;
    visitor->visit_stmt_block = printer_stmt_block;
    visitor->visit_stmt_val = printer_stmt_val;
    visitor->visit_stmt_var = printer_stmt_var;
    visitor->visit_stmt_vol = printer_stmt_vol;
    visitor->visit_stmt_def = printer_stmt_def;
    visitor->visit_stmt_use = printer_stmt_use;
    visitor->visit_stmt_str = printer_stmt_str;
    visitor->visit_stmt_uni = printer_stmt_uni;
    visitor->visit_stmt_fun = printer_stmt_fun;
    visitor->visit_stmt_if = printer_stmt_if;
    visitor->visit_stmt_or = printer_stmt_or;
    visitor->visit_stmt_for = printer_stmt_for;
    visitor->visit_stmt_brk = printer_stmt_brk;
    visitor->visit_stmt_cnt = printer_stmt_cnt;
    visitor->visit_stmt_ret = printer_stmt_ret;
    visitor->visit_type_arr = printer_type_arr;
    visitor->visit_type_ptr = printer_type_ptr;
    visitor->visit_type_fun = printer_type_fun;
    visitor->visit_type_str = printer_type_str;
    visitor->visit_type_uni = printer_type_uni;
    visitor->visit_field = printer_field;

    return true;
}
