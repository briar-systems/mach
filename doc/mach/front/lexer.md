# mach.front.lexer

## rec TokenStream

```mach
pub rec TokenStream;
```

## fun tokenize

```mach
pub fun tokenize(source: str, alloc: *A.Allocator, file_id: base_source.FileId) res[TokenStream, fail.Fail];
```

splits `source`, a NUL-terminated text, into tokens, the lex errors met, and the comment
runs a later doc lookup reads; the stream borrows `source` and owns the rest

## fun dnit

```mach
pub fun dnit(stream: *TokenStream, alloc: *A.Allocator);
```

release a token stream; a nil stream holds nothing

## fun doc_run_above

```mach
pub fun doc_run_above(stream: *TokenStream, decl_offset: usize) base_source.Span;
```

## fun leading_run

```mach
pub fun leading_run(stream: *TokenStream) base_source.Span;
```

the comment run that opens the file, when nothing but layout comes before it

## fun emit_diagnostics

```mach
pub fun emit_diagnostics(stream: *TokenStream, diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

