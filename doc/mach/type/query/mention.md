[mach](../../README.md) / [type](../README.md) / [query](README.md) / **mention**

# mach.type.query.mention

which type parameters a type mentions, and the declaration that owns them

Source: [`src/type/query/mention.mach`](../../../../src/type/query/mention.mach)

Uses types from [`mach.type`](../../type.md), [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

| Functions | |
| :--- | :--- |
| [`mentioned`](#mentioned) | whether `tid` mentions a type parameter anywhere it is spelled |
| [`owner`](#owner) | the declaration whose parameters a nominal's fields mention: the nominal itself for a named generic, the enclosing declaration for an anonymous aggregate declared inside one; none when the fields mention no parameter |
| [`has_param`](#has_param) | whether a nominal's fields mention a type parameter, which a nominal with no field table cannot be shown not to |

---

## Functions

### mentioned

```mach
pub fun mentioned(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether `tid` mentions a type parameter anywhere it is spelled

### owner

```mach
pub fun owner(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) opt[mach.type.GenericOwner]
```

the declaration whose parameters a nominal's fields mention: the nominal itself for a
named generic, the enclosing declaration for an anonymous aggregate declared inside one;
none when the fields mention no parameter

### has_param

```mach
pub fun has_param(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether a nominal's fields mention a type parameter, which a nominal with no field
table cannot be shown not to

