[mach](../README.md) / [type](README.md) / **query**

# mach.type.query

questions about the types in the store whose answers derive from the field tables:
substitution, the deep properties, extent, secrecy agreement, containment and
expansiveness, each a module under this one. the facts worth keeping are cached in one
record that only this module reads or writes, stamped with the projection generation
they were derived under

Source: [`src/type/query.mach`](../../../src/type/query.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.report.phase`](../report/phase.md), [`mach.type`](../type.md), [`mach.type.kind`](kind.md), [`mach.type.scan`](scan.md).

| Type | | |
| :--- | :--- | :--- |
| [`Facts`](#facts) | `rec` | the cached facts: whether each decided generic is expansive, and whether each nominal's fields mention a type parameter |
| [`Query`](#query) | `rec` | one question put to the store: what it reads, the facts it may consult and record, and where an internal failure lands |

| On `Facts` | |
| :--- | :--- |
| [`facts_init`](#facts_init), [`facts_dnit`](#facts_dnit) | |

| On `Query` | |
| :--- | :--- |
| [`descend`](#descend) | whether a walk that builds types may descend another level: false once the stack has no room left, which refuses the query and unwinds every walk under it |
| [`answer`](#answer) | a query's answer, or the internal failure it met, or its refusal of the input |
| [`settled`](#settled) | a query that answers nothing but whether it met an internal failure or refused the input |
| [`refused`](#refused), [`failed`](#failed) | |
| [`inherit`](#inherit) | a failure a query asked under this one returned: its refusal of the input is this query's, and any other failure is internal |
| [`scan_begin`](#scan_begin) | opens `scan` over the store; false, with the failure recorded, when it cannot |
| [`scan_end`](#scan_end) | |
| [`scan_fresh`](#scan_fresh) | whether `scan` meets `tid` for the first time; a mark that cannot be recorded is a failure and reads as already seen, so no walk descends past it |
| [`expansive_get`](#expansive_get) | the recorded verdict on whether generic nominal `nominal` is expansive, none while undecided |
| [`expansive_set`](#expansive_set) | records a verdict once; a nominal already decided keeps its first |
| [`mentions_get`](#mentions_get) | the recorded answer to whether `nominal`'s fields mention a type parameter |
| [`mentions_set`](#mentions_set) | |

| Functions | |
| :--- | :--- |
| [`nesting`](#nesting) | the refusal of a query whose walk ran out of stack |

---

## Facts

```mach
pub rec Facts {
    generation: u64;
    expansive:  std.collections.map.Map[mach.type.kind.TypeId, bool];
    mentions:   std.collections.map.Map[mach.type.kind.TypeId, bool];
}
```

the cached facts: whether each decided generic is expansive, and whether each nominal's
fields mention a type parameter. a lookup under a later generation drops them first

### facts_init

```mach
pub fun facts_init(a: *std.allocator.Allocator) Facts
```

### facts_dnit

```mach
pub fun facts_dnit(f: *Facts)
```

## Query

```mach
pub rec Query {
    types:  *mach.type.TypeInterner;
    facts:  *Facts;
    status: *mach.report.phase.PhaseStatus;
    # a walk that builds types met one nested deeper than the stack lets it descend, so
    # the query has no answer and refuses the input (`nesting`), which the node that asked
    # reports
    nested: bool;
}
```

one question put to the store: what it reads, the facts it may consult and record, and
where an internal failure lands

### descend

```mach
pub fun descend(q: *Query) bool
```

whether a walk that builds types may descend another level: false once the stack has no
room left, which refuses the query and unwinds every walk under it

### answer

```mach
pub fun answer[T](q: *Query, value: T) res[T, mach.base.fail.Fail]
```

a query's answer, or the internal failure it met, or its refusal of the input

### settled

```mach
pub fun settled(q: *Query) err[mach.base.fail.Fail]
```

a query that answers nothing but whether it met an internal failure or refused the input

### refused

```mach
pub fun refused(q: *Query, e: std.allocator.Error)
```

### failed

```mach
pub fun failed(q: *Query, f: mach.base.fail.Fail)
```

### inherit

```mach
pub fun inherit(q: *Query, f: mach.base.fail.Fail)
```

a failure a query asked under this one returned: its refusal of the input is this query's,
and any other failure is internal

### scan_begin

```mach
pub fun scan_begin(q: *Query, scan: *mach.type.scan.Scan) bool
```

opens `scan` over the store; false, with the failure recorded, when it cannot

### scan_end

```mach
pub fun scan_end(q: *Query, scan: *mach.type.scan.Scan)
```

### scan_fresh

```mach
pub fun scan_fresh(q: *Query, scan: *mach.type.scan.Scan, tid: mach.type.kind.TypeId) bool
```

whether `scan` meets `tid` for the first time; a mark that cannot be recorded is a
failure and reads as already seen, so no walk descends past it

### expansive_get

```mach
pub fun expansive_get(q: *Query, nominal: mach.type.kind.TypeId) opt[bool]
```

the recorded verdict on whether generic nominal `nominal` is expansive, none while undecided

### expansive_set

```mach
pub fun expansive_set(q: *Query, nominal: mach.type.kind.TypeId, expansive: bool)
```

records a verdict once; a nominal already decided keeps its first

### mentions_get

```mach
pub fun mentions_get(q: *Query, nominal: mach.type.kind.TypeId) opt[bool]
```

the recorded answer to whether `nominal`'s fields mention a type parameter

### mentions_set

```mach
pub fun mentions_set(q: *Query, nominal: mach.type.kind.TypeId, mentions: bool)
```

## Functions

### nesting

```mach
pub fun nesting() mach.base.fail.Fail
```

the refusal of a query whose walk ran out of stack

