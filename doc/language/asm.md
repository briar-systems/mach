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
- The tag comes from a closed set: `x86_64`, `aarch64`, `riscv64`, `riscv32`. Each has a working assembler that emits native bytes; the first three run in CI (riscv64 under qemu, including a self-host smoke — riscv64 is a self-hosting target with a byte-identical fixpoint, #1852).
- **The RISC-V tag names the machine, not the family.** An `asm riscv64` block is refused on a riscv32 target and the other way round, so a body reaching the assembler was written for the register width it is being assembled for. Guard the two with `$if ($mach.build.arch == $mach.arch.riscv32)` when a routine needs both. An RV64-only spelling (`ld`, `sd`, the `*W` group, the doubleword atomics) is refused under a `riscv32` tag, naming the instruction and the machine, because each instruction states the register widths it exists at and the assembler reads that rather than a second list of its own.
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

### A body that binds `{name}` may not move the stack pointer

A `{name}` becomes a fixed displacement off a base register, measured once when the
block is assembled. On aarch64 and riscv64 that base is the **stack pointer**, because
it is the only one whose displacement stays inside those ISAs' immediate forms however
deep the enclosing frame is. A statement that moves the stack pointer therefore moves
every `{name}` in the block out from under its own address, and the compiler refuses the
block rather than assembling a wrong one:

```mach
var x: i64 = 0;
asm aarch64 {
    ldr x9, {x}
    stp x1, x2, [sp, -16]!   # refused: this body binds {x}
}
```

The refusal covers the whole block, not the statements after the write, because a
backward branch reaches an earlier `{name}` again with the pointer already moved. Push
and pop around the block instead, or drop the `{name}` and stage the address into a
register yourself.

A body that binds no `{name}` is unaffected and may do whatever it likes with the stack
pointer, which is what a `#[naked]` function's hand-written prologue does.

## Calls and jumps (x86-64)

`call` and `jmp` take the same three shapes, and which one a statement means is
read off the operand:

```mach
asm x86_64 {
    call some_symbol     # direct: E8 rel32, relocated against the symbol
    call rax             # indirect through a register: FF /2, mod=11
    call [0x100018]      # indirect through an absolute address: ff 14 25 <disp32>
    call [rax + 8]       # indirect through a computed address

    jmp  some_symbol     # direct: E9 rel32
    jmp  rax             # indirect through a register: FF /4
    jmp  [rax + 8]       # ... and the same memory forms
}
```

The absolute form exists for a fixed-address ABI — one whose entry points are
addresses rather than symbols, like BareMetal's kernel call table at
`0x100010..0x100040`. Its displacement is sign-extended to 64 bits, so an address
outside signed 32-bit range is refused rather than silently truncated. `call
[symbol]` and `jmp [symbol]` are refused too: the rip-relative form would mean
"transfer to the pointer *stored* at the symbol", which is not what the direct
form beside it means.

Both indirect operands are fixed 64-bit in long mode, so a narrower register
(`jmp eax`) is refused rather than widened.

An indirect call clobbers exactly as a direct one does — the callee's caller-saved
registers, which the surrounding block's barrier already covers. The register or
memory holding the target is **read**, not written.

## Operand sizes (x86-64)

A register operand states its own width, so `mov eax, [rcx]` is a four-byte load
and needs nothing else. A memory operand states none, and where the instruction
does not settle it either the width must be written out, in nasm's spelling:

```mach
asm x86_64 {
    movzx eax, word [rcx]     # a two-byte load, zero-extended into eax
    movsx rax, dword [rcx]    # a four-byte load, sign-extended (movsxd)
    mov dword [rcx], 1        # a four-byte store, not the machine word
    neg qword [rcx]           # an eight-byte read-modify-write
}
```

`byte`, `word`, `dword` and `qword` are accepted before a memory operand and
nowhere else — on a register they would be redundant or contradictory, so
`mov qword rax, rcx` is refused rather than ignored.

**A prefix that contradicts the instruction is a build error, not a dropped
token.** What counts as a contradiction is per mnemonic:

| shape | rule |
|---|---|
| most instructions | every operand shares one width, so a prefix must agree with any register operand; with no register operand it *sets* the width |
| `movzx` / `movsx` | the source is narrower by design, so a memory source **must** be sized, and the size must be strictly narrower than the destination |
| `push` / `pop`, indirect `call` / `jmp` | fixed 64-bit in long mode, so any narrower prefix names no instruction |
| `lidt` | its pseudo-descriptor is ten bytes, which no keyword names |

So `mov eax, word [rcx]` is refused (two widths for one access), and
`movzx eax, [rcx]` is refused too — an unsized source names no width at all, and
reading it as a same-width move would silently assemble a plain `mov` where a
zero-extending load was written.

## Privileged and systems instructions (x86-64)

Beyond the ordinary surface, an OS-level block reaches:

```mach
asm x86_64 {
    cli / sti                 # the interrupt flag
    cld                       # clear DF before entering a program
    hlt                       # park the core
    in al, dx / out dx, al    # port i/o, by immediate port or through dx
    rdtsc / rdmsr / wrmsr     # the counter and the model-specific registers
    lidt [rax]                # install an interrupt descriptor table
    pushfq / popfq            # save and restore RFLAGS
    swapgs                    # per-CPU state on a syscall entry
    iretq                     # return from an interrupt handler
    mov rax, cr2              # the faulting address in a page-fault handler
    mov cr3, rax              # switch page tables
    mov eax, cs               # the live selector, for programming STAR
}
```

Control registers are `cr0`, `cr2`, `cr3`, `cr4` and `cr8` — CR1 and CR5–CR7 are
reserved and have no spelling. A control-register move takes a 64-bit
general-purpose register on its other side, since there is no narrower form.
Segment registers (`es`, `cs`, `ss`, `ds`, `fs`, `gs`) can be **read** into a
general-purpose register; writing one through `mov` is not supported, because
long mode does not admit it for CS and SS at all and the remaining loads carry
descriptor-cache and interrupt-shadow effects.

None of these writes a register the allocator tracks: the interrupt and direction
flags, RFLAGS, the stack pointer and a segment base are all outside the allocated
file, and `mov cr3, rax` writes a control register rather than any general-purpose
one. `rdtsc` and `rdmsr` are the exceptions — both land their result in EDX:EAX,
which the effect model reports.

`iretq` does not fall through, and the effect model has no way to say so: a
`Mnemonic` names registers written and nothing else. That is sound — nothing can
survive an instruction control never returns from — but it means statements after
an `iretq` are unreachable without the compiler saying so.

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

A raw encoding is an instruction stream the parser cannot read, so by default the
block's clobber set becomes **every register in every bank**, and an `#[oblivious]`
function may not contain one at all — which is why a real mnemonic is always preferable
where one exists.

## Declaring a raw encoding's effects

That default is correct and expensive: with nothing held live across the statement, the
allocator spills every value in a callee-saved register and the prologue saves every
callee-saved register the function could reach. A raw encoding may instead state what it
writes, with a `::` clause on the directive:

```mach
asm x86_64 {
    mov dx, {port}
    .byte 0xee :: writes()                 # out dx, al - reads dx and al, writes nothing
    .byte 0x0f, 0x31 :: writes(rax, rdx)   # rdtsc - lands its result in EDX:EAX
}

asm aarch64 {
    .word 0xd4000002 :: writes(x0, x1, x2, x3)   # hvc #0, returning per SMCCC
}

asm riscv64 {
    .word 0xc0102573 :: writes(a0)               # csrr a0, time
}
```

This is the same move the system-register surface above already makes. The named set is
deliberately not exhaustive because any register is *also* nameable by its encoding;
declared clobbers do for instructions what `s3_3_c14_c0_2` does for registers. The
curated mnemonic table stays the ergonomic path, and the escape hatch stops being a
cliff — an unmodeled instruction can be used at full codegen quality on the day it is
needed.

`writes()` with an empty list is a **declaration**, not an omission: it says the
encoding writes no register. Leaving the clause off entirely is what keeps today's
conservative default, so nothing written before this existed changes behaviour.

Registers are named in the target's own vocabulary, in either bank:

| target | general-purpose | float / vector |
|---|---|---|
| x86-64 | `rax`–`r15` | `xmm0`–`xmm15` |
| aarch64 | `x0`–`x30`, `sp`, `xzr` | `v0`–`v31` |
| riscv64 | `x0`–`x31`, psABI aliases (`a0`, `t0`, `sp`, …) | `f0`–`f31`, psABI aliases (`fa0`, `ft0`, …) |

A width is not a register: `eax`, `w0` and `v2.4s` are refused, because `eax` and `rax`
are one register whose bits the allocator tracks as a unit and accepting both spellings
would suggest the clobber set told them apart.

### What the compiler still guarantees, and what it does not

A declaration moves part of the correctness burden from the compiler to you, and it is
worth being precise about which part.

**Still checked.** The clause is accounted for like every other byte of the statement,
so a malformed one fails the build rather than being silently dropped — a mistyped
`writes(` is an error, not a declaration that quietly said nothing. Every register named
must resolve in this target's own name table, so a typo fails rather than declaring a
smaller set than you wrote. An attribute that does not exist is refused by name.

**Structurally bounded.** A clause may only follow a **data directive**. A modeled
instruction's effects come from its mnemonic and cannot be overridden, so a declaration
can only ever narrow the maximally conservative default, at exactly the statements where
the compiler had no information at all. It can never contradict something the compiler
derived, and a wrong one affects only the single statement carrying it.

**Not checked, and it cannot be.** Whether the bytes write what the clause says. The
compiler does not decode the payload — decoding it is what the mnemonic table *is*.
**If you under-declare, you get a miscompile**: the allocator will keep a value in a
register your encoding overwrites. This is the one place in the inline-asm surface where
correctness rests on the author. Declare what the instruction's manual says it writes,
including any implicit destination, and prefer a real mnemonic whenever one exists.

**A declaration buys allocation quality, not verification.** An `#[oblivious]` function
still refuses a data directive whether or not it is declared. A write set says which
registers change; it says nothing about whether the encoding branches on a secret or
divides in variable time, which is what that check exists to catch.

`writes(...)` is the only attribute today. The clause is a space-separated list so more
can be added, but an attribute is only added once it changes what the compiler does —
`noreturn` is not here yet because control flow reaching past an `asm` block is a CFG
fact the effect model does not touch, and a `barrier` attribute would be inert, since
every `asm` block is already assumed to modify arbitrary memory.

## System registers (aarch64)

`mrs` and `msr` name a system register by its architectural name, in either case:

```mach
asm aarch64 {
    mrs x0, cntvct_el0        # the virtual counter
    mrs x1, CNTFRQ_EL0        # ... and its frequency, capitalized as ARM spells it
    msr vbar_el1, x2          # install an exception vector base
    msr daifset, 0xf          # mask every interrupt
}
```

The named set covers what freestanding code reaches for — the generic timer, the
exception vectors and their syndrome registers, the MMU control registers, the thread
pointers, and enough identification registers to detect a CPU. It is deliberately not
exhaustive: **any** system register is also nameable by its encoding, exactly as ARM and
GNU as spell it, which is what makes the surface complete rather than a list that always
lags the architecture:

```mach
asm aarch64 {
    mrs x0, s3_3_c14_c0_2     # the same register as `mrs x0, cntvct_el0`
}
```

`op0` must be 2 or 3 — the whole of the `mrs` / `msr` register space — and each remaining
field is bounded by its own width. A field the architecture cannot hold is refused rather
than truncated, because a truncated selector would name a *different* register than the
text does.

`msr <field>, #imm` writes a PSTATE field (`daifset`, `daifclr`, `spsel`, `pan`, `uao`,
`ssbs`, `dit`, `tco`). The architecture spells these by name only, so there is no numeric
escape for this form.

Access permission is not checked: whether a register is writable depends on the exception
level the code runs at, which the compiler does not know. Writing a register that is
read-only at the current level traps at run time, as the architecture defines.

## Exception conduits and waits (aarch64)

Three instructions generate an exception at a higher level, and they differ only in
which level answers:

```mach
asm aarch64 {
    svc 0                     # the kernel, at EL1
    hvc 0                     # the hypervisor, at EL2
    smc 0                     # the secure monitor, at EL3
}
```

`hvc` and `smc` are how PSCI is reached, which is the only way to power off or
restart a `virt` board — `SYSTEM_OFF` and `SYSTEM_RESET` go through whichever
conduit the firmware provides.

Both follow the SMC Calling Convention, so the compiler declares them as
destroying **x0–x17**: the result and scratch registers a service may use. x18–x30
and SP survive, which makes a conduit *cheaper* than an ordinary `bl` — a
procedure call destroys x0–x18 and x30. Spelling the same instruction as
`.word 0xd4000002` instead costs every register in every bank, because a raw
payload is a stream the parser cannot read.

The two waiting hints suspend the core until something wakes it:

```mach
asm aarch64 {
    wfi                       # ... until an interrupt: the correct idle loop
    wfe                       # ... until an event
}
```

Neither writes anything, so an idle loop holds every live value across it. `yield`
is the weaker hint of the three — it asks a hypervisor to schedule elsewhere and
may do nothing at all, which is why `wfi` is what an idle loop should say.

## Control-and-status registers (riscv64)

The Zicsr extension's six instructions — read-write, read-set and read-clear, each
taking its source from a register or a five-bit immediate — reach a CSR by name:

```mach
asm riscv64 {
    csrrw a0, mstatus, a1   # read mstatus into a0, write a1 into it
    csrr  a0, mtvec         # csrrs a0, mtvec, x0 - the read-only pseudo
    csrw  stvec, a1         # csrrw x0, stvec, a1 - install a trap vector
    rdtime a0               # csrrs a0, time, x0  - the unprivileged counters
}
```

The named set covers what freestanding code reaches for — the machine and supervisor
trap vector / exception-PC / cause registers, the interrupt enable / pending pairs, the
address-translation root, the hart id an SMP boot path reads to tell cores apart, and
the three unprivileged counters `rdtime` / `rdcycle` / `rdinstret` name. It is
deliberately not exhaustive: the privileged spec defines several hundred addresses
across three privilege levels, so **any** CSR is also reachable by its numeric address,
exactly as a name resolves to one:

```mach
asm riscv64 {
    csrr a0, 0xc01   # the same register as `csrr a0, time`
}
```

Unlike aarch64's system registers, RISC-V spells no separate escape syntax for this —
a CSR operand simply parses as the ordinary integer literal it looks like, bounded to
the twelve bits a CSR address occupies. `csrrwi` / `csrrsi` / `csrrci` (and their
`csrwi` / `csrsi` / `csrci` pseudos) take a five-bit unsigned immediate in the same
position a register would occupy in the non-`i` form.

Access permission is not checked: whether a CSR is readable or writable depends on the
privilege level the code runs at, which the compiler does not know. Accessing a CSR the
current level cannot reach traps at run time, as the architecture defines.

## What the compiler infers

- **Operand direction.** Position within an instruction determines whether
  an operand is read or written.
- **Clobber set.** The compiler reads each instruction, knows what
  registers and flags it touches, and adds them to the surrounding
  function's clobber set.
- **Memory clobber.** Every `asm` block is conservatively assumed to
  modify arbitrary memory.
- **Raw encodings.** A data directive is a stream the parser cannot read, so
  it clobbers every register in every bank unless it
  [declares what it writes](#declaring-a-raw-encodings-effects).

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
