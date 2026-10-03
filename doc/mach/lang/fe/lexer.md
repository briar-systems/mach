# mach.lang.fe.lexer

## rec TokenStream

```mach
pub rec TokenStream;
```

## fun tokenize

```mach
pub fun tokenize(source: str, alloc: *A.Allocator, file_id: lang_source.FileId) res[TokenStream, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(stream: *TokenStream, alloc: *A.Allocator);
```

## fun doc_run_above

```mach
pub fun doc_run_above(stream: *TokenStream, decl_offset: usize) lang_source.Span;
```

## fun leading_run

```mach
pub fun leading_run(stream: *TokenStream) lang_source.Span;
```

the comment run that opens the file, when nothing but layout comes before it

## fun emit_diagnostics

```mach
pub fun emit_diagnostics(stream: *TokenStream, diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

