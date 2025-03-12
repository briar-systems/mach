#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

const char *node_kind_to_string(NodeKind kind)
{
    switch (kind)
    {
    case NODE_UNKNOWN:
        return "NODE_UNKNOWN";
    case NODE_ERROR:
        return "NODE_ERROR";
    case NODE_MODULE:
        return "NODE_MODULE";
    case NODE_COMMENT:
        return "NODE_COMMENT";
    case NODE_IDENTIFIER:
        return "NODE_IDENTIFIER";
    case NODE_LIT_INT:
        return "NODE_LIT_INT";
    case NODE_LIT_FLOAT:
        return "NODE_LIT_FLOAT";
    case NODE_LIT_CHAR:
        return "NODE_LIT_CHAR";
    case NODE_LIT_STRING:
        return "NODE_LIT_STRING";
    case NODE_POST_MEMBER:
        return "NODE_POST_MEMBER";
    case NODE_POST_CALL:
        return "NODE_POST_CALL";
    case NODE_POST_IDX_ARR:
        return "NODE_POST_IDX_ARR";
    case NODE_POST_CAST:
        return "NODE_POST_CAST";
    case NODE_EXPR_UNARY:
        return "NODE_EXPR_UNARY";
    case NODE_EXPR_BINARY:
        return "NODE_EXPR_BINARY";
    case NODE_STMT_EXPR:
        return "NODE_STMT_EXPR";
    case NODE_STMT_BLOCK:
        return "NODE_STMT_BLOCK";
    case NODE_STMT_VAL:
        return "NODE_STMT_VAL";
    case NODE_STMT_VAR:
        return "NODE_STMT_VAR";
    case NODE_STMT_VOL:
        return "NODE_STMT_VOL";
    case NODE_STMT_DEF:
        return "NODE_STMT_DEF";
    case NODE_STMT_USE:
        return "NODE_STMT_USE";
    case NODE_STMT_STR:
        return "NODE_STMT_STR";
    case NODE_STMT_UNI:
        return "NODE_STMT_UNI";
    case NODE_STMT_FUN:
        return "NODE_STMT_FUN";
    case NODE_STMT_IF:
        return "NODE_STMT_IF";
    case NODE_STMT_OR:
        return "NODE_STMT_OR";
    case NODE_STMT_FOR:
        return "NODE_STMT_FOR";
    case NODE_STMT_BRK:
        return "NODE_STMT_BRK";
    case NODE_STMT_CNT:
        return "NODE_STMT_CNT";
    case NODE_STMT_RET:
        return "NODE_STMT_RET";
    case NODE_TYPE_ARR:
        return "NODE_TYPE_ARR";
    case NODE_TYPE_PTR:
        return "NODE_TYPE_REF";
    case NODE_TYPE_FUN:
        return "NODE_TYPE_FUN";
    case NODE_TYPE_STR:
        return "NODE_TYPE_STR";
    case NODE_TYPE_UNI:
        return "NODE_TYPE_UNI";
    case NODE_FIELD:
        return "NODE_FIELD";
    }
}

bool node_init(Node *node, NodeKind kind, Token token)
{
    if (!node)
    {
        return false;
    }

    node->kind = kind;
    node->token = token;

    return true;
}

