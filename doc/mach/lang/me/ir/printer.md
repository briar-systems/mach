# mach.lang.me.ir.printer

## rec IrDebugContext

```mach
pub rec IrDebugContext;
```

## rec IrDebugWriter

```mach
pub rec IrDebugWriter;
```

## fun debug_context

```mach
pub fun debug_context(stage: str, target_name: str, isa_name: str, os_name: str,
abi_name: str, of_name: str) IrDebugContext;
```

## fun debug_writer

```mach
pub fun debug_writer(out: *writer.Writer, m: *ir.Module, interner: *intern.Interner,
context: IrDebugContext) IrDebugWriter;
```

## fun write_debug

```mach
pub fun write_debug(d: *IrDebugWriter) err[fail.Fail];
```

## fun print_global

```mach
pub fun print_global(out: *writer.Writer, m: *ir.Module, g: *ir.Global, interner: *intern.Interner) err[fail.Fail];
```

## fun print_function

```mach
pub fun print_function(out: *writer.Writer, m: *ir.Module, fn: *ir.Function, interner: *intern.Interner) err[fail.Fail];
```

## fun print_block

```mach
pub fun print_block(out: *writer.Writer, m: *ir.Module, fn: *ir.Function, blk: *ir.Block, interner: *intern.Interner) err[fail.Fail];
```

## fun print_instruction

```mach
pub fun print_instruction(out: *writer.Writer, m: *ir.Module, fn: *ir.Function, ins: *instruction.Instruction, iid: id.InstructionId, interner: *intern.Interner) err[fail.Fail];
```

## fun print_value

```mach
pub fun print_value(out: *writer.Writer, m: *ir.Module, v: *value.Value, interner: *intern.Interner) err[fail.Fail];
```

## fun print_type

```mach
pub fun print_type(out: *writer.Writer, m: *ir.Module, tid: type.IrTypeId) err[fail.Fail];
```

