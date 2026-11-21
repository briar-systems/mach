[mach](../../README.md) / [front](../README.md) / [parser](README.md) / **iasm**

# mach.front.parser.iasm

the parse of an inline `asm` statement

the head after `asm` is masc's selection and the body is masc's syntax, so the lexer hands
each on as one raw token and both are kept as spans of the source. the lexer reported a
body it could not read or close, so a body no `}` follows is reported nowhere here

Source: [`src/front/parser/iasm.mach`](../../../../src/front/parser/iasm.mach)

Uses types from [`mach.front.ast.id`](../ast/id.md), [`mach.front.parser.state`](state.md).

| Functions | |
| :--- | :--- |
| [`parse_asm`](#parse_asm) | parse an `asm` statement, keeping its head and body as source spans |

---

## Functions

### parse_asm

```mach
pub fun parse_asm(p: *mach.front.parser.state.Parser, noreturn: bool) mach.front.ast.id.StmtId
```

parse an `asm` statement, keeping its head and body as source spans

| Parameter | |
| :--- | :--- |
| `p` | the parser, at the `asm` keyword |
| `noreturn` | a `#[noreturn]` marks it |
| **returns** | the statement, or STMT_NIL when the parse has ended |

