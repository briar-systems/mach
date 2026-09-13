# mach.lang.target.isa.arm64.printer

## fun note_function

```mach
pub fun note_function(buf: *enc.ByteBuf, interner: *intern.Interner, f: *mir.MirFunction) err[fail.Fail];
```

## fun note_block

```mach
pub fun note_block(buf: *enc.ByteBuf, id: u32) err[fail.Fail];
```

## fun note_inst

```mach
pub fun note_inst(buf: *enc.ByteBuf, mi: *isa.Inst);
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

## fun test_sink_write

```mach
pub fun test_sink_write(ctx: ptr, buf: *u8, len: usize) res[usize, writer.WriteError];
```

## fun test_sink_reset

```mach
pub fun test_sink_reset();
```

## fun test_sink_text

```mach
pub fun test_sink_text() str;
```

