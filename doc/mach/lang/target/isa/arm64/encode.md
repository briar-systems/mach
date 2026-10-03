# mach.lang.target.isa.arm64.encode

## fun int_imm_fits

```mach
pub fun int_imm_fits(value: u64, bits: u32) bool;
```

whether one instruction builds the integer: one movz or movn, or one orr of
a logical immediate, the rule the middle end hoists a loop's constants by
. a value narrower than 64 bits is built at 32

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

