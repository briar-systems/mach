[mach](../README.md) / [front](README.md) / **token**

# mach.front.token

the tokens of the language: their kinds, fixed punctuation spellings and infix precedence

Source: [`src/front/token.mach`](../../../src/front/token.mach)

Uses types from [`mach.base.source`](../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` |  |
| [`Token`](#token) | `rec` | one token of a source |
| [`Spelling`](#spelling) | `rec` | a token the source spells with fixed punctuation |

| On `Kind` | |
| :--- | :--- |
| [`KIND_IDENT`](#kind), [`KIND_LIT_INT`](#kind), [`KIND_LIT_FLOAT`](#kind), [`KIND_LIT_CHAR`](#kind), [`KIND_LIT_STR`](#kind), [`KIND_LPAREN`](#kind), [`KIND_RPAREN`](#kind), [`KIND_LBRACE`](#kind), [`KIND_RBRACE`](#kind), [`KIND_LBRACKET`](#kind), [`KIND_RBRACKET`](#kind), [`KIND_SEMI`](#kind), [`KIND_COLON`](#kind), [`KIND_COMMA`](#kind), [`KIND_DOT`](#kind), [`KIND_QUESTION`](#kind), [`KIND_AT`](#kind), [`KIND_DOLLAR`](#kind), [`KIND_PLUS`](#kind), [`KIND_MINUS`](#kind), [`KIND_STAR`](#kind), [`KIND_SLASH`](#kind), [`KIND_PERCENT`](#kind), [`KIND_AMP`](#kind), [`KIND_PIPE`](#kind), [`KIND_CARET`](#kind), [`KIND_TILDE`](#kind), [`KIND_EQ`](#kind), [`KIND_EQ_EQ`](#kind), [`KIND_BANG`](#kind), [`KIND_BANG_EQ`](#kind), [`KIND_LT`](#kind), [`KIND_LT_EQ`](#kind), [`KIND_LT_LT`](#kind), [`KIND_GT`](#kind), [`KIND_GT_EQ`](#kind), [`KIND_GT_GT`](#kind), [`KIND_AMP_AMP`](#kind), [`KIND_PIPE_PIPE`](#kind), [`KIND_COLON_COLON`](#kind), [`KIND_DOT_DOT_DOT`](#kind), [`KIND_COLON_TILDE`](#kind), [`KIND_DECORATOR_OPEN`](#kind), [`KIND_COLON_GT`](#kind) | |
| [`KIND_ASM_HEAD`](#kind) | an inline-asm block's head and body, each raw text the lexer hands on whole |
| [`KIND_ASM_BODY`](#kind), [`KIND_EOF`](#kind), [`KIND_ERROR`](#kind) | |
| [`make`](#make) | a token of `kind` covering `len` bytes from `offset` |
| [`infix_precedence`](#infix_precedence) | the binding power of an infix operator token; 0 for every other kind, which is not an operator (a partition, enumerated by its test) |
| [`is_right_assoc`](#is_right_assoc) | whether an infix operator binds right to left, which only assignment does |

| On `Token` | |
| :--- | :--- |
| [`spelling_at`](#spelling_at) | the fixed spelling that starts at `pos` in NUL-terminated `source`, the longest one, as a token |

---

## Kind

```mach
pub def Kind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `KIND_IDENT` | `0` |  |
| `KIND_LIT_INT` | `1` |  |
| `KIND_LIT_FLOAT` | `2` |  |
| `KIND_LIT_CHAR` | `3` |  |
| `KIND_LIT_STR` | `4` |  |
| `KIND_LPAREN` | `5` |  |
| `KIND_RPAREN` | `6` |  |
| `KIND_LBRACE` | `7` |  |
| `KIND_RBRACE` | `8` |  |
| `KIND_LBRACKET` | `9` |  |
| `KIND_RBRACKET` | `10` |  |
| `KIND_SEMI` | `11` |  |
| `KIND_COLON` | `12` |  |
| `KIND_COMMA` | `13` |  |
| `KIND_DOT` | `14` |  |
| `KIND_QUESTION` | `15` |  |
| `KIND_AT` | `16` |  |
| `KIND_DOLLAR` | `17` |  |
| `KIND_PLUS` | `18` |  |
| `KIND_MINUS` | `19` |  |
| `KIND_STAR` | `20` |  |
| `KIND_SLASH` | `21` |  |
| `KIND_PERCENT` | `22` |  |
| `KIND_AMP` | `23` |  |
| `KIND_PIPE` | `24` |  |
| `KIND_CARET` | `25` |  |
| `KIND_TILDE` | `26` |  |
| `KIND_EQ` | `27` |  |
| `KIND_EQ_EQ` | `28` |  |
| `KIND_BANG` | `29` |  |
| `KIND_BANG_EQ` | `30` |  |
| `KIND_LT` | `31` |  |
| `KIND_LT_EQ` | `32` |  |
| `KIND_LT_LT` | `33` |  |
| `KIND_GT` | `34` |  |
| `KIND_GT_EQ` | `35` |  |
| `KIND_GT_GT` | `36` |  |
| `KIND_AMP_AMP` | `37` |  |
| `KIND_PIPE_PIPE` | `38` |  |
| `KIND_COLON_COLON` | `39` |  |
| `KIND_DOT_DOT_DOT` | `40` |  |
| `KIND_COLON_TILDE` | `41` |  |
| `KIND_DECORATOR_OPEN` | `42` |  |
| `KIND_COLON_GT` | `43` |  |
| `KIND_ASM_HEAD` | `44` | an inline-asm block's head and body, each raw text the lexer hands on whole |
| `KIND_ASM_BODY` | `45` |  |
| `KIND_EOF` | `254` |  |
| `KIND_ERROR` | `255` |  |

### make

```mach
pub fun make(kind: Kind, offset: usize, len: usize) Token
```

a token of `kind` covering `len` bytes from `offset`

| Parameter | |
| :--- | :--- |
| `kind` | the token's kind |
| `offset` | byte offset of the token in the source |
| `len` | byte length of the token |
| **returns** | the token |

### infix_precedence

```mach
pub fun infix_precedence(kind: Kind) u8
```

the binding power of an infix operator token; 0 for every other kind, which
is not an operator (a partition, enumerated by its test)

### is_right_assoc

```mach
pub fun is_right_assoc(kind: Kind) bool
```

whether an infix operator binds right to left, which only assignment does

| Parameter | |
| :--- | :--- |
| `kind` | the operator token's kind |
| **returns** | true for `=`, false for every other kind |

## Token

```mach
pub rec Token {
    span: mach.base.source.Span;
    kind: Kind;
}
```

one token of a source

| Field | Type | |
| :--- | :--- | :--- |
| `span` | `mach.base.source.Span` | where the token lies in the source |
| `kind` | `Kind` | what the token is |

### spelling_at

```mach
pub fun spelling_at(source: str, pos: usize) opt[Token]
```

the fixed spelling that starts at `pos` in NUL-terminated `source`, the longest one, as a token

## Spelling

```mach
pub rec Spelling {
    text: str;
    kind: Kind;
}
```

a token the source spells with fixed punctuation

