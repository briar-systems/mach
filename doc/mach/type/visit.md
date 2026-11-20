[mach](../README.md) / [type](README.md) / **visit**

# mach.type.visit

one walk over the type graph: a property is a leaf predicate, and the walk follows
the edges the property names from a type to the types it is built from. an aggregate is
entered once per scan, and a type id the store does not hold is an internal failure

Source: [`src/type/visit.mach`](../../../src/type/visit.mach)

Uses types from [`mach.report.phase`](../report/phase.md), [`mach.type`](../type.md), [`mach.type.kind`](kind.md), [`mach.type.scan`](scan.md).

| Type | | |
| :--- | :--- | :--- |
| [`Edges`](#edges) | `def` |  |
| [`Verdict`](#verdict) | `def` | what a leaf predicate says of one type |
| [`Walk`](#walk) | `rec` | one walk for one property |

| On `Edges` | |
| :--- | :--- |
| [`EDGE_BASE`](#edges) | a secret's base |
| [`EDGE_ELEMENT`](#edges) | an array's element |
| [`EDGE_POINTEE`](#edges) | |
| [`EDGE_SIGNATURE`](#edges) | a function's return, then its parameters |
| [`EDGE_ARGUMENT`](#edges) | an instance's arguments |
| [`EDGE_DECLARATION`](#edges) | the generic declaration an instance instantiates |
| [`EDGE_FIELD`](#edges) | a record's, union's or tag's declared fields |
| [`EDGES_VALUE`](#edges) | what a value holds in its own storage |
| [`EDGES_SPELLING`](#edges) | every type a type is spelled from, through pointers and signatures included |

| On `Verdict` | |
| :--- | :--- |
| [`DESCEND`](#verdict) | the property may lie below: follow the edges |
| [`PRUNE`](#verdict) | the property is not here and not below |
| [`FOUND`](#verdict) | the property is here, which ends the walk |

| On `Walk` | |
| :--- | :--- |
| [`reaches`](#reaches) | whether the property holds at `tid` or below it; false once the walk has failed |

---

## Edges

```mach
pub def Edges: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `EDGE_BASE` | `1` | a secret's base |
| `EDGE_ELEMENT` | `2` | an array's element |
| `EDGE_POINTEE` | `4` |  |
| `EDGE_SIGNATURE` | `8` | a function's return, then its parameters |
| `EDGE_ARGUMENT` | `16` | an instance's arguments |
| `EDGE_DECLARATION` | `32` | the generic declaration an instance instantiates |
| `EDGE_FIELD` | `64` | a record's, union's or tag's declared fields |
| `EDGES_VALUE` | `EDGE_BASE \| EDGE_ELEMENT \| EDGE_FIELD \| EDGE_INSTANCE_FIELD` | what a value holds in its own storage |
| `EDGES_SPELLING` | `EDGE_BASE \| EDGE_ELEMENT \| EDGE_POINTEE \| EDGE_SIGNATURE \| EDGE_ARGUMENT` | every type a type is spelled from, through pointers and signatures included |

## Verdict

```mach
pub def Verdict: u8
```

what a leaf predicate says of one type

| Name | Value | |
| :--- | :--- | :--- |
| `DESCEND` | `0` | the property may lie below: follow the edges |
| `PRUNE` | `1` | the property is not here and not below |
| `FOUND` | `2` | the property is here, which ends the walk |

## Walk

```mach
pub rec Walk[C] {
    types:  *mach.type.TypeInterner;
    status: *mach.report.phase.PhaseStatus;
    # the aggregates already entered; nil enters each every time it is met
    scan: *mach.type.scan.Scan;
    # with a scan, every type is entered once, not only an aggregate: for a property whose
    # answer below a type does not depend on the path the walk took to it, so a scan shared
    # across many roots visits each type once in all
    once:  bool;
    edges: Edges;
    ctx:   *C;
    leaf:  fun(*C, mach.type.kind.TypeId, *mach.type.Type) Verdict;
}
```

one walk for one property

| Field | Type | |
| :--- | :--- | :--- |
| `types` | `*mach.type.TypeInterner` | the store walked |
| `status` | `*mach.report.phase.PhaseStatus` | where an internal failure lands |
| `scan` | `*mach.type.scan.Scan` | the scan marking what was entered |
| `once` | `bool` | whether every type is entered once |
| `edges` | `Edges` | the edges the walk follows |
| `ctx` | `*C` | the property's context |
| `leaf` | `fun(*C, mach.type.kind.TypeId, *mach.type.Type) Verdict` | the property's verdict on each type the walk visits |

### reaches

```mach
pub fun reaches[C](w: *Walk[C], tid: mach.type.kind.TypeId) bool
```

whether the property holds at `tid` or below it; false once the walk has failed. the walk
keeps what it has yet to visit on a worklist rather than the stack, in the order a recursive
walk would visit it, so a type nested through flat defs deeper than any stack is still answered

