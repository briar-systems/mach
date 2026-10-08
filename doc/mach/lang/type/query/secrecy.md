# mach.lang.type.query.secrecy

whether two types laid over one storage keep every byte's secrecy class: a cast or
retype from one to the other, or two union variants

## def Agreement

```mach
pub def Agreement: u8
```

how two storages compare on secrecy: they agree, they disagree, or the
comparison would follow pointers into an expansive generic, whose instances
are unbounded, so it cannot be decided and is refused

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

## val UNBOUNDED_VARIANTS_MSG

```mach
pub val UNBOUNDED_VARIANTS_MSG: str = "union variants: cannot prove that overlapping fields agree on secrecy - they point into a generic whose argument grows through a pointer, which has unboundedly many instances, so the storage below the pointers cannot be compared"
```

## val UNBOUNDED_CAST_MSG

```mach
pub val UNBOUNDED_CAST_MSG:     str = "cannot prove that the two types agree on secrecy - they point into a generic whose argument grows through a pointer, which has unboundedly many instances, so the storage below the pointers cannot be compared"
```

## fun cast_allowed

```mach
pub fun cast_allowed(s: *session.Session, m: layout.Machine, from: type.TypeId, to: type.TypeId) res[Agreement, fail.Fail];
```

whether a `::` or `:~` from `from` to `to` keeps every byte's secrecy class or only makes public bytes secret

## fun overlay_agrees

```mach
pub fun overlay_agrees(s: *session.Session, m: layout.Machine, a: type.TypeId, b: type.TypeId) res[Agreement, fail.Fail];
```

whether two union variants overlaying one storage agree on every byte they share

