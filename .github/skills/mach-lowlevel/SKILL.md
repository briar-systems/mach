---
name: mach-lowlevel
description: Use when writing or reviewing Mach inline assembly, syscalls, or other target-specific code. Covers the asm <isa> { ... } form with {name} operand substitution, multi-arch dispatch via $if on $mach.target.arch, the compiler-infers-operands-and-clobbers model, and when to write asm versus call a standard-library wrapper.
---

# Mach low-level layer

Inline assembly and target-specific code. Two concerns: how to write an `asm`
block correctly, and when to reach for `asm` versus a stdlib wrapper.

## The `asm` form

One inline-assembly form: an ISA-tagged block of raw instruction lines.

```mach
asm <isa> {
    # raw instructions, one per line, # for comments
    mov rax, [{ptr}]
    mov {result}, rax
}
```

Locked rules:

- The ISA tag is **mandatory**. Bare `asm { ... }` does not exist.
- ISA tags are a closed set: `x86_64`, `aarch64`. **Only `x86_64` has a working backend today**; `aarch64` is a recognized tag for portable target-conditional source but has no code generator yet ([#1045](https://github.com/octalide/mach/issues/1045)). An `asm aarch64` block is therefore valid only inside a branch that gets comptime-discarded on an `x86_64` build.
- Body is **unquoted raw lines**, one instruction per line, in the ISA's native syntax. No surrounding quotes, no string concatenation. (Mach has no multi-line string; the `asm` body is not a string.)
- `#` introduces a line comment.

## Operand substitution — `{name}`

`{name}` substitutes a local in scope. The compiler resolves it to a memory or register operand from liveness and the instruction's expected operand class. Use the bare local name inside the braces; address it with the ISA's own syntax (e.g. `[{ptr}]` to dereference on x86_64).

```mach
pub fun add_via_asm(a: i64, b: i64) i64 {
    var result: i64 = 0;
    asm x86_64 {
        mov rax, {a}
        add rax, {b}
        mov {result}, rax
    }
    ret result;
}
```

`{name}` is the only substitution. There is no `in`/`out` syntax, no `%0` positional operands, no `=` constraint markers.

## What the compiler infers (do NOT declare these)

- **Operand direction** — read vs write, from operand position in the instruction.
- **Clobber set** — every register and flag each instruction touches, added to the enclosing function's clobber set automatically.
- **Memory clobber** — implicit and conservative: every `asm` block is assumed to modify arbitrary memory.

There is no clobber list and no operand-direction declaration. Writing one is invalid syntax, not optional metadata.

## Multi-arch dispatch

No nested arch-block construct exists inside `asm`. Dispatch at the outer level by wrapping each block in `$if` on `$mach.target.arch`:

```mach
$if ($mach.target.arch == $mach.arch.x86_64) {
    asm x86_64 {
        # x86_64 instructions
    }
}
$or ($mach.target.arch == $mach.arch.aarch64) {
    asm aarch64 {
        # aarch64 instructions
    }
}
$or {
    $error("unsupported arch");
}
```

- Only the taken branch compiles; discarded branches are not resolved or codegen'd. Each `asm` block only needs to be valid for its own tagged ISA.
- Compare with path values: `$mach.target.arch == $mach.arch.x86_64`. No `.id` suffix, no string compare.
- `$or` has **no condition** for the final else arm — `$or { ... }`. A conditional middle arm is `$or (COND) { ... }`. There is no `$or $if`.
- Same pattern keys off `$mach.target.os` against `$mach.os.linux` / `.darwin` / `.windows` / `.freestanding` for OS-specific bodies (e.g. syscall ABIs).

## When to write `asm` vs call stdlib

Write `asm` only for truly target-specific operations with no 1:1 stdlib wrapper: raw syscalls, special-register reads, stack-frame surgery.

For anything that maps to a named stdlib function — atomics, fences, traps, bit ops, the SIMD long-tail — **call the stdlib API**. Those wrappers already contain the arch-dispatched `asm`; reimplementing them inline duplicates the dispatch and loses the encapsulation. The standard library covers, among others:

- **Atomics** — load/store/cas/RMW, per arch × per ordering.
- **Memory fences and CPU hints** — pause, prefetch.
- **Traps / unreachable** — `trap()` with per-arch ISA-tagged blocks
  (`asm x86_64 { ud2 }` / `asm aarch64 { brk 0 }`).
- **Syscalls** — per-platform ABI wrappers.
- **Bit manipulation** — popcount, clz, ctz, bswap.
- **SIMD long tail** — shuffles, reductions, gather/scatter, saturating arithmetic, specialized math (once SIMD types land).

Rule of thumb: if the operation maps to a fixed instruction sequence per arch, there is (or should be) a stdlib wrapper — prefer it. Reach for raw `asm` only when no such wrapper exists.

## Variant parameters in stdlib wrappers

Memory orderings and similar runtime-relevant variants are passed as **comptime function parameters**, not as separate functions per variant. The `$name: T` form is for function parameters only — caller must pass a comptime-evaluable value, enforced at the call site.

```mach
fun atomic_load[T](ptr: *T, $order: Order) T { ... }
```

> **Implementation status.** A `$name: T` parameter is accepted in a signature
> but is not yet usable as a comptime constant inside the body (`$if (order ==
> RELAXED)` is not yet supported). Until it lands, branch on such a parameter
> with a runtime `if`.

## SIMD type spelling

Vector types follow `<u|i|f><width>x<count>`: `f32x4`, `i32x8`, `u8x16`. Higher dimensions are grammatically legal (`f32x4x4`). **Planned, not yet implemented** — these names do not resolve as types today. Once seeded, the basic operators on them are compiler intrinsics and everything beyond is stdlib.

## Mach gotchas that bite in low-level code

- **No type inference.** Every binding declares its type: `var result: i64 = 0;`. The `asm` block does not infer the result type from usage.
- **No compiler-known type aliases.** `usize`, `str` are stdlib `def`s, not built-ins. `$size_of`/`$align_of`/`$offset_of` resolve to comptime constant unsigned integers stored in whatever type the binding declares.
- **Strings are `"..."` → `*u8`, null-terminated** in the data segment. Backtick has **no role**: the lexer treats it as an unexpected character (a lex error) — never use it for an `asm` body or anything else.
- **`ext fun` is the only body-less function form.** No forward declarations; body-less signatures are reserved for C-ABI externals. Symbol rename via `$NAME.symbol = "real_name";`.
- **`fwd` is bare** (no `pub fwd`) and **always publishes** — it re-exports a symbol through a surface file, used by both topical splits (all impls forwarded unconditionally) and multiplatform splits (one impl forwarded per target).
- **No tagged unions, no `match`.** Discriminated values are a `rec` carrying a discriminator plus a `uni` payload (`rec{ kind: ValueKind; data: uni{...} }`); consumers branch with `if`/`or`. The compiler does not enforce kind/payload consistency.
- **Atomic / volatile are not in the primitive type name.** There are no bare-letter modifiers (`u32a` etc.); those concerns live outside the primitive name, via stdlib atomics.

## Reference

The authoritative low-level reference lives in the Mach repository under
[`doc/language/`](https://github.com/octalide/mach/tree/dev/doc/language) —
`asm.md` (the `asm` form, operand substitution, inferred clobbers, multi-arch
dispatch), `comptime-control.md` (`$if`/`$or` over `$mach.target.arch`), and
`policy.md` (the full compiler-vs-stdlib boundary). When a skill and the
reference disagree, the reference wins.
