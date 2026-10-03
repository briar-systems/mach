# mach.lang.target.isa.x64.encode

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

## fun returns

```mach
pub fun returns(body: str) bool;
```

whether an asm body returns, for the middle end, which reads no grammar

## fun grammar

```mach
pub fun grammar() isa_asm.Grammar;
```

