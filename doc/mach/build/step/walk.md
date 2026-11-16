[mach](../../README.md) / [build](../README.md) / [step](README.md) / **walk**

# mach.build.step.walk

a contained path walk: each component of a path beneath a root is read
without following a symlink, refused when it is not what the caller expects,
and made when the caller's policy creates it

Source: [`src/build/step/walk.mach`](../../../../src/build/step/walk.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Functions | |
| :--- | :--- |
| [`path_check`](#path_check), [`ancestors_check`](#ancestors_check), [`directory_ensure`](#directory_ensure) | |
| [`owned_directory_ensure`](#owned_directory_ensure) | as `directory_ensure`, in output layout the build owns |

| Constants | |
| :--- | :--- |
| [`PHYSICAL_REGULAR`](#constants), [`PHYSICAL_DIRECTORY`](#constants), [`PHYSICAL_ENTRY`](#constants) | |

---

## Functions

### path_check

```mach
pub fun path_check(alloc: *std.allocator.Allocator, root: str, rel: str, expected: u8, label: str) err[mach.base.fail.Fail]
```

### ancestors_check

```mach
pub fun ancestors_check(alloc: *std.allocator.Allocator, root: str, rel: str, label: str) err[mach.base.fail.Fail]
```

### directory_ensure

```mach
pub fun directory_ensure(alloc: *std.allocator.Allocator, root: str, rel: str, label: str) err[mach.base.fail.Fail]
```

### owned_directory_ensure

```mach
pub fun owned_directory_ensure(alloc: *std.allocator.Allocator, root: str, rel: str, label: str) err[mach.base.fail.Fail]
```

as `directory_ensure`, in output layout the build owns

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `PHYSICAL_REGULAR` | `u8` | `1` |  |
| `PHYSICAL_DIRECTORY` | `u8` | `2` |  |
| `PHYSICAL_ENTRY` | `u8` | `3` |  |

