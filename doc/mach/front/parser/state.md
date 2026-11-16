[mach](../../README.md) / [front](../README.md) / [parser](README.md) / **state**

# mach.front.parser.state

the parser's state: its cursor over the tokens, the tree it builds and what it reports

the grammar reads tokens and pushes nodes through it. it holds the nesting limit, the fatal
failure that ends a parse, the rollback of a reading that lost, and the buffer that keeps what a
walk deciding between the two readings of `name[` reports until its reading becomes the parse.

<sub>source: [`src/front/parser/state.mach`](../../../../src/front/parser/state.mach) · imported by 8 modules</sub>

<sub>`fail` is [`mach.base.fail`](../../base/fail.md) · `front_lexer` is [`mach.front.lexer`](../lexer.md) · `ast` is [`mach.front.ast`](../ast.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `diagnostic` is [`mach.base.diagnostic`](../../base/diagnostic.md) · `report_phase` is [`mach.report.phase`](../../report/phase.md) · `PendingDiag` is private · `token` is [`mach.front.token`](../token.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `A` is `std.allocator` · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md) · `ast_expr` is [`mach.front.ast.expr`](../ast/expr.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `ast_stmt` is [`mach.front.ast.stmt`](../ast/stmt.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md) · `ast_type` is [`mach.front.ast.type`](../ast/type.md) · `ast_module` is [`mach.front.ast.module`](../ast/module.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ParseFailKind`](#parsefailkind) | `def` |  |
| [`ParseFail`](#parsefail) | `rec` |  |
| [`Parser`](#parser) | `rec` | names: the interner every name the tree holds an id of is interned in |

| On `ParseFailKind` | |
| :--- | :--- |
| [`PARSE_FAIL_OOM`](#parsefailkind) · [`PARSE_FAIL_ICE`](#parsefailkind) · [`PARSE_FAIL_DIAG`](#parsefailkind) · [`PARSE_FAIL_DEPTH`](#parsefailkind) | |

| On `ParseFail` | |
| :--- | :--- |
| [`failure`](#failure) | |

| On `Parser` | |
| :--- | :--- |
| [`init`](#init) · [`current`](#current) · [`peek`](#peek) · [`advance`](#advance) | |
| [`name_id`](#name_id) | the id of the name `span` covers; a refused intern is fatal, and its name is STR_NIL |
| [`at`](#at) · [`at_kw`](#at_kw) · [`at_eof`](#at_eof) · [`at_decl_start`](#at_decl_start) · [`eat`](#eat) · [`eat_kw`](#eat_kw) · [`expect`](#expect) · [`previous`](#previous) · [`expect_end`](#expect_end) · [`diag_mark`](#diag_mark) · [`diag_discard`](#diag_discard) | |
| [`diag_flush`](#diag_flush) | the reading that recorded these is the parse, so they are real |
| [`diag_dnit`](#diag_dnit) · [`error_at_current`](#error_at_current) · [`error_recover_advance`](#error_recover_advance) · [`error_at`](#error_at) | |
| [`error_naming`](#error_naming) | an error at `span` whose text fills the one `{}` of `fmt` with the source text of `name` |
| [`fatal_oom_at`](#fatal_oom_at) · [`fatal_ice_at`](#fatal_ice_at) | |
| [`probed_init`](#probed_init) | a token position where the index reading of `name[` has already been tried and did not parse |
| [`probed_dnit`](#probed_dnit) · [`index_reading_failed_here`](#index_reading_failed_here) · [`record_index_reading_failed`](#record_index_reading_failed) · [`descend`](#descend) · [`ascend`](#ascend) · [`ast_mark`](#ast_mark) · [`ast_rollback`](#ast_rollback) · [`sync_to`](#sync_to) · [`sync_to_body`](#sync_to_body) · [`sync_to_decl`](#sync_to_decl) · [`push_expr`](#push_expr) · [`push_stmt`](#push_stmt) · [`push_decl`](#push_decl) · [`push_type`](#push_type) · [`push_module`](#push_module) | |
| [`list_init`](#list_init) | a list gathered while its length is unknown, then flushed into its AST table in one run |
| [`list_push`](#list_push) · [`list_dnit`](#list_dnit) | |
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
pub fun failure(f: ParseFail) fail.Fail
```

## Parser

```mach
pub rec Parser {
    tokens:        *front_lexer.TokenStream;
    pos:           usize;
    ast:           *ast.Ast;
    names:         *intern.Interner;
    diags:         *diagnostic.DiagnosticStore;
    status:        report_phase.PhaseStatus;
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
pub fun init(tokens: *front_lexer.TokenStream, out: *ast.Ast, names: *intern.Interner, diags: *diagnostic.DiagnosticStore) Parser
```

### current

```mach
pub fun current(p: *Parser) token.Token
```

### peek

```mach
pub fun peek(p: *Parser, offset: usize) token.Token
```

### advance

```mach
pub fun advance(p: *Parser) token.Token
```

### name_id

```mach
pub fun name_id(p: *Parser, span: base_source.Span) intern.StrId
```

the id of the name `span` covers; a refused intern is fatal, and its name is STR_NIL

### at

```mach
pub fun at(p: *Parser, kind: token.Kind) bool
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
pub fun eat(p: *Parser, kind: token.Kind) bool
```

### eat_kw

```mach
pub fun eat_kw(p: *Parser, kw: str) bool
```

### expect

```mach
pub fun expect(p: *Parser, kind: token.Kind, message: str) bool
```

### previous

```mach
pub fun previous(p: *Parser) token.Token
```

### expect_end

```mach
pub fun expect_end(p: *Parser, kind: token.Kind, message: str) token.Token
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
pub fun diag_dnit(p: *Parser, alloc: *A.Allocator)
```

### error_at_current

```mach
pub fun error_at_current(p: *Parser, k: diagnostic_kind.Kind, message: str)
```

### error_recover_advance

```mach
pub fun error_recover_advance(p: *Parser, k: diagnostic_kind.Kind, message: str)
```

### error_at

```mach
pub fun error_at(p: *Parser, k: diagnostic_kind.Kind, span: base_source.Span, message: str)
```

### error_naming

```mach
pub fun error_naming(p: *Parser, k: diagnostic_kind.Kind, span: base_source.Span, fmt: str, name: base_source.Span)
```

an error at `span` whose text fills the one `{}` of `fmt` with the source
text of `name`. it leaves panic alone, so the reading goes on where it
stands. the text is released once the store holds its own copy, so it is
never buffered: a caller reads outside every deciding walk

### fatal_oom_at

```mach
pub fun fatal_oom_at(p: *Parser, span: base_source.Span)
```

### fatal_ice_at

```mach
pub fun fatal_ice_at(p: *Parser, span: base_source.Span, message: str)
```

### probed_init

```mach
pub fun probed_init(p: *Parser, alloc: *A.Allocator)
```

a token position where the index reading of `name[` has already been tried
and did not parse. the verdict is about tokens alone, so it survives any
rollback, and without it a malformed chain re-derives the same failure at
every level and doubles with depth.

### probed_dnit

```mach
pub fun probed_dnit(p: *Parser, alloc: *A.Allocator)
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
pub fun ast_mark(p: *Parser) ast.AstMark
```

### ast_rollback

```mach
pub fun ast_rollback(p: *Parser, m: ast.AstMark)
```

### sync_to

```mach
pub fun sync_to(p: *Parser, k0: token.Kind, k1: token.Kind, k2: token.Kind)
```

### sync_to_body

```mach
pub fun sync_to_body(p: *Parser, k0: token.Kind, k1: token.Kind, k2: token.Kind)
```

### sync_to_decl

```mach
pub fun sync_to_decl(p: *Parser)
```

### push_expr

```mach
pub fun push_expr(p: *Parser, e: ast_expr.Expr) ast_id.ExprId
```

### push_stmt

```mach
pub fun push_stmt(p: *Parser, s: ast_stmt.Stmt) ast_id.StmtId
```

### push_decl

```mach
pub fun push_decl(p: *Parser, d: ast_decl.Decl) ast_id.DeclId
```

### push_type

```mach
pub fun push_type(p: *Parser, t: ast_type.Type) ast_id.TypeId
```

### push_module

```mach
pub fun push_module(p: *Parser, m: ast_module.Module) ast_id.ModuleNodeId
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
pub fun span_of(start: base_source.Span, end: base_source.Span) base_source.Span
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MAX_NEST_DEPTH` | `usize` | `2048` | the deepest nesting the parser will descend into, a sanity limit |

