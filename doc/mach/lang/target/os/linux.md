# mach.lang.target.os.linux

Linux operating-system impl. Implements
[`os.OsVTable`](../os.md#osvtable) and registers itself at compiler
startup with [`os.register`](../os.md#register).

Source is `new/lang/target/os/linux.mach` (currently empty).

## Identifying constants

- `id` = [`os.OS_LINUX`](../os.md#constants).
- `name` = `"linux"`.
- `entry_symbol` = `"_start"` — the kernel jumps here after exec;
  the runtime's `_start` shim sets up the initial stack frame and
  calls user `main`.
- `syscall_layer` = `"linux"` — selects `std.runtime.syscall_linux`
  as the syscall stub set.
- `libdir` = `"/usr/lib"`.

## Notes

Linux uses the [SysV](../abi/sysv.md) ABI on both x86\_64 and
aarch64; [`target.select`](../../target.md#select) pairs this OS
with [`ABI_SYSV`](../abi.md#constants) and
[`OF_ELF`](../of.md#constants).

## Dependencies

[`mach.lang.target.os`](../os.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
