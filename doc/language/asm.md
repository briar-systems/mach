# Inline assembly

Mach has one inline-assembly form: an ISA-tagged block of raw instructions
with local-variable substitution. The compiler parses the instruction
stream and infers operand direction and clobbers from the opcode semantics —
no `in` / `out` declarations, no clobber list.

## Grammar

```mach
asm <isa> {
    # raw instructions, one per line, # for comments
    mov rcx, {ptr}
    mov rax, [rcx]
    mov {result}, rax
}
```

- The ISA tag is mandatory. Bare `asm { ... }` does not exist.
- The tag comes from a closed set: `x86_64`, `aarch64`, `riscv64`. Each has a working assembler that emits native bytes; all three run in CI (riscv64 under qemu, including a self-host smoke — riscv64 is a self-hosting target with a byte-identical fixpoint, #1852).
- Each line is an instruction in the ISA's native syntax.
- `#` introduces a line comment: everything from `#` to the end of the line is ignored, whatever it contains (`;`, `{}`, `%`, and so on are all inert inside a comment).

## Operand substitution

`{name}` substitutes a local in scope. The compiler resolves the reference
to a memory or register operand based on liveness and the instruction's
expected operand class. In practice a `{name}` binds the local's storage —
typically a stack slot — so a pointer local's pointee is reached by staging
the pointer through a scratch register first (`mov rcx, {ptr}` then
`mov rax, [rcx]`), never by a direct `[{ptr}]` indirection.

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

## Calls (x86-64)

`call` takes three shapes, and which one a statement means is read off the
operand:

```mach
asm x86_64 {
    call some_symbol     # direct: E8 rel32, relocated against the symbol
    call rax             # indirect through a register: FF /2, mod=11
    call [0x100018]      # indirect through an absolute address: ff 14 25 <disp32>
    call [rax + 8]       # indirect through a computed address
}
```

The absolute form exists for a fixed-address ABI — one whose entry points are
addresses rather than symbols, like BareMetal's kernel call table at
`0x100010..0x100040`. Its displacement is sign-extended to 64 bits, so an address
outside signed 32-bit range is refused rather than silently truncated. `call
[symbol]` is refused too: the rip-relative form would mean "call the pointer
*stored* at the symbol", which is not what the `call symbol` beside it means.

An indirect call clobbers exactly as a direct one does — the callee's caller-saved
registers, which the surrounding block's barrier already covers. The register or
memory holding the target is **read**, not written.

## Raw encodings

Four data directives emit their values verbatim, for an encoding the ISA's mnemonic
table does not name. They work on every target:

```mach
asm x86_64 {
    .byte 0x0f, 0x01, 0xd0    # xgetbv
}

asm aarch64 {
    .word 0xd53be040          # mrs x0, cntvct_el0
}

asm riscv64 {
    .word 0xc0102573          # csrr a0, time
}
```

The widths are GNU as's, per target — `.word` is the one that differs:

| directive | x86-64 | aarch64 / riscv64 |
|---|---|---|
| `.byte` | 1 byte | 1 byte |
| `.word` | **2 bytes** | **4 bytes** |
| `.long` | 4 bytes | 4 bytes |
| `.quad` | 8 bytes | 8 bytes |

Values are written in the target's byte order, so `.word 0xd503201f` is the aarch64
`nop` as its manual prints it. A non-negative value is read unsigned (`.quad
0xFFFFFFFFFFFFFFFF` is a legal address); a negative one is its two's complement at the
directive's width. A value the width cannot hold is refused rather than truncated, and
one directive carries a whole sequence — up to 256 payload bytes — not four.

On aarch64 and riscv64 a statement must emit a whole number of instruction words:
`.byte 0x1f, 0x20, 0x03` is refused, because three bytes would misalign every
instruction after it. x86-64 has no such constraint.

A raw encoding is an instruction stream the parser cannot read, so the block's clobber
set becomes **every register in every bank**, and an `#[oblivious]` function may not
contain one at all — which is why a real mnemonic is always preferable where one
exists.

## What the compiler infers

- **Operand direction.** Position within an instruction determines whether
  an operand is read or written.
- **Clobber set.** The compiler reads each instruction, knows what
  registers and flags it touches, and adds them to the surrounding
  function's clobber set.
- **Memory clobber.** Every `asm` block is conservatively assumed to
  modify arbitrary memory.

## Multi-arch dispatch

Different architectures use different mnemonics, registers, and calling
conventions. There is no nested arch-block construct inside `asm`; instead,
wrap each `asm` block in `$if` on `$mach.build.arch`:

```mach
$if ($mach.build.arch == $mach.arch.x86_64) {
    asm x86_64 { ... }
}
$or ($mach.build.arch == $mach.arch.aarch64) {
    asm aarch64 { ... }
}
```

The discarded branches don't compile (see
[comptime-control.md](comptime-control.md)), so each `asm` block only needs
to be valid for its tagged ISA.

## When to use

- Truly target-specific operations: syscalls, register reads, stack-frame
  surgery.
- Anything that doesn't have a 1:1 stdlib wrapper.

For ops that exist as named stdlib functions (atomics, fences, traps,
SIMD long-tail), use the stdlib API — those wrappers already contain the
arch-dispatched `asm`.

## See also

- [decorators.md](decorators.md#naked--no-prologue-no-epilogue-body-as-written) — `#[naked]`, whose body may hold only `asm`
- [policy.md](policy.md) — compiler vs stdlib boundary
- [comptime-control.md](comptime-control.md) — `$if` over `$mach.build.arch`
