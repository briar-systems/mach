[mach](../README.md) / [target](README.md) / **tags**

# mach.target.tags

the target tags a program compares against, `$mach.sys.*`, `$mach.arch.*` and
`$mach.abi.*`, and the ids a resolved target carries for them

every tag is a row of mink's catalogs: a system of its system list, an
architecture of its architecture list, and a calling convention one of its
architecture families names. a tag's id is its row's position plus one, 0
standing for none, so mach states no tag of its own and a row mink adds is a
tag with no change here

Source: [`src/target/tags.mach`](../../../src/target/tags.mach)

| Functions | |
| :--- | :--- |
| [`sys_id`](#sys_id) | the id of mink's system `s`, NONE for one its list does not hold |
| [`arch_id`](#arch_id) | the id of mink's architecture `a`, NONE for one its list does not hold |
| [`sys_named`](#sys_named) | the id of the system spelled `name`, NONE for a spelling mink's list does not hold |
| [`arch_named`](#arch_named) | the id of the architecture spelled `name`, NONE for a spelling mink's list does not hold |
| [`sys_name`](#sys_name) | the name of the system tag `id`, "unknown" for NONE or an id past the list |
| [`arch_name`](#arch_name) | the name of the architecture tag `id`, "unknown" for NONE or an id past the list |
| [`abi_id`](#abi_id) | the id of mink's convention row `row`: the id of its spelling, which every row of that spelling shares, NONE for a row no family of mink's architectures names |
| [`abi_named`](#abi_named) | the id of the calling convention spelled `name`: one per distinct spelling the families name, NONE for a spelling none names |
| [`abi_name`](#abi_name) | the spelling of the convention tag `id`, "unknown" for NONE or an id past the rows |
| [`sys_named_view`](#sys_named_view) | the id of the system spelled by `name`, as `sys_named` |
| [`arch_named_view`](#arch_named_view) | the id of the architecture spelled by `name`, as `arch_named` |
| [`abi_named_view`](#abi_named_view) | the id of the convention spelled by `name`, as `abi_named` |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | the id of no tag |

---

## Functions

### sys_id

```mach
pub fun sys_id(s: *mink.catalog.os.System) u32
```

the id of mink's system `s`, NONE for one its list does not hold

### arch_id

```mach
pub fun arch_id(a: *mink.catalog.arch.Arch) u32
```

the id of mink's architecture `a`, NONE for one its list does not hold

### sys_named

```mach
pub fun sys_named(name: str) u32
```

the id of the system spelled `name`, NONE for a spelling mink's list does not hold

### arch_named

```mach
pub fun arch_named(name: str) u32
```

the id of the architecture spelled `name`, NONE for a spelling mink's list does not hold

### sys_name

```mach
pub fun sys_name(id: u32) str
```

the name of the system tag `id`, "unknown" for NONE or an id past the list

### arch_name

```mach
pub fun arch_name(id: u32) str
```

the name of the architecture tag `id`, "unknown" for NONE or an id past the list

### abi_id

```mach
pub fun abi_id(row: *mink.catalog.arch.Abi) u32
```

the id of mink's convention row `row`: the id of its spelling, which every row of that
spelling shares, NONE for a row no family of mink's architectures names

### abi_named

```mach
pub fun abi_named(name: str) u32
```

the id of the calling convention spelled `name`: one per distinct spelling the
families name, NONE for a spelling none names

### abi_name

```mach
pub fun abi_name(id: u32) str
```

the spelling of the convention tag `id`, "unknown" for NONE or an id past the rows

### sys_named_view

```mach
pub fun sys_named_view(name: View) u32
```

the id of the system spelled by `name`, as `sys_named`

### arch_named_view

```mach
pub fun arch_named_view(name: View) u32
```

the id of the architecture spelled by `name`, as `arch_named`

### abi_named_view

```mach
pub fun abi_named_view(name: View) u32
```

the id of the convention spelled by `name`, as `abi_named`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0` | the id of no tag |

