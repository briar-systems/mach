# mach.lang.type.query

questions about the types in the store whose answers derive from the field tables:
substitution, the deep properties, extent, secrecy agreement, containment and
expansiveness, each a module under this one. the facts worth keeping are cached in one
record that only this module reads or writes, stamped with the projection generation
they were derived under

## rec Facts

```mach
pub rec Facts;
```

the cached facts: whether each decided generic is expansive, and whether each nominal's
fields mention a type parameter. a lookup under a later generation drops them first

## fun facts_init

```mach
pub fun facts_init(a: *A.Allocator) Facts;
```

## fun facts_dnit

```mach
pub fun facts_dnit(f: *Facts);
```

## rec Query

```mach
pub rec Query;
```

one question put to the store: what it reads, the facts it may consult and record, and
where an internal failure lands

## fun descend

```mach
pub fun descend(q: *Query) bool;
```

whether a walk that builds types may descend another level: false once the stack has no
room left, which refuses the query and unwinds every walk under it

## fun nesting

```mach
pub fun nesting() fail.Fail;
```

the refusal of a query whose walk ran out of stack

## fun answer

```mach
pub fun answer[T](q: *Query, value: T) res[T, fail.Fail];
```

a query's answer, or the internal failure it met, or its refusal of the input

## fun settled

```mach
pub fun settled(q: *Query) err[fail.Fail];
```

a query that answers nothing but whether it met an internal failure or refused the input

## fun refused

```mach
pub fun refused(q: *Query, e: A.Error);
```

## fun failed

```mach
pub fun failed(q: *Query, f: fail.Fail);
```

## fun inherit

```mach
pub fun inherit(q: *Query, f: fail.Fail);
```

a failure a query asked under this one returned: its refusal of the input is this query's,
and any other failure is internal

## fun scan_begin

```mach
pub fun scan_begin(q: *Query, scan: *type.TypeScan) bool;
```

opens `scan` over the store; false, with the failure recorded, when it cannot

## fun scan_end

```mach
pub fun scan_end(q: *Query, scan: *type.TypeScan);
```

## fun scan_fresh

```mach
pub fun scan_fresh(q: *Query, scan: *type.TypeScan, tid: type.TypeId) bool;
```

whether `scan` meets `tid` for the first time; a mark that cannot be recorded is a
failure and reads as already seen, so no walk descends past it

## fun expansive_get

```mach
pub fun expansive_get(q: *Query, nominal: type.TypeId) opt[bool];
```

the recorded verdict on whether generic nominal `nominal` is expansive, none while undecided

## fun expansive_set

```mach
pub fun expansive_set(q: *Query, nominal: type.TypeId, expansive: bool);
```

records a verdict once; a nominal already decided keeps its first

## fun mentions_get

```mach
pub fun mentions_get(q: *Query, nominal: type.TypeId) opt[bool];
```

the recorded answer to whether `nominal`'s fields mention a type parameter

## fun mentions_set

```mach
pub fun mentions_set(q: *Query, nominal: type.TypeId, mentions: bool);
```

