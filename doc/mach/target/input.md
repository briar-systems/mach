[mach](../README.md) / [target](README.md) / **input**

# mach.target.input

link inputs: how a link token names a file or a library

a token names a file by its path, located here, or a library by name, which
mink's search finds when the link runs: the search its host row states for the
target's system, environment and architecture. mach looks for no library of
its own

Source: [`src/target/input.mach`](../../../src/target/input.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Form`](#form) | `def` | how an input is named: a file at a path, or a library or a framework by name that mink's search finds |
| [`Kind`](#kind) | `def` | what a library named by name must be, judged by mink's search from what the file holds |
| [`Input`](#input) | `rec` | a link input: a file at the path `text`, located, or a library or framework of the name `text` |

| On `Form` | |
| :--- | :--- |
| [`FILE`](#form), [`LIBRARY`](#form), [`FRAMEWORK`](#form) | |

| On `Kind` | |
| :--- | :--- |
| [`SHARED`](#kind), [`STATIC`](#kind) | |

| On `Input` | |
| :--- | :--- |
| [`token_input`](#token_input) | the input a link token names: a path, located as written or under `root`, or a library by name, left to mink's search |
| [`framework_input`](#framework_input) | a framework by name, left to mink's search |

| Functions | |
| :--- | :--- |
| [`token_is_path`](#token_is_path) | whether a token names a file by path rather than a library by name: it holds a separator, or it ends the way a file any system of mink's catalog searches for is named |
| [`names_file`](#names_file) | whether a library's name names a file, which states its own kind: a path, or a name ending in the suffix a system's naming states for the file its loader loads, as `kernel32.dll` does |

---

## Form

```mach
pub def Form: u8
```

how an input is named: a file at a path, or a library or a framework by
name that mink's search finds

| Name | Value | |
| :--- | :--- | :--- |
| `FILE` | `0` |  |
| `LIBRARY` | `1` |  |
| `FRAMEWORK` | `2` |  |

## Kind

```mach
pub def Kind: u8
```

what a library named by name must be, judged by mink's search from what the
file holds. nothing else is taken in its place, so a wrong kind or a miss is
a refusal

| Name | Value | |
| :--- | :--- | :--- |
| `SHARED` | `0` |  |
| `STATIC` | `1` |  |

## Input

```mach
pub rec Input {
    form:               Form;
    text:               str;
    include_referenced: bool;
    kind:               Kind;
}
```

a link input: a file at the path `text`, located, or a library or framework
of the name `text`. `include_referenced` whether the image loads it only
when a live import references it, and `kind` what a library by name must be,
SHARED for any other input

### token_input

```mach
pub fun token_input(a: *std.allocator.Allocator, root: str, tok: *u8) res[Input, mach.base.fail.Fail]
```

the input a link token names: a path, located as written or under `root`,
or a library by name, left to mink's search

### framework_input

```mach
pub fun framework_input(name: *u8) Input
```

a framework by name, left to mink's search

## Functions

### token_is_path

```mach
pub fun token_is_path(tok: *u8) bool
```

whether a token names a file by path rather than a library by name: it holds a
separator, or it ends the way a file any system of mink's catalog searches for
is named

### names_file

```mach
pub fun names_file(tok: *u8) bool
```

whether a library's name names a file, which states its own kind: a path, or
a name ending in the suffix a system's naming states for the file its loader
loads, as `kernel32.dll` does

