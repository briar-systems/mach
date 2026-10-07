# mach.lang.target.isa.x64.grammar

the inline assembly grammar: x64 statements parsed, checked and encoded

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

an inline-asm block encodes the instructions it spells, legacy sse included

