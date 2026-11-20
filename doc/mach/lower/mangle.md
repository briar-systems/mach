[mach](../README.md) / [lower](README.md) / **mangle**

# mach.lower.mangle

linkage names: the symbols of declarations, tests and generic, value and pack instances

a declaration is named by its module path and its name, joined by a dot, unless the build
declares a link name for it. an instance appends each type argument or comptime value after a
`$`, so its symbol is a function of its identity alone, and a test is
`<module path>#<identifier>`. the root module's path is spelled `main`.

Source: [`src/lower/mangle.mach`](../../../src/lower/mangle.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.front.comptime.value`](../front/comptime/value.md), [`mach.front.sema.instance`](../front/sema/instance.md), [`mach.lower.linkage`](linkage.md), [`mach.session`](../session.md), [`mach.session.module`](../session/module.md), [`mach.type.kind`](../type/kind.md).

| Functions | |
| :--- | :--- |
| [`linkage_name`](#linkage_name) | the linkage name of a declaration: its export name when the build declares one, its module path and bare name joined by a dot otherwise |
| [`declaration_linkage_name`](#declaration_linkage_name) | the linkage name of the declaration `decl` of module `mid`, whose canonical name is `canon`, exported under the name `l` declares for it, made once and kept in `l` |
| [`test_name`](#test_name) | a test's qualified name and symbol, `<module path>#<identifier>`: `#` is in no identifier or module path, so it names no other symbol |
| [`instance_link_name`](#instance_link_name) | the symbol the instance `item` is defined and referenced under, a function of its identity alone |
| [`instance_name`](#instance_name) | the symbol of a generic instance: its declaration's name with each type argument after a `$` |
| [`value_instance_name`](#value_instance_name) | the symbol of a value instance: its declaration's name with each comptime value after a `$` |
| [`pack_instance_name`](#pack_instance_name) | the symbol of a pack instance: its type arguments, then `$pack`, then the types of its pack |

---

## Functions

### linkage_name

```mach
pub fun linkage_name(
    s: *mach.session.Session,
    fqn: mach.base.intern.StrId,
    bare: mach.base.intern.StrId,
    export: mach.base.intern.StrId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the linkage name of a declaration: its export name when the build declares one, its
module path and bare name joined by a dot otherwise

| Parameter | |
| :--- | :--- |
| `s` | the session whose interner holds the names |
| `fqn` | the declaring module's canonical name |
| `bare` | the declaration's name |
| `export` | the export name the build declares, nil for none |
| **returns** | the interned name, or a failure when a name is not interned or allocation fails |

### declaration_linkage_name

```mach
pub fun declaration_linkage_name(l: *mach.lower.linkage.Linkage, s: *mach.session.Session, mid: mach.session.module.ModuleId, canon: mach.base.intern.StrId, decl: u32) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the linkage name of the declaration `decl` of module `mid`, whose canonical name is `canon`,
exported under the name `l` declares for it, made once and kept in `l`

### test_name

```mach
pub fun test_name(s: *mach.session.Session, fqn: mach.base.intern.StrId, ident: mach.base.intern.StrId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

a test's qualified name and symbol, `<module path>#<identifier>`: `#` is in no
identifier or module path, so it names no other symbol

### instance_link_name

```mach
pub fun instance_link_name(s: *mach.session.Session, item: *mach.front.sema.instance.Instance) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol the instance `item` is defined and referenced under, a function of its identity alone

### instance_name

```mach
pub fun instance_name(
    s: *mach.session.Session,
    fqn: mach.base.intern.StrId,
    bare: mach.base.intern.StrId,
    args: *mach.type.kind.TypeId,
    arg_count: u32) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol of a generic instance: its declaration's name with each type argument after a `$`

| Parameter | |
| :--- | :--- |
| `s` | the session whose interner and type table the names come from |
| `fqn` | the generic's module's canonical name |
| `bare` | the generic's name |
| `args` | the type arguments |
| `arg_count` | how many there are |
| **returns** | the interned symbol, or a failure when a type argument has no encoding |

### value_instance_name

```mach
pub fun value_instance_name(
    s: *mach.session.Session,
    fqn: mach.base.intern.StrId,
    bare: mach.base.intern.StrId,
    vals: *mach.front.comptime.value.CTValue,
    val_len: u32) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol of a value instance: its declaration's name with each comptime value after a `$`

an integer is its decimal magnitude with `n` before a negative one, a float is `fx` and the
sixteen hex digits of its binary64 bits, and a string is quoted with `\` and `"` escaped

| Parameter | |
| :--- | :--- |
| `s` | the session whose interner holds the names |
| `fqn` | the declaring module's canonical name |
| `bare` | the function's name |
| `vals` | the comptime values the instance is keyed by |
| `val_len` | how many there are |
| **returns** | the interned symbol, or a failure when a value is of a kind no symbol encodes |

### pack_instance_name

```mach
pub fun pack_instance_name(
    s: *mach.session.Session,
    fqn: mach.base.intern.StrId,
    bare: mach.base.intern.StrId,
    args: *mach.type.kind.TypeId,
    arg_len: u32,
    types: *mach.type.kind.TypeId,
    type_len: u32) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol of a pack instance: its type arguments, then `$pack`, then the types of its pack

| Parameter | |
| :--- | :--- |
| `s` | the session whose interner and type table the names come from |
| `fqn` | the declaring module's canonical name |
| `bare` | the function's name |
| `args` | the type arguments |
| `arg_len` | how many there are |
| `types` | the types the pack holds |
| `type_len` | how many there are |
| **returns** | the interned symbol, or a failure when a type has no encoding |

