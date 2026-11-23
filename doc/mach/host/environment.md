[mach](../README.md) / [host](README.md) / **environment**

# mach.host.environment

a child process's environment, held in the host's native units with one entry per name

an inherited variable with no UTF-8 spelling is carried unit for unit, and a name is
matched by the host's own name identity.

Source: [`src/host/environment.mach`](../../../src/host/environment.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Entry`](#entry) | `rec` | one variable, NAME=value in std.runtime.native units, so an inherited variable with no UTF-8 spelling is carried as it is |
| [`Environment`](#environment) | `rec` | a child's environment, one entry per name by the host's name identity |
| [`Strings`](#strings) | `rec` | a nil-terminated array of native NAME=value entries, owned, for exec.NativeEnvironment.given |

| On `Entry` | |
| :--- | :--- |
| [`entry_dnit`](#entry_dnit) | release an entry's native text; an empty entry owns none |
| [`entry_native`](#entry_native) | a native entry copied from `units`, nul-terminated |
| [`entry_text`](#entry_text) | a UTF-8 NAME=value entry in native units |

| On `Environment` | |
| :--- | :--- |
| [`init`](#init) | an empty environment |
| [`dnit`](#dnit) | release every entry and the environment's storage |
| [`put_owned`](#put_owned) | add `entry`, which the environment takes: a name already present keeps its value unless `replace`, and the entry not kept is released |
| [`put`](#put) | entries own both strings and use host identity before any overlay is applied |
| [`find`](#find) | the index of the entry named `name` by the host's name identity, none when the environment does not hold it |
| [`remove`](#remove) | drop the entry at `index`, keeping the order of the rest |
| [`capture`](#capture) | the environment `e` describes, in native units: a given UTF-8 array, or this process's own as the native capture holds it, so a variable with no UTF-8 spelling is carried unit for unit |
| [`to_strings`](#to_strings) | the entries sorted by name, each copied, for a child |

| On `Strings` | |
| :--- | :--- |
| [`strings_dnit`](#strings_dnit) | release a list of native strings; an empty list owns nothing |

| Functions | |
| :--- | :--- |
| [`units_length`](#units_length) | the length of a native entry in units |

---

## Entry

```mach
pub rec Entry {
    units:  *Unit;
    length: usize;
    name:   usize;
}
```

one variable, NAME=value in std.runtime.native units, so an inherited
variable with no UTF-8 spelling is carried as it is

| Field | Type | |
| :--- | :--- | :--- |
| `units` | `*Unit` | the entry, owned and terminated |
| `length` | `usize` | its length in units |
| `name` | `usize` | the units of its name, before its first '=' |

### entry_dnit

```mach
pub fun entry_dnit(a: *std.allocator.Allocator, entry: *Entry)
```

release an entry's native text; an empty entry owns none

### entry_native

```mach
pub fun entry_native(a: *std.allocator.Allocator, units: *Unit) res[Entry, mach.base.fail.Fail]
```

a native entry copied from `units`, nul-terminated

### entry_text

```mach
pub fun entry_text(a: *std.allocator.Allocator, text: str) res[Entry, mach.base.fail.Fail]
```

a UTF-8 NAME=value entry in native units

## Environment

```mach
pub rec Environment {
    entries: Vector[Entry];
}
```

a child's environment, one entry per name by the host's name identity

### init

```mach
pub fun init(a: *std.allocator.Allocator) Environment
```

an empty environment

| Parameter | |
| :--- | :--- |
| `a` | allocator the entries and their text are held in |
| **returns** | the environment, released with dnit |

### dnit

```mach
pub fun dnit(values: *Environment)
```

release every entry and the environment's storage

| Parameter | |
| :--- | :--- |
| `values` | the environment to release |

### put_owned

```mach
pub fun put_owned(values: *Environment, entry: Entry, replace: bool) err[mach.base.fail.Fail]
```

add `entry`, which the environment takes: a name already present keeps its
value unless `replace`, and the entry not kept is released

### put

```mach
pub fun put(values: *Environment, name: str, value: str, replace: bool) err[mach.base.fail.Fail]
```

entries own both strings and use host identity before any overlay is applied

### find

```mach
pub fun find(values: *Environment, name: str) res[opt[usize], mach.base.fail.Fail]
```

the index of the entry named `name` by the host's name identity, none when the
environment does not hold it

### remove

```mach
pub fun remove(values: *Environment, index: usize)
```

drop the entry at `index`, keeping the order of the rest

### capture

```mach
pub fun capture(a: *std.allocator.Allocator, e: std.process.exec.Environment) res[Environment, mach.base.fail.Fail]
```

the environment `e` describes, in native units: a given UTF-8 array, or this
process's own as the native capture holds it, so a variable with no UTF-8
spelling is carried unit for unit. duplicate names keep their first value,
matching native lookup

### to_strings

```mach
pub fun to_strings(values: *Environment) res[Strings, mach.base.fail.Fail]
```

the entries sorted by name, each copied, for a child

## Strings

```mach
pub rec Strings {
    items: **Unit;
    count: u32;
}
```

a nil-terminated array of native NAME=value entries, owned, for
exec.NativeEnvironment.given

### strings_dnit

```mach
pub fun strings_dnit(a: *std.allocator.Allocator, strings: *Strings)
```

release a list of native strings; an empty list owns nothing

## Functions

### units_length

```mach
pub fun units_length(units: *Unit) usize
```

the length of a native entry in units

