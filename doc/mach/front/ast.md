[mach](../README.md) / [front](README.md) / **ast**

# mach.front.ast

the syntax tree of one parsed file

each node kind and each side list lives in its own table, and a node is named by its index
in that table (`mach.front.ast.id`). a tree can be copied, marked and rolled back past a
failed parse, compared by shape, and searched for the node that encloses a byte offset.

<sub>source: [`src/front/ast.mach`](../../../src/front/ast.mach) · imported by 52 modules</sub>

<sub>`A` is `std.allocator` · `base_source` is [`mach.base.source`](../base/source.md) · `ast_id` is [`mach.front.ast.id`](ast/id.md) · `ast_module` is [`mach.front.ast.module`](ast/module.md) · `ast_expr` is [`mach.front.ast.expr`](ast/expr.md) · `ast_stmt` is [`mach.front.ast.stmt`](ast/stmt.md) · `ast_decl` is [`mach.front.ast.decl`](ast/decl.md) · `ast_type` is [`mach.front.ast.type`](ast/type.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Ast`](#ast) | `rec` | one parsed file: each node kind and each side list lives in its own table, and a node is named by its index in that table |
| [`AstMark`](#astmark) | `rec` | the length of every table, to roll a failed parse back to |

| On `Ast` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release a tree; a nil tree holds nothing |
| [`copy`](#copy) | an owned copy of `src` from `alloc`, every node at the id it has in `src` |
| [`add_module`](#add_module) · [`get_module`](#get_module) · [`add_expr`](#add_expr) · [`get_expr`](#get_expr) · [`add_stmt`](#add_stmt) · [`get_stmt`](#get_stmt) · [`add_decl`](#add_decl) · [`get_decl`](#get_decl) · [`decl_name_span`](#decl_name_span) · [`add_type`](#add_type) · [`get_type`](#get_type) · [`add_typed_name`](#add_typed_name) | |
| [`add_field_init`](#add_field_init) | reached only by mach-lsp |
| [`add_generic_name`](#add_generic_name) | |
| [`add_path_name`](#add_path_name) | the id of one segment of a named type's path, after the segments before it |
| [`add_decorator`](#add_decorator) | |
| [`literal_bytes_reserve`](#literal_bytes_reserve) | a table over storage the caller owns, for a test that builds nodes in place room for `n` more bytes at the end of the literal byte table, which literal_bytes_take then claims |
| [`literal_bytes_take`](#literal_bytes_take) | claims the first `n` reserved bytes as one literal's value |
| [`lit_str_value`](#lit_str_value) | the decoded bytes a string literal names |
| [`mark`](#mark) · [`rollback`](#rollback) | |
| [`same_shape`](#same_shape) | whether two trees have the same shape: the same number of every node, and the same expression, statement, declaration and type kinds, and declaration flags, in the same order |
| [`offset_to_expr`](#offset_to_expr) | the tightest expression enclosing a byte offset a linear scan of every expression; among equal spans the later one wins; an empty span never matches |
| [`offset_to_decl`](#offset_to_decl) | the tightest declaration enclosing a byte offset a linear scan of every declaration; among equal spans the later one wins; an empty span never matches |
| [`offset_to_type`](#offset_to_type) | the tightest type expression enclosing a byte offset a linear scan of every type node; among equal spans the later one wins; an empty span never matches |

| Functions | |
| :--- | :--- |
| [`table_push`](#table_push) | appends `value` to an AST table and answers the index it takes |
| [`expr_place_kind`](#expr_place_kind) | |
| [`span_contains`](#span_contains) | reached only by mach-lsp |

---

## Ast

```mach
pub rec Ast {
    a:                 *A.Allocator;
    file_id:           base_source.FileId;
    incarnation:       u64;
    root_module:       ast_id.ModuleNodeId;
    modules:           Vector[ast_module.Module];
    exprs:             Vector[ast_expr.Expr];
    stmts:             Vector[ast_stmt.Stmt];
    decls:             Vector[ast_decl.Decl];
    types:             Vector[ast_type.Type];
    typed_names:       Vector[ast_decl.TypedName];
    tag_cases:         Vector[ast_decl.TagCase];
    field_inits:       Vector[ast_expr.FieldInit];
    generic_names:     Vector[base_source.Span];
    path_names:        Vector[intern.StrId];
    comptime_branches: Vector[ast_decl.ComptimeBranch];
    decorators:        Vector[ast_decl.Decorator];
    decl_ids:          Vector[ast_id.DeclId];
    stmt_ids:          Vector[ast_id.StmtId];
    expr_ids:          Vector[ast_id.ExprId];
    type_ids:          Vector[ast_id.TypeId];
    literal_bytes:     Vector[u8];
}
```

one parsed file: each node kind and each side list lives in its own table, and a
node is named by its index in that table

### init

```mach
pub fun init(a: *A.Allocator, file_id: base_source.FileId) Ast
```

### dnit

```mach
pub fun dnit(a: *Ast)
```

release a tree; a nil tree holds nothing

### copy

```mach
pub fun copy(src: *Ast, alloc: *A.Allocator) res[Ast, fail.Fail]
```

an owned copy of `src` from `alloc`, every node at the id it has in `src`

### add_module

```mach
pub fun add_module(a: *Ast, m: ast_module.Module) res[ast_id.ModuleNodeId, fail.Fail]
```

### get_module

```mach
pub fun get_module(a: *Ast, id_: ast_id.ModuleNodeId) opt[*ast_module.Module]
```

### add_expr

```mach
pub fun add_expr(a: *Ast, e: ast_expr.Expr) res[ast_id.ExprId, fail.Fail]
```

### get_expr

```mach
pub fun get_expr(a: *Ast, id_: ast_id.ExprId) opt[*ast_expr.Expr]
```

### add_stmt

```mach
pub fun add_stmt(a: *Ast, s: ast_stmt.Stmt) res[ast_id.StmtId, fail.Fail]
```

### get_stmt

```mach
pub fun get_stmt(a: *Ast, id_: ast_id.StmtId) opt[*ast_stmt.Stmt]
```

### add_decl

```mach
pub fun add_decl(a: *Ast, d: ast_decl.Decl) res[ast_id.DeclId, fail.Fail]
```

### get_decl

```mach
pub fun get_decl(a: *Ast, id_: ast_id.DeclId) opt[*ast_decl.Decl]
```

### decl_name_span

```mach
pub fun decl_name_span(a: *Ast, id_: ast_id.DeclId) base_source.Span
```

### add_type

```mach
pub fun add_type(a: *Ast, t: ast_type.Type) res[ast_id.TypeId, fail.Fail]
```

### get_type

```mach
pub fun get_type(a: *Ast, id_: ast_id.TypeId) opt[*ast_type.Type]
```

### add_typed_name

```mach
pub fun add_typed_name(a: *Ast, tn: ast_decl.TypedName) res[u32, fail.Fail]
```

### add_field_init

```mach
pub fun add_field_init(a: *Ast, fi: ast_expr.FieldInit) res[u32, fail.Fail]
```

reached only by mach-lsp

### add_generic_name

```mach
pub fun add_generic_name(a: *Ast, name: base_source.Span) res[u32, fail.Fail]
```

### add_path_name

```mach
pub fun add_path_name(a: *Ast, name: intern.StrId) res[u32, fail.Fail]
```

the id of one segment of a named type's path, after the segments before it

### add_decorator

```mach
pub fun add_decorator(a: *Ast, d: ast_decl.Decorator) res[u32, fail.Fail]
```

### literal_bytes_reserve

```mach
pub fun literal_bytes_reserve(a: *Ast, n: usize) res[*u8, fail.Fail]
```

a table over storage the caller owns, for a test that builds nodes in place
room for `n` more bytes at the end of the literal byte table, which literal_bytes_take
then claims

### literal_bytes_take

```mach
pub fun literal_bytes_take(a: *Ast, n: usize) ast_expr.ByteRange
```

claims the first `n` reserved bytes as one literal's value

### lit_str_value

```mach
pub fun lit_str_value(a: *Ast, e: *ast_expr.Expr) View
```

the decoded bytes a string literal names

### mark

```mach
pub fun mark(a: *Ast) AstMark
```

### rollback

```mach
pub fun rollback(a: *Ast, m: AstMark) bool
```

### same_shape

```mach
pub fun same_shape(x: *Ast, y: *Ast) bool
```

whether two trees have the same shape: the same number of every node, and the same
expression, statement, declaration and type kinds, and declaration flags, in the same
order. spans, names and literal values are not compared

### offset_to_expr

```mach
pub fun offset_to_expr(a: *Ast, offset: usize) ast_id.ExprId
```

the tightest expression enclosing a byte offset
a linear scan of every expression; among equal spans the later one wins;
an empty span never matches

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `offset` | byte offset into the tree's source text |
| **returns** | the ExprId with the smallest span containing offset, or EXPR_NIL when none does |

### offset_to_decl

```mach
pub fun offset_to_decl(a: *Ast, offset: usize) ast_id.DeclId
```

the tightest declaration enclosing a byte offset
a linear scan of every declaration; among equal spans the later one wins;
an empty span never matches

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `offset` | byte offset into the tree's source text |
| **returns** | the DeclId with the smallest span containing offset, or DECL_NIL when none does |

### offset_to_type

```mach
pub fun offset_to_type(a: *Ast, offset: usize) ast_id.TypeId
```

the tightest type expression enclosing a byte offset
a linear scan of every type node; among equal spans the later one wins;
an empty span never matches

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `offset` | byte offset into the tree's source text |
| **returns** | the TypeId with the smallest span containing offset, or TYPE_NIL when none does |

## AstMark

```mach
pub rec AstMark {
    module_len:          usize;
    expr_len:            usize;
    stmt_len:            usize;
    decl_len:            usize;
    type_len:            usize;
    typed_name_len:      usize;
    tag_case_len:        usize;
    field_init_len:      usize;
    generic_name_len:    usize;
    path_name_len:       usize;
    comptime_branch_len: usize;
    decorator_len:       usize;
    decl_id_len:         usize;
    stmt_id_len:         usize;
    expr_id_len:         usize;
    type_id_len:         usize;
    literal_byte_len:    usize;
}
```

the length of every table, to roll a failed parse back to

## Functions

### table_push

```mach
pub fun table_push[T](v: *Vector[T], value: T) res[u32, fail.Fail]
```

appends `value` to an AST table and answers the index it takes

### expr_place_kind

```mach
pub fun expr_place_kind(e: *ast_expr.Expr) ast_expr.ExprKind
```

### span_contains

```mach
pub fun span_contains(span: base_source.Span, offset: usize) bool
```

reached only by mach-lsp

