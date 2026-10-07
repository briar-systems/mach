# mach.lang.target.isa.spirv.emit.op

the `#[op]` instructions a function calls, checked against their row and emitted

## fun read_condition

```mach
pub fun read_condition(e: *emitter.Emit, vm: *emitter.VMaps, op: *lang_mir.MirOperand, bits: u32) u32;
```

a value tested for truth at `bits`, 0 for its whole type

## fun spirv_op_emit

```mach
pub fun spirv_op_emit(e: *emitter.Emit, mf: *lang_mir.MirFunction, vm: *emitter.VMaps, mi: *lang_mir.MirInstr, fn: *me_ir.Function) err[fail.Fail];
```

a call to an `#[op]` declaration is the instruction its row names: the row says
whether it has a result id, and how each operand is written, so a pointer operand
takes the storage class of the argument's own access chain rather than a
parameter type's

## fun storage_class_name

```mach
pub fun storage_class_name(space: u32) str;
```

## fun require_image_type

```mach
pub fun require_image_type(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) err[fail.Fail];
```

a multisampled storage image is declared only under StorageImageMultisample, and an
arrayed one under ImageMSArray as well, both enabled by the device feature that permits them

