#ifndef AST_H
#define AST_H

#include "token.h"
#include <stdbool.h>

// forward statements
typedef struct Type   Type;
typedef struct Symbol Symbol;

typedef enum AstKind
{
    AST_PROGRAM,
    AST_MODULE,

    // statements
    AST_STMT_USE,
    AST_STMT_EXT,
    AST_STMT_DEF,
    AST_STMT_VAL,
    AST_STMT_VAR,
    AST_STMT_FUN,
    AST_STMT_FIELD,
    AST_STMT_PARAM,
    AST_STMT_STR,
    AST_STMT_UNI,
    AST_STMT_IF,
    AST_STMT_OR,
    AST_STMT_FOR,
    AST_STMT_BRK,
    AST_STMT_CNT,
    AST_STMT_RET,
    AST_STMT_BLOCK,
    AST_STMT_EXPR,
    AST_STMT_ASM,

    // expressions
    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_CALL,
    AST_EXPR_INDEX,
    AST_EXPR_FIELD,
    AST_EXPR_CAST,
    AST_EXPR_IDENT,
    AST_EXPR_LIT,
    AST_EXPR_NULL,
    AST_EXPR_ARRAY,
    AST_EXPR_VARARGS,
    AST_EXPR_STRUCT,

    // types
    AST_TYPE_NAME,
    AST_TYPE_PTR,
    AST_TYPE_ARRAY,
    AST_TYPE_PARAM,
    AST_TYPE_FUN,
    AST_TYPE_STR,
    AST_TYPE_UNI,
} AstKind;

typedef struct AstNode AstNode;
typedef struct AstList AstList;

// generic list for child nodes
struct AstList
{
    AstNode **items;
    int       count;
    int       capacity;
};

// base AST node
struct AstNode
{
    AstKind kind;
    Token  *token;  // source token for error reporting
    Type   *type;   // resolved type (filled during semantic analysis)
    Symbol *symbol; // symbol table entry (if applicable)

    union
    {
        // program root
        struct
        {
            AstList *stmts;
        } program;

        // module statement
        struct
        {
            char    *name;  // module name
            AstList *stmts; // statements in this module
        } module;

        // use statement
        struct
        {
            char *module_path;
            char *alias;
        } use_stmt;

        // external statement
        struct
        {
            char    *name;       // function name in Mach code
            char    *convention; // calling convention (e.g., "C")
            char    *symbol;     // target symbol name (default: same as name)
            AstNode *type;
            bool     is_public;
        } ext_stmt;

        // type definition
        struct
        {
            char    *name;
            AstNode *type;
            bool     is_public;
        } def_stmt;

        // value/variable statement
        struct
        {
            char    *name;
            AstNode *type; // explicit type or null
            AstNode *init; // initializer expression
            bool     is_val;
            bool     is_public;
            char    *mangle_name;
        } var_stmt;

        // function statement
        struct
        {
            char    *name;
            AstList *params;
            AstList *generics;    // optional generic parameters
            AstNode *return_type; // null for no return
            AstNode *body;        // null for external functions
            bool     is_variadic; // true if function has variadic arguments
            bool     is_public;
            char    *mangle_name;
            bool     is_method;
            AstNode *method_receiver; // typename before '.' for method declarations
        } fun_stmt;

        // struct statement
        struct
        {
            char    *name;
            AstList *generics;
            AstList *fields;
            bool     is_public;
        } str_stmt;

        // union statement
        struct
        {
            char    *name;
            AstList *generics;
            AstList *fields;
            bool     is_public;
        } uni_stmt;

        // field statement
        struct
        {
            char    *name;
            AstNode *type;
        } field_stmt;

        // parameter statement
        struct
        {
            char    *name;
            AstNode *type;
            bool     is_variadic; // sentinel for '...'
        } param_stmt;

        // block statement
        struct
        {
            AstList *stmts;
        } block_stmt;

        // expression statement
        struct
        {
            AstNode *expr;
        } expr_stmt;

        // inline asm statement
        struct
        {
            char *code;        // raw assembly text (single line for now)
            char *constraints; // optional LLVM asm constraints/clobbers string
        } asm_stmt;

        // return statement
        struct
        {
            AstNode *expr; // null for void return
        } ret_stmt;

        // if statement
        struct
        {
            AstNode *cond;
            AstNode *body;
            AstNode *stmt_or; // can be another conditional (or)
        } cond_stmt;

        // for loop
        struct
        {
            AstNode *cond; // null for infinite loop
            AstNode *body;
        } for_stmt;

        // binary expression
        struct
        {
            AstNode  *left;
            AstNode  *right;
            TokenKind op;
        } binary_expr;

        // unary expression
        struct
        {
            AstNode  *expr;
            TokenKind op;
        } unary_expr;

        // function call
        struct
        {
            AstNode *func;
            AstList *args;
            AstList *type_args;
            bool     is_method_call; // true when rewritten from receiver.method()
        } call_expr;

        // array indexing
        struct
        {
            AstNode *array;
            AstNode *index;
        } index_expr;

        // field access
        struct
        {
            AstNode *object;
            char    *field;
            bool     is_method; // true when referring to method symbol
        } field_expr;

        // type cast
        struct
        {
            AstNode *expr;
            AstNode *type;
        } cast_expr;

        // identifier
        struct
        {
            char *name;
        } ident_expr;

        // literal
        struct
        {
            TokenKind kind; // TOKEN_LIT_INT, etc.
            union
            {
                unsigned long long int_val;
                double             float_val;
                char               char_val;
                char              *string_val;
            };
        } lit_expr;

        // null literal (nil)
        struct
        {
            bool unused;
        } null_expr;

        // array literal
        struct
        {
            AstNode *type;             // element type
            AstList *elems;            // elements
            bool     is_slice_literal; // true when expressed as pointer+length literal
        } array_expr;

        // struct literal
        struct
        {
            AstNode *type;                 // struct type
            AstList *fields;               // field initializers
            bool     is_union_literal;     // true when literal uses 'uni' keyword without explicit type
            bool     is_anonymous_literal; // true when literal omits explicit type name
        } struct_expr;

        // type expressions
        struct
        {
            char    *name;
            AstList *generic_args;
        } type_name;

        struct
        {
            AstNode *base;
        } type_ptr;

        struct
        {
            AstNode *elem_type;
            AstNode *size; // null for unbound arrays [_]
        } type_array;

        struct
        {
            char *name;
        } type_param;

        struct
        {
            AstList *params;
            AstNode *return_type; // null for no return
            bool     is_variadic; // true if function type is variadic
        } type_fun;

        struct
        {
            char    *name; // can be null for anonymous
            AstList *fields;
        } type_str;

        struct
        {
            char    *name; // can be null for anonymous
            AstList *fields;
        } type_uni;
    };
};

// ast node operations
void ast_node_init(AstNode *node, AstKind kind);
void ast_node_dnit(AstNode *node);

// list operations
void ast_list_init(AstList *list);
void ast_list_dnit(AstList *list);
void ast_list_append(AstList *list, AstNode *node);
void ast_list_prepend(AstList *list, AstNode *node);

// cloning helpers
AstNode *ast_clone(const AstNode *node);
AstList *ast_list_clone(const AstList *list);

// pretty printing for debugging
void ast_print(AstNode *node, int indent);

const char *ast_node_kind_to_string(AstKind kind);

bool ast_emit(AstNode *node, const char *file_path);

#endif
