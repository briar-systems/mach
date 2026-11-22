# `fun`: function

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
constraints, and any type may be substituted. The compiler monomorphizes per
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
address and cannot be a value. Only an instance can.

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

The literal takes `T` under the same rules as anywhere else. An integer literal
is never implicitly a float, so `sum[f64]` is refused as `type.mismatch`
(`expected T (that is f64), found i64`) at the call that asks for it, pointing at
`var acc: T = 0;` in the body, just as `var x: f64 = 0;` is refused. A
generic meant for floats too converts the literal, as `var acc: T = 0::T;`, which
every numeric instance accepts.

A type that genuinely does not support the operator is refused at the
instantiation that asked for it, naming that type:

```
error[operator.operand_type] `Point` has no ordering. `<`, `<=`, `>` and `>=` order integers and floats
   @ src/main.mach:12:5
12 |     maxof[Point](p, q);
   |     ^^^^^^^^^^^^^^^^^^
   @ src/main.mach:4:9 in this generic body, checked against this instance's type arguments
 4 |     if (a > b) { ret a; }
   |         -----
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
(`f[*T]` from `T`, `f(n + 1)` from `n`, `f(?a)` from an element `a` that `$each`
binds over a pack) grows them, and a cycle of calls through such an argument
asks for a new instance at every step. A spread cannot grow a pack, since `va...`
must be the sole trailing argument and forwards the whole pack. A call that puts
another argument into the callee's pack beside it, as `h(x, va...)` inside
`fun h(va: ...)`, is refused as `call.spread`.

- **A cycle no comptime gate stops is refused** before any of it is expanded, as
  `generic.growing_cycle` at the growing call, naming the templates on the cycle.
  A gate is a comptime `$if` arm or an `$each`. A runtime `if` is not, since every
  instance of the caller compiles both of its branches.
- **A cycle with a gate on it runs** as far as the gate lets it. There is no step
  or count limit: `$if ($is_pointer(T)) { ret peel[$pointee_of(T)](x); }` peels any
  number of pointers. A gate that never closes runs until memory is refused, and
  under `-v` the readout shows each growing cycle as it expands, with the
  templates on it and its instance count so far.
- **Everything else is finite**: an instantiation outside a growing cycle can
  only reach the finitely many instances its arguments name.

```
error[generic.growing_cycle] `g` instantiates `f` here at an argument that grows, on a cycle of instances no comptime gate can stop (f, g), so its instances never end
```

### Where an instance's code lives

An instance is compiled into every object whose code reaches it, directly or
through other instances, and the link keeps one copy. A module's object holds
its own code and each instance that code calls, and its test object holds what
its tests reach. No object is made for instances alone.

- **Nothing in an object depends on build order.** A module lowers its own
  declarations, then each instance its code reaches, in the order its code
  reaches them.
- **Linkage.** An instance is a merged definition, hidden, so no shared library
  exports it, `#[export]` or not. Every object that holds it holds a copy, and the
  link keeps the first copy in link order. Each copy carries the instance's
  **identity** in two parts. Its source is a digest of what it was made from:
  its mangled name and the source of every module its typing reads (its
  declarations and the bodies of its generic, comptime-value and pack
  functions, the files it embeds and the same of every module it imports). Its settings are the profile's float relaxations (`relax`)
  and the target's instruction set and extensions (`ext`), the two settings
  that change what the code computes or where it can run. The compiler, the
  optimization setting, debug information and paths are left out. So two static
  libraries built from the same source and settings link together whatever
  profile or compiler build made each. Two built from different source, such as
  two versions of a dependency, are refused at link as built from different
  sources, with both inputs named, and the remedy is one version. Two built
  from the same source with a different `relax` or `ext` are refused as built
  with different settings, naming both inputs and the setting, and the remedy
  is building both with the same one. ELF, COFF and wasm linkers refuse either
  kind as a plain duplicate symbol. On Mach-O only mink refuses, since the
  format has no groups and other linkers coalesce weak definitions without a
  check. Every copy of an instance in one build is compiled the same way: a
  function's `#[extension]` admits instructions in its own inline `asm` only, so
  an instance it calls is compiled for the target's selection, or with the
  generic's own `#[extension]`, which is part of its source. A private
  declaration an instance body reaches, such as a helper or a table of the
  generic's module, is reached from another module's object as a hidden symbol
  of the same image. On a target whose objects hold no merged definitions
  (SPIR-V), an instance is a weak definition instead.
