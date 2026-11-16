[mach](../../README.md) / [front](../README.md) / [parser](README.md) / **iasm**

# mach.front.parser.iasm

the parse of an inline `asm` statement

the head after `asm` is masc's selection and the body is masc's syntax, so both are kept as
spans of the source. the head's tokens are read only for where it ends, and the body is found
by matching its braces in the text.

Source: [`src/front/parser/iasm.mach`](../../../../src/front/parser/iasm.mach)

Uses types from [`mach.front.ast.id`](../ast/id.md), [`mach.front.parser.state`](state.md).

| Functions | |
| :--- | :--- |
| [`parse_asm`](#parse_asm) | |

---

## Functions

### parse_asm

```mach
pub fun parse_asm(p: *mach.front.parser.state.Parser) mach.front.ast.id.StmtId
```

