# mach.lang.type.query.mention

which type parameters a type mentions, and the declaration that owns them

## fun mentioned

```mach
pub fun mentioned(q: *type_query.Query, tid: type.TypeId) bool;
```

whether `tid` mentions a type parameter anywhere it is spelled

## fun owner

```mach
pub fun owner(q: *type_query.Query, tid: type.TypeId) opt[type.GenericOwner];
```

the declaration whose parameters a nominal's fields mention: the nominal itself for a
named generic, the enclosing declaration for an anonymous aggregate declared inside one;
none when the fields mention no parameter

## fun has_param

```mach
pub fun has_param(q: *type_query.Query, tid: type.TypeId) bool;
```

whether a nominal's fields mention a type parameter, which a nominal with no field
table cannot be shown not to

