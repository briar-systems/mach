[mach](../../README.md) / [type](../README.md) / [query](README.md) / **secrecy**

# mach.type.query.secrecy

whether two types laid over one storage keep every byte's secrecy class: a cast or
retype from one to the other, or two union variants

Source: [`src/type/query/secrecy.mach`](../../../../src/type/query/secrecy.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.session`](../../session.md), [`mach.type.kind`](../kind.md), [`mach.type.query.extent`](extent.md).

| Type | | |
| :--- | :--- | :--- |
| [`Verdict`](#verdict) | `rec` | an answer and the field it turned on |

| On `Verdict` | |
| :--- | :--- |
| [`overlay_walk`](#overlay_walk) | two types laid over one storage from offset 0, as union variants are: every byte both cover has one class on both sides |
| [`cast_allowed`](#cast_allowed) | whether a `::` or `:~` from `from` to `to` keeps every byte's secrecy class or only makes public bytes secret in a value |
| [`overlay_agrees`](#overlay_agrees) | whether two union variants overlaying one storage agree on every byte they share |

| Constants | |
| :--- | :--- |
| [`AGREE`](#constants), [`DISAGREE`](#constants), [`UNDECIDABLE`](#constants), [`NO_FIELD`](#constants), [`UNBOUNDED_VARIANTS_MSG`](#constants), [`UNBOUNDED_CAST_MSG`](#constants) | |

---

## Verdict

```mach
pub rec Verdict {
    agreement: Agreement;
    owner:     mach.type.kind.TypeId;
    field:     u32;
}
```

an answer and the field it turned on. for a disagreement, `owner` is the aggregate whose
field `field` holds the first byte whose class differs; for an undecidable comparison it
is the instance that grows, with `field` the field whose laid arguments differ in size or
alignment between the sides, or NO_FIELD when the other side holds storage of another
shape. `owner` is TYPE_NIL when no field is known

### overlay_walk

```mach
pub fun overlay_walk(mz: *mach.type.query.extent.Measure, a: mach.type.kind.TypeId, b: mach.type.kind.TypeId) Verdict
```

two types laid over one storage from offset 0, as union variants are: every
byte both cover has one class on both sides

### cast_allowed

```mach
pub fun cast_allowed(s: *mach.session.Session, laying: *mach.type.query.extent.Laying, from: mach.type.kind.TypeId, to: mach.type.kind.TypeId) res[Verdict, mach.base.fail.Fail]
```

whether a `::` or `:~` from `from` to `to` keeps every byte's secrecy class or only makes public bytes secret in a value

### overlay_agrees

```mach
pub fun overlay_agrees(s: *mach.session.Session, laying: *mach.type.query.extent.Laying, a: mach.type.kind.TypeId, b: mach.type.kind.TypeId) res[Verdict, mach.base.fail.Fail]
```

whether two union variants overlaying one storage agree on every byte they share

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `AGREE` | `Agreement` | `0` |  |
| `DISAGREE` | `Agreement` | `1` |  |
| `UNDECIDABLE` | `Agreement` | `2` |  |
| `NO_FIELD` | `u32` | `0xFFFFFFFF` |  |
| `UNBOUNDED_VARIANTS_MSG` | `str` | `"cannot prove that overlapping union fields agree on secrecy"` |  |
| `UNBOUNDED_CAST_MSG` | `str` | `"cannot prove that the two types agree on secrecy"` |  |

