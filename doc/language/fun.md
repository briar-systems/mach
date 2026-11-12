# `fun` — function

A function takes typed arguments, optionally returns a typed value, and
has a body of statements.

## Grammar

```mach fragment
fun NAME(args) RET { ... }              # function with return type
fun NAME(args) { ... }                  # no return type
fun NAME[T](args) RET { ... }           # generic over type parameters
fun NAME($p: T, args) RET { ... }       # comptime value parameter
fun NAME(fixed, va: ...) RET { ... }    # variadic pack parameter
```

## Examples

```mach
rec Pair[T, U] { left: T; right: U; }
var counter: i64 = 0;

pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}

pub fun bump() { # no return value
    counter = counter + 1;
}

pub fun identity[T](value: T) T {
    ret value;
}

pub fun make_pair[T, U](a: T, b: U) Pair[T, U] {
    var p: Pair[T, U];
    p.left  = a;
    p.right = b;
    ret p;
}
```

## Generic type parameters

Generic functions take type parameters in brackets `[T]`. There are no
constraints; any type may be substituted. The compiler monomorphizes per
unique type instantiation.

Call sites supply the types explicitly:

```mach fragment
val x: i64 = identity[i64](42);
val p: Pair[i64, u8] = make_pair[i64, u8](1, 2u8);
```

The same spelling with no call after it **names the instance as a value**. Its
type is the instantiated signature, so it can be stored, passed, returned, and
addressed:

```mach fragment
val f: fun(i64) i64 = identity[i64];   # the i64 instance, as a value
val p: ptr          = ?identity[i64];  # its address
ret apply(identity[i64], 42);          # passed as a callback
```

A generic itself is a template rather than code, so a bare `identity` has no
address and cannot be a value; only an instance can.

### A body is checked at each instantiation

There are no constraints on `T`, so nothing about a type parameter is decided
before an instantiation supplies its argument. An operator, a cast, a `:~`
reinterpret, a literal beside a `T`-typed operand and a branch condition are all
resolved against the instance's concrete type, so an ordered or numeric algorithm
is writable without a comparator parameter:

```mach
fun maxof[T](a: T, b: T) T {
    if (a > b) { ret a; }
    ret b;
}

fun sum[T](p: *T, n: u64) T {
    var acc: T   = 0; # the literal takes `T`
    var i:   u64 = 0;
    for (i < n) {
        acc = acc + p[i];
        i   = i + 1;
    }
    ret acc;
}
```

A type that genuinely does not support the operator is refused at the
instantiation that asked for it, naming that type:

```
error[operator.operand_type]: `Point` has no ordering: `<`, `<=`, `>` and `>=` order integers and floats
  --> src/main.mach:12:5
   |
12 |     maxof[Point](p, q);
   |     ^^^^^^^^^^^^^^^^^^
   |
  --> src/main.mach:4:9
   |
 4 |     if (a > b) { ret a; }
   |         ----- in this generic body, checked against this instance's type arguments
```

The template itself reports nothing past name resolution. It types the body so
each instance and the lowering have an expression table to read, and every type
judgement is the instance's, even one no type argument changes: an unresolved name
in a generic body is reported in the generic's own file, and `val q: i64 = "s";`
in one is reported only where an instance of it is asked for.

**A generic nothing instantiates is not checked.** `fun never_used[T](a: T) T {
ret a + a; }` compiles, because there is no type to decide `+` against and no
site to report at. The way to check a generic is to instantiate it, so a library
instantiates its own generics in its own tests.

### Instantiation ends or is refused

An instance asks for the instances its body calls, and those ask for theirs. A
call whose arguments derive from its caller's parameters without being them
(`f[*T]` from `T`, `f(n + 1)` from `n`, `f(x, va...)` from a pack) grows them, and
a cycle of calls through such an argument asks for a new instance at every step.

- **A cycle no comptime gate stops is refused** before any of it is expanded, as
  `generic.growing_cycle` at the growing call, naming the templates on the cycle.
  A gate is a comptime `$if` arm or an `$each`; a runtime `if` is not, since every
  instance of the caller compiles both of its branches.
