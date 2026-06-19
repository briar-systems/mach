# Variadic packs

A **variadic pack** (`va: ...`) is a trailing function parameter that collects
a variable number of call-site arguments into a compile-time sequence. The
compiler monomorphizes the function once per distinct argument type-list at
each call site; there is no runtime structure, no `va_list`, and no `any`.

## Declaring a pack parameter

A pack parameter is written as a named parameter whose type is `...`:

```mach
fun name(va: ...) RetType { ... }
fun name(fixed: T, va: ...) RetType { ... }   # leading fixed params are allowed
```

The pack parameter must be last. Any number of fixed (typed) parameters may
precede it.

## Iterating with `$each`

`$each a in va` unrolls the body once per element, with `a` bound to the
element and re-typed to that element's concrete type per instantiation.
This is the only way to consume a pack.

```mach
fun sum(va: ...) i64 {
    var t: i64 = 0;
    $each a in va {
        t = t + a;
    }
    ret t;
}

sum(1, 2, 3)       # 6
sum(10, 20)        # 30
sum()              # 0 — empty pack, body never runs
```

Because each element has its own concrete type at monomorphization, the body
can handle heterogeneous packs:

```mach
fun sumc(va: ...) i64 {
    var t: i64 = 0;
    $each a in va {
        t = t + a::i64;          # cast each element's concrete type to i64
    }
    ret t;
}

sumc(1000, 50::i32, 7::u8, 200::i16)   # 1257
```

A `$each` body is a normal statement block; it may nest arbitrarily, call
functions, read outer-scope runtime variables, and write them back. Runtime
variables in the enclosing scope (e.g. a cursor) thread across all unrolled
iterations — each iteration reads where the previous one left off.

## `va.len` — element count

`va.len` folds to the instance's element count at compile time.

```mach
fun count(va: ...) i64 { ret va.len::i64; }

count(1, 2, 3)   # 3
count()          # 0
```

## `va...` — forwarding a whole pack

Inside a pack instance, `g(va...)` forwards the whole pack to another
pack-tailed function, which is monomorphized for the forwarded type-list.

```mach
fun outer(va: ...) i64 { ret sum(va...); }

outer(1, 2, 3)   # forwards (1, 2, 3) to sum → 6
```

Leading fixed arguments may precede the spread at the call site:

```mach
fun fwdpre(base: i64, va: ...) i64 { ret base + sum(va...); }

fwdpre(100, 1, 2, 3)   # 106
```

`va...` is valid **only as the sole trailing argument of a pack-tailed
callee**. The compiler rejects:
- Spreading into a callee with no pack parameter.
- Spreading when other arguments follow the spread.
- Spreading only part of a pack (partial forward is not supported).

## Monomorphization and ABI

A pack-tailed function is compiled once per distinct argument type-list at
each call site. Different arities, or the same arity with different types,
produce separate instances:

```mach
sum(1, 2, 3)           # instance: (i64, i64, i64)
sum(10, 20)            # instance: (i64, i64)
sum(5::u8, 1::u32)     # instance: (u8, u32)
```

Because a pack-tailed function has no single entry point, it is **not a
stable-ABI symbol** — it cannot be the target of `ext fun` or a function
pointer shared across compilation units. It is source-level only.

## Cross-module packs

Pack-tailed functions may call functions in other modules from inside the
`$each` body. The compiler re-infers the body per monomorphization instance
against the full module set.

```mach
fun sumdbl(va: ...) i64 {
    var t: i64 = 0;
    $each a in va {
        t = t + helper.dbl(a);   # helper.dbl resolved per element type
    }
    ret t;
}
```

## Real-world example: format

The standard library's `vformat` is a pack-tailed function. Each `$each`
iteration handles one format argument in order, with `$type_of` dispatch
selecting the right writer per element type:

```mach
pub fun vformat(w: *Writer, fmt: str, va: ...) Result[usize, str] {
    var i: usize = 0;
    $each arg in va {
        for (fmt[i] != 0 && fmt[i] != '{') { write_byte(w, fmt[i]); i = i + 1; }
        if (fmt[i] != '{') { ret err[usize, str](ERR_FEW_HOLES); }
        i = i + 2;
        $if ($type_of(arg) == str) { write_str(w, arg); }
        $or ($type_of(arg) == i64) { write_i64(w, arg); }
        $or { $error("no writer for this argument type"); }
    }
    for (fmt[i] != 0) { write_byte(w, fmt[i]); i = i + 1; }
    ret ok[usize, str](i);
}
```

The runtime format cursor `i` threads across all unrolled iterations; the
arg sequence is consumed entirely at compile time.

## See also

- [fun.md](fun.md) — function declarations, generic and comptime parameters
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$type_of`, `$fields`, `$each`
- [comptime-control.md](comptime-control.md) — `$if` / `$or` used inside pack bodies
