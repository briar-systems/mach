[mach](../README.md) / [type](README.md) / **scan**

# mach.type.scan

the visited sets of nested scans over the type table

each open scan takes the next nesting level and marks a type, or a pair of types, seen once in
that scan. a level's set is reused from scan to scan by bumping its epoch rather than clearing
it.

Source: [`src/type/scan.mach`](../../../src/type/scan.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Sets`](#sets) | `rec` | the visited sets of every scan over the type table, one per nesting level, reused from scan to scan by bumping the level's epoch |
| [`Scan`](#scan) | `rec` | a scan over types, open from begin to end |
| [`Pair`](#pair) | `rec` | a scan over pairs of types, open from pair_begin to pair_end |
| [`Mark`](#mark-1) | `def` |  |

| On `Sets` | |
| :--- | :--- |
| [`sets_init`](#sets_init) | empty visited sets |
| [`sets_dnit`](#sets_dnit) | release every visited set |
| [`begin`](#begin) | open a scan at the next nesting level, with nothing marked |
| [`end`](#end) | close a scan and every scan opened inside it, a closed scan left as it is |
| [`mark`](#mark) | mark a type seen in an open scan |
| [`pair_begin`](#pair_begin) | open a pair scan at the next nesting level, with nothing marked |
| [`pair_end`](#pair_end) | close a pair scan and every pair scan opened inside it, a closed scan left as it is |
| [`pair_mark`](#pair_mark) | mark an ordered pair of types seen in an open pair scan |

| On `Mark` | |
| :--- | :--- |
| [`VISIT_FRESH`](#mark-1), [`VISIT_SEEN`](#mark-1) | |

---

## Sets

```mach
pub rec Sets {
    a: *std.allocator.Allocator;

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
pub fun sets_init(a: *std.allocator.Allocator) Sets
```

empty visited sets

| Parameter | |
| :--- | :--- |
| `a` | the allocator the sets are held in |
| **returns** | the sets, released by sets_dnit |

### sets_dnit

```mach
pub fun sets_dnit(sets: *Sets)
```

release every visited set

| Parameter | |
| :--- | :--- |
| `sets` | the sets |

### begin

```mach
pub fun begin(sets: *Sets, scan: *Scan) err[mach.base.fail.Fail]
```

open a scan at the next nesting level, with nothing marked

| Parameter | |
| :--- | :--- |
| `sets` | the sets |
| `scan` | the scan to open |
| **returns** | ok, or the failure growing the levels, which leaves the scan closed |

### end

```mach
pub fun end(sets: *Sets, scan: *Scan)
```

close a scan and every scan opened inside it, a closed scan left as it is

| Parameter | |
| :--- | :--- |
| `sets` | the sets |
| `scan` | the scan |

### mark

```mach
pub fun mark(sets: *Sets, scan: *Scan, tid: TypeId) res[Mark, mach.base.fail.Fail]
```

mark a type seen in an open scan

| Parameter | |
| :--- | :--- |
| `sets` | the sets |
| `scan` | the open scan |
| `tid` | the type met |
| **returns** | VISIT_FRESH the first time, VISIT_SEEN after and for nil or the error type, or a failure for a closed scan or a set that cannot grow |

### pair_begin

```mach
pub fun pair_begin(sets: *Sets, scan: *Pair) err[mach.base.fail.Fail]
```

open a pair scan at the next nesting level, with nothing marked

| Parameter | |
| :--- | :--- |
| `sets` | the sets |
| `scan` | the scan to open |
| **returns** | ok, or the failure growing the levels, which leaves the scan closed |

### pair_end

```mach
pub fun pair_end(sets: *Sets, scan: *Pair)
```

close a pair scan and every pair scan opened inside it, a closed scan left as it is

| Parameter | |
| :--- | :--- |
| `sets` | the sets |
| `scan` | the scan |

### pair_mark

```mach
pub fun pair_mark(sets: *Sets, scan: *Pair, a: TypeId, b: TypeId) res[Mark, mach.base.fail.Fail]
```

mark an ordered pair of types seen in an open pair scan

| Parameter | |
| :--- | :--- |
| `sets` | the sets |
| `scan` | the open scan |
| `a` | the first type |
| `b` | the second type |
| **returns** | VISIT_FRESH the first time, VISIT_SEEN after, or a failure for a closed scan or a set that cannot grow |

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

