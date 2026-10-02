# mach.lang.fe.ast

## rec Ast

```mach
pub rec Ast;
```

## fun init

```mach
pub fun init(a: *std_allocator.Allocator, file_id: lang_source.FileId) Ast;
```

## fun dnit

```mach
pub fun dnit(a: *Ast);
```

## fun add_module

```mach
pub fun add_module(a: *Ast, m: ast_module.Module) res[ast_id.ModuleNodeId, fail.Fail];
```

## fun get_module

```mach
pub fun get_module(a: *Ast, id_: ast_id.ModuleNodeId) opt[*ast_module.Module];
```

## fun add_expr

```mach
pub fun add_expr(a: *Ast, e: ast_expr.Expr) res[ast_id.ExprId, fail.Fail];
```

## fun get_expr

```mach
pub fun get_expr(a: *Ast, id_: ast_id.ExprId) opt[*ast_expr.Expr];
```

## fun expr_place_kind

```mach
pub fun expr_place_kind(e: *ast_expr.Expr) ast_expr.ExprKind;
```

## fun add_stmt

```mach
pub fun add_stmt(a: *Ast, s: ast_stmt.Stmt) res[ast_id.StmtId, fail.Fail];
```

## fun get_stmt

```mach
pub fun get_stmt(a: *Ast, id_: ast_id.StmtId) opt[*ast_stmt.Stmt];
```

## fun add_decl

```mach
pub fun add_decl(a: *Ast, d: ast_decl.Decl) res[ast_id.DeclId, fail.Fail];
```

## fun get_decl

```mach
pub fun get_decl(a: *Ast, id_: ast_id.DeclId) opt[*ast_decl.Decl];
```

## fun decl_name_span

```mach
pub fun decl_name_span(a: *Ast, id_: ast_id.DeclId) token.Span;
```

## fun add_type

```mach
pub fun add_type(a: *Ast, t: ast_type.Type) res[ast_id.TypeId, fail.Fail];
```

## fun get_type

```mach
pub fun get_type(a: *Ast, id_: ast_id.TypeId) opt[*ast_type.Type];
```

## fun add_typed_name

```mach
pub fun add_typed_name(a: *Ast, tn: ast_decl.TypedName) res[u32, fail.Fail];
```

## fun add_tag_case

```mach
pub fun add_tag_case(a: *Ast, member: ast_decl.TagCase) res[u32, fail.Fail];
```

## fun add_field_init

```mach
pub fun add_field_init(a: *Ast, fi: ast_expr.FieldInit) res[u32, fail.Fail];
```

## fun add_generic_name

```mach
pub fun add_generic_name(a: *Ast, name: token.Span) res[u32, fail.Fail];
```

## fun add_comptime_branch

```mach
pub fun add_comptime_branch(a: *Ast, br: ast_decl.ComptimeBranch) res[u32, fail.Fail];
```

## fun add_decorator

```mach
pub fun add_decorator(a: *Ast, d: ast_decl.Decorator) res[u32, fail.Fail];
```

## fun add_decl_id

```mach
pub fun add_decl_id(a: *Ast, id_: ast_id.DeclId) res[u32, fail.Fail];
```

## fun add_stmt_id

```mach
pub fun add_stmt_id(a: *Ast, id_: ast_id.StmtId) res[u32, fail.Fail];
```

## fun add_expr_id

```mach
pub fun add_expr_id(a: *Ast, id_: ast_id.ExprId) res[u32, fail.Fail];
```

## fun add_type_id

```mach
pub fun add_type_id(a: *Ast, id_: ast_id.TypeId) res[u32, fail.Fail];
```

## rec AstMark

```mach
pub rec AstMark;
```

## fun mark

```mach
pub fun mark(a: *Ast) AstMark;
```

## fun rollback

```mach
pub fun rollback(a: *Ast, m: AstMark) bool;
```

## fun span_contains

```mach
pub fun span_contains(span: token.Span, offset: usize) bool;
```

## fun offset_to_expr

```mach
pub fun offset_to_expr(a: *Ast, offset: usize) ast_id.ExprId;
```

the tightest expression enclosing a byte offset
a linear scan of every expression; among equal spans the later one wins;
an empty span never matches

a: the tree
offset: byte offset into the tree's source text
ret: the ExprId with the smallest span containing offset, or EXPR_NIL when none does

## fun offset_to_decl

```mach
pub fun offset_to_decl(a: *Ast, offset: usize) ast_id.DeclId;
```

the tightest declaration enclosing a byte offset
a linear scan of every declaration; among equal spans the later one wins;
an empty span never matches

a: the tree
offset: byte offset into the tree's source text
ret: the DeclId with the smallest span containing offset, or DECL_NIL when none does

## fun offset_to_type

```mach
pub fun offset_to_type(a: *Ast, offset: usize) ast_id.TypeId;
```

the tightest type expression enclosing a byte offset
a linear scan of every type node; among equal spans the later one wins;
an empty span never matches

a: the tree
offset: byte offset into the tree's source text
ret: the TypeId with the smallest span containing offset, or TYPE_NIL when none does

