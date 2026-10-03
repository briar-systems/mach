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

## rec Token

```mach
pub rec Token;
```

## fun make

```mach
pub fun make(kind: Kind, offset: usize, len: usize) Token;
```

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

