# lang/be/codegen

The codegen pipeline: IR to encoded machine bytes. `codegen.mach` in the parent directory is the public entry point.

## Pipeline

```
IR → isel → MIR → frame → regalloc → encode → bytes
```

## Files

- `mir.mach` — machine IR. A target-specific representation with real opcodes, register classes, and explicit calling-convention detail. The bridge between the target-independent IR and final encoding; lets the register allocator and encoder operate on real data structures rather than re-deriving them per pass.
- `isel.mach` — instruction selection. Lowers IR operations to MIR instructions.
- `frame.mach` — stack frame layout and calling-convention lowering. Materializes argument slots, spill space, and prologue/epilogue structure.
- `regalloc.mach` — register allocation. Assigns physical registers to MIR virtual registers, inserting spills and reloads as needed.
- `encode.mach` — encodes MIR into target machine bytes.
