# mach.lang.fe.parser.state

The [`Parser`](#parser) record plus the shared helpers every other
parser file builds on: cursor (`current`, `peek`, `advance`), predicate
(`at`, `at_kw`, `eat`, `eat_kw`), error reporting (`error_at`,
`error_at_current`, `expect`, `sync_to`), span arithmetic (`span_of`),
and the `push_*` wrappers that log allocation failures and return
`*_NIL` rather than aborting. All sibling files depend on this one;
nothing here depends back on them.

## Types

### `Parser`

```mach
pub rec Parser {
    tokens: *lexer.TokenStream;
    pos:    usize;
    ast:    *ast.Ast;
    diags:  *diag.DiagList;
    panic:  bool;
}
```

Parser cursor and output sinks; lives for the duration of one parse.

| Field  | Type                                                  | Description                                                |
|--------|-------------------------------------------------------|------------------------------------------------------------|
| tokens | [`*lexer.TokenStream`](../lexer.md#tokenstream)       | Stream produced by the lexer.                              |
| pos    | `usize`                                               | Index into `tokens.tokens` of the current token.           |
| ast    | [`*ast.Ast`](../ast.md#ast)                           | Output AST; nodes are appended here as parsing progresses. |
| diags  | [`*diag.DiagList`](../../diagnostic.md#diaglist)      | Diagnostic sink for parse errors and recovery messages.    |
| panic  | `bool`                                                | `true` while skipping tokens during error recovery; cleared at the next synchronization point. |

## Functions

### `init`

```mach
pub fun init(tokens: *lexer.TokenStream, out: *ast.Ast, diags: *diag.DiagList) Parser
```

Initialises a [`Parser`](#parser) over the given token stream, filling
output into `out` and `diags`. Infallible.

| Param  | Type                                                  | Description                                  |
|--------|-------------------------------------------------------|----------------------------------------------|
| tokens | [`*lexer.TokenStream`](../lexer.md#tokenstream)       | Lexer output to consume.                     |
| out    | [`*ast.Ast`](../ast.md#ast)                           | AST to append parsed nodes into.             |
| diags  | [`*diag.DiagList`](../../diagnostic.md#diaglist)      | Diagnostic sink.                             |

Returns a fresh [`Parser`](#parser) positioned at the first token.

### Cursor

#### `current`

```mach
pub fun current(p: *Parser) token.Token
```

Returns the current token without consuming it.

| Param | Type                  | Description    |
|-------|-----------------------|----------------|
| p     | [`*Parser`](#parser)  | Parser state.  |

#### `peek`

```mach
pub fun peek(p: *Parser, offset: usize) token.Token
```

Returns the token at `p.pos + offset` without consuming it. Clamps to
the trailing [`KIND_EOF`](../token.md#constants) when `offset` would
walk past the end.

| Param  | Type                  | Description                                  |
|--------|-----------------------|----------------------------------------------|
| p      | [`*Parser`](#parser)  | Parser state.                                |
| offset | `usize`               | Positive offset from the current position.   |

#### `advance`

```mach
pub fun advance(p: *Parser) token.Token
```

Consumes the current token and returns it. Stops advancing once
[`KIND_EOF`](../token.md#constants) is reached; further calls return
EOF without moving the cursor.

| Param | Type                  | Description    |
|-------|-----------------------|----------------|
| p     | [`*Parser`](#parser)  | Parser state.  |

#### `at_eof`

```mach
pub fun at_eof(p: *Parser) bool
```

Returns `true` when [`current(p).kind`](#current) is
[`KIND_EOF`](../token.md#constants).

| Param | Type                  | Description    |
|-------|-----------------------|----------------|
| p     | [`*Parser`](#parser)  | Parser state.  |

### Predicates

#### `at`

```mach
pub fun at(p: *Parser, kind: token.Kind) bool
```

Reports whether the current token has the given kind.

| Param | Type                          | Description                  |
|-------|-------------------------------|------------------------------|
| p     | [`*Parser`](#parser)          | Parser state.                |
| kind  | [`token.Kind`](../token.md#kind) | Token kind to test against.|

#### `eat`

```mach
pub fun eat(p: *Parser, kind: token.Kind) bool
```

Consumes the current token if its kind matches, reporting whether it
did.

| Param | Type                          | Description                  |
|-------|-------------------------------|------------------------------|
| p     | [`*Parser`](#parser)          | Parser state.                |
| kind  | [`token.Kind`](../token.md#kind) | Token kind to match.       |

Returns `true` when the token was consumed.

#### `at_kw`

```mach
pub fun at_kw(p: *Parser, kw: str) bool
```

Reports whether the current token is an [`KIND_IDENT`](../token.md#constants)
whose source text matches the supplied keyword.

| Param | Type                  | Description                                  |
|-------|-----------------------|----------------------------------------------|
| p     | [`*Parser`](#parser)  | Parser state.                                |
| kw    | `str`                 | Keyword to match against the IDENT's text.   |

#### `eat_kw`

```mach
pub fun eat_kw(p: *Parser, kw: str) bool
```

Consumes the current token when [`at_kw(p, kw)`](#at_kw) holds.

| Param | Type                  | Description                                  |
|-------|-----------------------|----------------------------------------------|
| p     | [`*Parser`](#parser)  | Parser state.                                |
| kw    | `str`                 | Keyword to match and consume.                |

Returns `true` when the keyword was consumed.

### Error reporting

#### `expect`

```mach
pub fun expect(p: *Parser, kind: token.Kind, message: str) bool
```

Consumes the current token when its kind matches; otherwise emits an
[`error_at_current`](#error_at_current) with `message` and returns
`false`.

| Param   | Type                              | Description                                       |
|---------|-----------------------------------|---------------------------------------------------|
| p       | [`*Parser`](#parser)              | Parser state.                                     |
| kind    | [`token.Kind`](../token.md#kind)  | Expected token kind.                              |
| message | `str`                             | Diagnostic text used when the expected kind is absent. |

#### `error_at_current`

```mach
pub fun error_at_current(p: *Parser, message: str)
```

Emits a [`SEVERITY_ERROR`](../../diagnostic.md#constants) diagnostic
pinned to the current token's span and sets [`p.panic = true`](#parser).
No-op if `p.panic` is already set.

| Param   | Type                  | Description                          |
|---------|-----------------------|--------------------------------------|
| p       | [`*Parser`](#parser)  | Parser state.                        |
| message | `str`                 | Diagnostic text to record.           |

#### `error_at`

```mach
pub fun error_at(p: *Parser, span: token.Span, message: str)
```

Emits a [`SEVERITY_ERROR`](../../diagnostic.md#constants) diagnostic
pinned to the supplied span and sets [`p.panic = true`](#parser).
No-op if `p.panic` is already set.

| Param   | Type                                  | Description                          |
|---------|---------------------------------------|--------------------------------------|
| p       | [`*Parser`](#parser)                  | Parser state.                        |
| span    | [`token.Span`](../token.md#span)      | Span to pin the diagnostic to.       |
| message | `str`                                 | Diagnostic text to record.           |

#### `sync_to`

```mach
pub fun sync_to(p: *Parser, k0: token.Kind, k1: token.Kind, k2: token.Kind)
```

Advances until [`current(p).kind`](#current) is one of `k0`/`k1`/`k2`
or [`KIND_EOF`](../token.md#constants), then clears [`p.panic`](#parser).
Use [`KIND_ERROR`](../token.md#constants) as a placeholder for unused
slots when only one or two synchronization kinds are needed.

| Param | Type                              | Description                                              |
|-------|-----------------------------------|----------------------------------------------------------|
| p     | [`*Parser`](#parser)              | Parser state.                                            |
| k0    | [`token.Kind`](../token.md#kind)  | First synchronization kind.                              |
| k1    | [`token.Kind`](../token.md#kind)  | Second synchronization kind.                             |
| k2    | [`token.Kind`](../token.md#kind)  | Third synchronization kind.                              |

### Span arithmetic

#### `span_of`

```mach
pub fun span_of(start: token.Span, end: token.Span) token.Span
```

Joins two spans into the minimal span that covers both. The result
spans from `start.offset` to `end.offset + end.len`.

| Param | Type                                  | Description                              |
|-------|---------------------------------------|------------------------------------------|
| start | [`token.Span`](../token.md#span)      | Span at the beginning of the region.     |
| end   | [`token.Span`](../token.md#span)      | Span at the end of the region.           |

### Node-append wrappers

All wrappers below append a node to the [`Ast`](../ast.md#ast). On
allocation failure they emit an `error_at` diagnostic and return the
corresponding `*_NIL` sentinel; the caller is expected to propagate the
failure by also checking [`p.panic`](#parser).

#### `push_expr`

```mach
pub fun push_expr(p: *Parser, e: expr.Expr) id.ExprId
```

Appends `e` via [`ast.add_expr`](../ast.md#add_module-add_expr-add_stmt-add_decl-add_type).
Returns the new [`ExprId`](../ast/id.md#exprid), or
[`EXPR_NIL`](../ast/id.md#constants) on failure.

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| e     | [`expr.Expr`](../ast/expr.md#expr)    | Expression to append.        |

#### `push_stmt`

```mach
pub fun push_stmt(p: *Parser, s: stmt.Stmt) id.StmtId
```

Appends `s` via [`ast.add_stmt`](../ast.md#add_module-add_expr-add_stmt-add_decl-add_type).
Returns the new [`StmtId`](../ast/id.md#stmtid), or
[`STMT_NIL`](../ast/id.md#constants) on failure.

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| s     | [`stmt.Stmt`](../ast/stmt.md#stmt)    | Statement to append.         |

#### `push_decl`

```mach
pub fun push_decl(p: *Parser, d: decl.Decl) id.DeclId
```

Appends `d` via [`ast.add_decl`](../ast.md#add_module-add_expr-add_stmt-add_decl-add_type).
Returns the new [`DeclId`](../ast/id.md#declid), or
[`DECL_NIL`](../ast/id.md#constants) on failure.

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| d     | [`decl.Decl`](../ast/decl.md#decl)    | Declaration to append.       |

#### `push_type`

```mach
pub fun push_type(p: *Parser, t: m_type.Type) id.TypeId
```

Appends `t` via [`ast.add_type`](../ast.md#add_module-add_expr-add_stmt-add_decl-add_type).
Returns the new [`TypeId`](../ast/id.md#typeid), or
[`TYPE_NIL`](../ast/id.md#constants) on failure.

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| t     | [`m_type.Type`](../ast/type.md#type)  | Type to append.              |

#### `push_module`

```mach
pub fun push_module(p: *Parser, m: module.Module) id.ModuleId
```

Appends `m` via [`ast.add_module`](../ast.md#add_module-add_expr-add_stmt-add_decl-add_type).
Returns the new [`ModuleId`](../ast/id.md#moduleid), or
[`MODULE_NIL`](../ast/id.md#constants) on failure.

| Param | Type                                       | Description                  |
|-------|--------------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                       | Parser state.                |
| m     | [`module.Module`](../ast/module.md#module) | Module to append.            |

#### `push_decl_id`

```mach
pub fun push_decl_id(p: *Parser, id_: id.DeclId) u32
```

Appends a [`DeclId`](../ast/id.md#declid) to
[`ast.decl_ids`](../ast.md#ast). Returns the new pool index, or `0` on
failure (caller must check [`p.panic`](#parser)).

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| id_   | [`id.DeclId`](../ast/id.md#declid)    | DeclId to append.            |

#### `push_stmt_id`

```mach
pub fun push_stmt_id(p: *Parser, id_: id.StmtId) u32
```

Appends a [`StmtId`](../ast/id.md#stmtid) to
[`ast.stmt_ids`](../ast.md#ast). Returns the new pool index, or `0` on
failure (caller must check [`p.panic`](#parser)).

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| id_   | [`id.StmtId`](../ast/id.md#stmtid)    | StmtId to append.            |

#### `push_expr_id`

```mach
pub fun push_expr_id(p: *Parser, id_: id.ExprId) u32
```

Appends an [`ExprId`](../ast/id.md#exprid) to
[`ast.expr_ids`](../ast.md#ast). Returns the new pool index, or `0` on
failure (caller must check [`p.panic`](#parser)).

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| id_   | [`id.ExprId`](../ast/id.md#exprid)    | ExprId to append.            |

#### `push_type_id`

```mach
pub fun push_type_id(p: *Parser, id_: id.TypeId) u32
```

Appends a [`TypeId`](../ast/id.md#typeid) to
[`ast.type_ids`](../ast.md#ast). Returns the new pool index, or `0` on
failure (caller must check [`p.panic`](#parser)).

| Param | Type                                  | Description                  |
|-------|---------------------------------------|------------------------------|
| p     | [`*Parser`](#parser)                  | Parser state.                |
| id_   | [`id.TypeId`](../ast/id.md#typeid)    | TypeId to append.            |

## Recovery model

The parser is *malformed-input tolerant*. When an error is detected
[`error_at`](#error_at) or [`error_at_current`](#error_at_current) is
called, which:

1. No-ops if [`p.panic`](#parser) is already set (the parser is still
   skipping from a prior error).
2. Otherwise sets `p.panic = true` and pushes the diagnostic.

While `p.panic` is set, further error-emitting calls are silent — this
prevents one syntactic mishap from producing a cascade of misleading
errors. The grammar functions clear `p.panic` at known synchronization
points by calling [`sync_to`](#sync_to) with kinds appropriate to the
enclosing construct (`;` and `}` for statements; matching brackets for
list bodies; the start of the next top-level keyword for declarations).

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, [`mach.lang.fe.token`](../token.md),
[`mach.lang.fe.lexer`](../lexer.md), [`mach.lang.diagnostic`](../../diagnostic.md),
[`mach.lang.fe.ast`](../ast.md), [`mach.lang.fe.ast.id`](../ast/id.md),
[`mach.lang.fe.ast.expr`](../ast/expr.md),
[`mach.lang.fe.ast.stmt`](../ast/stmt.md),
[`mach.lang.fe.ast.decl`](../ast/decl.md),
[`mach.lang.fe.ast.type`](../ast/type.md),
[`mach.lang.fe.ast.module`](../ast/module.md).
