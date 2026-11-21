# `$if` / `$or`: comptime control flow

`$if` and `$or` branch on comptime-evaluable conditions. Only the taken
branch compiles. The discarded branches are not resolved, type-checked,
or emitted into the binary. This is fundamentally different from runtime
`if` / `or`, which generates a branch at runtime.

A name a module declares only in a branch this build does not take, at module
scope, is left out of the build, and a use of it is reported with the reason.
The report of the name that does not resolve (`name.unresolved` within the
module, `visibility.not_exported` from another module, through a qualified
name, a `use` or a `fwd`) points at the declaration and at the condition that
left it out: the branch's own condition, which is false for this build, or, for
a branch after the one the build takes, the taken branch's condition. Inside a
nested chain the innermost condition that was not met is the one shown, and a
name declared in several branches that were not taken is shown at the first:

```
error[visibility.not_exported] no symbol `cpu_count` exported by `app.os`
  @ src/main.mach:9:23
9 |     val n: usize = os.cpu_count();
  |                       ^^^^^^^^^
  @ src/os.mach:6:14 declared here, in a branch this build does not select
6 |     fwd impl.cpu_count;
  |              ---------
  @ src/os.mach:5:6 this condition is false in this build
5 | $if (HAS_THREADS) {
  |      -----------
```

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
- Comparisons of comptime constants (`val` declarations)
- Comparisons of a comptime function parameter (`$mode`), see below

A condition is a `u1`, and a constant it reads has the width and sign its
declaration gives it. A constant declared through a `def`, as std's `bool` is
`def bool: u1`, takes the integer primitive the `def` chain ends at, followed
across `use` and `fwd`, so `$if (capability.HAS_FILES)` reads a `u1`. Any other integer is refused
with `gate.not_u1`. A chain
that ends outside the integers (a record, a pointer, a float) is refused at the
gate with `gate.const_not_integer`, in a message naming the chain. A `def` declared inside a `$if` arm is not
read this way: a declaring gate on a constant typed through one is decided once
types are checked.

A comptime comparison or arithmetic relates the **mathematical values** of its
operands, exactly as the runtime operators do (see
[operators.md](operators.md)). A constant in `2^63 .. 2^64-1` is its true
unsigned magnitude, so `$if (0xFFFFFFFFFFFFFFFFu64 > 0)` is taken and a cross-sign
comparison agrees with the runtime `if`: `$if (X < Y)` never selects a branch
that `if (X < Y)` would not. A gate's literal is typed as any literal is (see
[literals.md](literals.md)): an unsuffixed literal with no typed context is
`i64`, so the magnitude above needs its suffix. Comptime arithmetic computes the
exact result and never wraps to an operand's width: with `val Z: u8 = 255`,
`$if (Z + 1 > 255)` is taken. A result outside the 128-bit range the
evaluator works in (`i128`, or `u128` when an operand is unsigned) is refused
with `comptime.overflow` rather than wrapped.

## Examples

### Target-conditional code

