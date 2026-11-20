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
| [`ParseFail`](#parsefail) | `rec` | why a parse ended without a tree |
| [`Parser`](#parser) | `rec` | names: the interner every name the tree holds an id of is interned in |

| On `ParseFailKind` | |
| :--- | :--- |
| [`PARSE_FAIL_OOM`](#parsefailkind), [`PARSE_FAIL_ICE`](#parsefailkind), [`PARSE_FAIL_DIAG`](#parsefailkind), [`PARSE_FAIL_DEPTH`](#parsefailkind) | |

| On `ParseFail` | |
| :--- | :--- |
| [`failure`](#failure) | the failure a parse failure stands for in the caller's terms |

| On `Parser` | |
| :--- | :--- |
| [`init`](#init) | a parser at the first token, its status already rejected when the stream holds lex errors |
| [`current`](#current) | the token at the cursor |
| [`peek`](#peek) | the token `offset` places past the cursor, the end of file token when that is past the end |
| [`advance`](#advance) | step the cursor past the current token, never past the end of file token |
| [`name_id`](#name_id) | the id of the name `span` covers; a refused intern is fatal, and its name is STR_NIL |
| [`at`](#at) | whether the current token is of `kind` |
| [`at_kw`](#at_kw) | whether the current token is the identifier spelled `kw` |
| [`at_eof`](#at_eof) | whether the cursor is at the end of file token |
| [`at_decl_start`](#at_decl_start) | whether the current token can open a declaration: a decorator, a `$` or a declaration keyword |
| [`eat`](#eat) | step past the current token when it is of `kind` |
| [`eat_kw`](#eat_kw) | step past the current token when it is the identifier spelled `kw` |
| [`expect`](#expect) | take a token of `kind`, or report `message` at the current token as an expected token error |
| [`previous`](#previous) | the token just before the cursor |
| [`expect_end`](#expect_end) | take a closing token of `kind` as `expect` does, for the end of a node's span |
| [`diag_mark`](#diag_mark) | a mark in the buffer of pending diagnostics, for `diag_discard` or `diag_flush` |
| [`diag_discard`](#diag_discard) | drop every pending diagnostic recorded since the mark, because the reading that recorded them lost |
| [`diag_flush`](#diag_flush) | the reading that recorded these is the parse, so they are real |
| [`diag_dnit`](#diag_dnit) | free the buffer of pending diagnostics |
| [`error_at_current`](#error_at_current) | report a syntax error at the current token and enter panic, or only note it as suppressed while already in panic |
| [`error_recover_advance`](#error_recover_advance) | report as `error_at_current` does, and step past the offending token when the parser was not already in panic |
| [`error_at`](#error_at) | report a syntax error at `span` and enter panic, or only note it as suppressed while already in panic |
| [`error_naming`](#error_naming) | an error at `span` whose text fills the one `{}` of `fmt` with the source text of `name` |
| [`fatal_oom_at`](#fatal_oom_at) | end the parse for running out of memory, reported at `span` as the allocator's refusal |
| [`fatal_failed_at`](#fatal_failed_at) | the parse ends at `span` because a step that adds to the tree failed with `f`, which is reported as its own kind |
| [`fatal_ice_at`](#fatal_ice_at) | end the parse for an internal compiler error, reported at `span` |
| [`probed_init`](#probed_init) | a token position where the index reading of `name[` has already been tried and did not parse |
| [`probed_dnit`](#probed_dnit) | free the table of positions where the index reading failed |
| [`index_reading_failed_here`](#index_reading_failed_here) | whether the index reading of `name[` already failed at a token position |
| [`record_index_reading_failed`](#record_index_reading_failed) | record that the index reading of `name[` failed at a token position |
| [`descend`](#descend) | enter one more level of nesting, ending the parse when it passes MAX_NEST_DEPTH or the stack runs short |
| [`ascend`](#ascend) | leave a level of nesting `descend` entered |
| [`ast_mark`](#ast_mark) | a mark in the tree, for rolling back a reading that loses |
| [`ast_rollback`](#ast_rollback) | drop every node pushed since the mark, ending the parse as an internal error when the mark is invalid |
| [`sync_to`](#sync_to) | skip tokens up to one of the three kinds or the end of input, and leave panic |
| [`sync_to_body`](#sync_to_body) | skip tokens up to one of the three kinds outside any bracket opened on the way, or the end of input, and leave panic |
| [`sync_to_decl`](#sync_to_decl) | skip at least one token, then up to the next token that can open a declaration or the end of input, and leave panic |
| [`push_expr`](#push_expr) | add an expression to the tree |
| [`push_stmt`](#push_stmt) | add a statement to the tree |
| [`push_decl`](#push_decl) | add a declaration to the tree |
| [`push_type`](#push_type) | add a type to the tree |
| [`push_module`](#push_module) | add a module node to the tree |
| [`list_init`](#list_init) | a list gathered while its length is unknown, then flushed into its AST table in one run |
| [`list_push`](#list_push) | append to a gathered list, ending the parse when it runs out of memory |
| [`list_dnit`](#list_dnit) | free a gathered list, ending the parse as an internal error when the free is refused |
| [`list_flush`](#list_flush) | appends the gathered list to `table`, frees it, and answers where it starts in the table, with its length in `out_len` |

| Functions | |
| :--- | :--- |
| [`span_of`](#span_of) | the span from the start of one span to the end of another |

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
    failure: mach.base.fail.Fail;
}
```

why a parse ended without a tree

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `ParseFailKind` | what ended the parse: PARSE_FAIL_OOM, PARSE_FAIL_ICE, PARSE_FAIL_DIAG or PARSE_FAIL_DEPTH |
| `failure` | `mach.base.fail.Fail` | why, placed at the token it stopped at, reported for an input that nests too deep |

### failure

```mach
pub fun failure(f: ParseFail) mach.base.fail.Fail
```

the failure a parse failure stands for in the caller's terms

| Parameter | |
| :--- | :--- |
| `f` | the parse failure |
| **returns** | an already reported failure for a nesting limit, a failure carrying the message otherwise |

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
    fatal_failure: mach.base.fail.Fail;
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

a parser at the first token, its status already rejected when the stream holds lex errors

| Parameter | |
| :--- | :--- |
| `tokens` | the lexed file, ending in an end of file token |
| `out` | the tree the parse pushes nodes into |
| `names` | the interner names are interned in, which outlives the tree |
| `diags` | the store syntax errors are reported to |
| **returns** | the parser, whose probed table the caller sets up with `probed_init` and releases with `probed_dnit`, and whose diagnostic buffer it releases with `diag_dnit` |

### current

```mach
pub fun current(p: *Parser) mach.front.token.Token
```

the token at the cursor

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | the current token |

### peek

```mach
pub fun peek(p: *Parser, offset: usize) mach.front.token.Token
```

the token `offset` places past the cursor, the end of file token when that is past the end

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `offset` | how far ahead to look, 0 for the current token |
| **returns** | the token there |

### advance

```mach
pub fun advance(p: *Parser) mach.front.token.Token
```

step the cursor past the current token, never past the end of file token

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | the token stepped past |

### name_id

```mach
pub fun name_id(p: *Parser, span: mach.base.source.Span) mach.base.intern.StrId
```

the id of the name `span` covers; a refused intern is fatal, and its name is STR_NIL

### at

```mach
pub fun at(p: *Parser, kind: mach.front.token.Kind) bool
```

whether the current token is of `kind`

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `kind` | the kind to test for |
| **returns** | true when it is |

### at_kw

```mach
pub fun at_kw(p: *Parser, kw: str) bool
```

whether the current token is the identifier spelled `kw`

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `kw` | the reserved word to test for |
| **returns** | true when it is |

### at_eof

```mach
pub fun at_eof(p: *Parser) bool
```

whether the cursor is at the end of file token

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | true at the end of input |

### at_decl_start

```mach
pub fun at_decl_start(p: *Parser) bool
```

whether the current token can open a declaration: a decorator, a `$` or a declaration keyword

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | true when it can |

### eat

```mach
pub fun eat(p: *Parser, kind: mach.front.token.Kind) bool
```

step past the current token when it is of `kind`

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `kind` | the kind to take |
| **returns** | true when a token was taken |

### eat_kw

```mach
pub fun eat_kw(p: *Parser, kw: str) bool
```

step past the current token when it is the identifier spelled `kw`

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `kw` | the reserved word to take |
| **returns** | true when a token was taken |

### expect

```mach
pub fun expect(p: *Parser, kind: mach.front.token.Kind, message: str) bool
```

take a token of `kind`, or report `message` at the current token as an expected token error

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `kind` | the kind required |
| `message` | the error text when it is missing |
| **returns** | true when the token was taken |

### previous

```mach
pub fun previous(p: *Parser) mach.front.token.Token
```

the token just before the cursor

| Parameter | |
| :--- | :--- |
| `p` | the parser, past at least one token |
| **returns** | the previous token |

### expect_end

```mach
pub fun expect_end(p: *Parser, kind: mach.front.token.Kind, message: str) mach.front.token.Token
```

take a closing token of `kind` as `expect` does, for the end of a node's span

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `kind` | the kind required |
| `message` | the error text when it is missing |
| **returns** | the token taken, or the previous token when it is missing |

### diag_mark

```mach
pub fun diag_mark(p: *Parser) usize
```

a mark in the buffer of pending diagnostics, for `diag_discard` or `diag_flush`

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | how many diagnostics are pending |

### diag_discard

```mach
pub fun diag_discard(p: *Parser, m: usize)
```

drop every pending diagnostic recorded since the mark, because the reading that recorded them lost

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `m` | a mark from `diag_mark` |

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

free the buffer of pending diagnostics

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `alloc` | the allocator the buffer grew from, the tree's |

### error_at_current

```mach
pub fun error_at_current(p: *Parser, k: mach.base.diagnostic.kind.Kind, message: str)
```

report a syntax error at the current token and enter panic, or only note it as suppressed while already in panic

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `k` | the diagnostic kind |
| `message` | the error text |

### error_recover_advance

```mach
pub fun error_recover_advance(p: *Parser, k: mach.base.diagnostic.kind.Kind, message: str)
```

report as `error_at_current` does, and step past the offending token when the parser was not already in panic

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `k` | the diagnostic kind |
| `message` | the error text |

### error_at

```mach
pub fun error_at(p: *Parser, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, message: str)
```

report a syntax error at `span` and enter panic, or only note it as suppressed while already in panic

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `k` | the diagnostic kind |
| `span` | where the error is reported |
| `message` | the error text |

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
pub fun fatal_oom_at(p: *Parser, span: mach.base.source.Span, e: std.allocator.Error)
```

end the parse for running out of memory, reported at `span` as the allocator's refusal

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `span` | where the failure is reported |
| `e` | the allocator's refusal |

### fatal_failed_at

```mach
pub fun fatal_failed_at(p: *Parser, span: mach.base.source.Span, f: mach.base.fail.Fail)
```

the parse ends at `span` because a step that adds to the tree failed with `f`, which is
reported as its own kind

### fatal_ice_at

```mach
pub fun fatal_ice_at(p: *Parser, span: mach.base.source.Span, message: str)
```

end the parse for an internal compiler error, reported at `span`

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `span` | where the failure is reported |
| `message` | what went wrong |

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

free the table of positions where the index reading failed

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `alloc` | the allocator `probed_init` took it from |

### index_reading_failed_here

```mach
pub fun index_reading_failed_here(p: *Parser, at: usize) bool
```

whether the index reading of `name[` already failed at a token position

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `at` | the token position of the `[` |
| **returns** | true when it was recorded as failed |

### record_index_reading_failed

```mach
pub fun record_index_reading_failed(p: *Parser, at: usize)
```

record that the index reading of `name[` failed at a token position

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `at` | the token position of the `[` |

### descend

```mach
pub fun descend(p: *Parser) bool
```

enter one more level of nesting, ending the parse when it passes MAX_NEST_DEPTH or the stack runs short

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | true when the level was entered, which the caller leaves with `ascend` |

### ascend

```mach
pub fun ascend(p: *Parser)
```

leave a level of nesting `descend` entered

| Parameter | |
| :--- | :--- |
| `p` | the parser |

### ast_mark

```mach
pub fun ast_mark(p: *Parser) mach.front.ast.AstMark
```

a mark in the tree, for rolling back a reading that loses

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| **returns** | the tree's current mark |

### ast_rollback

```mach
pub fun ast_rollback(p: *Parser, m: mach.front.ast.AstMark)
```

drop every node pushed since the mark, ending the parse as an internal error when the mark is invalid

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `m` | a mark from `ast_mark` |

### sync_to

```mach
pub fun sync_to(p: *Parser, k0: mach.front.token.Kind, k1: mach.front.token.Kind, k2: mach.front.token.Kind)
```

skip tokens up to one of the three kinds or the end of input, and leave panic

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `k0` | a kind to stop at |
| `k1` | a kind to stop at |
| `k2` | a kind to stop at |

### sync_to_body

```mach
pub fun sync_to_body(p: *Parser, k0: mach.front.token.Kind, k1: mach.front.token.Kind, k2: mach.front.token.Kind)
```

skip tokens up to one of the three kinds outside any bracket opened on the way, or the end of input, and leave panic

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `k0` | a kind to stop at |
| `k1` | a kind to stop at |
| `k2` | a kind to stop at |

### sync_to_decl

```mach
pub fun sync_to_decl(p: *Parser)
```

skip at least one token, then up to the next token that can open a declaration or the end of input, and leave panic

| Parameter | |
| :--- | :--- |
| `p` | the parser |

### push_expr

```mach
pub fun push_expr(p: *Parser, e: mach.front.ast.expr.Expr) mach.front.ast.id.ExprId
```

add an expression to the tree

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `e` | the expression |
| **returns** | its id, or EXPR_NIL when the parse has ended or the add ran out of memory, which ends it |

### push_stmt

```mach
pub fun push_stmt(p: *Parser, s: mach.front.ast.stmt.Stmt) mach.front.ast.id.StmtId
```

add a statement to the tree

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `s` | the statement |
| **returns** | its id, or STMT_NIL when the parse has ended or the add ran out of memory, which ends it |

### push_decl

```mach
pub fun push_decl(p: *Parser, d: mach.front.ast.decl.Decl) mach.front.ast.id.DeclId
```

add a declaration to the tree

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `d` | the declaration |
| **returns** | its id, or DECL_NIL when the parse has ended or the add ran out of memory, which ends it |

### push_type

```mach
pub fun push_type(p: *Parser, t: mach.front.ast.type.Type) mach.front.ast.id.TypeId
```

add a type to the tree

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `t` | the type |
| **returns** | its id, or TYPE_NIL when the parse has ended or the add ran out of memory, which ends it |

### push_module

```mach
pub fun push_module(p: *Parser, m: mach.front.ast.module.Module) mach.front.ast.id.ModuleNodeId
```

add a module node to the tree

| Parameter | |
| :--- | :--- |
| `p` | the parser |
| `m` | the module node |
| **returns** | its id, or MODULE_NODE_NIL when the parse has ended or the add ran out of memory, which ends it |

### list_init

```mach
pub fun list_init[T](p: *Parser) Vector[T]
```

a list gathered while its length is unknown, then flushed into its AST table in one run

### list_push

```mach
pub fun list_push[T](p: *Parser, b: *Vector[T], x: T)
```

append to a gathered list, ending the parse when it runs out of memory

| Parameter | |
| :--- | :--- |
| `[T]` | the element type |
| `p` | the parser |
| `b` | the list |
| `x` | the element |

### list_dnit

```mach
pub fun list_dnit[T](p: *Parser, b: *Vector[T])
```

free a gathered list, ending the parse as an internal error when the free is refused

| Parameter | |
| :--- | :--- |
| `[T]` | the element type |
| `p` | the parser |
| `b` | the list |

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

the span from the start of one span to the end of another

| Parameter | |
| :--- | :--- |
| `start` | the first span |
| `end` | the last span, at or after `start` |
| **returns** | the span covering both |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MAX_NEST_DEPTH` | `usize` | `2048` | the deepest nesting the parser will descend into, a sanity limit |

