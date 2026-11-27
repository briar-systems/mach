# Inline assembly

An `asm` block holds raw instructions for one instruction set, with the function's locals substituted by name. The compiler reads every instruction, so a block declares no inputs, outputs or clobbers.

## The asm block

```mach
use std.print;
use std.runtime;

val STEP: i64 = 8;

fun sum(xs: *i64, n: i64) i64 {
    var total: i64 = 0;
    asm x86-64 {
        mov rsi, {xs}
        mov rcx, {n}
        xor eax, eax
        test rcx, rcx
        jz 2f
    1:
        add rax, [rsi]       # rsi holds the pointer
        add rsi, {STEP}
        dec rcx
        jnz 1b
    2:
        mov {total}, rax
    }
    ret total;
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var xs: [4]i64 = [4]i64{10, 20, 30, 40};
    print.printlnf("{}", sum(?xs[0], 4));
    # prints: 100
    ret 0;
}
```

The word after `asm` is the head. It names the instruction set and is mandatory. Each line of the body is one instruction in that set's own assembler syntax.

## Instruction sets

| Architecture | Heads, for example | Comment |
|---|---|---|
| x86-64 | `x86-64`, `x86-64,avx2` | `#` |
| aarch64 | `armv8-a`, `armv8.2-a+sha2` | `//` |
| RISC-V | `rv64i`, `rv64gc_zba` | `#` |
| WebAssembly | `mvp`, `wasm1,simd128` | `;;` |

- The head must be a selection for the architecture being built. Any other is refused.
- A comment inside a block uses its dialect's character, not always Mach's `#`. On aarch64 `#` begins an immediate, as in `add x0, x0, #1`.
- A head can add an extension for its block alone. Without one, an instruction the target does not select is refused:

```text
error[asm.extension] an asm block of 'app.bin.main.bits' uses `popcnt`, which the target's extensions do not admit. select popcnt in the target's `isa` or `ext`, or in the function's `#[extension]`
```

The head `x86-64,popcnt` admits it. Whether the processor has the extension is then the program's to check.

## Substitution

`{name}` names a local, a parameter or a constant in scope, placed by what it is and what the instruction accepts:

- A constant, such as the module `val` `STEP` above, becomes an immediate.
- An integer, pointer, float or vector goes to a register when every instruction naming it takes one. Otherwise it is memory.
- A record, union, tag or array is always memory.

Locals are read before the block and written back after it, so a block may change any local it names. A name may land in memory, so load a pointer into a register before dereferencing it, as `sum` does with `rsi`. `[{xs}]` is refused.

## What is inferred

- **Direction.** An operand's position in its instruction says whether it is read or written.
- **Clobbers.** The compiler knows every register and flag the instructions write and keeps live values out of them. A `{name}` is never placed in a register the block writes.
- **Memory.** Every block is assumed to read and write any memory.

A `ret` inside a block is refused, since it would skip the function's epilogue. Fall out of the block and return from Mach.

## Labels

A block defines numbered labels only, as `1:`. A branch names the nearest one behind it as `1b` and the nearest one ahead as `1f`. A label defined by name is refused.

## A block per architecture

`$if` on `$mach.build.arch` chooses one block per architecture, and its bare `$or` holds the portable version.

```mach
use std.print;
use std.runtime;

# the high 64 bits of a 64 by 64 bit product
fun mul_high(a: u64, b: u64) u64 {
    var hi: u64 = 0;
    $if ($mach.build.arch == $mach.arch.x86_64) {
        asm x86-64 {
            mov rax, {a}
            mul {b}              # rdx:rax = rax * b
            mov {hi}, rdx
        }
    }
    $or ($mach.build.arch == $mach.arch.aarch64) {
        asm armv8-a {
            umulh {hi}, {a}, {b}     // comments here start with //
        }
    }
    $or ($mach.build.arch == $mach.arch.riscv64) {
        asm rv64i {
            mulhu {hi}, {a}, {b}
        }
    }
    $or {
        hi = ((a::u128 * b::u128) >> 64)::u64;
    }
    ret hi;
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{:x}", mul_high(0xFFFF_FFFF_FFFF_FFFF, 0x10));
    # prints: f
    ret 0;
}
```

Arms that are not taken are never checked against the target, so each block needs to be valid only for its own architecture. A project from `mach init` declares all three targets:

```sh
mach build .
mach build . -t linux-aarch64
mach build . -t linux-riscv64
```

A shader target has no assembler and takes no `asm` block. Where std already wraps an operation, such as an atomic or a trap, call std instead.

## Naked functions

`#[naked]` emits a function's blocks and nothing else: no frame, no argument moves and no return.

```mach
#[naked]
fun add_naked(a: i64, b: i64) i64 {
    asm x86-64 {
        lea rax, [rdi + rsi]     # the arguments are where the ABI put them
        ret
    }
}
```

- The body holds only `asm` blocks and the `$if` that chooses among them.
- A block in it binds no `{name}`. Arguments are read from the ABI's registers.
- The block writes the return. Falling off the end reaches a trap.
- It is called like any function, with its parameters and result checked.

It is the form for an entry point, an interrupt handler or a context switch.

Operand sizes, privileged and system instructions, vector registers, extensions and raw encodings are in the reference for [inline assembly](../language/asm.md) and [decorators](../language/decorators.md).
