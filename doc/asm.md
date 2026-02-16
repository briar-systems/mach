# Inline Assembly

The `asm` keyword embeds assembly instructions directly in Mach code. Operands can reference local variables by name.


## Syntax

```mach
asm {
    # instructions here
}
```

The compiler stores the block content as a raw string and interprets it during code generation. Nested braces are supported.


## Portable Syscall DSL

The `syscall` directive provides an ABI-agnostic way to make system calls:

```mach
asm {
    syscall result, syscall_number, arg0, arg1, arg2
}
```

The compiler translates this to the correct register placement for the target platform. Up to 6 arguments are supported.

Example from the standard library:

```mach
pub fun syscall0(n: usize) i64 {
    var out: i64;
    asm {
        syscall out, n
    }
    ret out;
}

pub fun syscall3(n: usize, a0: usize, a1: usize, a2: usize) i64 {
    var out: i64;
    asm {
        syscall out, n, a0, a1, a2
    }
    ret out;
}
```


## ISA-Specific Blocks

For target-specific instructions, wrap the assembly in an ISA block:

```mach
asm {
    x86_64 {
        mov rax, 1
        mov rdx, rsi
        mov rsi, rdi
        mov rdi, 1
        syscall
    }
}
```

ISA-specific blocks emit target instructions directly, bypassing the portable IR layer. This is required for instructions with target-specific semantics like flags or register constraints.

Supported ISA names: `x86_64`.


## Limitations

- Operands reference local variables by name; there is no explicit constraint or clobber syntax
- Only valid inside function bodies
- No inline assembly interpolation or register allocation integration
