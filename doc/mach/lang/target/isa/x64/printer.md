# mach.lang.target.isa.x64.printer

## fun emit_header

```mach
pub fun emit_header(out: *writer.Writer) err[fail.Fail];
```

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
pub fun note_inst(buf: *enc.ByteBuf, mi: *isa.Inst, start: usize);
```

notify first, render only for a writer: the stream is the same on every build

## fun mnemonic

```mach
pub fun mnemonic(op: u16) opt[str];
```

the spelling a diagnostic names an emitted instruction by

## fun reg_name

```mach
pub fun reg_name(id: i32, width: u8) str;
```