- **A cycle with a gate on it runs** as far as the gate lets it. There is no step
  or count limit: `$if ($is_pointer(T)) { ret peel[$pointee_of(T)](x); }` peels any
  number of pointers. A gate that never closes runs until memory is refused, and
  under `-v` the readout shows each growing cycle as it expands, with the
  templates on it and its instance count so far.
- **Everything else is finite**: an instantiation outside a growing cycle can
  only reach the finitely many instances its arguments name.

```
error[generic.growing_cycle]: `g` instantiates `f` here at an argument that grows, on a cycle of instances no comptime gate can stop (f, g), so its instances never end
```

### Where an instance's code lives

Every instance is compiled once per build, whichever modules call it. A module's
object only declares the instances it calls. The code goes into an **instance
unit**: one object per generic's module and bucket, where the bucket is a fixed
hash (FNV-1a) of the instance's mangled name modulo 8. A generic, comptime-value
or pack instance of `m.f` therefore lands in one of the objects `m#instances.0`
through `m#instances.7`, and which one depends on its name alone.

- **Nothing in a unit depends on build order.** A unit lowers its instances in
  mangled-name order, and unit objects follow every module object in the link,
  ordered by module name and then bucket. Adding or dropping a use of an
  instance changes at most the one unit that holds it.
- **Linkage.** An instance is a merged definition, hidden, so no shared library
  exports it, `#[export]` or not. Every object that holds it holds a copy, and the
  link keeps the first copy in link order. Each copy carries the instance's
  **identity** in two parts. Its source is a digest of what it was made from:
  its mangled name and the source of every module its typing reads (its
  declarations and generic bodies, the files it embeds and the same of every
  module it imports). Its settings are the profile's float relaxations (`relax`)
  and the target's instruction set and extensions (`ext`), the two settings
  that change what the code computes or where it can run. The compiler, the
  optimization level, debug information and paths are left out. So two static
  libraries built from the same source and settings link together whatever
  profile or compiler build made each. Two built from different source, such as
  two versions of a dependency, are refused at link as built from different
  sources, with both inputs named, and the remedy is one version. Two built
  from the same source with a different `relax` or `ext` are refused as built
  with different settings, naming both inputs and the setting, and the remedy
  is building both with the same one. ELF, COFF and wasm linkers refuse either
  kind as a plain duplicate symbol. On Mach-O only mink refuses, since the
  format has no groups and other linkers coalesce weak definitions without a
  check. One build compiles one copy of each instance: a function's
  `#[extension]` admits instructions in its own inline `asm` only, so an
  instance it calls is compiled for the target's selection, or with the
  generic's own `#[extension]`, which is part of its source. A private declaration
  an instance body reaches, such as a helper or a table of the generic's module,
  is reached from the unit as a hidden symbol of the same image.
- **What the identity does not catch.** It is no authentication. An object made
  by hand with another toolchain that copies a real identity onto other code
  merges silently with the copies it imitates when it comes first in link order.
  Mach source cannot make one: a `#[symbol]` name that collides with an instance
  is refused at link as a definition beside the copies.
- **Debugging a merged instance.** The copy that runs is the one the link kept,
  built with the settings of the library that held it. When that library was
  built under another profile, a debugger steps through code optimized, or
  described, as that library was built, not as your own objects were. To choose
  the copy, put the library whose build you want first in link order.
- **Constant time.** A `constant_time` function's code is verified only against the
  callees its own object holds. So an instance it calls is also compiled into the
  caller's object, as a local copy under the same name, and the caller calls that
  copy.
- **Typed once, reported by every caller.** An instance is type checked once per
  build, against its generic's module and the modules declaring its type arguments,
  never against a caller, and its refusals are made and kept once. Each module whose
  own code reaches an instance with a refusal, directly or through other instances,
  reports it as its own, at its first call site by offset that leads there, with the
  body's own span as the related site. A module that reaches it through several
  call sites reports it once, so `mach check` shows a refusal once for each module
  that must change, and every file that asks for a broken instance shows it.