- **What the identity does not catch.** It is no authentication. An object made
  by hand with another toolchain that copies a real identity onto other code
  merges silently with the copies it imitates when it comes first in link order.
  Mach source cannot make one: a `#[symbol]` name that collides with an instance
  is refused when its module is compiled if the instance is in the same module,
  and at link as a definition beside the copies otherwise.
- **Debugging a merged instance.** The copy that runs is the one the link kept,
  built with the settings of the library that held it. When that library was
  built under another profile, a debugger steps through code optimized, or
  described, as that library was built, not as your own objects were. To choose
  the copy, put the library whose build you want first in link order.
- **Constant time.** A `constant_time` function's code is verified only against the
  callees its own object holds. So an instance it calls is a local copy in the
  caller's object, under the same name, and the caller calls that copy. Other code
  of the same object calls that copy too.
- **Typed once, reported by every caller.** An instance is type checked once per
  build, against its generic's module and the modules declaring its type arguments,
  never against a caller, and its refusals are made and kept once. Each module whose
  own code reaches an instance with a refusal, directly or through other instances,
  reports it as its own, at its first call site by offset that leads there, with the
  body's own span as the related site. A module that reaches it through several
  call sites reports it once, so `mach check` shows a refusal once for each module
  that must change, and every file that asks for a broken instance shows it.
- **Reused across builds.** An instance is keyed by its mangled name, the text
  of its generic's module without the bodies of its plain functions and tests,
  and the same of every module its typing reads, and of every module each of
  those imports. A module's object is keyed by its own source and the
  declarations of the modules it imports, never their function bodies, and its
  record lists each instance it holds under the instance's key. The object is
  rebuilt when one of those keys changes, so changing a generic's body rebuilds
  the objects that hold an instance of it and no other. A module whose object is
  reused types none of its instances, and when nothing rebuilt imports it, it
  runs no front end. A warning an instance reports is replayed with the
  object, and the object is rebuilt when a source outside its module that a
  replayed warning points into has changed. The same holds in every profile,
  `optimize` included.

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

A comptime value parameter sizes types in the body of its function. Each
instance resolves the types its parameters size at its own values, so
`var xs: [N]i64;` in `fun h($N: i64)` is three elements long in `h(3)` and one in
`h(1)`. `$length_of([N]i64)` and `$size_of([N]i64)` read the same instance's
length:

```mach
use std.runtime;

fun count($N: i64) i64 {
    var xs: [N]i64;
    ret $length_of([N]i64) + $size_of([N]i64)::i64;
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    if (count(3) != 27) { ret 1; }
    ret 0;
}
```

A body with such a type is judged at each instance, as a generic body is: what
it reports, it reports at the call that asked for the instance, against that
instance's values, and a function no call instantiates is not judged. A body
whose types no comptime parameter sizes is judged once, where it is written.

A comptime parameter never sizes a type in the function's own signature. One
in a parameter type or the return type is refused as `array.length_not_const`:

```mach error a comptime parameter sizes a type only in the body of its function, never in a parameter or return type
fun first($N: i64, xs: [N]i64) i64 {
    ret xs[0];
}
```

Comptime value parameters apply to function parameters only, never to record
fields or any other context.

## Variadic packs

A function with a trailing named pack parameter (`va: ...`) accepts a
variable number of trailing arguments. The compiler monomorphizes the
function once per distinct call-site type-list. The pack is consumed by
`$each a in va` at compile time, and no runtime `va_list` is involved.

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

A call is always a call. A call whose result the function returns unchanged, as
in `ret f(a, b);`, is compiled as an ordinary call followed by a return, in
every profile, `optimize = true` included. A call to the function itself is not
turned into a loop and a call to another function is not turned into a jump, so
each level of recursion, mutual recursion included, takes a frame of its own and
the depth a recursion can reach is bounded by the stack. An algorithm that must
run at any depth is written as a `for` loop.

Since every frame stays on the stack, a debugger or unwinder sees each call in a
backtrace, under `debug = true` and with any `optimize` setting.

## See also

- [ext-fun.md](ext-fun.md): body-less external functions
- [variadics.md](variadics.md): variadic pack parameter reference
- [comptime-control.md](comptime-control.md): `$if` inside function bodies, and
  the two regimes a generic body is checked under
- [operators.md](operators.md): the operator table an instance is checked against
- [expressions.md](expressions.md): function calls and generic
  instantiation
