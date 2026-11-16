[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **typename**

# mach.front.sema.typename

a semantic type spelled as a program writes it, for diagnostics and debug names

Source: [`src/front/sema/typename.mach`](../../../../src/front/sema/typename.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.session`](../../session.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`type_to_str`](#type_to_str) | |
| [`type_str_free`](#type_str_free) | release a spelled type; a nil spelling owns nothing |
| [`name_str`](#name_str) | the spelling of a type's name; an anonymous type, whose name is absent or empty, is spelled `<anon>` |

---

## Functions

### type_to_str

```mach
pub fun type_to_str(s: *mach.session.Session, t: mach.type.kind.TypeId) res[str, mach.base.fail.Fail]
```

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

