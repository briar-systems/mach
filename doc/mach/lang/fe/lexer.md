# mach.lang.fe.lexer

## def LexErrorCode

```mach
pub def LexErrorCode: u8
```

## val LEX_ERR_UNEXPECTED_CHAR

```mach
pub val LEX_ERR_UNEXPECTED_CHAR:   LexErrorCode = 1
```

## val LEX_ERR_UNTERMINATED_STR

```mach
pub val LEX_ERR_UNTERMINATED_STR:  LexErrorCode = 2
```

## val LEX_ERR_UNTERMINATED_CHAR

```mach
pub val LEX_ERR_UNTERMINATED_CHAR: LexErrorCode = 3
```

## val LEX_ERR_CONTROL_CHAR

```mach
pub val LEX_ERR_CONTROL_CHAR:      LexErrorCode = 4
```

## rec LexError

```mach
pub rec LexError;
```

## rec CommentRun

```mach
pub rec CommentRun;
```

## rec TokenStream

```mach
pub rec TokenStream;
```

## fun tokenize

```mach
pub fun tokenize(source: str, alloc: *A.Allocator, file_id: src.FileId) res[TokenStream, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(stream: *TokenStream, alloc: *A.Allocator);
```

## fun doc_run_above

```mach
pub fun doc_run_above(stream: *TokenStream, decl_offset: usize) token.Span;
```

## fun emit_diagnostics

```mach
pub fun emit_diagnostics(stream: *TokenStream, diags: *diagnostic.DiagnosticStore) err[fail.Fail];
```

## fun is_ident_start

```mach
pub fun is_ident_start(c: u8) bool;
```

## fun is_ident_char

```mach
pub fun is_ident_char(c: u8) bool;
```

## fun is_identifier

```mach
pub fun is_identifier(text: str, len: usize) bool;
```

`text[0..len]` lexes as exactly one identifier token; keywords lex as identifiers too

