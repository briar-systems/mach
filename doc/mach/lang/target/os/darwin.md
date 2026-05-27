# mach.lang.target.os.darwin

macOS / Darwin operating-system impl. Implements
[`os.OsVTable`](../os.md#osvtable) and registers itself at compiler
startup with [`os.register`](../os.md#register).

Source is `new/lang/target/os/darwin.mach` (currently empty).

## Identifying constants

- `id` = [`os.OS_DARWIN`](../os.md#constants).
- `name` = `"darwin"`.
- `entry_symbol` = `"_main"` — the Darwin dyld stub calls `_main`
  with `(argc, argv, envp, apple)`; the runtime forwards to user
  `main`.
- `syscall_layer` = `"darwin"` — selects `std.runtime.syscall_darwin`
  as the syscall stub set (BSD-style numbering with the Darwin offset).
- `libdir` = `"/usr/lib"`.

## Notes

Darwin uses the [SysV](../abi/sysv.md) ABI variant on both x86\_64
and aarch64 with platform-specific tweaks (notably section naming
and the Mach-O layout); [`target.select`](../../target.md#select)
pairs this OS with [`ABI_SYSV`](../abi.md#constants) and
[`OF_MACHO`](../of.md#constants).

## Dependencies

[`mach.lang.target.os`](../os.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