```mach fragment
$if ($mach.build.sys == $mach.sys.linux) {
    use std.system.os.linux;
}
$or ($mach.build.sys == $mach.sys.windows) {
    use std.system.os.windows;
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
use std.print;
use std.runtime;

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
  call site (a literal, a `val` constant, or another comptime parameter). A runtime
  value is rejected with `comptime.argument_not_constant` (`comptime argument
  is not a compile-time constant`). A
  cross-module constant works, whether imported by bare name or as a qualified
  member (`alias.CONST`).
- Each arm gate of a comptime-parameter `$if` must itself be comptime-foldable:
  its identifiers must all be comptime (the comptime parameters or comptime
  constants). A gate referencing a runtime local/parameter is rejected.
- Any gate is refused with `gate.runtime_value` at a run-time binding it reads,
  and with `gate.not_constant` when it has no compile-time value for another
  reason, such as a call.
- A comptime parameter has no storage, so its address cannot be taken
  (`?mode` is rejected with `address.comptime_param`, `cannot take the address
  of a comptime parameter`).
- A comptime-parameter function is a template, not a value: it can only be
  called, not assigned, passed, or compared (`val fp: fun(i64) i64 = apply;` is
  rejected with `comptime.param_fn_value`, `cannot reference a comptime-parameter
  function as a value`).
- Comptime parameters carry no runtime cost: they are stripped from the lowered
  signature and ABI, so only the runtime parameters are passed.
- A comptime parameter may be mixed freely with runtime parameters in any order.
- A comptime parameter on a **generic** function (`fun f[T]($mode: u8, ...)`) is
  refused: a function has type instances or value instances, never both. The
  refusal stands at each call, which is reported as
  `generic.comptime_value_param` (`comptime value parameters on a generic
  function are not yet supported`). Like any generic, a declaration nothing
  calls is not type checked and reports nothing.
- The function may live in any module: a value-parameter instance is emitted
  against its declaring module and folds its `$if` gates against that module's
  own comptime constants, so a library can make a comptime-parameter function public
  gated on its own `val`s, private ones included.
- A comptime parameter may gate per-target asm safely, since each instance only
  compiles its taken arm:

```mach fragment
pub fun load($order: Order, ptr: *i64) i64 {
    var result: i64 = 0;

    $if ($mach.build.arch == $mach.arch.aarch64) {
        $if (order == RELAXED) {
            asm armv8-a {
                ldr x9, {ptr}
                ldr x9, [x9]
                str x9, {result}
            }
        }
        $or (order == ACQUIRE) {
            asm armv8-a {
                ldr x9, {ptr}
                ldar x9, [x9]
                str x9, {result}
            }
        }
    }

    ret result;
}
```

## When a declaration-scope `$if` is decided

A `$if` chain written in declaration scope runs at one of two times, and what its
arms contain picks which.

- **Some arm declares something.** The chain is decided while the modules load,
  because what it decides is which declarations exist and every later stage reads
  the resulting declaration set. Nothing has a type at that point, so the gate
  cannot ask a type question: `$size_of`, `$align_of`, `$length_of`, `$offset_of`, `$type_id`,
  `$type_of`, `$type_name` or an `$is_*` predicate there is rejected, with a
  message naming the question. So is one the gate reaches through a constant it
  reads, and the message names the constants it goes through.
- **No arm declares anything.** The chain contributes no name and no type whichever
  arm is taken, so nothing depends on deciding it early. It is decided during type
  checking instead, where its gate may measure a type (`$size_of`, `$align_of`,
  `$length_of`), query one (`$is_record` and friends), or compare one.

The question is answered from the **syntax**, over every arm (`$if`, every `$or`,
and the final `$else`-style `$or {}`) before any gate is evaluated. One declaring
arm anywhere keeps the whole chain at the earlier time. Per-arm answers are not
possible: which stage runs the gate would then depend on which arm the gate selects,
and the stage that would have to know that is the one being chosen.

A `use` is a declaration, so a conditional import is always decided while the modules
load. That is what makes the common target-gating form work.

A declaring gate reads build facts (`$mach.*`, `$bin.*`, `$project.*`) and `val`
constants from any module. The gates are decided in one pass that reads constants in
the order they depend on each other, not the order they are written in: a gate may
read a constant declared later in its own module, one declared under a later `$if`
(whose gate is decided first), or one a later `use` imports. A `val` a gate reads may
be typed through a `def`, followed across `use` and `fwd`, and a cast in the gate
converts to the integer its destination names. Constants and gates that depend on
each other have no order that decides them, so the gate is rejected with a message
naming the cycle:

```mach error depend on each other, so no order decides them
$if (A == 4) { pub val B: u32 = 4; }

$if (B == 4) { pub val A: u32 = 4; }
```

```mach
rec MeshUniforms { model: [16]f32; }

# no arm declares: decided during type checking, so the gate may measure
$if ($size_of(MeshUniforms) != 64) {
    $error("MeshUniforms must be 64 bytes");
}
```

```mach error `$size_of` asks a type question
rec MeshUniforms { model: [16]f32; }

