# mach.lang.type.query.logical

the types a logical-addressing target can declare. such a target has no untyped pointer,
so a pointer is a reference to the type it points at and every type is declared in full.
a cycle closes only where a pointer member of a record or union points back at a record
or union on the cycle, which names that record rather than spelling it again, and a
reference into a generic whose argument grows through a pointer names a fresh type at every
step, so it has no declaration

## def Refusal

```mach
pub def Refusal: u8
```

## val REFUSAL_NONE

```mach
pub val REFUSAL_NONE: Refusal = 0
```

## val REFUSAL_RECURSIVE

```mach
pub val REFUSAL_RECURSIVE: Refusal = 1
```

a type reaches itself other than through a pointer that names a record on the cycle

## val REFUSAL_UNBOUNDED

```mach
pub val REFUSAL_UNBOUNDED: Refusal = 2
```

a reference into a generic whose argument grows through a pointer

## val RECURSIVE_MSG

```mach
pub val RECURSIVE_MSG: str = "recursive reference types require pointer capabilities absent from the Logical Shader environment"
```

## val UNBOUNDED_MSG

```mach
pub val UNBOUNDED_MSG: str = "a reference into a generic whose argument grows through a pointer has unboundedly many types, which the Logical Shader environment cannot declare"
```

## rec Path

```mach
pub rec Path;
```

the types a declaration is being spelled through, innermost first

## fun pointer_names

```mach
pub fun pointer_names(q: *type_query.Query, path: *Path, ptr: type.TypeId) opt[type.TypeId];
```

the record or union a pointer to `ptr`'s base names instead of spelling it, when that
base lies on a cycle with the record or union `path` is spelling it inside

## fun on_cycle

```mach
pub fun on_cycle(q: *type_query.Query, tid: type.TypeId) bool;
```

whether record or union `tid` reaches itself through its members

## fun refusal

```mach
pub fun refusal(q: *type_query.Query, tid: type.TypeId) Refusal;
```

why a logical-addressing target cannot declare `tid`, REFUSAL_NONE when it can

