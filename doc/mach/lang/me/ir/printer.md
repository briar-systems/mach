# mach.lang.me.ir.printer

## def IrForm

```mach
pub def IrForm: u8
```

which rendering of a module the writer produces; the rows are IR_FORMS

## val IR_FORM_DEBUG

```mach
pub val IR_FORM_DEBUG: IrForm = 0
```

the ir-debug dump: every instruction carries its whole record, so no two
distinct instructions print the same. It is the compiler's debugging form and
its text is not a contract

## val IR_FORM_LISTING

```mach
pub val IR_FORM_LISTING: IrForm = 1
```

a readable listing: language type spellings, elided state, and positions as
line and column. It is the form tooling reads

## val IR_FORM_N

```mach
pub val IR_FORM_N: usize                 = 2
```

## val IR_FORM_HELP

```mach
pub val IR_FORM_HELP: str = "emit per-module .ir text beside each object
```

## val IR_FORM_ERROR

```mach
pub val IR_FORM_ERROR: str = "--emit-ir expects 'debug' or 'listing'"
```

## fun ir_form_from_name

```mach
pub fun ir_form_from_name(name: str) opt[IrForm];
```

the form a `--emit-ir=<name>` value names

## rec IrContext

```mach
pub rec IrContext;
```

## rec IrWriter

```mach
pub rec IrWriter;
```

a module rendering in progress

sources: resolves an instruction's byte offset to a line, a column and a path; the listing needs it and the dump does not read it
form: the row of IR_FORMS write_ir dispatches on
col: bytes emitted on the current output line, which the listing pads against

## fun ir_context

```mach
pub fun ir_context(stage: str, target_name: str, isa_name: str, os_name: str,
abi_name: str, of_name: str) IrContext;
```

## fun ir_writer

```mach
pub fun ir_writer(out: *writer.Writer, m: *ir.Module, interner: *intern.Interner,
sources: *source.SourceMap, context: IrContext, form: IrForm) IrWriter;
```

## fun write_ir

```mach
pub fun write_ir(d: *IrWriter) err[fail.Fail];
```

render the module in the writer's form; a new row of IR_FORMS lands here

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

