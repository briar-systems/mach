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
| [`init`](#init) | an empty tree for one file, every table allocated from `a` |
| [`dnit`](#dnit) | release a tree; a nil tree holds nothing |
| [`copy`](#copy) | an owned copy of `src` from `alloc`, every node at the id it has in `src` |
| [`add_module`](#add_module) | append a module node |
| [`get_module`](#get_module) | the module node an id names |
| [`add_expr`](#add_expr) | append an expression node |
| [`get_expr`](#get_expr) | the expression node an id names |
| [`add_stmt`](#add_stmt) | append a statement node |
| [`get_stmt`](#get_stmt) | the statement node an id names |
| [`add_decl`](#add_decl) | append a declaration node |
| [`get_decl`](#get_decl) | the declaration node an id names |
| [`decl_name_span`](#decl_name_span) | the span of the name a declaration binds |
| [`add_type`](#add_type) | append a type-syntax node |
| [`get_type`](#get_type) | the type-syntax node an id names |
| [`add_typed_name`](#add_typed_name) | append a typed name to the side list of parameters and fields |
| [`add_field_init`](#add_field_init) | reached only by mach-lsp |
| [`add_generic_name`](#add_generic_name) | append a generic parameter name to its side list |
| [`names_remap`](#names_remap) | every name the tree holds an id of, renamed through `remap`: the ids a parse made in a name table of its own, as the table it was merged into names them |
| [`add_path_name`](#add_path_name) | the id of one segment of a named type's path, after the segments before it |
| [`add_decorator`](#add_decorator) | append a decorator to its side list |
| [`literal_bytes_reserve`](#literal_bytes_reserve) | room for `n` more bytes at the end of the literal byte table, which literal_bytes_take then claims |
| [`literal_bytes_take`](#literal_bytes_take) | claims the first `n` reserved bytes as one literal's value |
| [`lit_str_value`](#lit_str_value) | the decoded bytes a string literal names |
| [`mark`](#mark) | the length of every table, for a later rollback |
| [`rollback`](#rollback) | truncate every table back to the lengths of a mark nothing is truncated when any table is already shorter than its mark |
| [`same_shape`](#same_shape) | whether two trees have the same shape: the same number of every node, and the same expression, statement, declaration and type kinds, and declaration flags, in the same order |
| [`offset_to_expr`](#offset_to_expr) | the tightest expression enclosing a byte offset a linear scan of every expression; among equal spans the later one wins; an empty span never matches |
| [`offset_to_decl`](#offset_to_decl) | the tightest declaration enclosing a byte offset a linear scan of every declaration; among equal spans the later one wins; an empty span never matches |
| [`offset_to_type`](#offset_to_type) | the tightest type expression enclosing a byte offset a linear scan of every type node; among equal spans the later one wins; an empty span never matches |

| Functions | |
| :--- | :--- |
| [`table_push`](#table_push) | appends `value` to an AST table and answers the index it takes |
| [`expr_place_kind`](#expr_place_kind) | the kind of an expression when it can stand as a place an identifier, member, projection, index, call or dereference is a place |
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

an empty tree for one file, every table allocated from `a`

| Parameter | |
| :--- | :--- |
| `a` | allocator every table grows from, held by the tree until dnit |
| `file_id` | the file the tree is parsed from |
| **returns** | a tree with no nodes and a nil root module |

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

append a module node

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `m` | the module node |
| **returns** | the new node's id, or the failure when the table cannot grow or its id domain is spent |

### get_module

```mach
pub fun get_module(a: *Ast, id_: mach.front.ast.id.ModuleNodeId) opt[*mach.front.ast.module.Module]
```

the module node an id names

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `id_` | the module node id |
| **returns** | a pointer into the module table, valid until the table next grows, or none when the id is out of range |

### add_expr

```mach
pub fun add_expr(a: *Ast, e: mach.front.ast.expr.Expr) res[mach.front.ast.id.ExprId, mach.base.fail.Fail]
```

append an expression node

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `e` | the expression node |
| **returns** | the new node's id, or the failure when the table cannot grow or its id domain is spent |

### get_expr

```mach
pub fun get_expr(a: *Ast, id_: mach.front.ast.id.ExprId) opt[*mach.front.ast.expr.Expr]
```

the expression node an id names

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `id_` | the expression id |
| **returns** | a pointer into the expression table, valid until the table next grows, or none when the id is out of range |

### add_stmt

```mach
pub fun add_stmt(a: *Ast, s: mach.front.ast.stmt.Stmt) res[mach.front.ast.id.StmtId, mach.base.fail.Fail]
```

append a statement node

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `s` | the statement node |
| **returns** | the new node's id, or the failure when the table cannot grow or its id domain is spent |

### get_stmt

```mach
pub fun get_stmt(a: *Ast, id_: mach.front.ast.id.StmtId) opt[*mach.front.ast.stmt.Stmt]
```

the statement node an id names

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `id_` | the statement id |
| **returns** | a pointer into the statement table, valid until the table next grows, or none when the id is out of range |

### add_decl

```mach
pub fun add_decl(a: *Ast, d: mach.front.ast.decl.Decl) res[mach.front.ast.id.DeclId, mach.base.fail.Fail]
```

append a declaration node

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `d` | the declaration node |
| **returns** | the new node's id, or the failure when the table cannot grow or its id domain is spent |

### get_decl

```mach
pub fun get_decl(a: *Ast, id_: mach.front.ast.id.DeclId) opt[*mach.front.ast.decl.Decl]
```

the declaration node an id names

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `id_` | the declaration id |
| **returns** | a pointer into the declaration table, valid until the table next grows, or none when the id is out of range |

### decl_name_span

```mach
pub fun decl_name_span(a: *Ast, id_: mach.front.ast.id.DeclId) mach.base.source.Span
```

the span of the name a declaration binds

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `id_` | the declaration id |
| **returns** | the written alias of a use or fwd, empty when it has none, the name of any other named declaration, or an empty span for a test, a comptime declaration, an error node or an id out of range |

### add_type

```mach
pub fun add_type(a: *Ast, t: mach.front.ast.type.Type) res[mach.front.ast.id.TypeId, mach.base.fail.Fail]
```

append a type-syntax node

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `t` | the type-syntax node |
| **returns** | the new node's id, or the failure when the table cannot grow or its id domain is spent |

### get_type

```mach
pub fun get_type(a: *Ast, id_: mach.front.ast.id.TypeId) opt[*mach.front.ast.type.Type]
```

the type-syntax node an id names

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `id_` | the type-syntax id |
| **returns** | a pointer into the type table, valid until the table next grows, or none when the id is out of range |

### add_typed_name

```mach
pub fun add_typed_name(a: *Ast, tn: mach.front.ast.decl.TypedName) res[u32, mach.base.fail.Fail]
```

append a typed name to the side list of parameters and fields

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `tn` | the typed name |
| **returns** | its index in the typed name table, or the failure when the table cannot grow or its id domain is spent |

### add_field_init

```mach
pub fun add_field_init(a: *Ast, fi: mach.front.ast.expr.FieldInit) res[u32, mach.base.fail.Fail]
```

reached only by mach-lsp

### add_generic_name

```mach
pub fun add_generic_name(a: *Ast, name: mach.base.source.Span) res[u32, mach.base.fail.Fail]
```

append a generic parameter name to its side list

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `name` | the span of the parameter name |
| **returns** | its index in the generic name table, or the failure when the table cannot grow or its id domain is spent |

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

append a decorator to its side list

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `d` | the decorator |
| **returns** | its index in the decorator table, or the failure when the table cannot grow or its id domain is spent |

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

the length of every table, for a later rollback

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| **returns** | the mark |

### rollback

```mach
pub fun rollback(a: *Ast, m: AstMark) bool
```

truncate every table back to the lengths of a mark
nothing is truncated when any table is already shorter than its mark

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `m` | a mark taken from this tree |
| **returns** | true when the tree was rolled back, false when the mark is ahead of the tree |

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

the kind of an expression when it can stand as a place
an identifier, member, projection, index, call or dereference is a place

| Parameter | |
| :--- | :--- |
| `e` | the expression |
| **returns** | its kind when it is a place, EXPR_KIND_ERROR otherwise |

### span_contains

```mach
pub fun span_contains(span: mach.base.source.Span, offset: usize) bool
```

reached only by mach-lsp

