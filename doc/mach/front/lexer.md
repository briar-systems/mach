[mach](../README.md) / [front](README.md) / **lexer**

# mach.front.lexer

the lexer: a file's source as a token stream, with its lexical errors and the runs of
line comments that documentation is read from

line-adjacent `#` comments that each start their own line fold into one run, and the
parser asks for the run directly above a declaration (`doc_run_above`) and the run
opening the file (`leading_run`)

Source: [`src/front/lexer.mach`](../../../src/front/lexer.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.front.token`](token.md).

| Type | | |
| :--- | :--- | :--- |
| [`TokenStream`](#tokenstream) | `rec` |  |

| On `TokenStream` | |
| :--- | :--- |
| [`tokenize`](#tokenize) | splits `source`, a NUL-terminated text, into tokens, the lex errors met, and the comment runs a later doc lookup reads; the stream borrows `source` and owns the rest |
| [`dnit`](#dnit) | release a token stream; a nil stream holds nothing |
| [`doc_run_above`](#doc_run_above) | |
| [`leading_run`](#leading_run) | the comment run that opens the file, when nothing but layout comes before it |
| [`emit_diagnostics`](#emit_diagnostics) | |

---

## TokenStream

```mach
pub rec TokenStream {
    tokens: *mach.front.token.Token;
    len:    usize;
    cap:    usize;

    source:  str;
    file_id: mach.base.source.FileId;

    errors:    *LexError;
    error_len: usize;
    error_cap: usize;

    comments:    *CommentRun;
    comment_len: usize;
    comment_cap: usize;
}
```

### tokenize

```mach
pub fun tokenize(source: str, alloc: *std.allocator.Allocator, file_id: mach.base.source.FileId) res[TokenStream, mach.base.fail.Fail]
```

splits `source`, a NUL-terminated text, into tokens, the lex errors met, and the comment
runs a later doc lookup reads; the stream borrows `source` and owns the rest

### dnit

```mach
pub fun dnit(stream: *TokenStream, alloc: *std.allocator.Allocator)
```

release a token stream; a nil stream holds nothing

### doc_run_above

```mach
pub fun doc_run_above(stream: *TokenStream, decl_offset: usize) mach.base.source.Span
```

### leading_run

```mach
pub fun leading_run(stream: *TokenStream) mach.base.source.Span
```

the comment run that opens the file, when nothing but layout comes before it

### emit_diagnostics

```mach
pub fun emit_diagnostics(stream: *TokenStream, diags: *mach.base.diagnostic.DiagnosticStore) err[mach.base.fail.Fail]
```

