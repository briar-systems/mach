[mach](../../README.md) / [base](../README.md) / [diagnostic](README.md) / **annotation**

# mach.base.diagnostic.annotation

the related places and machine-applicable fixes a report names beyond its own location

a failure and a diagnostic share these. each place is a span of a file of the source map the
report was raised against.

Source: [`src/base/diagnostic/annotation.mach`](../../../../src/base/diagnostic/annotation.mach)

Uses types from [`mach.base.source`](../source.md).

| Type | | |
| :--- | :--- | :--- |
| [`ChildKind`](#childkind) | `def` |  |
| [`Child`](#child) | `rec` | a note or a help line attached to a diagnostic |
| [`Related`](#related) | `rec` | another place a report concerns: the other edge of a cycle, the other claim of a collision |
| [`Edit`](#edit) | `rec` | one rewrite of a fix: the bytes at `loc` become `replacement` |
| [`Fix`](#fix) | `rec` | a machine-applicable rewrite of the input, applied whole |

| On `ChildKind` | |
| :--- | :--- |
| [`CHILD_NOTE`](#childkind), [`CHILD_HELP`](#childkind) | |

| On `Related` | |
| :--- | :--- |
| [`related_dnit`](#related_dnit) | release a related place's label, allocated through `a` |

| On `Fix` | |
| :--- | :--- |
| [`fix_none`](#fix_none) | a fix carrying no rewrite |
| [`fix_dnit`](#fix_dnit) | release a fix's label, each edit's replacement and the edit storage, allocated through `a` |
| [`fix_copy`](#fix_copy) | a copy of `src` owning its label, its edits and their replacements through `a`, released with `fix_dnit` |

---

## ChildKind

```mach
pub def ChildKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CHILD_NOTE` | `0` |  |
| `CHILD_HELP` | `1` |  |

## Child

```mach
pub rec Child {
    kind: ChildKind;
    text: str;
}
```

a note or a help line attached to a diagnostic

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `ChildKind` | CHILD_NOTE or CHILD_HELP |
| `text` | `str` | the line, owned by the allocator of the report that holds it |

## Related

```mach
pub rec Related {
    loc:   mach.base.source.Location;
    label: str;
}
```

another place a report concerns: the other edge of a cycle, the other claim of a collision

| Field | Type | |
| :--- | :--- | :--- |
| `loc` | `mach.base.source.Location` | where it is |
| `label` | `str` | what it is, nil to show the place without one |

### related_dnit

```mach
pub fun related_dnit(a: *std.allocator.Allocator, r: *Related)
```

release a related place's label, allocated through `a`

## Edit

```mach
pub rec Edit {
    loc:         mach.base.source.Location;
    replacement: str;
}
```

one rewrite of a fix: the bytes at `loc` become `replacement`

## Fix

```mach
pub rec Fix {
    label: str;
    edits: Vector[Edit];
}
```

a machine-applicable rewrite of the input, applied whole

| Field | Type | |
| :--- | :--- | :--- |
| `label` | `str` | what the rewrite does, as a fix line names it; nil is no fix |
| `edits` | `Vector[Edit]` | the rewrites, none overlapping another |

### fix_none

```mach
pub fun fix_none() Fix
```

a fix carrying no rewrite

### fix_dnit

```mach
pub fun fix_dnit(a: *std.allocator.Allocator, fx: *Fix)
```

release a fix's label, each edit's replacement and the edit storage, allocated through `a`

### fix_copy

```mach
pub fun fix_copy(src: *Fix, a: *std.allocator.Allocator) res[Fix, std.allocator.Error]
```

a copy of `src` owning its label, its edits and their replacements through `a`, released with
`fix_dnit`

