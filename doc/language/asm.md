# Inline assembly

Mach has one inline-assembly form: an ISA-tagged block of raw instructions
with local-variable substitution. The compiler parses the instruction
stream and infers operand direction and clobbers from the opcode semantics —
no `in` / `out` declarations, no clobber list.

## Grammar

```mach
asm <isa> {
    # raw instructions, one per line, # for comments
    mov rax, [{ptr}]
    mov {result}, rax
}
```

- The ISA tag is mandatory. Bare `asm { ... }` does not exist.
- The tag comes from a closed set: `x86_64`, `aarch64`. Only `x86_64` has a working code generator today; `aarch64` is recognized for portable target-conditional source but is not yet a buildable target (see issue #1045).
- Each line is an instruction in the ISA's native syntax.
- `#` introduces a line comment.

## Operand substitution

`{name}` substitutes a local in scope. The compiler resolves the reference
to a memory or register operand based on liveness and the instruction's
expected operand class.

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
wrap each `asm` block in `$if` on `$mach.target.arch`:

```mach
$if ($mach.target.arch == $mach.arch.x86_64) {
    asm x86_64 { ... }
}
$or ($mach.target.arch == $mach.arch.aarch64) {
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

- [policy.md](policy.md) — compiler vs stdlib boundary
- [comptime-control.md](comptime-control.md) — `$if` over `$mach.target.arch`
