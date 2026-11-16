[mach](../../README.md) / [type](../README.md) / [query](README.md) / **secrecy**

# mach.type.query.secrecy

whether two types laid over one storage keep every byte's secrecy class: a cast or
retype from one to the other, or two union variants

<sub>source: [`src/type/query/secrecy.mach`](../../../../src/type/query/secrecy.mach) · imported by 4 modules</sub>

<sub>`Agreement` is private · `type_kind` is [`mach.type.kind`](../kind.md) · `extent` is [`mach.type.query.extent`](extent.md) · `session` is [`mach.session`](../../session.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

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
| [`AGREE`](#constants) · [`DISAGREE`](#constants) · [`UNDECIDABLE`](#constants) · [`NO_FIELD`](#constants) · [`UNBOUNDED_VARIANTS_MSG`](#constants) · [`UNBOUNDED_CAST_MSG`](#constants) | |

---

## Verdict

```mach
pub rec Verdict {
    agreement: Agreement;
    owner:     type_kind.TypeId;
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
pub fun overlay_walk(mz: *extent.Measure, a: type_kind.TypeId, b: type_kind.TypeId) Verdict
```

two types laid over one storage from offset 0, as union variants are: every
byte both cover has one class on both sides

### cast_allowed

```mach
pub fun cast_allowed(s: *session.Session, laying: *extent.Laying, from: type_kind.TypeId, to: type_kind.TypeId) res[Verdict, fail.Fail]
```

whether a `::` or `:~` from `from` to `to` keeps every byte's secrecy class or only makes public bytes secret in a value

### overlay_agrees

```mach
pub fun overlay_agrees(s: *session.Session, laying: *extent.Laying, a: type_kind.TypeId, b: type_kind.TypeId) res[Verdict, fail.Fail]
```

whether two union variants overlaying one storage agree on every byte they share

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `AGREE` | `Agreement` | `0` |  |
| `DISAGREE` | `Agreement` | `1` |  |
| `UNDECIDABLE` | `Agreement` | `2` |  |
| `NO_FIELD` | `u32` | `0xFFFFFFFF` |  |
| `UNBOUNDED_VARIANTS_MSG` | `str` | `"union variants: cannot prove that overlapping fields agree on secrecy"` |  |
| `UNBOUNDED_CAST_MSG` | `str` | `"cannot prove that the two types agree on secrecy"` |  |

