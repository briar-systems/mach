# mach.lang.target.os.windows

Windows operating-system impl. Implements
[`os.OsVTable`](../os.md#osvtable) and registers itself at compiler
startup with [`os.register`](../os.md#register).

Source is `new/lang/target/os/windows.mach` (currently empty).

## Identifying constants

- `id` = [`os.OS_WINDOWS`](../os.md#constants).
- `name` = `"windows"`.
- `entry_symbol` = `"mainCRTStartup"` — the conventional entry the
  Windows loader calls; the runtime sets up the CRT and calls user
  `main`.
- `syscall_layer` = `"windows"` — selects the NT-API / Win32 shim
  set (Windows does not have a stable syscall numbering; the layer
  is library-call-based).
- `libdir` = `"C:\\Windows\\System32"`.

## Notes

Windows uses the [Win64](../abi/win64.md) ABI on both x86\_64 and
aarch64; [`target.select`](../../target.md#select) pairs this OS
with [`ABI_WIN64`](../abi.md#constants) and
[`OF_COFF`](../of.md#constants).

## Dependencies

[`mach.lang.target.os`](../os.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.intern`](../../intern.md).