void node_free(Node *node)
{
    if (!node)
    {
        return;
    }

    switch (node->kind)
    {
    case NODE_ERROR:
        break;
    case NODE_MODULE:
        node_list_free(node->module.stmts);
        node->module.stmts = NULL;
        break;
    case NODE_POST_MEMBER:
        node_free(node->post_member.target);
        node->post_member.target = NULL;
        node_free(node->post_member.member);
        node->post_member.member = NULL;
        break;
    case NODE_POST_CALL:
        node_free(node->post_call.target);
        node->post_call.target = NULL;
        node_list_free(node->post_call.args);
        node->post_call.args = NULL;
        break;
    case NODE_POST_IDX_ARR:
        node_free(node->post_idx_arr.target);
        node->post_idx_arr.target = NULL;
        node_free(node->post_idx_arr.index);
        node->post_idx_arr.index = NULL;
        break;
    case NODE_POST_CAST:
        node_free(node->post_cast.target);
        node->post_cast.target = NULL;
        node_free(node->post_cast.type_node);
        node->post_cast.type_node = NULL;
        break;
    case NODE_EXPR_UNARY:
        node_free(node->expr_unary.right);
        node->expr_unary.right = NULL;
        break;
    case NODE_EXPR_BINARY:
        node_free(node->expr_binary.left);
        node->expr_binary.left = NULL;
        node_free(node->expr_binary.right);
        node->expr_binary.right = NULL;
        break;
    case NODE_STMT_EXPR:
        node_free(node->stmt_expr.expr);
        node->stmt_expr.expr = NULL;
        break;
    case NODE_STMT_BLOCK:
        node_list_free(node->stmt_block.stmts);
        node->stmt_block.stmts = NULL;
        break;
    case NODE_STMT_VAL:
        node_free(node->stmt_val.ident);
        node->stmt_val.ident = NULL;
        node_free(node->stmt_val.type_node);
        node->stmt_val.type_node = NULL;
        node_free(node->stmt_val.expr);
        node->stmt_val.expr = NULL;
        break;
    case NODE_STMT_VAR:
        node_free(node->stmt_var.ident);
        node->stmt_var.ident = NULL;
        node_free(node->stmt_var.type_node);
        node->stmt_var.type_node = NULL;
        node_free(node->stmt_var.expr);
        node->stmt_var.expr = NULL;
        break;
    case NODE_STMT_VOL:
        node_free(node->stmt_vol.ident);
        node->stmt_vol.ident = NULL;
        node_free(node->stmt_vol.type_node);
        node->stmt_vol.type_node = NULL;
        node_free(node->stmt_vol.expr);
        node->stmt_vol.expr = NULL;
        break;
    case NODE_STMT_DEF:
        node_free(node->stmt_def.ident);
        node->stmt_def.ident = NULL;
        node_free(node->stmt_def.type_node);
        node->stmt_def.type_node = NULL;
        break;
    case NODE_STMT_USE:
        node_free(node->stmt_use.alias);
        node->stmt_use.alias = NULL;
        node_free(node->stmt_use.path);
        node->stmt_use.path = NULL;
        break;
    case NODE_STMT_STR:
        node_free(node->stmt_str.ident);
        node->stmt_str.ident = NULL;
        node_list_free(node->stmt_str.fields);
        node->stmt_str.fields = NULL;
        break;
    case NODE_STMT_UNI:
        node_free(node->stmt_uni.ident);
        node->stmt_uni.ident = NULL;
        node_list_free(node->stmt_uni.fields);
        node->stmt_uni.fields = NULL;
        break;
    case NODE_STMT_FUN:
        node_free(node->stmt_fun.type_node);
        node->stmt_fun.type_node = NULL;
        break;
    case NODE_STMT_IF:
        node_free(node->stmt_if.cond);
        node->stmt_if.cond = NULL;
        node_free(node->stmt_if.body);
        node->stmt_if.body = NULL;
        break;
    case NODE_STMT_OR:
        node_free(node->stmt_or.cond);
        node->stmt_or.cond = NULL;
        node_free(node->stmt_or.body);
        node->stmt_or.body = NULL;
        break;
    case NODE_STMT_FOR:
        node_free(node->stmt_for.cond);
        node->stmt_for.cond = NULL;
        node_free(node->stmt_for.body);
        node->stmt_for.body = NULL;
        break;
    case NODE_STMT_RET:
        node_free(node->stmt_ret.expr);
        node->stmt_ret.expr = NULL;
        break;
    case NODE_TYPE_ARR:
        node_free(node->type_arr.size);
        node->type_arr.size = NULL;
        node_free(node->type_arr.type_node);
        node->type_arr.type_node = NULL;
        break;
    case NODE_TYPE_PTR:
        node_free(node->type_ptr.type_node);
        node->type_ptr.type_node = NULL;
        break;
    case NODE_TYPE_FUN:
        node_list_free(node->type_fun.params);
        node->type_fun.params = NULL;
        node_free(node->type_fun.type_node);
        node->type_fun.type_node = NULL;
        break;
    case NODE_TYPE_STR:
        node_list_free(node->type_str.fields);
        node->type_str.fields = NULL;
        break;
    case NODE_TYPE_UNI:
        node_list_free(node->type_uni.fields);
        node->type_uni.fields = NULL;
        break;
    case NODE_FIELD:
        node_free(node->field.type_node);
        node->field.type_node = NULL;
        break;
    default:
        break;
    }

    free(node);
}

bool node_list_init(NodeList *list)
{
    if (!list)
    {
        return false;
    }

    list->len = 0;
    list->cap = 1;
    list->nodes = calloc(sizeof(Node *), list->cap);
    if (!list->nodes)
    {
        free(list);
        return false;
    }

    return true;
}

void node_list_free(NodeList *list)
{
    if (!list)
    {
        return;
    }

    for (int i = 0; i < list->len; i++)
    {
        node_free(list->nodes[i]);
        list->nodes[i] = NULL;
    }

    free(list->nodes);
    list->nodes = NULL;

    free(list);
}

Node *node_list_add(NodeList *list, Node *node)
{
    if (list->len >= list->cap)
    {
        list->cap *= 2;
        Node **nodes = realloc(list->nodes, sizeof(Node *) * list->cap);
        if (!nodes)
        {
            return NULL;
        }

        list->nodes = nodes;
    }

    list->nodes[list->len] = node;
    list->len++;

    return node;
}

Node *node_list_get(NodeList *list, int index)
{
    if (index < 0 || index >= list->len)
    {
        return NULL;
    }

    return list->nodes[index];
}
