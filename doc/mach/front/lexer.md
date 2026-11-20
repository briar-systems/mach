[mach](../README.md) / [front](README.md) / **lexer**

# mach.front.lexer

the lexer: a file's source as a token stream, with its lexical errors and the runs of
line comments that documentation is read from

line-adjacent `#` comments that each start their own line fold into one run, and the
parser asks for the run directly above a declaration (`doc_run_above`) and the run
opening the file (`leading_run`)

`asm` where a statement begins, followed on its line by a word, opens an inline-asm block:
its head runs to the `{` on that line, and its body, the instruction set's text, is one
token whose end the dialect of the set the head names finds (`mach.front.asm`)

Source: [`src/front/lexer.mach`](../../../src/front/lexer.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.front.token`](token.md).

| Type | | |
| :--- | :--- | :--- |
| [`TokenStream`](#tokenstream) | `rec` | a lexed file: its tokens ending in one end of file token, its lex errors and its comment runs |

| On `TokenStream` | |
| :--- | :--- |
| [`tokenize`](#tokenize) | splits `source`, a NUL-terminated text, into tokens, the lex errors met, and the comment runs a later doc lookup reads; the stream borrows `source` and owns the rest |
| [`dnit`](#dnit) | release a token stream; a nil stream holds nothing |
| [`doc_run_above`](#doc_run_above) | the doc comment of a declaration: the last comment run ending before it, when that run starts its own line and sits on the line directly above with only layout between |
| [`leading_run`](#leading_run) | the comment run that opens the file, when nothing but layout comes before it |
| [`emit_diagnostics`](#emit_diagnostics) | report every lex error the stream holds, in the order met |

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

a lexed file: its tokens ending in one end of file token, its lex errors and its comment runs

| Field | Type | |
| :--- | :--- | :--- |
| `tokens` | `*mach.front.token.Token` | the tokens in source order, owned by the stream |
| `len` | `usize` | how many tokens are held |
| `cap` | `usize` | how many tokens `tokens` has room for |
| `source` | `str` | the text lexed, borrowed |
| `file_id` | `mach.base.source.FileId` | the file the text belongs to, where errors are reported |
| `errors` | `*LexError` | the lex errors met, owned by the stream |
| `error_len` | `usize` | how many errors are held |
| `error_cap` | `usize` | how many errors `errors` has room for |
| `comments` | `*CommentRun` | the comment runs, each a lone trailing comment or line adjacent own-line comments, in source order, owned by the stream |
| `comment_len` | `usize` | how many runs are held |
| `comment_cap` | `usize` | how many runs `comments` has room for |

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

the doc comment of a declaration: the last comment run ending before it, when that run
starts its own line and sits on the line directly above with only layout between

| Parameter | |
| :--- | :--- |
| `stream` | the lexed file |
| `decl_offset` | where the declaration begins |
| **returns** | the run's span, or an empty span at offset 0 when the declaration has none |

### leading_run

```mach
pub fun leading_run(stream: *TokenStream) mach.base.source.Span
```

the comment run that opens the file, when nothing but layout comes before it

### emit_diagnostics

```mach
pub fun emit_diagnostics(stream: *TokenStream, diags: *mach.base.diagnostic.DiagnosticStore) err[mach.base.fail.Fail]
```

report every lex error the stream holds, in the order met

| Parameter | |
| :--- | :--- |
| `stream` | the lexed file |
| `diags` | store the errors are reported to |
| **returns** | ok once all are reported, or the failure to report one or a code outside the catalog |

