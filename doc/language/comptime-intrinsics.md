# Intrinsics

Intrinsics are compiler-shipped comptime functions. They have the same
syntactic shape as user-defined function calls (`$name(args)`), but their
names are reserved and their implementations are built into the compiler.

The set is closed; adding a new intrinsic requires a compiler change.

## Value intrinsics

Return comptime constant unsigned integers. The storage type is whatever
the binding declares — Mach has no compiler-known `usize`.

```mach
$size_of(T)             # byte size of type T
$align_of(T)            # byte alignment of type T
$offset_of(T, field)    # byte offset of T's field
```

```mach
pub val POINT_SIZE: i64 = $size_of(Point);
pub val POINT_X:    i64 = $offset_of(Point, x);
```

## Diagnostic intrinsics

> **Not yet implemented.** `$error` and `$assert` parse as comptime
> directives but the compiler does not yet evaluate them — a `$error(...)`
> directive is currently accepted and ignored rather than failing
> compilation. The shapes below describe the intended behavior.

Operate at compile time. `$error` fails compilation; `$assert` is sugar
over `$if` plus `$error`.

```mach
$error("msg")
$assert(cond, "msg")    # sugar: $if (!cond) { $error("msg"); }
```

```mach
$assert($size_of(i64) == 8, "expected 64-bit i64");

$if (!supported) {
    $error("this target is not supported");
}
```

## Not provided as intrinsics

Code intrinsics — runtime-instruction emitters like `trap`, `fence`,
`pause` — are not in the compiler-shipped set. They belong in stdlib as
functions with per-arch `asm` bodies. See [policy.md](policy.md).

## See also

- [comptime.md](comptime.md) — channel overview
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
