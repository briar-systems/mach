# mach.lang.target.isa.x64.printer

## fun emit_header

```mach
pub fun emit_header(out: *io_writer.Writer) err[fail.Fail];
```

## fun note_function

```mach
pub fun note_function(buf: *codegen_encode.ByteBuf, interner: *intern.Interner, f: *codegen_mir.MirFunction) err[fail.Fail];
```

## fun note_block

```mach
pub fun note_block(buf: *codegen_encode.ByteBuf, id: u32) err[fail.Fail];
```

## fun note_inst

```mach
pub fun note_inst(buf: *codegen_encode.ByteBuf, mi: *isa.Inst, start: usize);
```

notify first, render only for a writer: the stream is the same on every build

## fun note_local_label

```mach
pub fun note_local_label(buf: *codegen_encode.ByteBuf, number: u32);
```

the definition of a numbered local label, at the current offset

## fun mnemonic

```mach
pub fun mnemonic(op: u16) opt[str];
```

the spelling a diagnostic names an emitted instruction by

## fun reg_name

```mach
pub fun reg_name(id: i32, width: u8) str;
```

