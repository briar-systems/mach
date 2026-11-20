[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **typename**

# mach.front.sema.typename

a semantic type spelled as a program writes it, for diagnostics and debug names

Source: [`src/front/sema/typename.mach`](../../../../src/front/sema/typename.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.id`](../ast/id.md), [`mach.session`](../../session.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`type_to_str`](#type_to_str) | spell a semantic type as a program writes it, in a fresh nul-terminated buffer |
| [`annotation_str`](#annotation_str) | the annotation `tid` of the syntax `a` as its source spells it, each run of white space one space, released with `type_str_free`; none where the node or its source is gone |
| [`type_str_free`](#type_str_free) | release a spelled type; a nil spelling owns nothing |
| [`name_str`](#name_str) | the spelling of a type's name; an anonymous type, whose name is absent or empty, is spelled `<anon>` |

---

## Functions

### type_to_str

```mach
pub fun type_to_str(s: *mach.session.Session, t: mach.type.kind.TypeId) res[str, mach.base.fail.Fail]
```

spell a semantic type as a program writes it, in a fresh nul-terminated buffer

| Parameter | |
| :--- | :--- |
| `s` | session whose type store and allocator are used |
| `t` | type to spell, the error or nil type spells `<error>` |
| **returns** | the spelling, owned by the caller and released with type_str_free, or the failure when the type nests too deeply, names no stored type, reaches a kind the spelling does not name, or the allocation is refused |

### annotation_str

```mach
pub fun annotation_str(s: *mach.session.Session, a: *mach.front.ast.Ast, tid: mach.front.ast.id.TypeId) res[opt[str], mach.base.fail.Fail]
```

the annotation `tid` of the syntax `a` as its source spells it, each run of white space one
space, released with `type_str_free`; none where the node or its source is gone

### type_str_free

```mach
pub fun type_str_free(s: *mach.session.Session, owned: str)
```

release a spelled type; a nil spelling owns nothing

### name_str

```mach
pub fun name_str(s: *mach.session.Session, name: mach.base.intern.StrId) str
```

the spelling of a type's name; an anonymous type, whose name is absent or
empty, is spelled `<anon>`

