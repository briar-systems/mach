[mach](../../README.md) / [front](../README.md) / [parser](README.md) / **state**

# mach.front.parser.state

the parser's state: its cursor over the tokens, the tree it builds and what it reports

the grammar reads tokens and pushes nodes through it. it holds the nesting limit, the fatal
failure that ends a parse, the rollback of a reading that lost, and the buffer that keeps what a
walk deciding between the two readings of `name[` reports until its reading becomes the parse.

Source: [`src/front/parser/state.mach`](../../../../src/front/parser/state.mach)

Uses types from [`mach.base.diagnostic`](../../base/diagnostic.md), [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.ast.module`](../ast/module.md), [`mach.front.ast.stmt`](../ast/stmt.md), [`mach.front.ast.type`](../ast/type.md), [`mach.front.lexer`](../lexer.md), [`mach.front.token`](../token.md), [`mach.report.phase`](../../report/phase.md).

| Type | | |
| :--- | :--- | :--- |
| [`ParseFailKind`](#parsefailkind) | `def` |  |
| [`ParseFail`](#parsefail) | `rec` |  |
| [`Parser`](#parser) | `rec` | names: the interner every name the tree holds an id of is interned in |

| On `ParseFailKind` | |
| :--- | :--- |
| [`PARSE_FAIL_OOM`](#parsefailkind), [`PARSE_FAIL_ICE`](#parsefailkind), [`PARSE_FAIL_DIAG`](#parsefailkind), [`PARSE_FAIL_DEPTH`](#parsefailkind) | |

| On `ParseFail` | |
| :--- | :--- |
| [`failure`](#failure) | |

| On `Parser` | |
| :--- | :--- |
| [`init`](#init), [`current`](#current), [`peek`](#peek), [`advance`](#advance) | |
| [`name_id`](#name_id) | the id of the name `span` covers; a refused intern is fatal, and its name is STR_NIL |
| [`at`](#at), [`at_kw`](#at_kw), [`at_eof`](#at_eof), [`at_decl_start`](#at_decl_start), [`eat`](#eat), [`eat_kw`](#eat_kw), [`expect`](#expect), [`previous`](#previous), [`expect_end`](#expect_end), [`diag_mark`](#diag_mark), [`diag_discard`](#diag_discard) | |
| [`diag_flush`](#diag_flush) | the reading that recorded these is the parse, so they are real |
| [`diag_dnit`](#diag_dnit), [`error_at_current`](#error_at_current), [`error_recover_advance`](#error_recover_advance), [`error_at`](#error_at) | |
| [`error_naming`](#error_naming) | an error at `span` whose text fills the one `{}` of `fmt` with the source text of `name` |
| [`fatal_oom_at`](#fatal_oom_at), [`fatal_ice_at`](#fatal_ice_at) | |
| [`probed_init`](#probed_init) | a token position where the index reading of `name[` has already been tried and did not parse |
| [`probed_dnit`](#probed_dnit), [`index_reading_failed_here`](#index_reading_failed_here), [`record_index_reading_failed`](#record_index_reading_failed), [`descend`](#descend), [`ascend`](#ascend), [`ast_mark`](#ast_mark), [`ast_rollback`](#ast_rollback), [`sync_to`](#sync_to), [`sync_to_body`](#sync_to_body), [`sync_to_decl`](#sync_to_decl), [`push_expr`](#push_expr), [`push_stmt`](#push_stmt), [`push_decl`](#push_decl), [`push_type`](#push_type), [`push_module`](#push_module) | |
| [`list_init`](#list_init) | a list gathered while its length is unknown, then flushed into its AST table in one run |
| [`list_push`](#list_push), [`list_dnit`](#list_dnit) | |
| [`list_flush`](#list_flush) | appends the gathered list to `table`, frees it, and answers where it starts in the table, with its length in `out_len` |

| Functions | |
| :--- | :--- |
| [`span_of`](#span_of) | |

| Constants | |
| :--- | :--- |
| [`MAX_NEST_DEPTH`](#constants) | the deepest nesting the parser will descend into, a sanity limit |

---

## ParseFailKind

```mach
pub def ParseFailKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `PARSE_FAIL_OOM` | `0` |  |
| `PARSE_FAIL_ICE` | `1` |  |
| `PARSE_FAIL_DIAG` | `2` |  |
| `PARSE_FAIL_DEPTH` | `3` |  |

## ParseFail

```mach
pub rec ParseFail {
    kind:    ParseFailKind;
    message: str;
}
```

### failure

```mach
pub fun failure(f: ParseFail) mach.base.fail.Fail
```

## Parser

```mach
pub rec Parser {
    tokens:        *mach.front.lexer.TokenStream;
    pos:           usize;
    ast:           *mach.front.ast.Ast;
    names:         *mach.base.intern.Interner;
    diags:         *mach.base.diagnostic.DiagnosticStore;
    status:        mach.report.phase.PhaseStatus;
    panic:         bool;
    fatal:         bool;
    fatal_kind:    ParseFailKind;
    fatal_message: str;
    no_struct_lit: bool;
    depth:         usize;
    suppressed:    bool;
    probed:        *u8;
    probed_len:    usize;

    pending:     *PendingDiag;
    pending_len: usize;
    pending_cap: usize;
    buffering:   bool;
}
```

names: the interner every name the tree holds an id of is interned in. it outlives the tree
       and never takes a string back, so each id stays the name's for as long as the tree lives

### init

```mach
pub fun init(tokens: *mach.front.lexer.TokenStream, out: *mach.front.ast.Ast, names: *mach.base.intern.Interner, diags: *mach.base.diagnostic.DiagnosticStore) Parser
```

### current

```mach
pub fun current(p: *Parser) mach.front.token.Token
```

### peek

```mach
pub fun peek(p: *Parser, offset: usize) mach.front.token.Token
```

### advance

```mach
pub fun advance(p: *Parser) mach.front.token.Token
```

### name_id

```mach
pub fun name_id(p: *Parser, span: mach.base.source.Span) mach.base.intern.StrId
```

the id of the name `span` covers; a refused intern is fatal, and its name is STR_NIL

### at

```mach
pub fun at(p: *Parser, kind: mach.front.token.Kind) bool
```

### at_kw

```mach
pub fun at_kw(p: *Parser, kw: str) bool
```

### at_eof

```mach
pub fun at_eof(p: *Parser) bool
```

### at_decl_start

```mach
pub fun at_decl_start(p: *Parser) bool
```

### eat

```mach
pub fun eat(p: *Parser, kind: mach.front.token.Kind) bool
```

### eat_kw

```mach
pub fun eat_kw(p: *Parser, kw: str) bool
```

### expect

```mach
pub fun expect(p: *Parser, kind: mach.front.token.Kind, message: str) bool
```

### previous

```mach
pub fun previous(p: *Parser) mach.front.token.Token
```

### expect_end

```mach
pub fun expect_end(p: *Parser, kind: mach.front.token.Kind, message: str) mach.front.token.Token
```

### diag_mark

```mach
pub fun diag_mark(p: *Parser) usize
```

### diag_discard

```mach
pub fun diag_discard(p: *Parser, m: usize)
```

### diag_flush

```mach
pub fun diag_flush(p: *Parser, m: usize)
```

the reading that recorded these is the parse, so they are real. inside an
enclosing deciding walk they stay in the buffer, because that walk may yet
lose and take them with it; at the top they go to the sink, in order.

### diag_dnit

```mach
pub fun diag_dnit(p: *Parser, alloc: *std.allocator.Allocator)
```

### error_at_current

```mach
pub fun error_at_current(p: *Parser, k: mach.base.diagnostic.kind.Kind, message: str)
```

### error_recover_advance

```mach
pub fun error_recover_advance(p: *Parser, k: mach.base.diagnostic.kind.Kind, message: str)
```

### error_at

```mach
pub fun error_at(p: *Parser, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, message: str)
```

### error_naming

```mach
pub fun error_naming(p: *Parser, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, fmt: str, name: mach.base.source.Span)
```

an error at `span` whose text fills the one `{}` of `fmt` with the source
text of `name`. it leaves panic alone, so the reading goes on where it
stands. the text is released once the store holds its own copy, so it is
never buffered: a caller reads outside every deciding walk

### fatal_oom_at

```mach
pub fun fatal_oom_at(p: *Parser, span: mach.base.source.Span)
```

### fatal_ice_at

```mach
pub fun fatal_ice_at(p: *Parser, span: mach.base.source.Span, message: str)
```

### probed_init

```mach
pub fun probed_init(p: *Parser, alloc: *std.allocator.Allocator)
```

a token position where the index reading of `name[` has already been tried
and did not parse. the verdict is about tokens alone, so it survives any
rollback, and without it a malformed chain re-derives the same failure at
every level and doubles with depth.

### probed_dnit

```mach
pub fun probed_dnit(p: *Parser, alloc: *std.allocator.Allocator)
```

### index_reading_failed_here

```mach
pub fun index_reading_failed_here(p: *Parser, at: usize) bool
```

### record_index_reading_failed

```mach
pub fun record_index_reading_failed(p: *Parser, at: usize)
```

### descend

```mach
pub fun descend(p: *Parser) bool
```

### ascend

```mach
pub fun ascend(p: *Parser)
```

### ast_mark

```mach
pub fun ast_mark(p: *Parser) mach.front.ast.AstMark
```

### ast_rollback

```mach
pub fun ast_rollback(p: *Parser, m: mach.front.ast.AstMark)
```

### sync_to

```mach
pub fun sync_to(p: *Parser, k0: mach.front.token.Kind, k1: mach.front.token.Kind, k2: mach.front.token.Kind)
```

### sync_to_body

```mach
pub fun sync_to_body(p: *Parser, k0: mach.front.token.Kind, k1: mach.front.token.Kind, k2: mach.front.token.Kind)
```

### sync_to_decl

```mach
pub fun sync_to_decl(p: *Parser)
```

### push_expr

```mach
pub fun push_expr(p: *Parser, e: mach.front.ast.expr.Expr) mach.front.ast.id.ExprId
```

### push_stmt

```mach
pub fun push_stmt(p: *Parser, s: mach.front.ast.stmt.Stmt) mach.front.ast.id.StmtId
```

### push_decl

```mach
pub fun push_decl(p: *Parser, d: mach.front.ast.decl.Decl) mach.front.ast.id.DeclId
```

### push_type

```mach
pub fun push_type(p: *Parser, t: mach.front.ast.type.Type) mach.front.ast.id.TypeId
```

### push_module

```mach
pub fun push_module(p: *Parser, m: mach.front.ast.module.Module) mach.front.ast.id.ModuleNodeId
```

### list_init

```mach
pub fun list_init[T](p: *Parser) Vector[T]
```

a list gathered while its length is unknown, then flushed into its AST table in one run

### list_push

```mach
pub fun list_push[T](p: *Parser, b: *Vector[T], x: T)
```

### list_dnit

```mach
pub fun list_dnit[T](p: *Parser, b: *Vector[T])
```

### list_flush

```mach
pub fun list_flush[T](p: *Parser, b: *Vector[T], table: *Vector[T], out_len: *u32) u32
```

appends the gathered list to `table`, frees it, and answers where it starts in the
table, with its length in `out_len`

## Functions

### span_of

```mach
pub fun span_of(start: mach.base.source.Span, end: mach.base.source.Span) mach.base.source.Span
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MAX_NEST_DEPTH` | `usize` | `2048` | the deepest nesting the parser will descend into, a sanity limit |

