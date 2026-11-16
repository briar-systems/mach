[mach](../../README.md) / [build](../README.md) / [step](README.md) / **walk**

# mach.build.step.walk

a contained path walk: each component of a path beneath a root is read
without following a symlink, refused when it is not what the caller expects,
and made when the caller's policy creates it

<sub>source: [`src/build/step/walk.mach`](../../../../src/build/step/walk.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`path_check`](#path_check) · [`ancestors_check`](#ancestors_check) · [`directory_ensure`](#directory_ensure) | |
| [`owned_directory_ensure`](#owned_directory_ensure) | as `directory_ensure`, in output layout the build owns |

| Constants | |
| :--- | :--- |
| [`PHYSICAL_REGULAR`](#constants) · [`PHYSICAL_DIRECTORY`](#constants) · [`PHYSICAL_ENTRY`](#constants) | |

---

## Functions

### path_check

```mach
pub fun path_check(alloc: *A.Allocator, root: str, rel: str, expected: u8, label: str) err[fail.Fail]
```

### ancestors_check

```mach
pub fun ancestors_check(alloc: *A.Allocator, root: str, rel: str, label: str) err[fail.Fail]
```

### directory_ensure

```mach
pub fun directory_ensure(alloc: *A.Allocator, root: str, rel: str, label: str) err[fail.Fail]
```

### owned_directory_ensure

```mach
pub fun owned_directory_ensure(alloc: *A.Allocator, root: str, rel: str, label: str) err[fail.Fail]
```

as `directory_ensure`, in output layout the build owns

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `PHYSICAL_REGULAR` | `u8` | `1` |  |
| `PHYSICAL_DIRECTORY` | `u8` | `2` |  |
| `PHYSICAL_ENTRY` | `u8` | `3` |  |

