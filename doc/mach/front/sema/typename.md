[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **typename**

# mach.front.sema.typename

a semantic type spelled as a program writes it, for diagnostics and debug names

<sub>source: [`src/front/sema/typename.mach`](../../../../src/front/sema/typename.mach) · imported by 6 modules</sub>

<sub>`session` is [`mach.session`](../../session.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `intern` is [`mach.base.intern`](../../base/intern.md)</sub>

| Functions | |
| :--- | :--- |
| [`type_to_str`](#type_to_str) | |
| [`type_str_free`](#type_str_free) | release a spelled type; a nil spelling owns nothing |
| [`name_str`](#name_str) | the spelling of a type's name; an anonymous type, whose name is absent or empty, is spelled `<anon>` |

---

## Functions

### type_to_str

```mach
pub fun type_to_str(s: *session.Session, t: type_kind.TypeId) res[str, fail.Fail]
```

### type_str_free

```mach
pub fun type_str_free(s: *session.Session, owned: str)
```

release a spelled type; a nil spelling owns nothing

### name_str

```mach
pub fun name_str(s: *session.Session, name: intern.StrId) str
```

the spelling of a type's name; an anonymous type, whose name is absent or
empty, is spelled `<anon>`

