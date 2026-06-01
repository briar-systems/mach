# `$if` / `$or` — comptime control flow

`$if` and `$or` branch on comptime-evaluable conditions. Only the taken
branch compiles — the discarded branches are not resolved, type-checked,
or emitted into the binary. This is fundamentally different from runtime
`if` / `or`, which generates a branch at runtime.

## Grammar

```mach
$if (cond) {
    ...
}
$or (cond) {
    ...
}
$or {
    ...                 # comptime else
}
```

The condition must be a comptime expression. Common shapes:

- `$mach.*` reads for target / build conditions
- Comparisons of comptime constants
- Comptime function parameters

## Examples

### Target-conditional code

```mach
$if ($mach.target.os == $mach.os.linux) {
    use full.os.linux;
}
$or ($mach.target.os == $mach.os.windows) {
    use full.os.windows;
}
$or {
    $error("unsupported OS");
}
```

### Dispatch on a comptime function parameter

```mach
pub fun load($order: Order, ptr: *i64) i64 {
    var result: i64 = 0;

    $if ($mach.target.arch == $mach.arch.aarch64) {
        $if (order == RELAXED) {
            asm aarch64 { ldr {result}, [{ptr}] }
        }
        $or (order == ACQUIRE) {
            asm aarch64 { ldar {result}, [{ptr}] }
        }
    }

    ret result;
}
```

## Discarded branches

A `$if` branch that isn't taken is entirely absent from the compiled
output. The compiler doesn't even resolve names inside it. This is the
mechanism that makes per-target asm blocks safe even when one block
references registers the other backend doesn't know about.

## See also

- [comptime-mach.md](comptime-mach.md) — `$mach.*` for target reads
- [asm.md](asm.md) — `asm` blocks gated by `$if`
- [statements.md](statements.md) — runtime `if` / `or` counterpart
