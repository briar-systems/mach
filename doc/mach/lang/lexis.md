# mach.lang.lexis

## val KW_ASM

```mach
pub val KW_ASM:   str = "asm"
```

the reserved words: each lexes as an identifier and is refused as a name

## val KW_BRK

```mach
pub val KW_BRK:   str = "brk"
```

## val KW_CNT

```mach
pub val KW_CNT:   str = "cnt"
```

## val KW_DEF

```mach
pub val KW_DEF:   str = "def"
```

## val KW_EACH

```mach
pub val KW_EACH:  str = "each"
```

## val KW_ERROR

```mach
pub val KW_ERROR: str = "error"
```

## val KW_EXT

```mach
pub val KW_EXT:   str = "ext"
```

## val KW_FIN

```mach
pub val KW_FIN:   str = "fin"
```

## val KW_FOR

```mach
pub val KW_FOR:   str = "for"
```

## val KW_FUN

```mach
pub val KW_FUN:   str = "fun"
```

## val KW_FWD

```mach
pub val KW_FWD:   str = "fwd"
```

## val KW_IF

```mach
pub val KW_IF:    str = "if"
```

## val KW_IN

```mach
pub val KW_IN:    str = "in"
```

## val KW_NIL

```mach
pub val KW_NIL:   str = "nil"
```

## val KW_OR

```mach
pub val KW_OR:    str = "or"
```

## val KW_PUB

```mach
pub val KW_PUB:   str = "pub"
```

## val KW_REC

```mach
pub val KW_REC:   str = "rec"
```

## val KW_RET

```mach
pub val KW_RET:   str = "ret"
```

## val KW_SEL

```mach
pub val KW_SEL:   str = "sel"
```

## val KW_TAG

```mach
pub val KW_TAG:   str = "tag"
```

## val KW_TEST

```mach
pub val KW_TEST:  str = "test"
```

## val KW_UNI

```mach
pub val KW_UNI:   str = "uni"
```

## val KW_USE

```mach
pub val KW_USE:   str = "use"
```

## val KW_VAL

```mach
pub val KW_VAL:   str = "val"
```

## val KW_VAR

```mach
pub val KW_VAR:   str = "var"
```

## fun is_keyword

```mach
pub fun is_keyword(text: str, len: usize) bool;
```

`text[0..len]` is a reserved word

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

