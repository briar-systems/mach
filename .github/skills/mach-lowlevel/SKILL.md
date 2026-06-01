---
name: mach-lowlevel
description: Use when writing or reviewing Mach inline assembly or deciding whether functionality belongs in the compiler or the stdlib. Covers the asm <isa> { ... } form with {name} operand substitution, multi-arch dispatch via $if on $mach.target.arch, the compiler-infers-operands-and-clobbers model, and the compiler-vs-stdlib boundary policy (SIMD operators and asm encoding in the compiler; atomics, fences, syscalls, SIMD long-tail in stdlib).
---

# Mach low-level layer

Inline assembly and the compiler-vs-stdlib boundary. Two concerns: how to write an `asm` block correctly, and where a low-level operation belongs.

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
- ISA tags are a closed set tracking backend support: `x86_64`, `aarch64`, … A tag exists because the compiler can target it.
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

For anything that maps to a named stdlib function — atomics, fences, traps, bit ops, the SIMD long-tail — **call the stdlib API**. Those wrappers already contain the arch-dispatched `asm`; reimplementing them inline duplicates the dispatch and loses the encapsulation.

## Compiler vs stdlib boundary

> Compiler handles things that need to **feel like the language**. Stdlib handles things that map **1:1 to specific instruction sequences** with predictable lowerings.

**Compiler** (intrinsic, language-level):
- Type system, control flow (`if`/`or`, `for`, `ret`, `brk`, `cnt`, `fin`).
- **SIMD operators on primitive vector types** — arithmetic (`+ - * / %`), bitwise (`& | ^ ~ << >>`), comparison, lane indexing, vector literals. One CPU instruction per operator per ISA.
- **`asm` parsing, encoding, and operand allocation** per ISA.
- The comptime channel — `$mach.*` reads, attribute writes, the closed intrinsic set, `$if`/`$or`.
- Comptime function-parameter dispatch (`$name: T`).

**Stdlib** (1:1 instruction sequences, built on `asm`):
- **Atomics** — load/store/cas/RMW, per arch × per ordering.
- **Memory fences and CPU hints** — pause, prefetch.
- **Traps / unreachable** — `trap()` with per-arch `asm { ud2 / brk 0 / ... }`.
- **Syscalls** — per-platform ABI wrappers.
- **CPU feature detection** — CPUID-style runtime reads.
- **SIMD long tail** — shuffles, reductions, gather/scatter, saturating arithmetic, specialized math. Functions over the compiler-known SIMD types.
- **Bit manipulation** — popcount, clz, ctz, bswap.
- **Formatting, parsing, math, allocators** — pure Mach over the primitives.

Decision rule for placement:
- Does it need to feel like the language (an operator, a type, control flow)? Compiler.
- Does it map to a fixed instruction sequence per arch with a predictable lowering? Stdlib, via `$if ($mach.target.arch == ...)` → `asm <isa> { ... }`.
- The compiler grows **only** when something genuinely cannot be expressed as a 1:1 instruction sequence per arch (autovectorization, 128-bit arithmetic needing context-dependent lowering). Default is stdlib.

## Variant parameters in stdlib wrappers

Memory orderings and similar runtime-relevant variants are passed as **comptime function parameters**, not as separate functions per variant. The `$name: T` form is for function parameters only — caller must pass a comptime-evaluable value, enforced at the call site.

```mach
fun atomic_load[T](ptr: *T, $order: Order) T { ... }
```

## SIMD type spelling

Vector types follow `<u|i|f><width>x<count>`: `f32x4`, `i32x8`, `u8x16`. Higher dimensions are grammatically legal (`f32x4x4`) and populated as backends accelerate. The set is compiler-shipped and closed per target. Operators on these types are compiler intrinsics; everything beyond the basic operator set is stdlib.

## Mach gotchas that bite in low-level code

- **No type inference.** Every binding declares its type: `var result: i64 = 0;`. The `asm` block does not infer the result type from usage.
- **No compiler-known type aliases.** `usize`, `str` are stdlib `def`s, not built-ins. `$size_of`/`$align_of`/`$offset_of` resolve to comptime constant unsigned integers stored in whatever type the binding declares.
- **Strings are `"..."` → `*u8`, null-terminated** in the data segment. Backtick is **reserved and unused** — never use it for an `asm` body or anything else.
- **`ext fun` is the only body-less function form.** No forward declarations; body-less signatures are reserved for C-ABI externals. Symbol rename via `$NAME.symbol = "real_name";`.
- **`fwd` is bare** (no `pub fwd`) and **always publishes** — it re-exports a symbol through a surface file, used by both topical splits (all impls forwarded unconditionally) and multiplatform splits (one impl forwarded per target).
- **No tagged unions, no `match`.** Discriminated values are a `rec` carrying a discriminator plus a `uni` payload (`rec{ kind: ValueKind; data: uni{...} }`); consumers branch with `if`/`or`. The compiler does not enforce kind/payload consistency.
- **Atomic / volatile are not in the primitive type name.** There are no bare-letter modifiers (`u32a` etc.); those concerns live outside the primitive name, via stdlib atomics.

## Reference

- [doc/language/asm.md](../../../doc/language/asm.md) — the `asm` form, operand substitution, inferred clobbers, multi-arch dispatch.
- [doc/language/policy.md](../../../doc/language/policy.md) — the compiler-vs-stdlib boundary in full.
- [doc/language/comptime-control.md](../../../doc/language/comptime-control.md) — `$if`/`$or` over `$mach.target.arch`.
- [doc/language/comptime-intrinsics.md](../../../doc/language/comptime-intrinsics.md) — what is a compiler intrinsic vs deferred to stdlib.
- [SPEC.md](../../../SPEC.md) — locked decisions, source of truth.
