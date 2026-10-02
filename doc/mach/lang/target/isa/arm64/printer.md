# mach.lang.target.isa.arm64.printer

## fun note_function

```mach
pub fun note_function(buf: *codegen_encode.ByteBuf, interner: *intern.Interner, f: *codegen_mir.MirFunction) err[fail.Fail];
```

## fun note_block

```mach
pub fun note_block(buf: *codegen_encode.ByteBuf, id: u32) err[fail.Fail];
```

## fun note_local_label

```mach
pub fun note_local_label(buf: *codegen_encode.ByteBuf, number: u32) err[fail.Fail];
```

an asm block's numbered label, spelled as the block spells it: GNU as and
the block both resolve `1f` to the next `1:` and `1b` to the last one

## fun note_inst

```mach
pub fun note_inst(buf: *codegen_encode.ByteBuf, mi: *isa.Inst);
```

the notification carries the instruction the encoder assembled, already
spelled as an assembler reads it back; rendering is a separate step that
needs a writer

## fun fmov_imm8_exponent

```mach
pub fun fmov_imm8_exponent(imm8: u32) i32;
```

## fun reg_name

```mach
pub fun reg_name(id: i32, size: u8) str;
```

the spelling a diagnostic names a register by; index 31 is the zero
register in a data position, which is where the walk reads it

