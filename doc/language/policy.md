# Backend abstraction policy

Where things live — compiler vs stdlib. The boundary is drawn to keep the
compiler small and the stdlib readable.

## Compiler handles

Things that need to feel like the language:

- **Type system.** Primitive types, pointers, arrays, function types,
  records, unions, generics.
- **Control flow.** `if` / `or`, `for`, `ret`, `brk`, `cnt`, `fin`,
  blocks.
- **SIMD operators on primitive vector types.** Lane-wise arithmetic,
  bitwise, comparison-to-mask, lane indexing, and full-arity vector
  literals over the seeded 128-bit vector types (the honest per-operator
  table is in [operators.md](operators.md)). On a target with the hardware
  (SSE2 on x86_64, NEON on aarch64) the compiler emits one instruction per
  operator; on a target without it the compiler emits a **defined unrolled
  scalar expansion** of the same operator — scalarize operators, never
  algorithms — and reports the scalarization at build time. What to do on
  an incapable target is the `simd` profile lever (see
  [manifest.md](manifest.md)), not a compiler default.
- **`asm` parsing, encoding, and operand allocation** for each supported
  ISA.
- **The comptime channel** — `$mach.*` reads, the closed intrinsic set,
  `$if` / `$or`.
- **Comptime function parameter dispatch** — turning `$name: T` parameters
  into per-instantiation specializations.

## Stdlib handles

Things that map 1:1 to specific instruction sequences:

- **Atomics** — load, store, cas, RMW family. Per arch × per ordering, via
  `asm` bodies inside library functions.
- **Memory fences and CPU hints** — pause, prefetch.
- **Traps and unreachable markers** — `trap()` is a stdlib function with
  per-arch `asm { ud2 / brk 0 / ... }`.
- **Syscalls** — per-platform syscall ABI wrappers.
- **CPU feature detection** — CPUID-style reads at runtime.
- **The long tail of SIMD ops** — shuffles, reductions, gather/scatter,
  saturating arithmetic, specialized math. Functions over the
  compiler-known SIMD types.
- **Bit manipulation** — popcount, clz, ctz, bswap. Wrappers around the
  arch-specific instruction.
- **String / number formatting, parsing, math, allocators** — pure Mach
  built on the primitives above.

## The dividing rule

> Compiler handles things that need to feel like the language. Stdlib
> handles things that map 1:1 to specific instruction sequences with
> predictable lowerings.

The compiler grows only when something genuinely cannot be expressed as a
1:1 instruction sequence per arch — autovectorization, 128-bit arithmetic
that benefits from context-dependent lowering, and similar.

## The constant-time multiply fails closed

A secret `*` is the one operator whose legality is a fact about the machine
rather than the source, and it is split along the same line, with one rule
on both sides: **a secret multiply either reaches the machine as the declared
data-independent instruction or does not run at all.** Nothing on either side
substitutes a slower or leakier path.

- **The compiler owns the decision.** Each instruction set declares the
  multiply cells it can execute in data-independent time and the condition
  each holds under (always, PSTATE.DIT on, or extensions selected), and the
  lowering gate, the `#[oblivious]` validators and `$mach.build.ct_mul` read
  one admission function over those rows. An undeclared cell, an unmet
  condition or an operating system that declares no guarantee for the mode
  is a compile error naming what is missing. The compiler never emits a
  bit-serial loop, a shift-add expansion or a call in its place, and it has
  no multiply strength reduction, so a secret square or a secret multiply by
  a constant is still the one instruction. A library that must build on
  every target picks its own serial path under `$if ($mach.build.ct_mul(...))`
  rather than being handed one.
- **The stdlib owns the mode.** Where a row holds only under PSTATE.DIT, the
  compiler marks the module and the linker writes one byte,
  `__mach_dit_required`, into every executable linked for an OS that declares
  the guarantee. std's start code reads it before `main` and at the entry of
  every thread it creates: a zero byte touches nothing, a nonzero byte turns
  the mode on when the OS reports the processor has it, and otherwise, or
  when the answer cannot be read, std writes its refusal line and ends the
  process with status 255 before any secret is multiplied. The decision is a
  pure function of the byte and the availability answer, unit tested on any
  processor.

The per-instruction-set table, the conditions and the runtime rule are in
[secrecy.md](secrecy.md#constant-time-multiply-by-instruction-set) and
[PSTATE.DIT at run time](secrecy.md#pstatedit-at-run-time).

## Why this works

This boundary minimizes compiler intrinsics. Users can read stdlib source
to see exactly what their code lowers to, and fork it for exotic needs.
The compiler stays small because the stdlib does most of the platform
work in plain Mach with `asm` bodies.

## See also

- [asm.md](asm.md) — the `asm` form stdlib functions are built on
- [comptime-control.md](comptime-control.md) — `$if` for per-arch dispatch
- [comptime-intrinsics.md](comptime-intrinsics.md) — what IS a compiler
  intrinsic vs what's deferred to stdlib
