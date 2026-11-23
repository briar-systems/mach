[mach](../README.md) / [front](README.md) / **asm**

# mach.front.asm

the lexis of an inline-asm body: the instruction set its head names, whose source dialect
says where the body's comments and literals lie, and the `{name}` references outside them
that bind a local. everything else in the body is the instruction set's, and reaches its
assembler as written

Source: [`src/front/asm.mach`](../../../src/front/asm.mach)

Uses types from [`mach.base.source`](../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Ref`](#ref) | `rec` | one brace group of a body: its kind and the text between its braces |

| On `Ref` | |
| :--- | :--- |
| [`ref_next`](#ref_next) | the next brace group of `body` at or after `at`, outside any comment or literal of `dialect`; `at` moves past it |

| Functions | |
| :--- | :--- |
| [`dialect`](#dialect) | the one instruction set of masc's whose catalog reads the head at `head` of `source` as a selection, whose source dialect says where the body's comments and literals lie |

| Constants | |
| :--- | :--- |
| [`REF_END`](#constants) | the body has no more braces |
| [`REF_NAME`](#constants) | `{name}`, a local the block reads or writes |
| [`REF_EMPTY`](#constants) | `{}` |

---

## Ref

```mach
pub rec Ref {
    kind:  RefKind;
    inner: mach.base.source.Span;
}
```

one brace group of a body: its kind and the text between its braces

### ref_next

```mach
pub fun ref_next(dialect: *masc.core.source.Source, source: str, body: mach.base.source.Span, at: *usize) Ref
```

the next brace group of `body` at or after `at`, outside any comment or literal of
`dialect`; `at` moves past it. the lexer closed every brace of the body

| Parameter | |
| :--- | :--- |
| `dialect` | the body's source dialect |
| `source` | the text the body indexes |
| `body` | the body's span in `source` |
| `at` | the scan's offset into `source`, which starts at body.offset |

## Functions

### dialect

```mach
pub fun dialect(a: *std.allocator.Allocator, source: str, head: mach.base.source.Span) res[masc.isa.Reader, std.allocator.Error]
```

the one instruction set of masc's whose catalog reads the head at `head` of `source` as a
selection, whose source dialect says where the body's comments and literals lie. a head a
previous major wrote names the set of the selection that stands for it

| Parameter | |
| :--- | :--- |
| `a` | allocates the head's spelling and masc's lookup |
| `source` | the text the head indexes |
| `head` | the head's span in `source` |
| **returns** | the reader masc finds, or the allocator's refusal |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `REF_END` | `RefKind` | `0` | the body has no more braces |
| `REF_NAME` | `RefKind` | `1` | `{name}`, a local the block reads or writes |
| `REF_EMPTY` | `RefKind` | `3` | `{}` |

