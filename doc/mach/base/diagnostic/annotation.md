[mach](../../README.md) / [base](../README.md) / [diagnostic](README.md) / **annotation**

# mach.base.diagnostic.annotation

the related places and machine-applicable fixes a report names beyond its own location

a failure and a diagnostic share these. each place is a span of a file of the source map the
report was raised against.

<sub>source: [`src/base/diagnostic/annotation.mach`](../../../../src/base/diagnostic/annotation.mach) · imported by 4 modules</sub>

<sub>`base_source` is [`mach.base.source`](../source.md) · `A` is `std.allocator`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Related`](#related) | `rec` | another place a report concerns: the other edge of a cycle, the other claim of a collision |
| [`Edit`](#edit) | `rec` | one rewrite of a fix: the bytes at `loc` become `replacement` |
| [`Fix`](#fix) | `rec` | a machine-applicable rewrite of the input, applied whole |

| On `Related` | |
| :--- | :--- |
| [`related_dnit`](#related_dnit) | release a related place's label, allocated through `a` |

| On `Fix` | |
| :--- | :--- |
| [`fix_none`](#fix_none) | a fix carrying no rewrite |
| [`fix_present`](#fix_present) | whether the fix carries a rewrite |
| [`fix_dnit`](#fix_dnit) | release a fix's label, each edit's replacement and the edit storage, allocated through `a` |
| [`fix_copy`](#fix_copy) | a copy of `src` owning its label, its edits and their replacements through `a`, released with `fix_dnit` |

---

## Related

```mach
pub rec Related {
    loc:   base_source.Location;
    label: str;
}
```

another place a report concerns: the other edge of a cycle, the other claim of a collision

| Field | Type | |
| :--- | :--- | :--- |
| `loc` | `base_source.Location` | where it is |
| `label` | `str` | what it is, nil to show the place without one |

### related_dnit

```mach
pub fun related_dnit(a: *A.Allocator, r: *Related)
```

release a related place's label, allocated through `a`

## Edit

```mach
pub rec Edit {
    loc:         base_source.Location;
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

### fix_present

```mach
pub fun fix_present(fx: *Fix) bool
```

whether the fix carries a rewrite

### fix_dnit

```mach
pub fun fix_dnit(a: *A.Allocator, fx: *Fix)
```

release a fix's label, each edit's replacement and the edit storage, allocated through `a`

### fix_copy

```mach
pub fun fix_copy(src: *Fix, a: *A.Allocator) res[Fix, A.Error]
```

a copy of `src` owning its label, its edits and their replacements through `a`, released with
`fix_dnit`

