# mach.lang.fe.token

## def Kind

```mach
pub def Kind: u8
```

## val KIND_IDENT

```mach
pub val KIND_IDENT:     Kind = 0
```

## val KIND_LIT_INT

```mach
pub val KIND_LIT_INT:   Kind = 1
```

## val KIND_LIT_FLOAT

```mach
pub val KIND_LIT_FLOAT: Kind = 2
```

## val KIND_LIT_CHAR

```mach
pub val KIND_LIT_CHAR:  Kind = 3
```

## val KIND_LIT_STR

```mach
pub val KIND_LIT_STR:   Kind = 4
```

## val KIND_LPAREN

```mach
pub val KIND_LPAREN:   Kind = 5
```

## val KIND_RPAREN

```mach
pub val KIND_RPAREN:   Kind = 6
```

## val KIND_LBRACE

```mach
pub val KIND_LBRACE:   Kind = 7
```

## val KIND_RBRACE

```mach
pub val KIND_RBRACE:   Kind = 8
```

## val KIND_LBRACKET

```mach
pub val KIND_LBRACKET: Kind = 9
```

## val KIND_RBRACKET

```mach
pub val KIND_RBRACKET: Kind = 10
```

## val KIND_SEMI

```mach
pub val KIND_SEMI:     Kind = 11
```

## val KIND_COLON

```mach
pub val KIND_COLON:    Kind = 12
```

## val KIND_COMMA

```mach
pub val KIND_COMMA:    Kind = 13
```

## val KIND_DOT

```mach
pub val KIND_DOT:      Kind = 14
```

## val KIND_QUESTION

```mach
pub val KIND_QUESTION: Kind = 15
```

## val KIND_AT

```mach
pub val KIND_AT:       Kind = 16
```

## val KIND_DOLLAR

```mach
pub val KIND_DOLLAR:   Kind = 17
```

## val KIND_PLUS

```mach
pub val KIND_PLUS:    Kind = 18
```

## val KIND_MINUS

```mach
pub val KIND_MINUS:   Kind = 19
```

## val KIND_STAR

```mach
pub val KIND_STAR:    Kind = 20
```

## val KIND_SLASH

```mach
pub val KIND_SLASH:   Kind = 21
```

## val KIND_PERCENT

```mach
pub val KIND_PERCENT: Kind = 22
```

## val KIND_AMP

```mach
pub val KIND_AMP:     Kind = 23
```

## val KIND_PIPE

```mach
pub val KIND_PIPE:    Kind = 24
```

## val KIND_CARET

```mach
pub val KIND_CARET:   Kind = 25
```

## val KIND_TILDE

```mach
pub val KIND_TILDE:   Kind = 26
```

## val KIND_EQ

```mach
pub val KIND_EQ:      Kind = 27
```

## val KIND_EQ_EQ

```mach
pub val KIND_EQ_EQ:   Kind = 28
```

## val KIND_BANG

```mach
pub val KIND_BANG:    Kind = 29
```

## val KIND_BANG_EQ

```mach
pub val KIND_BANG_EQ: Kind = 30
```

## val KIND_LT

```mach
pub val KIND_LT:      Kind = 31
```

## val KIND_LT_EQ

```mach
pub val KIND_LT_EQ:   Kind = 32
```

## val KIND_LT_LT

```mach
pub val KIND_LT_LT:   Kind = 33
```

## val KIND_GT

```mach
pub val KIND_GT:      Kind = 34
```

## val KIND_GT_EQ

```mach
pub val KIND_GT_EQ:   Kind = 35
```

## val KIND_GT_GT

```mach
pub val KIND_GT_GT:   Kind = 36
```

## val KIND_AMP_AMP

```mach
pub val KIND_AMP_AMP:   Kind = 37
```

## val KIND_PIPE_PIPE

```mach
pub val KIND_PIPE_PIPE: Kind = 38
```

## val KIND_COLON_COLON

```mach
pub val KIND_COLON_COLON: Kind = 39
```

## val KIND_DOT_DOT_DOT

```mach
pub val KIND_DOT_DOT_DOT: Kind = 40
```

## val KIND_COLON_TILDE

```mach
pub val KIND_COLON_TILDE: Kind = 41
```

## val KIND_ATTR_OPEN

```mach
pub val KIND_ATTR_OPEN: Kind = 42
```

## val KIND_COLON_GT

```mach
pub val KIND_COLON_GT:  Kind = 43
```

## val KIND_EOF

```mach
pub val KIND_EOF:   Kind = 254
```

## val KIND_ERROR

```mach
pub val KIND_ERROR: Kind = 255
```

## val KW_ASM

```mach
pub val KW_ASM:   str = "asm"
```

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

## rec Span

```mach
pub rec Span;
```

## rec Token

```mach
pub rec Token;
```

## fun make

```mach
pub fun make(kind: Kind, offset: usize, len: usize) Token;
```

## fun kind_str

```mach
pub fun kind_str(kind: Kind) opt[str];
```

the spelling of a kind; absent for a tag outside the catalog

## fun infix_precedence

```mach
pub fun infix_precedence(kind: Kind) u8;
```

the binding power of an infix operator token; 0 for every other kind, which
is not an operator (a partition, enumerated by its test)

## fun is_right_assoc

```mach
pub fun is_right_assoc(kind: Kind) bool;
```

