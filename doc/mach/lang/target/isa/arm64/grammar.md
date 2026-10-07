# mach.lang.target.isa.arm64.grammar

the inline assembly grammar: arm64 statements parsed, checked and encoded

## fun mnemonic

```mach
pub fun mnemonic(op: u16) opt[str];
```

## fun returns

```mach
pub fun returns(body: str) bool;
```

whether an asm body returns, for the middle end, which reads no grammar

## fun grammar

```mach
pub fun grammar() isa_asm.Grammar;
```

## fun encode_asm_block

```mach
pub fun encode_asm_block(st: *isa_encode.EncodeState, f: *lang_mir.MirFunction, fn_base: u32, mi: *lang_mir.MirInstr) err[fail.Fail];
```

