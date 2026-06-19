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

A comptime comparison or arithmetic relates the **mathematical values** of its
operands, exactly as the runtime operators do (see
[operators.md](operators.md)). A constant in `2^63 .. 2^64-1` is its true
unsigned magnitude, so `$if (0xFFFFFFFFFFFFFFFF > 0)` is taken and a cross-sign
comparison agrees with the runtime `if` — `$if (X < Y)` never selects a branch
that `if (X < Y)` would not. Comptime arithmetic that overflows the value's
range is a compile error rather than a silent wrap.

## Examples

### Target-conditional code

```mach
$if ($mach.build.os == $mach.os.linux) {
    use full.os.linux;
}
$or ($mach.build.os == $mach.os.windows) {
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
  call site (a literal, a `pub val`, or another comptime parameter); a runtime
  value is rejected with `comptime argument is not a compile-time constant`. A
  cross-module constant works, whether imported by bare name or as a qualified
  member (`alias.CONST`).
- Each arm gate of a comptime-parameter `$if` must itself be comptime-foldable:
  its identifiers must all be comptime (the comptime parameters or comptime
  constants). A gate referencing a runtime local/parameter is rejected.
- A comptime parameter has no storage, so its address cannot be taken
  (`?$mode` is rejected with `cannot take the address of a comptime parameter`).
- A comptime-parameter function is a template, not a value: it can only be
  called, not assigned, passed, or compared (`val fp = apply;` is rejected with
  `cannot reference a comptime-parameter function as a value`).
- Comptime parameters carry no runtime cost: they are stripped from the lowered
  signature and ABI, so only the runtime parameters are passed.
- A comptime parameter may be mixed freely with runtime parameters in any order.
- A comptime parameter on a **generic** function (`fun f[T]($mode: u8, ...)`) is
  not yet supported — combining a type instance with a value instance is a
  pending extension and is reported with a clear diagnostic.
- The function may live in any module: a value-parameter instance is emitted
  against its declaring module and folds its `$if` gates against that module's
  own comptime constants, so a library can export a comptime-parameter function
  gated on its own `pub val`s.
- A comptime parameter may gate per-target asm safely, since each instance only
  compiles its taken arm:

```mach
pub fun load($order: Order, ptr: *i64) i64 {
    var result: i64 = 0;

    $if ($mach.build.arch == $mach.arch.aarch64) {
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

A `$if` gated on a `$type_of` type comparison is *not* such an exception: at
monomorphization the operand's concrete type is known, so the provably-dead arms
are **pruned** and only the selected arm is type-checked (and emitted). Each arm
may therefore use its value at its own concrete type with no per-arm cast — see
[comptime-intrinsics.md](comptime-intrinsics.md).

## See also

- [comptime-mach.md](comptime-mach.md) — `$mach.*` for target reads
- [asm.md](asm.md) — `asm` blocks gated by `$if`
- [statements.md](statements.md) — runtime `if` / `or` counterpart
