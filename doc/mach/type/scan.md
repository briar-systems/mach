[mach](../README.md) / [type](README.md) / **scan**

# mach.type.scan

the visited sets of nested scans over the type table

each open scan takes the next nesting level and marks a type, or a pair of types, seen once in
that scan. a level's set is reused from scan to scan by bumping its epoch rather than clearing
it.

<sub>source: [`src/type/scan.mach`](../../../src/type/scan.mach) · imported by 14 modules</sub>

<sub>`A` is `std.allocator` · `VisitSet` is private · `PairSet` is private · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Sets`](#sets) | `rec` | the visited sets of every scan over the type table, one per nesting level, reused from scan to scan by bumping the level's epoch |
| [`Scan`](#scan) | `rec` | a scan over types, open from begin to end |
| [`Pair`](#pair) | `rec` | a scan over pairs of types, open from pair_begin to pair_end |
| [`Mark`](#mark-1) | `def` |  |

| On `Sets` | |
| :--- | :--- |
| [`sets_init`](#sets_init) · [`sets_dnit`](#sets_dnit) · [`begin`](#begin) · [`end`](#end) · [`mark`](#mark) · [`pair_begin`](#pair_begin) · [`pair_end`](#pair_end) · [`pair_mark`](#pair_mark) | |

| On `Mark` | |
| :--- | :--- |
| [`VISIT_FRESH`](#mark-1) · [`VISIT_SEEN`](#mark-1) | |

---

## Sets

```mach
pub rec Sets {
    a: *A.Allocator;

    visits:     *VisitSet;
    visits_len: u32;
    visits_cap: u32;

    pairs:     *PairSet;
    pairs_len: u32;
    pairs_cap: u32;
}
```

the visited sets of every scan over the type table, one per nesting level,
reused from scan to scan by bumping the level's epoch

### sets_init

```mach
pub fun sets_init(a: *A.Allocator) Sets
```

### sets_dnit

```mach
pub fun sets_dnit(sets: *Sets)
```

### begin

```mach
pub fun begin(sets: *Sets, scan: *Scan) err[fail.Fail]
```

### end

```mach
pub fun end(sets: *Sets, scan: *Scan)
```

### mark

```mach
pub fun mark(sets: *Sets, scan: *Scan, tid: TypeId) res[Mark, fail.Fail]
```

### pair_begin

```mach
pub fun pair_begin(sets: *Sets, scan: *Pair) err[fail.Fail]
```

### pair_end

```mach
pub fun pair_end(sets: *Sets, scan: *Pair)
```

### pair_mark

```mach
pub fun pair_mark(sets: *Sets, scan: *Pair, a: TypeId, b: TypeId) res[Mark, fail.Fail]
```

## Scan

```mach
pub rec Scan {
    level: u32;
}
```

a scan over types, open from begin to end

## Pair

```mach
pub rec Pair {
    level: u32;
}
```

a scan over pairs of types, open from pair_begin to pair_end

## Mark

```mach
pub def Mark: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `VISIT_FRESH` | `0` |  |
| `VISIT_SEEN` | `1` |  |