# the second arm declares, so the whole chain is decided while the modules
# load - and the gate is rejected there
$if ($size_of(MeshUniforms) != 64) {
    $error("MeshUniforms must be 64 bytes");
}
$or {
    val PADDING: u32 = 0;
}
```

What the time changes for a module-scope `$error` is when it fires, which the
next section describes.

A `$if` inside a **function body** is always decided during type checking: it
selects statements rather than declarations, so the question above does not arise
and its gate may always ask about a type.

## A module refused by its own `$error`

A `$error` written at module scope, bare or in an arm of a declaration-scope
chain, states that the module cannot exist in this build. A module in which one
fires is **refused**:

- It reports every module-scope `$error` that fires in it and nothing else: no
  other error and no warning of the module is reported.
- A module whose source has a lexical or syntax error is never refused. It reports
  those errors, and none of its module-scope `$error`s is evaluated, since the gates
  they stand under were read from a tree the parser recovered. A syntax error is
  the same on every target, so it is never hidden behind a `$error` that fires on
  some targets only.
- A module that imports a refused module, directly or through modules that import
  it, is not analysed and reports nothing, its own module-scope `$error` included.
  So one refused module gives one diagnostic however long the chain of imports
  above it is, and two refused modules that do not import each other give one
  each.
- A refused module that is a file of the project being built reports at its
  `$error`. One outside the project, in a dependency such as std, reports where the
  project reached it, as a `$error` in an instance reports at the call that asked
  for the instance: the diagnostic stands at the import in the project's own source
  that reaches the refused module, directly or through refused modules between,
  with each `$error` that fired as the related site. Several project files may
  reach one refused module: the diagnostic stands at the first import that does,
  the project's files taken in the order the build loads them, the entry first, and
  each file's imports in their order. The walk from an import stops at a project
  file, which reports at its own imports.
- The build fails as it does for any error, and the summary counts the diagnostics
  it printed.

```mach fragment
# on any target but linux this module gives this one diagnostic and nothing else,
# and a module that imports it, or imports one that does, gives nothing. in a
# dependency, the diagnostic stands at the project's import that reaches it
$if ($mach.build.sys == $mach.sys.linux) {
    use native: std.system.os.linux;
}
$or {
    $error("this module needs a linux target");
}
```

Where the `$error` sits decides when the module is refused. Bare, or under a chain
name resolution decides (one that declares something, or one whose gate asks no
type question), it fires while names are resolved. Then neither the module nor
anything that imports it is resolved or type checked, so a `$error` of the module
under a chain only type checking decides is not reached. Under a chain that
declares nothing and asks a type question, it fires during type checking. Type
checking runs only once every module of the build resolves without error or
refusal, so until then the name-resolution errors are reported, the refused
module's included, and the `$error` is not yet reached. Once it fires, the modules
that import the refused one are not type checked, and nothing that loading or name
resolution said of the refused module or of them is reported.

A `$error` in a function body, or in an arm an instance selects, refuses nothing:
it is reported where the next sections say, unless its module is refused or not
analysed. `mach doc` only parses, decides no gate and fires no `$error`, so it
documents a module a build would refuse.

## Discarded branches

A `$if` branch that isn't taken is entirely absent from the compiled
output. For a target- or constant-gated `$if`, the compiler doesn't even
resolve names inside the untaken branches. This is the mechanism that makes
per-target asm blocks safe even when one block references registers the other
backend doesn't know about.

Inside a function body, a gate that names a constant from another module (for
example `$if (capability.HOSTED) { ... }`) is decided during type checking, after
names are resolved. Its arms are read the same way: a name that doesn't exist in
an arm the gate discards is never reported. A name
that doesn't exist in the arm the gate **selects** is reported as an ordinary
`unresolved identifier` or `unresolved type name` error, once, however many times
the enclosing function is instantiated. So such an arm may name what only exists
on the targets that select it.

```mach fragment
use capability: std.system.capability;
use io_error: std.io.error;

$if (capability.HOSTED) {
    use std.system.os;
}

fun field_token(c: *Cursor, error: io_error.Error) {
    $if (capability.HOSTED) {
        # `os` is imported only on a hosted target
        put_quoted(c, os.message(error));
    }
}
```

A `$if` gated on a comptime **parameter** is the other exception: because arm
selection happens per call site (at monomorphization), name resolution and
type checking run over **all** arms structurally, and only the selected arm is
emitted into each instance. Each arm must therefore be independently
resolvable and type-checkable.

What only a value can decide waits for the instance. Each instance evaluates
the gates against its own arguments, and a gate that fails there (an
`integer division by zero` in `8 / (n - 2)` when `n` is 2) is reported for
that instance. A `$error` reached in the arm an instance selects, directly or
under a nested `$if`, `$or` or `$each`, is reported once for each distinct
instance that selects it, at the call that asked for the instance, with the
`$error` itself as the related site. A `$error` in an arm no instance selects
never fires, and neither does one in a function nothing calls. A `$error`
outside any gated arm is reported once, at the `$error`, without waiting for
a call. None of these refuses its module, and none is reported from a module
that is [refused or not analysed](#a-module-refused-by-its-own-error).

```mach error no two
fun shifted($n: u8, x: u64) u64 {
    $if (n == 2) {
        $error("no two");
    }
    ret x << n;
}

fun use_it(x: u64) u64 {
    ret shifted(1, x) + shifted(2, x);   # shifted(2, ..) is refused, shifted(1, ..) is not
}
```

A `$if` gated on a `$type_of` type comparison is *not* such an exception: at
monomorphization the operand's concrete type is known, so the provably-dead arms
are **pruned** and only the selected arm is type-checked (and emitted). Each arm
may therefore use its value at its own concrete type with no per-arm cast (see
[comptime-intrinsics.md](comptime-intrinsics.md)). Names are still resolved in
every arm, so a name that does not exist is reported even in an arm no
instantiation selects.

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
| a `$if` gated on a comptime **value parameter** | per call site | **all** arms, structurally, before any is selected, and its gates and the `$error`s of the selected arm, per instance |
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
concrete type (see [fun.md](fun.md)).

The consequence is worth stating plainly: a generic that nothing instantiates is
not type checked at all. Only name resolution runs over it, so an unresolved
name in its body is still reported.

## See also

- [comptime-mach.md](comptime-mach.md): `$mach.*` for target reads
- [asm.md](asm.md): `asm` blocks gated by `$if`
- [statements.md](statements.md): runtime `if` / `or` counterpart
- [fun.md](fun.md): generic type parameters and per-instance checking
