[mach](../README.md) / [front](README.md) / **asm**

# mach.front.asm

the lexis of an inline-asm body that the language owns: its `#` comments, and the
`{name}` references that bind a local. everything else in the body is the instruction
set's, and reaches its assembler as written

<sub>source: [`src/front/asm.mach`](../../../src/front/asm.mach) · imported by 2 modules</sub>

<sub>`RefKind` is private · `base_source` is [`mach.base.source`](../base/source.md) · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Ref`](#ref) | `rec` | one brace group of a body: its kind and the text between its braces |

| On `Ref` | |
| :--- | :--- |
| [`ref_next`](#ref_next) | the next brace group of `body` at or after `at`, outside any comment; `at` moves past it |

| Functions | |
| :--- | :--- |
| [`comments_strip`](#comments_strip) | the body with each comment blanked to spaces, the text an assembler reads, every offset the body's own; owned by `a`, body.len + 1 bytes |

| Constants | |
| :--- | :--- |
| [`REF_END`](#constants) | the body has no more braces |
| [`REF_NAME`](#constants) | `{name}`, a local the block reads or writes |
| [`REF_UNTERMINATED`](#constants) | a `{` the body never closes |
| [`REF_EMPTY`](#constants) | `{}` |

---

## Ref

```mach
pub rec Ref {
    kind:  RefKind;
    inner: base_source.Span;
}
```

one brace group of a body: its kind and the text between its braces

### ref_next

```mach
pub fun ref_next(source: str, body: base_source.Span, at: *usize) Ref
```

the next brace group of `body` at or after `at`, outside any comment; `at` moves past it

| Parameter | |
| :--- | :--- |
| `source` | the text the body indexes |
| `body` | the body's span in `source` |
| `at` | the scan's offset into `source`, which starts at body.offset |

## Functions

### comments_strip

```mach
pub fun comments_strip(a: *A.Allocator, source: str, body: base_source.Span) res[str, fail.Fail]
```

the body with each comment blanked to spaces, the text an assembler reads, every offset
the body's own; owned by `a`, body.len + 1 bytes

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `REF_END` | `RefKind` | `0` | the body has no more braces |
| `REF_NAME` | `RefKind` | `1` | `{name}`, a local the block reads or writes |
| `REF_UNTERMINATED` | `RefKind` | `3` | a `{` the body never closes |
| `REF_EMPTY` | `RefKind` | `4` | `{}` |

