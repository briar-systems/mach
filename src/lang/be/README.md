# lang/be — Backend

Lowers the optimized IR into target machine code and emits object files.

## Pipeline

```
IR → codegen (isel → frame → regalloc → encode) → machine bytes → obj → object file
                                                                   ↓
                                                                linker → executable
```

## Files

- `codegen.mach` — public entry. Takes an IR module and a `Target` and returns encoded machine code. Internal subdivision in `codegen/`.
- `obj.mach` — object file writer. Formats machine code per the target's object format (ELF, Mach-O, COFF).
- `linker.mach` — links object files into an executable, resolving relocations and symbol references.

## Target parameterization

The backend receives a `Target` record (from `lang/target.mach`) that bundles the ISA, ABI, OS, and object-format implementations. All target-specific behavior flows through that record: no hard-coded register names, opcodes, or relocation kinds live in target-agnostic backend code.
