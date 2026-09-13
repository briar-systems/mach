# mach.lang.target.isa.riscv.printer

## fun render_notes

```mach
pub fun render_notes(buf: *enc.ByteBuf) err[fail.Fail];
```

## fun gp_name

```mach
pub fun gp_name(idx: u32) str;
```

## fun fp_name

```mach
pub fun fp_name(idx: u32) str;
```

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

