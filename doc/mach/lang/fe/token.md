# mach.lang.fe.token

Token kinds and the token + span record types consumed by the
[lexer](lexer.md) and the [parser](parser.md). Also hosts the
infix-precedence and right-associativity tables that drive the parser's
Pratt climber.

## Types

### `Kind`

```mach
pub def Kind: u8;
```

Token kind discriminator. See [Constants](#constants) for the
enumerated values.

### `Span`

```mach
pub rec Span {
    offset: usize;
    len:    usize;
}
```

Byte range within a source file.

| Field  | Type    | Description              |
|--------|---------|--------------------------|
| offset | `usize` | Starting byte offset.    |
| len    | `usize` | Length in bytes.         |

### `Token`

```mach
pub rec Token {
    span: Span;
    kind: Kind;
}
```

A lexed unit paired with the source range it covers.

| Field | Type                | Description                  |
|-------|---------------------|------------------------------|
| span  | [`Span`](#span)     | Byte range of the token.     |
| kind  | [`Kind`](#kind)     | Which kind of token it is.   |

## Constants

```mach
pub val KIND_IDENT:       Kind = 0;
pub val KIND_LIT_INT:     Kind = 1;
pub val KIND_LIT_FLOAT:   Kind = 2;
pub val KIND_LIT_CHAR:    Kind = 3;
pub val KIND_LIT_STR:     Kind = 4;
pub val KIND_LIT_ZSTR:    Kind = 5;
pub val KIND_LPAREN:      Kind = 6;
pub val KIND_RPAREN:      Kind = 7;
pub val KIND_LBRACE:      Kind = 8;
pub val KIND_RBRACE:      Kind = 9;
pub val KIND_LBRACKET:    Kind = 10;
pub val KIND_RBRACKET:    Kind = 11;
pub val KIND_SEMI:        Kind = 12;
pub val KIND_COLON:       Kind = 13;
pub val KIND_COMMA:       Kind = 14;
pub val KIND_DOT:         Kind = 15;
pub val KIND_QUESTION:    Kind = 16;
pub val KIND_AT:          Kind = 17;
pub val KIND_DOLLAR:      Kind = 18;
pub val KIND_PLUS:        Kind = 19;
pub val KIND_MINUS:       Kind = 20;
pub val KIND_STAR:        Kind = 21;
pub val KIND_SLASH:       Kind = 22;
pub val KIND_PERCENT:     Kind = 23;
pub val KIND_AMP:         Kind = 24;
pub val KIND_PIPE:        Kind = 25;
pub val KIND_CARET:       Kind = 26;
pub val KIND_TILDE:       Kind = 27;
pub val KIND_EQ:          Kind = 28;
pub val KIND_EQ_EQ:       Kind = 29;
pub val KIND_BANG:        Kind = 30;
pub val KIND_BANG_EQ:     Kind = 31;
pub val KIND_LT:          Kind = 32;
pub val KIND_LT_EQ:       Kind = 33;
pub val KIND_LT_LT:       Kind = 34;
pub val KIND_GT:          Kind = 35;
pub val KIND_GT_EQ:       Kind = 36;
pub val KIND_GT_GT:       Kind = 37;
pub val KIND_AMP_AMP:     Kind = 38;
pub val KIND_PIPE_PIPE:   Kind = 39;
pub val KIND_COLON_COLON: Kind = 40;
pub val KIND_DOT_DOT_DOT: Kind = 41;
pub val KIND_EOF:         Kind = 254;
pub val KIND_ERROR:       Kind = 255;
```

[`Kind`](#kind) values.

| Constant            | Value | Surface                                                |
|---------------------|-------|--------------------------------------------------------|
| `KIND_IDENT`        | 0     | identifier                                             |
| `KIND_LIT_INT`      | 1     | integer literal                                        |
| `KIND_LIT_FLOAT`    | 2     | float literal                                          |
| `KIND_LIT_CHAR`     | 3     | character literal                                      |
| `KIND_LIT_STR`      | 4     | double-quoted string literal (fat-pointer `str`)       |
| `KIND_LIT_ZSTR`     | 5     | backtick literal (`*u8` / `zstr`)                      |
| `KIND_LPAREN`       | 6     | `(`                                                    |
| `KIND_RPAREN`       | 7     | `)`                                                    |
| `KIND_LBRACE`       | 8     | `{`                                                    |
| `KIND_RBRACE`       | 9     | `}`                                                    |
| `KIND_LBRACKET`     | 10    | `[`                                                    |
| `KIND_RBRACKET`     | 11    | `]`                                                    |
| `KIND_SEMI`         | 12    | `;`                                                    |
| `KIND_COLON`        | 13    | `:`                                                    |
| `KIND_COMMA`        | 14    | `,`                                                    |
| `KIND_DOT`          | 15    | `.`                                                    |
| `KIND_QUESTION`     | 16    | `?` (address-of)                                       |
| `KIND_AT`           | 17    | `@` (dereference)                                      |
| `KIND_DOLLAR`       | 18    | `$` (comptime ident prefix)                            |
| `KIND_PLUS`         | 19    | `+`                                                    |
| `KIND_MINUS`        | 20    | `-`                                                    |
| `KIND_STAR`         | 21    | `*`                                                    |
| `KIND_SLASH`        | 22    | `/`                                                    |
| `KIND_PERCENT`      | 23    | `%`                                                    |
| `KIND_AMP`          | 24    | `&`                                                    |
| `KIND_PIPE`         | 25    | `\|`                                                   |
| `KIND_CARET`        | 26    | `^`                                                    |
| `KIND_TILDE`        | 27    | `~`                                                    |
| `KIND_EQ`           | 28    | `=`                                                    |
| `KIND_EQ_EQ`        | 29    | `==`                                                   |
| `KIND_BANG`         | 30    | `!`                                                    |
| `KIND_BANG_EQ`      | 31    | `!=`                                                   |
| `KIND_LT`           | 32    | `<`                                                    |
| `KIND_LT_EQ`        | 33    | `<=`                                                   |
| `KIND_LT_LT`        | 34    | `<<`                                                   |
| `KIND_GT`           | 35    | `>`                                                    |
| `KIND_GT_EQ`        | 36    | `>=`                                                   |
| `KIND_GT_GT`        | 37    | `>>`                                                   |
| `KIND_AMP_AMP`      | 38    | `&&`                                                   |
| `KIND_PIPE_PIPE`    | 39    | `\|\|`                                                 |
| `KIND_COLON_COLON`  | 40    | `::` (cast)                                            |
| `KIND_DOT_DOT_DOT`  | 41    | `...` (variadic)                                       |
| `KIND_EOF`          | 254   | end-of-stream sentinel                                 |
| `KIND_ERROR`        | 255   | invalid-input sentinel                                 |

There is no token kind for `\`. Backslashes only appear inside string,
char, and zstr literal spans where the [lexer](lexer.md#recognition)
consumes them as part of the literal body; they never reach the
parser as standalone tokens.

## Functions

### `make`

```mach
pub fun make(kind: Kind, offset: usize, len: usize) Token
```

Constructs a [`Token`](#token) from its components.

| Param  | Type            | Description                  |
|--------|-----------------|------------------------------|
| kind   | [`Kind`](#kind) | Token kind.                  |
| offset | `usize`         | Byte offset into source.     |
| len    | `usize`         | Byte length.                 |

Returns a [`Token`](#token) carrying `(kind, Span { offset, len })`.

### `kind_str`

```mach
pub fun kind_str(kind: Kind) Option[str]
```

Returns a human-readable name for a token kind (e.g. `"("`, `"=="`,
`"ident"`).

| Param | Type            | Description           |
|-------|-----------------|-----------------------|
| kind  | [`Kind`](#kind) | The kind to describe. |

Returns `some(label)` for an enumerated kind, `none` for a value
outside the enumerated set. A `none` here means the caller holds a
[`Kind`](#kind) that the lexer never produces — a bug upstream — and
must be handled explicitly, not papered over with a default string.

### `infix_precedence`

```mach
pub fun infix_precedence(kind: Kind) u8
```

Returns the Pratt binding power for an infix operator, or `0` if the
kind is not infix.

| Param | Type            | Description           |
|-------|-----------------|-----------------------|
| kind  | [`Kind`](#kind) | Token kind to query.  |

Returns the precedence level (higher binds tighter). The full table:

| Precedence | Kinds                                                    |
|------------|----------------------------------------------------------|
| 1          | `KIND_EQ`                                                |
| 2          | `KIND_PIPE_PIPE`                                         |
| 3          | `KIND_AMP_AMP`                                           |
| 4          | `KIND_PIPE`                                              |
| 5          | `KIND_CARET`                                             |
| 6          | `KIND_AMP`                                               |
| 7          | `KIND_EQ_EQ`, `KIND_BANG_EQ`                             |
| 8          | `KIND_LT`, `KIND_LT_EQ`, `KIND_GT`, `KIND_GT_EQ`         |
| 9          | `KIND_LT_LT`, `KIND_GT_GT`                               |
| 10         | `KIND_PLUS`, `KIND_MINUS`                                |
| 11         | `KIND_STAR`, `KIND_SLASH`, `KIND_PERCENT`                |

### `is_right_assoc`

```mach
pub fun is_right_assoc(kind: Kind) bool
```

Reports whether an infix operator associates right-to-left. Only
`KIND_EQ` (assignment) is right-associative.

| Param | Type            | Description           |
|-------|-----------------|-----------------------|
| kind  | [`Kind`](#kind) | Token kind to query.  |

Returns `true` for right-associative operators; `false` otherwise.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`.
