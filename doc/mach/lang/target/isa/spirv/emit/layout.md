# mach.lang.target.isa.spirv.emit.layout

how a type is laid out in memory and the SPIR-V type that carries it: block
layout rules, physical pointers and the narrow and half-width bridges

## fun is_block_iface

```mach
pub fun is_block_iface(iface: u8) bool;
```

## fun half_memory

```mach
pub fun half_memory(e: *emitter.Emit) bool;
```

an f16 is the OpTypeFloat 16 it is where the selection holds Float16: in a stage
interface, a buffer, push block or `#[shared]` variable, and a local,
parameter or result

## fun interface_type

```mach
pub fun interface_type(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u32;
```

the type a stage input or output of the ir type `id` is declared with: as host-shared
memory, with an f16 the OpTypeFloat 16 it is under every selection, since a pipeline
interpolates a float and matches stages by type, and without float16 a function carries
its bits

## fun require_interface_access

```mach
pub fun require_interface_access(e: *emitter.Emit, value_ty: u32, gi: u32) err[fail.Fail];
```

a stage input or output holding a 16-bit scalar needs storage_input_output16, and one
holding an 8-bit scalar is refused, since Vulkan defines no 8-bit stage interface

## fun memory_bridged

```mach
pub fun memory_bridged(e: *emitter.Emit, a: u32, b: u32) bool;
```

a memory type and the type a function carries its value in that differ only in how
the function holds it: a narrow integer of host-shared memory, or a vector of them, and
the wider integer it is computed in where its width has no arithmetic capability

## fun half_bits_bridged

```mach
pub fun half_bits_bridged(e: *emitter.Emit, a: u32, b: u32) bool;
```

an OpTypeFloat 16 of a stage interface and the 16- or 32-bit integer bits, or a vector
of them lane for lane, a function carries an f16 in without float16

## fun half_bits_type

```mach
pub fun half_bits_type(e: *emitter.Emit, ty: u32) u32;
```

the integer bits a function carries an f16 of a stage interface in without float16: a
scalar, or each lane of a vector, at the carrier of its width. any other type as it is

## fun is_half_type

```mach
pub fun is_half_type(e: *emitter.Emit, ty: u32) bool;
```

## fun lane_of

```mach
pub fun lane_of(e: *emitter.Emit, ty: u32, out_count: *u32) u32;
```

a vector type's lane type and count, and a scalar as one lane of itself

## fun half_bits_as

```mach
pub fun half_bits_as(e: *emitter.Emit, value: u32, from_ty: u32, to_ty: u32) u32;
```

an f16 moved across half_bits_bridged, lane by lane through binary32: without Float16
no instruction but a width conversion takes an OpTypeFloat 16, so its bits are not
bitcast but packed and unpacked, and an f16 is exact in binary32 both ways

## fun half_inst

```mach
pub fun half_inst(e: *emitter.Emit, opcode: u32, ty: u32, a: u32, b: u32, c: u32, argc: u32) u32;
```

an instruction of half_bits_as: a result of `ty` from `argc` operand words

## fun construct_emit

```mach
pub fun construct_emit(e: *emitter.Emit, ty: u32, parts: *u32, n: u32) u32;
```

an OpCompositeConstruct of `ty` from its `n` parts

## fun bitcast_emit

```mach
pub fun bitcast_emit(e: *emitter.Emit, to_ty: u32, value: u32) u32;
```

## fun block_type

```mach
pub fun block_type(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId, role: u8) res[u32, fail.Fail];
```

## fun vec_at

```mach
pub fun vec_at(v: *Vector[u32], at: u32) u32;
```

## fun phys_pointer

```mach
pub fun phys_pointer(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) res[u32, fail.Fail];
```

the PhysicalStorageBuffer pointer a pointer type `id` of `gt` is in memory: to the
explicit-layout type its pointee is held in, or to the record its key names

## fun use_at

```mach
pub fun use_at(e: *emitter.Emit, loc: lang_source.Location);
```

the use the types built from here serve: an instruction, whose expression has the type,
a function's declaration, whose signature does, or an interface global's declaration.
a capability a type raises is located at its first use

## fun site_loc

```mach
pub fun site_loc(e: *emitter.Emit) lang_source.Location;
```

where a refusal of what is being built points: the instruction in a function, or the
declaration of the global whose types are being built

## fun phys_pointer_quiet

```mach
pub fun phys_pointer_quiet(e: *emitter.Emit, gt: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u32;
```

the same, for a type query that answers 0 for what it cannot declare: the refusal is held
for the refusal the 0 becomes

## fun pointer_align

```mach
pub fun pointer_align(e: *emitter.Emit, ptr: u32) u32;
```

the alignment an access through the physical pointer `ptr` carries, 0 for a logical one

## fun mark_physical_ptr

```mach
pub fun mark_physical_ptr(e: *emitter.Emit, ptr: u32, ty: u32);
```

notes that `ptr` is a physical pointer of the pointer type `ty`

## fun mark_physical

```mach
pub fun mark_physical(e: *emitter.Emit, ptr: u32, pointee: u32);
```

notes that `ptr` is a physical pointer to the type `pointee`

## fun carrier_bits

```mach
pub fun carrier_bits(e: *emitter.Emit, bits: u32) u32;
```

the width an integer of `bits` is carried at outside host-shared memory: its
own where the environment has its capability, otherwise the narrowest wider
width the model's alu computes at under every environment

## fun spv_type_of

```mach
pub fun spv_type_of(e: *emitter.Emit, tys: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u32;
```

the spirv type of an ir type. an f16, carried in the ir as a 16-bit integer whose form
is binary16, is the OpTypeFloat 16 it stands for wherever half_memory holds, in memory
and in a function alike, in a stage interface always, and its 16-bit
integer bits otherwise

## fun member_spv_type

```mach
pub fun member_spv_type(e: *emitter.Emit, tys: *ir_type.IrTypeTable, id: ir_type.IrTypeId) u32;
```

the type of a member or an element: a pointer held in an aggregate is a physical one

## fun ext_set_id

```mach
pub fun ext_set_id(e: *emitter.Emit, set: u32) u32;
```

## fun lane_width_as

```mach
pub fun lane_width_as(e: *emitter.Emit, value: u32, from_ty: u32, to_ty: u32) u32;
```

an integer, or a vector of them lane for lane, moved between a width memory holds and
its carrier's: OpUConvert where the two differ. a narrowing keeps the low bits, which
hold the value

