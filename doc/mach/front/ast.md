[mach](../README.md) / [front](README.md) / **ast**

# mach.front.ast

the syntax tree of one parsed file

each node kind and each side list lives in its own table, and a node is named by its index
in that table (`mach.front.ast.id`). a tree can be copied, marked and rolled back past a
failed parse, compared by shape, and searched for the node that encloses a byte offset.

Source: [`src/front/ast.mach`](../../../src/front/ast.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.ast.decl`](ast/decl.md), [`mach.front.ast.expr`](ast/expr.md), [`mach.front.ast.id`](ast/id.md), [`mach.front.ast.module`](ast/module.md), [`mach.front.ast.stmt`](ast/stmt.md), [`mach.front.ast.type`](ast/type.md).

| Type | | |
| :--- | :--- | :--- |
| [`Ast`](#ast) | `rec` | one parsed file: each node kind and each side list lives in its own table, and a node is named by its index in that table |
| [`AstMark`](#astmark) | `rec` | the length of every table, to roll a failed parse back to |

| On `Ast` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release a tree; a nil tree holds nothing |
| [`copy`](#copy) | an owned copy of `src` from `alloc`, every node at the id it has in `src` |
| [`add_module`](#add_module), [`get_module`](#get_module), [`add_expr`](#add_expr), [`get_expr`](#get_expr), [`add_stmt`](#add_stmt), [`get_stmt`](#get_stmt), [`add_decl`](#add_decl), [`get_decl`](#get_decl), [`decl_name_span`](#decl_name_span), [`add_type`](#add_type), [`get_type`](#get_type), [`add_typed_name`](#add_typed_name) | |
| [`add_field_init`](#add_field_init) | reached only by mach-lsp |
| [`add_generic_name`](#add_generic_name) | |
| [`names_remap`](#names_remap) | every name the tree holds an id of, renamed through `remap`: the ids a parse made in a name table of its own, as the table it was merged into names them |
| [`add_path_name`](#add_path_name) | the id of one segment of a named type's path, after the segments before it |
| [`add_decorator`](#add_decorator) | |
| [`literal_bytes_reserve`](#literal_bytes_reserve) | room for `n` more bytes at the end of the literal byte table, which literal_bytes_take then claims |
| [`literal_bytes_take`](#literal_bytes_take) | claims the first `n` reserved bytes as one literal's value |
| [`lit_str_value`](#lit_str_value) | the decoded bytes a string literal names |
| [`mark`](#mark), [`rollback`](#rollback) | |
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
    a:                 *std.allocator.Allocator;
    file_id:           mach.base.source.FileId;
    incarnation:       u64;
    root_module:       mach.front.ast.id.ModuleNodeId;
    modules:           Vector[mach.front.ast.module.Module];
    exprs:             Vector[mach.front.ast.expr.Expr];
    stmts:             Vector[mach.front.ast.stmt.Stmt];
    decls:             Vector[mach.front.ast.decl.Decl];
    types:             Vector[mach.front.ast.type.Type];
    typed_names:       Vector[mach.front.ast.decl.TypedName];
    tag_cases:         Vector[mach.front.ast.decl.TagCase];
    field_inits:       Vector[mach.front.ast.expr.FieldInit];
    generic_names:     Vector[mach.base.source.Span];
    path_names:        Vector[mach.base.intern.StrId];
    comptime_branches: Vector[mach.front.ast.decl.ComptimeBranch];
    decorators:        Vector[mach.front.ast.decl.Decorator];
    decl_ids:          Vector[mach.front.ast.id.DeclId];
    stmt_ids:          Vector[mach.front.ast.id.StmtId];
    expr_ids:          Vector[mach.front.ast.id.ExprId];
    type_ids:          Vector[mach.front.ast.id.TypeId];
    literal_bytes:     Vector[u8];
}
```

one parsed file: each node kind and each side list lives in its own table, and a
node is named by its index in that table

### init

```mach
pub fun init(a: *std.allocator.Allocator, file_id: mach.base.source.FileId) Ast
```

### dnit

```mach
pub fun dnit(a: *Ast)
```

release a tree; a nil tree holds nothing

### copy

```mach
pub fun copy(src: *Ast, alloc: *std.allocator.Allocator) res[Ast, mach.base.fail.Fail]
```

an owned copy of `src` from `alloc`, every node at the id it has in `src`

### add_module

```mach
pub fun add_module(a: *Ast, m: mach.front.ast.module.Module) res[mach.front.ast.id.ModuleNodeId, mach.base.fail.Fail]
```

### get_module

```mach
pub fun get_module(a: *Ast, id_: mach.front.ast.id.ModuleNodeId) opt[*mach.front.ast.module.Module]
```

### add_expr

```mach
pub fun add_expr(a: *Ast, e: mach.front.ast.expr.Expr) res[mach.front.ast.id.ExprId, mach.base.fail.Fail]
```

### get_expr

```mach
pub fun get_expr(a: *Ast, id_: mach.front.ast.id.ExprId) opt[*mach.front.ast.expr.Expr]
```

### add_stmt

```mach
pub fun add_stmt(a: *Ast, s: mach.front.ast.stmt.Stmt) res[mach.front.ast.id.StmtId, mach.base.fail.Fail]
```

### get_stmt

```mach
pub fun get_stmt(a: *Ast, id_: mach.front.ast.id.StmtId) opt[*mach.front.ast.stmt.Stmt]
```

### add_decl

```mach
pub fun add_decl(a: *Ast, d: mach.front.ast.decl.Decl) res[mach.front.ast.id.DeclId, mach.base.fail.Fail]
```

### get_decl

```mach
pub fun get_decl(a: *Ast, id_: mach.front.ast.id.DeclId) opt[*mach.front.ast.decl.Decl]
```

### decl_name_span

```mach
pub fun decl_name_span(a: *Ast, id_: mach.front.ast.id.DeclId) mach.base.source.Span
```

### add_type

```mach
pub fun add_type(a: *Ast, t: mach.front.ast.type.Type) res[mach.front.ast.id.TypeId, mach.base.fail.Fail]
```

### get_type

```mach
pub fun get_type(a: *Ast, id_: mach.front.ast.id.TypeId) opt[*mach.front.ast.type.Type]
```

### add_typed_name

```mach
pub fun add_typed_name(a: *Ast, tn: mach.front.ast.decl.TypedName) res[u32, mach.base.fail.Fail]
```

### add_field_init

```mach
pub fun add_field_init(a: *Ast, fi: mach.front.ast.expr.FieldInit) res[u32, mach.base.fail.Fail]
```

reached only by mach-lsp

### add_generic_name

```mach
pub fun add_generic_name(a: *Ast, name: mach.base.source.Span) res[u32, mach.base.fail.Fail]
```

### names_remap

```mach
pub fun names_remap(a: *Ast, remap: *mach.base.intern.Remap)
```

every name the tree holds an id of, renamed through `remap`: the ids a parse made in a name table
of its own, as the table it was merged into names them. a parse interns only identifiers,
member and field-initializer names, the case a member selects and path segments

### add_path_name

```mach
pub fun add_path_name(a: *Ast, name: mach.base.intern.StrId) res[u32, mach.base.fail.Fail]
```

the id of one segment of a named type's path, after the segments before it

### add_decorator

```mach
pub fun add_decorator(a: *Ast, d: mach.front.ast.decl.Decorator) res[u32, mach.base.fail.Fail]
```

### literal_bytes_reserve

```mach
pub fun literal_bytes_reserve(a: *Ast, n: usize) res[*u8, mach.base.fail.Fail]
```

room for `n` more bytes at the end of the literal byte table, which literal_bytes_take
then claims

### literal_bytes_take

```mach
pub fun literal_bytes_take(a: *Ast, n: usize) mach.front.ast.expr.ByteRange
```

claims the first `n` reserved bytes as one literal's value

### lit_str_value

```mach
pub fun lit_str_value(a: *Ast, e: *mach.front.ast.expr.Expr) View
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
pub fun offset_to_expr(a: *Ast, offset: usize) mach.front.ast.id.ExprId
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
pub fun offset_to_decl(a: *Ast, offset: usize) mach.front.ast.id.DeclId
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
pub fun offset_to_type(a: *Ast, offset: usize) mach.front.ast.id.TypeId
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
pub fun table_push[T](v: *Vector[T], value: T) res[u32, mach.base.fail.Fail]
```

appends `value` to an AST table and answers the index it takes

### expr_place_kind

```mach
pub fun expr_place_kind(e: *mach.front.ast.expr.Expr) mach.front.ast.expr.ExprKind
```

### span_contains

```mach
pub fun span_contains(span: mach.base.source.Span, offset: usize) bool
```

reached only by mach-lsp

