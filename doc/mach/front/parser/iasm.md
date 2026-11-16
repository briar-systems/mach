[mach](../../README.md) / [front](../README.md) / [parser](README.md) / **iasm**

# mach.front.parser.iasm

the parse of an inline `asm` statement

the head after `asm` is masc's selection and the body is masc's syntax, so both are kept as
spans of the source. the head's tokens are read only for where it ends, and the body is found
by matching its braces in the text.

<sub>source: [`src/front/parser/iasm.mach`](../../../../src/front/parser/iasm.mach) · imported by 1 module</sub>

<sub>`state` is [`mach.front.parser.state`](state.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md)</sub>

| Functions | |
| :--- | :--- |
| [`parse_asm`](#parse_asm) | |

---

## Functions

### parse_asm

```mach
pub fun parse_asm(p: *state.Parser) ast_id.StmtId
```

