# mach.lang.target.isa.x64.encode

MIR lowered to x64 instructions, with the frame each function lays out

## val HOOKS

```mach
pub val HOOKS: isa_encode.EncodeHooks = isa_encode.EncodeHooks;
```

the hooks the shared encode driver runs this encoder through

## fun reads_const_operand

```mach
pub fun reads_const_operand(mi: *lang_mir.MirInstr, index: u32) bool;
```

the scalar float arithmetic and compare read their second source, a
constant there included, as the r/m operand: a constant is one pool
reference inside the instruction. a compare's second source is its
right operand, or its left where the condition swaps them

