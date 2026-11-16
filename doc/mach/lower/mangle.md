[mach](../README.md) / [lower](README.md) / **mangle**

# mach.lower.mangle

linkage names: the symbols of declarations, tests and generic, value and pack instances

a declaration is named by its module path and its name, joined by a dot, unless the build
declares a link name for it. an instance appends each type argument or comptime value after a
`$`, so its symbol is a function of its identity alone, and a test is
`<module path>#<identifier>`. the root module's path is spelled `main`.

<sub>source: [`src/lower/mangle.mach`](../../../src/lower/mangle.mach) · imported by 5 modules</sub>

<sub>`session` is [`mach.session`](../session.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `linkage` is [`mach.lower.linkage`](linkage.md) · `module` is [`mach.session.module`](../session/module.md) · `sema_instance` is [`mach.front.sema.instance`](../front/sema/instance.md) · `type_kind` is [`mach.type.kind`](../type/kind.md) · `comptime_value` is [`mach.front.comptime.value`](../front/comptime/value.md)</sub>

| Functions | |
| :--- | :--- |
| [`linkage_name`](#linkage_name) | |
| [`declaration_linkage_name`](#declaration_linkage_name) | the linkage name of the declaration `decl` of module `mid`, whose canonical name is `canon`, exported under the name `l` declares for it, made once and kept in `l` |
| [`test_name`](#test_name) | a test's qualified name and symbol, `<module path>#<identifier>`: `#` is in no identifier or module path, so it names no other symbol |
| [`instance_link_name`](#instance_link_name) | the symbol the instance `item` is defined and referenced under, a function of its identity alone |
| [`instance_name`](#instance_name) · [`value_instance_name`](#value_instance_name) · [`pack_instance_name`](#pack_instance_name) | |

---

## Functions

### linkage_name

```mach
pub fun linkage_name(
    s: *session.Session,
    fqn: intern.StrId,
    bare: intern.StrId,
    export: intern.StrId) res[intern.StrId, fail.Fail]
```

### declaration_linkage_name

```mach
pub fun declaration_linkage_name(l: *linkage.Linkage, s: *session.Session, mid: module.ModuleId, canon: intern.StrId, decl: u32) res[intern.StrId, fail.Fail]
```

the linkage name of the declaration `decl` of module `mid`, whose canonical name is `canon`,
exported under the name `l` declares for it, made once and kept in `l`

### test_name

```mach
pub fun test_name(s: *session.Session, fqn: intern.StrId, ident: intern.StrId) res[intern.StrId, fail.Fail]
```

a test's qualified name and symbol, `<module path>#<identifier>`: `#` is in no
identifier or module path, so it names no other symbol

### instance_link_name

```mach
pub fun instance_link_name(s: *session.Session, item: *sema_instance.Instance) res[intern.StrId, fail.Fail]
```

the symbol the instance `item` is defined and referenced under, a function of its identity alone

### instance_name

```mach
pub fun instance_name(
    s: *session.Session,
    fqn: intern.StrId,
    bare: intern.StrId,
    args: *type_kind.TypeId,
    arg_count: u32) res[intern.StrId, fail.Fail]
```

### value_instance_name

```mach
pub fun value_instance_name(
    s: *session.Session,
    fqn: intern.StrId,
    bare: intern.StrId,
    vals: *comptime_value.CTValue,
    val_len: u32) res[intern.StrId, fail.Fail]
```

### pack_instance_name

```mach
pub fun pack_instance_name(
    s: *session.Session,
    fqn: intern.StrId,
    bare: intern.StrId,
    args: *type_kind.TypeId,
    arg_len: u32,
    types: *type_kind.TypeId,
    type_len: u32) res[intern.StrId, fail.Fail]
```

