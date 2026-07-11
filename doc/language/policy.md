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
  [manifest.md](../manifest.md)), not a compiler default.
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
