# mach.lang.type.visit

one walk over the type graph: a property is a leaf predicate, and the walk follows
the edges the property names from a type to the types it is built from. an aggregate is
entered once per scan, and a type id the store does not hold is an internal failure

## def Edges

```mach
pub def Edges: u8
```

## val EDGE_BASE

```mach
pub val EDGE_BASE: Edges = 1
```

a secret's base

## val EDGE_ELEMENT

```mach
pub val EDGE_ELEMENT: Edges = 2
```

an array's element

## val EDGE_POINTEE

```mach
pub val EDGE_POINTEE: Edges = 4
```

## val EDGE_SIGNATURE

```mach
pub val EDGE_SIGNATURE: Edges = 8
```

a function's return, then its parameters

## val EDGE_ARGUMENT

```mach
pub val EDGE_ARGUMENT: Edges = 16
```

an instance's arguments

## val EDGE_DECLARATION

```mach
pub val EDGE_DECLARATION: Edges = 32
```

the generic declaration an instance instantiates

## val EDGE_FIELD

```mach
pub val EDGE_FIELD: Edges = 64
```

a record's, union's or tag's declared fields

## val EDGES_VALUE

```mach
pub val EDGES_VALUE: Edges = EDGE_BASE | EDGE_ELEMENT | EDGE_FIELD | EDGE_INSTANCE_FIELD
```

what a value holds in its own storage

## val EDGES_SPELLING

```mach
pub val EDGES_SPELLING: Edges = EDGE_BASE | EDGE_ELEMENT | EDGE_POINTEE | EDGE_SIGNATURE | EDGE_ARGUMENT
```

every type a type is spelled from, through pointers and signatures included

## def Verdict

```mach
pub def Verdict: u8
```

what a leaf predicate says of one type

## val DESCEND

```mach
pub val DESCEND: Verdict = 0
```

the property may lie below: follow the edges

## val PRUNE

```mach
pub val PRUNE: Verdict = 1
```

the property is not here and not below

## val FOUND

```mach
pub val FOUND: Verdict = 2
```

the property is here, which ends the walk

## rec Walk

```mach
pub rec Walk[C];
```

## fun reaches

```mach
pub fun reaches[C](w: *Walk[C], tid: type.TypeId) bool;
```

whether the property holds at `tid` or below it; false once the walk has failed

