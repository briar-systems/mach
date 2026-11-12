# mach.lang.type.query.secrecy

whether two types laid over one storage keep every byte's secrecy class: a cast or
retype from one to the other, or two union variants

## val AGREE

```mach
pub val AGREE:       Agreement = 0
```

## val DISAGREE

```mach
pub val DISAGREE:    Agreement = 1
```

## val UNDECIDABLE

```mach
pub val UNDECIDABLE: Agreement = 2
```

## rec Verdict

```mach
pub rec Verdict;
```

an answer and the field it turned on. for a disagreement, `owner` is the aggregate whose
field `field` holds the first byte whose class differs; for an undecidable comparison it
is the instance that grows, with `field` the field whose laid arguments differ in size or
alignment between the sides, or NO_FIELD when the other side holds storage of another
shape. `owner` is TYPE_NIL when no field is known

## val NO_FIELD

```mach
pub val NO_FIELD: u32 = 0xFFFFFFFF
```

## fun overlay_walk

```mach
pub fun overlay_walk(mz: *extent.Measure, a: type_kind.TypeId, b: type_kind.TypeId) Verdict;
```

two types laid over one storage from offset 0, as union variants are: every
byte both cover has one class on both sides

## val UNBOUNDED_VARIANTS_MSG

```mach
pub val UNBOUNDED_VARIANTS_MSG: str = "union variants: cannot prove that overlapping fields agree on secrecy"
```

## val UNBOUNDED_CAST_MSG

```mach
pub val UNBOUNDED_CAST_MSG:     str = "cannot prove that the two types agree on secrecy"
```

## fun cast_allowed

```mach
pub fun cast_allowed(s: *session.Session, laying: *extent.Laying, from: type_kind.TypeId, to: type_kind.TypeId) res[Verdict, fail.Fail];
```

whether a `::` or `:~` from `from` to `to` keeps every byte's secrecy class or only makes public bytes secret in a value

## fun overlay_agrees

```mach
pub fun overlay_agrees(s: *session.Session, laying: *extent.Laying, a: type_kind.TypeId, b: type_kind.TypeId) res[Verdict, fail.Fail];
```

whether two union variants overlaying one storage agree on every byte they share

