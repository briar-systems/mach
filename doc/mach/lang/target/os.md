# mach.lang.target.os

Operating system interface and id registry. Defines an
[`OsVTable`](#osvtable) that every per-OS implementation under
`target/os/` registers, plus stable numeric ids consumed by
comptime predicates such as `$mach.os.<name>.id` and
`$mach.build.target.os.id`. The ids in [Constants](#constants) are
already in source; the vtable surface is the design intent the
implementations land against.

## Types

### `OsVTable`

```mach
pub rec OsVTable {
    id:            u32;
    name:          intern.StrId;
    entry_symbol:  intern.StrId;
    syscall_layer: intern.StrId;
    libdir:        intern.StrId;
}
```

Uniform per-OS interface. The driver / linker / runtime startup
shims reach into this vtable via [`Target.os`](../target.md#target).

| Field         | Type                                          | Description                                                                  |
|---------------|-----------------------------------------------|------------------------------------------------------------------------------|
| id            | `u32`                                         | One of [`OS_*`](#constants).                                                 |
| name          | [`intern.StrId`](../intern.md#strid)          | Canonical name (`"linux"`, `"darwin"`, `"windows"`).                         |
| entry_symbol  | [`intern.StrId`](../intern.md#strid)          | Required entry-point symbol the runtime crt0 emits (`"_start"` on Linux, `"_main"` on Darwin, `"mainCRTStartup"` on Windows). |
| syscall_layer | [`intern.StrId`](../intern.md#strid)          | Identifier of the syscall stub set `std.runtime` uses for this OS. |
| libdir        | [`intern.StrId`](../intern.md#strid)          | Canonical system-library directory (informational; linker uses it as a search-path default). |

## Constants

```mach
pub val OS_UNKNOWN: u32 = 0;
pub val OS_LINUX:   u32 = 1;
pub val OS_DARWIN:  u32 = 2;
pub val OS_WINDOWS: u32 = 3;
```

Canonical OS ids. Stable across compiler versions; user code
dispatches on them through `$mach.os.<name>.id` and
`$mach.build.target.os.id`.

| Constant       | Value | Meaning                                          |
|----------------|-------|--------------------------------------------------|
| `OS_UNKNOWN`   | 0     | Unrecognised OS (default fallback).              |
| `OS_LINUX`     | 1     | Linux.                                           |
| `OS_DARWIN`    | 2     | macOS / Darwin kernel.                           |
| `OS_WINDOWS`   | 3     | Windows.                                         |

## Functions

### `lookup`

```mach
pub fun lookup(os_id: u32) Option[*OsVTable]
```

Returns the registered [`OsVTable`](#osvtable) for `os_id`, or
`none` if no impl is linked into this build. Called by
[`target.select`](../target.md#select).

### `register`

```mach
pub fun register(vt: *OsVTable)
```

Registers a platform impl's vtable in the OS registry. Each
implementation file under [`target/os/`](../target.md) self-registers
from its module-init function; the compiler imports every OS impl it
supports, so each runs exactly once before `main`. The registry is
process-global, write-once at startup, read-only thereafter, and
never freed — it holds static platform data, not per-session state.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
[`mach.lang.intern`](../intern.md).
