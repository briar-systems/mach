# mach.lang.target.isa.riscv.encode

## val HOOKS

```mach
pub val HOOKS: isa_encode.EncodeHooks = isa_encode.EncodeHooks;
```

the hooks the shared encode driver runs this encoder through

## fun returns

```mach
pub fun returns(body: str) bool;
```

whether an asm body returns, for the middle end, which reads no grammar

## fun grammar

```mach
pub fun grammar() isa_asm.Grammar;
```

