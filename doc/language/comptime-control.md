# `$if` / `$or` — comptime control flow

`$if` and `$or` branch on comptime-evaluable conditions. Only the taken
branch compiles — the discarded branches are not resolved, type-checked,
or emitted into the binary. This is fundamentally different from runtime
`if` / `or`, which generates a branch at runtime.

## Grammar

```mach fragment
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

```mach fragment
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
use std.runtime;
use print: std.print;

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
#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{} {}", apply(MODE_DOUBLE, 7), apply(MODE_SQUARE, 7));
    ret 0;
}
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
  refused: a function has type instances or value instances, never both, and
  the declaration is reported as `comptime value parameters on a generic
  function are not yet supported`.
- The function may live in any module: a value-parameter instance is emitted
  against its declaring module and folds its `$if` gates against that module's
  own comptime constants, so a library can export a comptime-parameter function
  gated on its own `pub val`s.
- A comptime parameter may gate per-target asm safely, since each instance only
  compiles its taken arm:

```mach fragment
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

## When a declaration-scope `$if` is decided

A `$if` chain written in declaration scope runs at one of two times, and what its
arms contain picks which.

- **Some arm declares something.** The chain is decided while names are being
  resolved, because what it decides is which declarations exist and every later
  stage reads the resulting declaration set. Nothing has a type at that point, so
  the gate cannot ask a type question: a layout intrinsic, a type predicate or a
  `$type_of` comparison there is rejected, with a message naming the reason.
- **No arm declares anything.** The chain contributes no name and no type whichever
  arm is taken, so nothing depends on deciding it early. It is decided during type
  checking instead, where its gate may measure a type (`$size_of`, `$align_of`,
  `$length_of`), query one (`$is_record` and friends), or compare one.

The question is answered from the **syntax**, over every arm (`$if`, every `$or`,
and the final `$else`-style `$or {}`) before any gate is evaluated. One declaring
arm anywhere keeps the whole chain at the earlier time. Per-arm answers are not
possible: which stage runs the gate would then depend on which arm the gate selects,
and the stage that would have to know that is the one being chosen.

A `use` is a declaration, so a conditional import is always decided while names are
resolved. That is what makes the common target-gating form work.

```mach
rec MeshUniforms { model: [16]f32; }

# no arm declares: decided during type checking, so the gate may measure
$if ($size_of(MeshUniforms) != 64) {
    $error("MeshUniforms must be 64 bytes");
}
```

```mach error a layout intrinsic is only comptime-evaluable after type checking
rec MeshUniforms { model: [16]f32; }

# the second arm declares, so the whole chain is decided while names are
# resolved - and the gate is rejected there
$if ($size_of(MeshUniforms) != 64) {
    $error("MeshUniforms must be 64 bytes");
}
$or {
    val PADDING: u32 = 0;
}
```

The one visible consequence is ordering. A `$error` reached under a chain that
declares nothing is reported during type checking, so an unrelated name-resolution
error elsewhere in the same module is reported before it rather than after.

A `$if` inside a **function body** is always decided during type checking: it
selects statements rather than declarations, so the question above does not arise
and its gate may always ask about a type.

## Discarded branches

A `$if` branch that isn't taken is entirely absent from the compiled
output. For a target- or constant-gated `$if`, the compiler doesn't even
resolve names inside the untaken branches — this is the mechanism that makes
per-target asm blocks safe even when one block references registers the other
backend doesn't know about.

Inside a function body, a gate that names a constant from another module (for
example `$if (capability.HOSTED) { ... }`) is decided during type checking, after
names are resolved. Its arms are read the same way: a name that doesn't exist in
an arm the gate discards is never reported, exactly as with C's `#if`. A name
that doesn't exist in the arm the gate **selects** is reported as an ordinary
`unresolved identifier` or `unresolved type name` error, once, however many times
the enclosing function is instantiated. So such an arm may name what only exists
on the targets that select it.

```mach fragment
use capability: std.system.capability;

fun field_token(c: *Cursor, error: io_error.Error) {
    $if (capability.HOSTED) {
        # names that only a hosted target provides
        put_quoted(c, io_error.message(error));
    }
}
```

A `$if` gated on a comptime **parameter** is the other exception: because arm
selection happens per call site (at monomorphization), name resolution and
type checking run over **all** arms structurally, and only the selected arm is
emitted into each instance. Each arm must therefore be independently
resolvable and type-checkable.

A `$if` gated on a `$type_of` type comparison is *not* such an exception: at
monomorphization the operand's concrete type is known, so the provably-dead arms
are **pruned** and only the selected arm is type-checked (and emitted). Each arm
may therefore use its value at its own concrete type with no per-arm cast — see
[comptime-intrinsics.md](comptime-intrinsics.md).

Such a chain is decided only at an instantiation. While the operand's type still
names a generic parameter no arm is selected and none is type-checked, so a `!=`
gate and a bare `$or` fallback wait for the concrete type like every other gate,
and a `$error` in an arm the instantiation does not select never fires. An
instantiation that matches no arm selects the fallback, and its `$error` is
reported at that instantiation.

## Two regimes inside a generic body

A generic body is checked under two rules, and which one applies depends on what
the question is about. Both are stated here because the boundary between them is
the only thing a reader has to hold.

| the question | when it is answered | what is checked |
|---|---|---|
| a `$if` gated on a comptime **value parameter** | per call site | **all** arms, structurally, before any is selected |
| a `$if` gated on a `$type_of` or a type predicate | per instantiation | only the arm that instantiation selects |
| an operator, cast, `:~`, literal or condition on a **type parameter** | per instantiation | the whole body, once per distinct instantiation |

The first is the exception described under *Discarded branches*: arm selection
happens per call site, so every arm must be independently resolvable and
type-checkable and only the selected one is emitted.

The second and third are the same rule applied to different constructs. Nothing
about a type parameter is decided while it is still a parameter, because there is
no concrete type to decide it against: a gate waits for the instantiation, and so
does every operator. The template types the body so each instance and the lowering
have an expression table to read, and reports nothing of its own. A refusal
belongs to the instantiation that asked for the instance and names that instance's
concrete type — see [fun.md](fun.md).

The consequence is worth stating plainly: a generic that nothing instantiates is
not checked at all.

## See also

- [comptime-mach.md](comptime-mach.md) — `$mach.*` for target reads
- [asm.md](asm.md) — `asm` blocks gated by `$if`
- [statements.md](statements.md) — runtime `if` / `or` counterpart
- [fun.md](fun.md) — generic type parameters and per-instance checking
