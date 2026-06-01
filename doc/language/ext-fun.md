# `ext fun` — external function

`ext fun` declares a function with C ABI as a forward reference — body-less.
The linker resolves the symbol at link time. This is the only body-less
function form Mach allows.

## Grammar

```mach
ext fun NAME(args) RET;
```

- No body block. The declaration ends with a semicolon.
- Argument and return types must be representable in C.
- `pub ext fun` exposes the import to other modules; without `pub`, the
  import is file-private.

## Examples

```mach
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;

ext fun strlen(s: *u8) i64;             # private, file-local
```

## Symbol rename

By default the linker symbol matches the Mach name. Override it with the
`.symbol` attribute:

```mach
$libc_write.symbol = "write";
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

Common reasons to rename:

- The C name (e.g. `write`) would shadow other things in your namespace.
- The target's calling convention or ABI decorates symbol names and you
  need to match the decorated form.

## See also

- [fun.md](fun.md) — regular function declarations
- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [comptime-attrs.md](comptime-attrs.md) — `.symbol` and other attributes
