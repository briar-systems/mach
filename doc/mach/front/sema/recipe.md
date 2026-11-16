[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **recipe**

# mach.front.sema.recipe

type recipes: the canonical encoding behind `$type_id` and the names of anonymous types

a recipe names a nominal type by its owning module and name, so every unit of one program
encodes a type alike. `identity` is the first 64 bits of sha-256 over a domain tag and the
recipe, worked out once per type and kept in the type table, which `$type_id` answers and debug
information keys types by. `anonymous` interns the recipe of an anonymous aggregate's fields
as its name.

<sub>source: [`src/front/sema/recipe.mach`](../../../../src/front/sema/recipe.mach) · imported by 2 modules</sub>

<sub>`session` is [`mach.session`](../../session.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `type_field` is [`mach.type.field`](../../type/field.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`anonymous`](#anonymous) | |
| [`identity`](#identity) | the first 64 bits of sha-256 over a domain tag and the type's canonical recipe; the recipe names nominal types by owning module and name, so every unit of one program derives the same value |

---

## Functions

### anonymous

```mach
pub fun anonymous(s: *session.Session, kind: type_kind.TypeKind,
    fields: *type_field.Entry, count: u32) res[intern.StrId, fail.Fail]
```

### identity

```mach
pub fun identity(s: *session.Session, tid: type_kind.TypeId) res[u64, fail.Fail]
```

the first 64 bits of sha-256 over a domain tag and the type's canonical recipe; the recipe
names nominal types by owning module and name, so every unit of one program derives the same value.
it is worked out once per type and kept in the type table

