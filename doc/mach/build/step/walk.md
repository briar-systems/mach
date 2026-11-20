[mach](../../README.md) / [build](../README.md) / [step](README.md) / **walk**

# mach.build.step.walk

a contained path walk: each component of a path beneath a root is read
without following a symlink, refused when it is not what the caller expects,
and made when the caller's policy creates it

Source: [`src/build/step/walk.mach`](../../../../src/build/step/walk.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Functions | |
| :--- | :--- |
| [`path_check`](#path_check) | check that `rel` beneath `root` names an entry of the `expected` kind and that every component before it exists and is a directory, following no symlink |
| [`ancestors_check`](#ancestors_check) | check that each component of `rel` beneath `root` that exists is a directory and no symlink, stopping at the first one missing, since nothing beyond it can escape `root` |
| [`directory_ensure`](#directory_ensure) | make `rel` beneath `root` a directory, creating each missing component and refusing a symlink or a non-directory on the way |
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

check that `rel` beneath `root` names an entry of the `expected` kind and that every component
before it exists and is a directory, following no symlink

| Parameter | |
| :--- | :--- |
| `alloc` | owns the failure's text |
| `root` | the directory `rel` is beneath |
| `rel` | a canonical relative path, or `.` for the root itself |
| `expected` | PHYSICAL_REGULAR, PHYSICAL_DIRECTORY or PHYSICAL_ENTRY for either |
| `label` | what the path is, for the refusal |
| **returns** | ok, or the failure, the user's when a component is missing, a symlink or of the wrong kind |

### ancestors_check

```mach
pub fun ancestors_check(alloc: *std.allocator.Allocator, root: str, rel: str, label: str) err[mach.base.fail.Fail]
```

check that each component of `rel` beneath `root` that exists is a directory and no symlink,
stopping at the first one missing, since nothing beyond it can escape `root`

| Parameter | |
| :--- | :--- |
| `alloc` | owns the failure's text |
| `root` | the directory `rel` is beneath |
| `rel` | a canonical relative path, empty for nothing to check |
| `label` | what the path is, for the refusal |
| **returns** | ok, or the failure, the user's when a component is a symlink or not a directory |

### directory_ensure

```mach
pub fun directory_ensure(alloc: *std.allocator.Allocator, root: str, rel: str, label: str) err[mach.base.fail.Fail]
```

make `rel` beneath `root` a directory, creating each missing component and refusing a
symlink or a non-directory on the way

| Parameter | |
| :--- | :--- |
| `alloc` | owns the failure's text |
| `root` | the directory `rel` is beneath |
| `rel` | a canonical relative path |
| `label` | what the path is, for the refusal |
| **returns** | ok, or the failure of a component that cannot be used or made |

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

