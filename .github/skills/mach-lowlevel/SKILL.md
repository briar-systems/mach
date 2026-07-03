---
name: mach-lowlevel
description: Use when writing or reviewing Mach inline assembly, syscalls, or other target-specific code. Covers the asm <isa> { ... } form (x86_64/aarch64/riscv64) with {name} operand substitution, the compiler-infers-operands-and-clobbers model, multi-arch dispatch via $if on $mach.build.arch, module target guards, comptime-parameter variant dispatch, and when to write asm versus call the standard library.
---

# Mach low-level layer

Inline assembly and target-specific code: how to write an `asm` block
correctly, and when to reach for `asm` versus a stdlib wrapper.

## The `asm` form

One inline-assembly form: an ISA-tagged block of raw instruction lines.

```mach
asm x86_64 {
    # raw instructions, one per line, # for comments
    mov rcx, {ptr}
    mov rax, [rcx]
    mov {result}, rax
}
```

Locked rules:

- The ISA tag is **mandatory**; bare `asm { }` is rejected. The tag set is
  closed: `x86_64`, `aarch64`, `riscv64` — each with a working native
  assembler. All three are exercised in CI (riscv64 binaries run under
  qemu; the compiler itself does not yet self-host on riscv64).
- The body is **raw text**, not tokens: unquoted lines in the ISA's native
  syntax, captured to the brace-matched `}` (nested braces balance). Mach has
  no multi-line string; the `asm` body is not a string.
- `#` starts a line comment; everything after it on the line is inert.

## Operand substitution — `{name}`

`{name}` substitutes a local in scope; the compiler resolves it to a register
or memory operand from liveness and the instruction's operand class. In
practice a `{name}` binds the local's **storage — typically a stack slot** —
so to reach a pointee, stage the pointer through a scratch register first;
never write a double indirection like `[{ptr}]`:

```mach
mov rcx, {ptr}          # x86_64: load the pointer value
mov rax, [rcx]          # then address through the register
                        # aarch64: ldr x12, {ptr}  /  ldr x9, [x12]
```

Branch targets inside a block are numeric local labels with direction
suffixes (`1:` … `bnez a4, 1b`, `b.ne 2f`), the stdlib's convention across
all three ISAs.

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

`{name}` is the only substitution: no `in`/`out` lists, no `%0` positionals,
no `=` constraints. The compiler infers **operand direction** (from position),
the **clobber set** (from each instruction's semantics), and assumes a
conservative **memory clobber** for every block. Writing a clobber list is a
syntax error, not optional metadata.

## Multi-arch and multi-OS dispatch

No nested arch construct exists inside `asm`. Dispatch at the outer level with
`$if` on `$mach.build.arch` (or `.os` for syscall ABIs); discarded branches
never compile, so each block only needs to be valid for its own ISA:

```mach
$if ($mach.build.arch == $mach.arch.x86_64) {
    asm x86_64 { hlt }
}
$or ($mach.build.arch == $mach.arch.aarch64) {
    asm aarch64 { brk 0 }
}
$or ($mach.build.arch == $mach.arch.riscv64) {
    asm riscv64 { ebreak }
}
$or {
    $error("mymod.trap: unsupported architecture");
}
```

Compare path-values (`$mach.arch.x86_64`), final else is bare `$or { }`. A
platform-specific module guards itself at the top so misuse fails loudly at
compile time — the stdlib pattern:

```mach
$if ($mach.build.arch != $mach.arch.x86_64) {
    $error("myproj.os.linux.x86_64: requires x86_64 target");
}
```

## Variant dispatch — comptime parameters

Memory orderings and similar variants are one function with a `$name: T`
comptime parameter, not a function per variant. The compiler monomorphizes per
distinct value and each instance compiles only its selected `asm` arm:

```mach
pub fun load($order: u8, ptr: *i64) i64 {
    var result: i64 = 0;
    $if ($mach.build.arch == $mach.arch.aarch64) {
        $if (order == RELAXED) {
            asm aarch64 {
                ldr x12, {ptr}
                ldr x9, [x12]
                str x9, {result}
            }
        }
        $or (order == ACQUIRE) {
            asm aarch64 {
                ldr x12, {ptr}
                ldar x9, [x12]
                str x9, {result}
            }
        }
    }
    ret result;
}
```

The call site must pass a comptime-evaluable value. Full rules in the
**mach-comptime** skill.

## `asm` vs stdlib

Write `asm` only for truly target-specific operations with no stdlib wrapper:
raw syscalls, special-register reads, stack-frame surgery. For anything that
maps to a named function — atomics, fences, `trap()`, bit ops (popcount, clz,
bswap), syscall wrappers — **call the stdlib**: those functions already
contain the arch-dispatched `asm`, and reimplementing them inline duplicates
the dispatch. Rule of thumb: if the operation is a fixed instruction sequence
per arch, there is (or should be) a stdlib wrapper.

## Secrecy — `asm` is a trusted-base crossing

The `^` secret qualifier's flow rules stop at an `asm` boundary: the type
system cannot check instruction streams, so an `asm` block can observe or
launder secrets silently. Inside `asm` that touches `^` data, constant-time
discipline (no secret-dependent branches, addresses, or variable-latency
instructions) is entirely on you. See `doc/language/secrecy.md`.

## Gotchas that bite in low-level code

- **Every binding declares its type** — `var result: i64 = 0;` before the
  block; nothing is inferred from `asm` usage.
- **Decorators are `#[...]`**: pin a symbol for an asm branch target with
  `#[symbol("_my_trampoline")]`; import a syscall with
  `#[symbol("write")] ext fun ...`. Backticks are removed syntax.
- **`usize`/`str` are stdlib defs**; `$size_of`/`$offset_of` produce untyped
  comptime integers stored in whatever type the binding declares.
- **`ext fun` is the only body-less form** — reserved for C-ABI externals.
- **No tagged unions/`match`** — discriminated values are `rec` + `uni` with
  `if`/`or` chains.
- **SIMD vector types are not seeded yet** (`f32x4` does not resolve); the
  SIMD long tail will live in stdlib once they land.

## Reference

Authoritative docs in the Mach repository under
[`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language):
`asm.md` (form, substitution, inference), `comptime-control.md` (`$if`
dispatch), `secrecy.md` (the trusted base), and `policy.md` (the compiler-vs-
stdlib boundary). The reference wins on any disagreement.
