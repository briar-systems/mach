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
- Comparisons of comptime constants (`pub val` declarations)
- Comparisons of a comptime function parameter (`$mode`) — see below

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

A comptime function parameter (`$mode: u8`) is a compile-time-known argument
fixed per call site. `$if` / `$or` may branch on it: the compiler
**monomorphizes** the function body once per distinct comptime-argument value,
and each instance compiles only the arm its value selects.

```mach
val MODE_DOUBLE: u8 = 0;
val MODE_SQUARE: u8 = 1;

fun apply($mode: u8, n: i64) i64 {
    $if (mode == MODE_DOUBLE) {
        ret n + n;
    }
    $or (mode == MODE_SQUARE) {
        ret n * n;
    }
    ret 0;
}

# apply(MODE_DOUBLE, ..) and apply(MODE_SQUARE, ..) emit two distinct bodies,
# each carrying only its selected arm.
```

Rules:

- The argument bound to a `$`-parameter must be a compile-time constant at the
  call site; a runtime value is rejected with `comptime argument is not a
  compile-time constant`.
- Comptime parameters carry no runtime cost: they are stripped from the lowered
  signature and ABI, so only the runtime parameters are passed.
- A comptime parameter may be mixed freely with runtime parameters in any order.
- A comptime parameter may gate per-target asm safely, since each instance only
  compiles its taken arm:

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
output. For a target- or constant-gated `$if`, the compiler doesn't even
resolve names inside the untaken branches — this is the mechanism that makes
per-target asm blocks safe even when one block references registers the other
backend doesn't know about.

A `$if` gated on a comptime **parameter** is the one exception: because arm
selection happens per call site (at monomorphization), name resolution and
type checking run over **all** arms structurally, and only the selected arm is
emitted into each instance. Each arm must therefore be independently
resolvable and type-checkable.

## See also

- [comptime-mach.md](comptime-mach.md) — `$mach.*` for target reads
- [asm.md](asm.md) — `asm` blocks gated by `$if`
- [statements.md](statements.md) — runtime `if` / `or` counterpart
