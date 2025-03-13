#ifndef AST_H
#define AST_H

#include "operator.h"
#include "token.h"
#include <llvm-c-19/llvm-c/Types.h>

typedef enum NodeKind NodeKind;
typedef struct Node Node;
typedef struct NodeList NodeList;
typedef struct NodeTable NodeTable;

enum NodeKind
{
    NODE_UNKNOWN,

    NODE_ERROR,
    NODE_MODULE,

    NODE_COMMENT,

    NODE_IDENTIFIER,

    NODE_LIT_INT,
    NODE_LIT_FLOAT,
    NODE_LIT_CHAR,
    NODE_LIT_STRING,

    NODE_POST_MEMBER,
    NODE_POST_CALL,
    NODE_POST_IDX_ARR,
    NODE_POST_CAST,
    NODE_EXPR_UNARY,
    NODE_EXPR_BINARY,

    NODE_STMT_EXPR,
    NODE_STMT_BLOCK,
    NODE_STMT_VAL,
    NODE_STMT_VAR,
    NODE_STMT_VOL,
    NODE_STMT_DEF,
    NODE_STMT_USE,
    NODE_STMT_STR,
    NODE_STMT_UNI,
    NODE_STMT_FUN,
    NODE_STMT_IF,
    NODE_STMT_OR,
    NODE_STMT_FOR,
    NODE_STMT_BRK,
    NODE_STMT_CNT,
    NODE_STMT_RET,

    NODE_TYPE_ARR,
    NODE_TYPE_PTR,
    NODE_TYPE_FUN,
    NODE_TYPE_STR,
    NODE_TYPE_UNI,
    NODE_FIELD,
};

struct NodeList
{
    int len;
    int cap;
    Node **nodes;
};

struct Node
{
    NodeKind kind;
    Token token;

    union
    {
        struct
        {
            const char *message;
        } error;

        struct
        {
            NodeList *stmts;
        } module;

        struct
        {
            int64_t value;
        } lit_int;

        struct
        {
            double value;
        } lit_float;

        struct
        {
            char value;
        } lit_char;

        struct
        {
            char *value;
            int len;
        } lit_string;

        struct
        {
            Node *target;
            Node *member;
        } post_member;

        struct
        {
            Node *target;
            NodeList *args;
        } post_call;

        struct
        {
            Node *target;
            Node *index;
        } post_idx_arr;

        struct
        {
            Node *target;
            Node *type_node;
        } post_cast;

        struct
        {
            Operator op;
            Node *expr;
        } expr_unary;

        struct
        {
            Operator op;
            Node *left;
            Node *right;
        } expr_binary;

        struct
        {
            Node *expr;
        } stmt_expr;

        struct
        {
            NodeList *stmts;
        } stmt_block;

        struct
        {
            Node *ident;
            Node *type_node;
            Node *expr;
        } stmt_val;

        struct
        {
            Node *ident;
            Node *type_node;
            Node *expr;
        } stmt_var;

        struct
        {
            Node *ident;
            Node *type_node;
            Node *expr;
        } stmt_vol;

        struct
        {
            Node *ident;
            Node *type_node;
        } stmt_def;

        struct
        {
            Node *alias;
            Node *path;
        } stmt_use;

        struct
        {
            Node *ident;
            NodeList *fields;
        } stmt_str;

        struct
        {
            Node *ident;
            NodeList *fields;
        } stmt_uni;

        struct
        {
            Node *ident;
            NodeList *params;
            Node *type_node;
            Node *body;
        } stmt_fun;

        struct
        {
            Node *parent;
            Node *cond;
            Node *body;
            Node *branch;
        } stmt_if;

        struct
        {
            Node *parent;
            Node *cond;
            Node *body;
            Node *branch;
        } stmt_or;

        struct
        {
            Node *cond;
            Node *body;
        } stmt_for;

        struct
        {
            Node *expr;
        } stmt_ret;

        struct
        {
            Node *size;
            Node *type_node;
        } type_arr;

        struct
        {
            Node *type_node;
        } type_ptr;

        struct
        {
            NodeList *params;
            Node *type_node;
        } type_fun;

        struct
        {
            NodeList *fields;
        } type_str;

        struct
        {
            NodeList *fields;
        } type_uni;

        struct
        {
            Node *ident;
            Node *type_node;
        } field;
    };
};

struct NodeTable {
    Node **keys;
    void **values;
    int len;
    int cap;
};

const char *node_kind_to_string(NodeKind kind);

bool node_init(Node *node, NodeKind kind, Token token);
void node_free(Node *node);

bool node_list_init(NodeList *list);
void node_list_free(NodeList *list);
Node *node_list_add(NodeList *list, Node *node);
Node *node_list_get(NodeList *list, int index);

bool node_table_init(NodeTable *table);
void node_table_free(NodeTable *table);
void *node_table_get(NodeTable *table, Node *key);
bool node_table_set(NodeTable *table, Node *key, void *value);
bool node_table_del(NodeTable *table, Node *key);

#endif // AST_H
