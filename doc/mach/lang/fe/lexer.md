# mach.lang.fe.lexer

Tokenises source text into a [`TokenStream`](#tokenstream) terminated
by a [`KIND_EOF`](token.md#kind) token. Lex errors are accumulated in
a sibling [`LexError`](#lexerror) buffer rather than aborting the pass;
the parser later cross-references the stream and the errors.

## Types

### `LexError`

```mach
pub rec LexError {
    span: token.Span;
    code: u8;
}
```

A structured lex error pairing a bad span with an error code.

| Field | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| span  | [`token.Span`](token.md#span)         | Byte range that was malformed.       |
| code  | `u8`                                  | One of the `LEX_ERR_*` constants.    |

### `TokenStream`

```mach
pub rec TokenStream {
    tokens:    *token.Token;
    len:       usize;
    cap:       usize;
    source:    str;
    file_id:   src.FileId;
    errors:    *LexError;
    error_len: usize;
    error_cap: usize;
}
```

A flat, owning buffer of tokens and lex errors produced over one source
file.

| Field      | Type                                  | Description                                       |
|------------|---------------------------------------|---------------------------------------------------|
| tokens     | [`*token.Token`](token.md#token)      | Contiguous array of tokens ending in `KIND_EOF`.  |
| len        | `usize`                               | Number of tokens currently stored.                |
| cap        | `usize`                               | Allocated slots in `tokens`.                      |
| source     | `str`                                 | Pointer to the source text that was lexed.        |
| file_id    | [`src.FileId`](../source.md#fileid)   | Identifier of the file this stream came from.     |
| errors     | [`*LexError`](#lexerror)              | Contiguous array of [`LexError`](#lexerror); `nil` if none. |
| error_len  | `usize`                               | Number of errors currently stored.                |
| error_cap  | `usize`                               | Allocated slots in `errors` (0 when nil).         |

## Constants

```mach
pub val LEX_ERR_UNEXPECTED_CHAR:     u8 = 1;
pub val LEX_ERR_UNTERMINATED_STR:    u8 = 2;
pub val LEX_ERR_UNTERMINATED_CHAR:   u8 = 3;
pub val LEX_ERR_UNTERMINATED_ZSTR:   u8 = 4;
```

Error codes stored on [`LexError.code`](#lexerror).

| Code                              | Meaning                                                  |
|-----------------------------------|----------------------------------------------------------|
| `LEX_ERR_UNEXPECTED_CHAR`         | A byte at a leaf dispatch slot did not match any token.  |
| `LEX_ERR_UNTERMINATED_STR`        | A `"`-quoted literal reached EOF before the closing `"`. |
| `LEX_ERR_UNTERMINATED_CHAR`       | A `'`-quoted literal reached EOF before the closing `'`. |
| `LEX_ERR_UNTERMINATED_ZSTR`       | A `` ` ``-quoted literal reached EOF before the closing `` ` ``. |

```mach
val INITIAL_CAP:       usize = 256;
val INITIAL_ERROR_CAP: usize = 8;
```

| Constant            | Description                                                |
|---------------------|------------------------------------------------------------|
| `INITIAL_CAP`       | Starting capacity for [`TokenStream.tokens`](#tokenstream). The array doubles on overflow.|
| `INITIAL_ERROR_CAP` | Starting capacity for [`TokenStream.errors`](#tokenstream). The array doubles on overflow.|

## Functions

### `tokenize`

```mach
pub fun tokenize(source: str, alloc: *Allocator, file_id: src.FileId) Result[TokenStream, str]
```

Lexes `source` into a [`TokenStream`](#tokenstream) terminated by
[`KIND_EOF`](token.md#kind). Malformed input produces
[`LexError`](#lexerror) entries; lexing continues past errors so a
single bad input does not abort the stream.

| Param   | Type                                  | Description                                              |
|---------|---------------------------------------|----------------------------------------------------------|
| source  | `str`                                 | Null-terminated source text.                             |
| alloc   | `*Allocator`                          | Allocator used for the token and error buffers.          |
| file_id | [`src.FileId`](../source.md#fileid)   | File identifier stored on the stream for downstream diagnostics.|

Returns the owning [`TokenStream`](#tokenstream), or an allocation
error.

### `dnit`

```mach
pub fun dnit(stream: *TokenStream, alloc: *Allocator)
```

Releases the token and error buffers and clears the stream. `nil` is a
no-op.

| Param  | Type                              | Description                                  |
|--------|-----------------------------------|----------------------------------------------|
| stream | [`*TokenStream`](#tokenstream)    | Stream to tear down. `nil` is a no-op.       |
| alloc  | `*Allocator`                      | The allocator originally passed to `tokenize`.|

### `text`

```mach
pub fun text(stream: *TokenStream, tok: token.Token) str
```

Returns a fat-string view into `stream.source` covering `tok.span`.
Valid for the source buffer's lifetime.

| Param  | Type                              | Description                          |
|--------|-----------------------------------|--------------------------------------|
| stream | [`*TokenStream`](#tokenstream)    | Stream whose source the token belongs to.|
| tok    | [`token.Token`](token.md#token)   | Token to extract text for.           |

Returns a `str` view spanning the token's bytes (no allocation).

### `push_error`

```mach
fun push_error(stream: *TokenStream, alloc: *Allocator, offset: usize, len: usize, code: u8) Result[bool, str]
```

Internal helper used by [`tokenize`](#tokenize) to record a
[`LexError`](#lexerror) into `stream.errors`. Grows the errors array on
demand.

| Param  | Type                              | Description                                |
|--------|-----------------------------------|--------------------------------------------|
| stream | [`*TokenStream`](#tokenstream)    | Stream whose error buffer receives the entry.|
| alloc  | `*Allocator`                      | Allocator backing the error buffer.        |
| offset | `usize`                           | Starting byte offset of the bad span.      |
| len    | `usize`                           | Length in bytes.                           |
| code   | `u8`                              | One of the [`LEX_ERR_*`](#constants) codes.|

Returns `true` on success, or an allocation error.

### `is_ident_start`

```mach
fun is_ident_start(c: u8) bool
```

Returns `true` when `c` matches the regex `[a-zA-Z_]` — the set of
bytes that may begin an identifier.

### `is_ident_char`

```mach
fun is_ident_char(c: u8) bool
```

Returns `true` when `c` matches the regex `[a-zA-Z_0-9]` — the set of
bytes that may continue an identifier.

### `is_hex_char`

```mach
fun is_hex_char(c: u8) bool
```

Returns `true` when `c` matches the regex `[0-9a-fA-F]` — the set of
bytes that may appear in a hexadecimal integer literal.

## Recognition

The recogniser walks `source.data` byte-by-byte, dispatching by leading
character. Whitespace (` `, `\t`, `\n`, `\r`) and line comments (from
`#` to the next `\n`) are skipped without emitting tokens.

### Single- and multi-byte operators

| Lead char | Lookahead `+1` | Token                       |
|-----------|----------------|-----------------------------|
| `(`       | —              | `KIND_LPAREN`               |
| `)`       | —              | `KIND_RPAREN`               |
| `{`       | —              | `KIND_LBRACE`               |
| `}`       | —              | `KIND_RBRACE`               |
| `[`       | —              | `KIND_LBRACKET`             |
| `]`       | —              | `KIND_RBRACKET`             |
| `;`       | —              | `KIND_SEMI`                 |
| `,`       | —              | `KIND_COMMA`                |
| `?`       | —              | `KIND_QUESTION`             |
| `@`       | —              | `KIND_AT`                   |
| `$`       | —              | `KIND_DOLLAR`               |
| `~`       | —              | `KIND_TILDE`                |
| `+`       | —              | `KIND_PLUS`                 |
| `-`       | —              | `KIND_MINUS`                |
| `*`       | —              | `KIND_STAR`                 |
| `/`       | —              | `KIND_SLASH`                |
| `%`       | —              | `KIND_PERCENT`              |
| `^`       | —              | `KIND_CARET`                |
| `:`       | `:`            | `KIND_COLON_COLON`          |
| `:`       | (else)         | `KIND_COLON`                |
| `.`       | `..`           | `KIND_DOT_DOT_DOT`          |
| `.`       | (else)         | `KIND_DOT`                  |
| `=`       | `=`            | `KIND_EQ_EQ`                |
| `=`       | (else)         | `KIND_EQ`                   |
| `!`       | `=`            | `KIND_BANG_EQ`              |
| `!`       | (else)         | `KIND_BANG`                 |
| `<`       | `=`            | `KIND_LT_EQ`                |
| `<`       | `<`            | `KIND_LT_LT`                |
| `<`       | (else)         | `KIND_LT`                   |
| `>`       | `=`            | `KIND_GT_EQ`                |
| `>`       | `>`            | `KIND_GT_GT`                |
| `>`       | (else)         | `KIND_GT`                   |
| `&`       | `&`            | `KIND_AMP_AMP`              |
| `&`       | (else)         | `KIND_AMP`                  |
| `\|`      | `\|`           | `KIND_PIPE_PIPE`            |
| `\|`      | (else)         | `KIND_PIPE`                 |

### Literals

| Lead char        | Token             | Body rule                                              |
|------------------|-------------------|--------------------------------------------------------|
| `"`              | `KIND_LIT_STR`    | Bytes up to the next unescaped `"`. Escapes via `\`.   |
| `'`              | `KIND_LIT_CHAR`   | Bytes up to the next unescaped `'`. Escapes via `\`.   |
| `` ` ``          | `KIND_LIT_ZSTR`   | Bytes up to the next unescaped `` ` ``. Escapes via `\`.|
| `0..9`           | `KIND_LIT_INT` or `KIND_LIT_FLOAT` | See "Numeric literals" below.        |
| ident-start char | `KIND_IDENT`      | Ident-start byte followed by any number of ident-char bytes.|

A `\` inside a string/char/zstr literal consumes the following byte,
making escape sequences self-terminating. The lexer does not interpret
the escape — only `\"`, `\'`, `\\`, and `` \` `` matter for parsing the
closing delimiter; the escape's runtime meaning is the parser's
responsibility.

If a string/char/zstr literal reaches EOF before the closing delimiter,
an entry is pushed to `stream.errors` with the corresponding
`LEX_ERR_UNTERMINATED_*` code and the literal token is still emitted
with the span up to EOF.

### Numeric literals

```
INT       := dec_int | hex_int | bin_int | oct_int
dec_int   := digit (digit | '_')*  [ ('u'|'i') digit* ]
hex_int   := '0x' hex_char (hex_char | '_')*
bin_int   := '0b' [01]    ([01]    | '_')*
oct_int   := '0o' [0-7]   ([0-7]   | '_')*
FLOAT     := digit (digit | '_')*  '.' digit (digit | '_')*  [ 'f' digit* ]
```

The lexer emits `KIND_LIT_INT` unless a `.` followed by a digit appears
in the decimal form, in which case the token becomes `KIND_LIT_FLOAT`.
A trailing `u<digits>`, `i<digits>`, or `f<digits>` suffix (type
suffix) is consumed into the token's span; the parser interprets the
suffix.

### Identifiers

```
ident_start := [a-zA-Z_]
ident_char  := [a-zA-Z_0-9]
```

An identifier is one ident-start byte followed by zero or more
ident-char bytes. Keyword recognition is the parser's responsibility;
the lexer always emits `KIND_IDENT`.

### Unexpected characters

Any byte not matched above produces a single-byte
[`Token`](token.md#token) with `kind == KIND_ERROR` and an entry pushed
to `stream.errors` with [`LEX_ERR_UNEXPECTED_CHAR`](#constants).
Lexing continues at the next byte.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, `std.allocator`,
[`mach.lang.fe.token`](token.md), [`mach.lang.source`](../source.md).
