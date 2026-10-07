# mach.lang.target.isa.spirv.emit.interface

the stage interface: which globals each entry point reaches and how it accesses
them, their declaration, and the checks across stages

## fun build_gl_access

```mach
pub fun build_gl_access(e: *emitter.Emit) err[fail.Fail];
```

## fun confined_flow

```mach
pub fun confined_flow(e: *emitter.Emit, fx: u32, mf: *lang_mir.MirFunction) err[fail.Fail];
```

each confined `op` result is consumed only as an operand of another `op` call later in
the block that makes it. sema refuses returning one and passing one to a function, so
what reaches here is a block an operand evaluated after it splits, or a merge

## fun return_storage

```mach
pub fun return_storage(e: *emitter.Emit, fx: u32, key: u32) u32;
```

the storage class the pointer a function returns points into: its global's, the
global the copy binds its parameter to, or Function for a local

## fun declare_interface

```mach
pub fun declare_interface(e: *emitter.Emit) err[fail.Fail];
```

## fun output_zeroing_emit

```mach
pub fun output_zeroing_emit(e: *emitter.Emit, fx: u32) err[fail.Fail];
```

the zero each output output_zeroed_by_stage names starts at, stored by a stage that reaches it

## fun wg_is_spec

```mach
pub fun wg_is_spec(fn: *me_ir.Function) bool;
```

## fun wg_dim_id

```mach
pub fun wg_dim_id(e: *emitter.Emit, fn: *me_ir.Function, d: u32) res[u32, fail.Fail];
```

the id a workgroup dimension is: its `#[spec]` var's constant, or a 32-bit OpConstant

## fun plan_workgroup

```mach
pub fun plan_workgroup(e: *emitter.Emit) err[fail.Fail];
```

without LocalSizeId a specialized workgroup is the WorkgroupSize built-in over a
spec composite, which sizes every compute stage in the module, so they must agree

## fun build_iface_uses

```mach
pub fun build_iface_uses(e: *emitter.Emit) err[fail.Fail];
```

## fun uses_iface

```mach
pub fun uses_iface(e: *emitter.Emit, fn_ix: u32, slot: u32) bool;
```

## fun check_call_graph

```mach
pub fun check_call_graph(e: *emitter.Emit) err[fail.Fail];
```

## fun build_fn_stages

```mach
pub fun build_fn_stages(e: *emitter.Emit) err[fail.Fail];
```

the stages that reach each function: an entry point's own, carried to everything it calls

## fun check_stage_interfaces

```mach
pub fun check_stage_interfaces(e: *emitter.Emit) err[fail.Fail];
```

## fun shared_native_zero

```mach
pub fun shared_native_zero(e: *emitter.Emit) bool;
```

a `#[shared]` variable carries an OpConstantNull initializer where the selection
holds zero_init_workgroup, and each compute stage reaching one zeroes it otherwise