- **Reused across builds.** A unit's object stays in `obj/` beside its generic
  module's, keyed by its members and the surfaces of the modules they name, so a
  rebuild that changes none of those reuses it. A module's object record lists the
  instances its code asks for, and the cache directory keeps a record of each
  instance's typing (`.cache/instances`: the instances its body asks for and what
  it reported), keyed by the surfaces of the modules it reads. A rebuild reads the
  demand from those records, so a module whose object is reused and that nothing
  rebuilt imports runs no front end, and an instance whose record stands is not
  typed again. Changing a generic's body still rebuilds the modules that import it.
- **Every program of a build links every unit**, test programs included, and a
  `kind = "static"` library holds the unit objects next to its module objects.
- `-vv` lists units as `<module>#instances.<bucket>` beside the modules.

## Comptime value parameters

A parameter marked with `$name: T` must be supplied with a value the
compiler can resolve at compile time. The function body can branch on the
comptime parameter via `$if`, producing different code per call-site
instantiation.

```mach fragment
pub fun pick_op($mode: Mode, a: i64, b: i64) i64 {
    $if (mode == MODE_FAST) {
        ret a + b;
    }
    $or (mode == MODE_SAFE) {
        # extra logic here
        ret a + b;
    }
}
```

The argument is held to the parameter's type the way a runtime argument is. A
value the type cannot represent is refused as `comptime.overflow` at the
argument, never truncated: `narrow(300)` against `fun narrow($n: u8)` reports
`comptime argument 300 is out of range for u8 (0..255)`. An argument that names
an enclosing comptime parameter, as `narrow(m + 200)` inside `fun wide($m: u8)`,
is held to the type at each instance of the enclosing function, where `m` has a
value.

Comptime value parameters apply to function parameters only — not record
fields, not other contexts.

## Variadic packs

A function with a trailing named pack parameter (`va: ...`) accepts a
variable number of trailing arguments. The compiler monomorphizes the
function once per distinct call-site type-list; the pack is consumed by
`$each a in va` at compile time — there is no runtime `va_list`.

```mach
use std.print;
use std.runtime;

pub fun sum(va: ...) i64 {
    var t: i64 = 0;
    $each a in va {
        t = t + a;
    }
    ret t;
}

# leading fixed params are allowed before the pack
pub fun bias(base: i64, va: ...) i64 {
    var t: i64 = base;
    $each a in va { t = t + a; }
    ret t;
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{} {}", sum(10i64, 20i64, 30i64), bias(1, 10i64, 20i64, 30i64));
    ret 0;
}
```

`va.len` folds to the element count. `g(va...)` forwards the whole pack to
another pack-tailed callee. See [variadics.md](variadics.md) for the full
reference.

## Tail calls

A build with `optimize = true` treats a call whose result the function
returns unchanged, as in `ret f(a, b);`, as a tail call. A call to the function
itself becomes a loop: the recursion runs in the stack of one frame at any
depth. `ret f(n - 1) + x`, and the same with `*`, `|` or `^` on integers, is a
loop too, with the pending operands carried in an accumulator. A call to any
other function becomes a jump that hands the callee the caller's frame, so a
chain of such calls, mutual recursion included, also runs in constant stack.

A call stays a call when the function has a `fin` pending at the `ret`, when
the address of one of its locals escapes (to the callee or to any earlier
call), when it runs inline assembly or is `#[naked]` or `#[constant_time]`, when a
by-value argument is a copy the caller has to make, or when the callee takes
more stack arguments than the caller received. A build with `optimize = false`
makes no tail calls, and neither does one that skips the `tailrec` and `tailcall`
passes.

`debug = true` changes none of this: debug information is added to the same code. A
debugger or unwinder therefore never sees the frame a tail call left. A
backtrace taken inside the callee shows it directly below the caller's own
caller, and a self tail call shows as the one frame of the loop, its parameters
holding the current iteration's values. A local of the caller that holds the
call's result reads as optimized out, since the caller never receives it.

## See also

- [ext-fun.md](ext-fun.md) — body-less external functions
- [variadics.md](variadics.md) — variadic pack parameter reference
- [comptime-control.md](comptime-control.md) — `$if` inside function bodies, and
  the two regimes a generic body is checked under
- [operators.md](operators.md) — the operator table an instance is checked against
- [expressions.md](expressions.md) — function calls and generic
  instantiation
