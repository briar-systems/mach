# mach.lang.target.isa.riscv.grammar

the inline assembly grammar: riscv statements parsed, checked and encoded

## fun mnemonic

```mach
pub fun mnemonic(op: u16) opt[str];
```

## fun reg_name

```mach
pub fun reg_name(id: i32, size: u8) str;
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

