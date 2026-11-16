[mach](../README.md) / [front](README.md) / **lexis**

# mach.front.lexis

the reserved words and identifier characters the lexer, the parser and the migrator share

<sub>source: [`src/front/lexis.mach`](../../../src/front/lexis.mach) · imported by 4 modules</sub>

| Functions | |
| :--- | :--- |
| [`is_keyword`](#is_keyword) | `text[0..len]` is a reserved word |
| [`is_ident_start`](#is_ident_start) · [`is_ident_char`](#is_ident_char) | |
| [`is_identifier`](#is_identifier) | `text[0..len]` lexes as exactly one identifier token; keywords lex as identifiers too |

| Constants | |
| :--- | :--- |
| [`KW_ASM`](#constants) | the reserved words: each lexes as an identifier and is refused as a name |
| [`KW_BRK`](#constants) · [`KW_CNT`](#constants) · [`KW_DEF`](#constants) · [`KW_EACH`](#constants) · [`KW_ERROR`](#constants) · [`KW_EXT`](#constants) · [`KW_FIN`](#constants) · [`KW_FOR`](#constants) · [`KW_FUN`](#constants) · [`KW_FWD`](#constants) · [`KW_IF`](#constants) · [`KW_IN`](#constants) · [`KW_NIL`](#constants) · [`KW_OR`](#constants) · [`KW_PUB`](#constants) · [`KW_REC`](#constants) · [`KW_RET`](#constants) · [`KW_SEL`](#constants) · [`KW_TAG`](#constants) · [`KW_TEST`](#constants) · [`KW_UNI`](#constants) · [`KW_USE`](#constants) · [`KW_VAL`](#constants) · [`KW_VAR`](#constants) | |

---

## Functions

### is_keyword

```mach
pub fun is_keyword(text: str, len: usize) bool
```

`text[0..len]` is a reserved word

### is_ident_start

```mach
pub fun is_ident_start(c: u8) bool
```

### is_ident_char

```mach
pub fun is_ident_char(c: u8) bool
```

### is_identifier

```mach
pub fun is_identifier(text: str, len: usize) bool
```

`text[0..len]` lexes as exactly one identifier token; keywords lex as identifiers too

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `KW_ASM` | `str` | `"asm"` | the reserved words: each lexes as an identifier and is refused as a name |
| `KW_BRK` | `str` | `"brk"` |  |
| `KW_CNT` | `str` | `"cnt"` |  |
| `KW_DEF` | `str` | `"def"` |  |
| `KW_EACH` | `str` | `"each"` |  |
| `KW_ERROR` | `str` | `"error"` |  |
| `KW_EXT` | `str` | `"ext"` |  |
| `KW_FIN` | `str` | `"fin"` |  |
| `KW_FOR` | `str` | `"for"` |  |
| `KW_FUN` | `str` | `"fun"` |  |
| `KW_FWD` | `str` | `"fwd"` |  |
| `KW_IF` | `str` | `"if"` |  |
| `KW_IN` | `str` | `"in"` |  |
| `KW_NIL` | `str` | `"nil"` |  |
| `KW_OR` | `str` | `"or"` |  |
| `KW_PUB` | `str` | `"pub"` |  |
| `KW_REC` | `str` | `"rec"` |  |
| `KW_RET` | `str` | `"ret"` |  |
| `KW_SEL` | `str` | `"sel"` |  |
| `KW_TAG` | `str` | `"tag"` |  |
| `KW_TEST` | `str` | `"test"` |  |
| `KW_UNI` | `str` | `"uni"` |  |
| `KW_USE` | `str` | `"use"` |  |
| `KW_VAL` | `str` | `"val"` |  |
| `KW_VAR` | `str` | `"var"` |  |

