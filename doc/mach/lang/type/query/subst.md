# mach.lang.type.query.subst

a generic's body under type arguments: substitution, and the field table an instance reads

## fun apply

```mach
pub fun apply(q: *type_query.Query, body_type: type.TypeId, who: *type.GenericOwner, args: *type.TypeId, arg_count: u32) type.TypeId;
```

## fun ensure

```mach
pub fun ensure(q: *type_query.Query, tid: type.TypeId);
```

builds the field table of instance `tid` for the current projection, once

## fun apply_or_self

```mach
pub fun apply_or_self(q: *type_query.Query, tid: type.TypeId, who: *type.GenericOwner, args: *type.TypeId, arg_len: u32) type.TypeId;
```

`tid` under `who`'s arguments, or itself when there are none

## fun substitute

```mach
pub fun substitute(s: *session.Session, body_type: type.TypeId, who: *type.GenericOwner, args: *type.TypeId, arg_count: u32) res[type.TypeId, fail.Fail];
```

replace `who`'s parameters in `body_type` with `args`, position for position;
a parameter of any other declaration (an enclosing generic's, in an open
instance) stays, so a substituted type reads as the identity under a second pass

## fun ensure_fields

```mach
pub fun ensure_fields(s: *session.Session, tid: type.TypeId) err[fail.Fail];
```

