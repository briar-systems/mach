[mach](../README.md) / [front](README.md) / **lexer**

# mach.front.lexer

the lexer: a file's source as a token stream, with its lexical errors and the runs of
line comments that documentation is read from

line-adjacent `#` comments that each start their own line fold into one run, and the
parser asks for the run directly above a declaration (`doc_run_above`) and the run
opening the file (`leading_run`)

<sub>source: [`src/front/lexer.mach`](../../../src/front/lexer.mach) · imported by 7 modules</sub>

<sub>`token` is [`mach.front.token`](token.md) · `base_source` is [`mach.base.source`](../base/source.md) · `LexError` is private · `CommentRun` is private · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md)</sub>

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
    tokens: *token.Token;
    len:    usize;
    cap:    usize;

    source:  str;
    file_id: base_source.FileId;

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
pub fun tokenize(source: str, alloc: *A.Allocator, file_id: base_source.FileId) res[TokenStream, fail.Fail]
```

splits `source`, a NUL-terminated text, into tokens, the lex errors met, and the comment
runs a later doc lookup reads; the stream borrows `source` and owns the rest

### dnit

```mach
pub fun dnit(stream: *TokenStream, alloc: *A.Allocator)
```

release a token stream; a nil stream holds nothing

### doc_run_above

```mach
pub fun doc_run_above(stream: *TokenStream, decl_offset: usize) base_source.Span
```

### leading_run

```mach
pub fun leading_run(stream: *TokenStream) base_source.Span
```

the comment run that opens the file, when nothing but layout comes before it

### emit_diagnostics

```mach
pub fun emit_diagnostics(stream: *TokenStream, diags: *diagnostic.DiagnosticStore) err[fail.Fail]
```

