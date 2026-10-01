# mach.lang.target.isa.spirv.defs

## val SPV_SET_NONE

```mach
pub val SPV_SET_NONE:    u8 = 0
```

## val SPV_SET_CORE

```mach
pub val SPV_SET_CORE:    u8 = 1
```

## val SPV_SET_GLSL450

```mach
pub val SPV_SET_GLSL450: u8 = 2
```

## val SET_CORE_NAME

```mach
pub val SET_CORE_NAME:    str = "core"
```

## val SET_GLSL450_NAME

```mach
pub val SET_GLSL450_NAME: str = "GLSL.std.450"
```

## val GLSL450_PACK_HALF_2X16

```mach
pub val GLSL450_PACK_HALF_2X16:   u32 = 58
```

the GLSL.std.450 instructions the emitter itself moves an f16's bits through

## val GLSL450_UNPACK_HALF_2X16

```mach
pub val GLSL450_UNPACK_HALF_2X16: u32 = 62
```

## val CTOR_IMAGE_NAME

```mach
pub val CTOR_IMAGE_NAME:         str = "image"
```

## val CTOR_SAMPLED_IMAGE_NAME

```mach
pub val CTOR_SAMPLED_IMAGE_NAME: str = "sampled_image"
```

## val CTOR_SAMPLER_NAME

```mach
pub val CTOR_SAMPLER_NAME:       str = "sampler"
```

## val SPV_CTOR_IMAGE

```mach
pub val SPV_CTOR_IMAGE:         u32 = 0
```

## val SPV_CTOR_SAMPLED_IMAGE

```mach
pub val SPV_CTOR_SAMPLED_IMAGE: u32 = 1
```

## val SPV_CTOR_SAMPLER

```mach
pub val SPV_CTOR_SAMPLER:       u32 = 2
```

## val TEXEL_F32

```mach
pub val TEXEL_F32: u32 = 0
```

## val TEXEL_I32

```mach
pub val TEXEL_I32: u32 = 1
```

## val TEXEL_U32

```mach
pub val TEXEL_U32: u32 = 2
```

## val TEXEL_I64

```mach
pub val TEXEL_I64: u32 = 3
```

## val TEXEL_U64

```mach
pub val TEXEL_U64: u32 = 4
```

## val IMAGE_ARITY

```mach
pub val IMAGE_ARITY:      u32 = 7
```

## val IMAGE_OP_TEXEL

```mach
pub val IMAGE_OP_TEXEL:   u32 = 0
```

## val IMAGE_OP_DIM

```mach
pub val IMAGE_OP_DIM:     u32 = 1
```

## val IMAGE_OP_DEPTH

```mach
pub val IMAGE_OP_DEPTH:   u32 = 2
```

## val IMAGE_OP_ARRAYED

```mach
pub val IMAGE_OP_ARRAYED: u32 = 3
```

## val IMAGE_OP_MS

```mach
pub val IMAGE_OP_MS:      u32 = 4
```

## val IMAGE_OP_SAMPLED

```mach
pub val IMAGE_OP_SAMPLED: u32 = 5
```

## val IMAGE_OP_FORMAT

```mach
pub val IMAGE_OP_FORMAT:  u32 = 6
```

## val OP_DEF_COUNT

```mach
pub val OP_DEF_COUNT:   usize = 107
```

## val TYPE_DEF_COUNT

```mach
pub val TYPE_DEF_COUNT: usize = 3
```

## val GROUP_OPERATION_REDUCE

```mach
pub val GROUP_OPERATION_REDUCE:           u32 = 0
```

## val GROUP_OPERATION_INCLUSIVE_SCAN

```mach
pub val GROUP_OPERATION_INCLUSIVE_SCAN:   u32 = 1
```

## val GROUP_OPERATION_EXCLUSIVE_SCAN

```mach
pub val GROUP_OPERATION_EXCLUSIVE_SCAN:   u32 = 2
```

## val GROUP_OPERATION_CLUSTERED_REDUCE

```mach
pub val GROUP_OPERATION_CLUSTERED_REDUCE: u32 = 3
```

## val ARITHMETIC_GROUP_OPERATION

```mach
pub val ARITHMETIC_GROUP_OPERATION: isa.OpEnum = isa.OpEnum;
```

## val IMAGE_OPERANDS_BIAS

```mach
pub val IMAGE_OPERANDS_BIAS:          u32 = 0x1
```

## val IMAGE_OPERANDS_LOD

```mach
pub val IMAGE_OPERANDS_LOD:           u32 = 0x2
```

## val IMAGE_OPERANDS_GRAD

```mach
pub val IMAGE_OPERANDS_GRAD:          u32 = 0x4
```

## val IMAGE_OPERANDS_CONST_OFFSET

```mach
pub val IMAGE_OPERANDS_CONST_OFFSET:  u32 = 0x8
```

## val IMAGE_OPERANDS_OFFSET

```mach
pub val IMAGE_OPERANDS_OFFSET:        u32 = 0x10
```

## val IMAGE_OPERANDS_CONST_OFFSETS

```mach
pub val IMAGE_OPERANDS_CONST_OFFSETS: u32 = 0x20
```

## val IMAGE_OPERANDS_SAMPLE

```mach
pub val IMAGE_OPERANDS_SAMPLE:        u32 = 0x40
```

## val IMAGE_OPERANDS_MIN_LOD

```mach
pub val IMAGE_OPERANDS_MIN_LOD:       u32 = 0x80
```

## val IMAGE_OPERANDS_OFFSETS

```mach
pub val IMAGE_OPERANDS_OFFSETS: u32 = IMAGE_OPERANDS_CONST_OFFSET | IMAGE_OPERANDS_OFFSET | IMAGE_OPERANDS_CONST_OFFSETS
```

the bits that offset a coordinate, of which an instruction takes one

## val IMAGE_OPERANDS_MAKE_TEXEL_AVAILABLE

```mach
pub val IMAGE_OPERANDS_MAKE_TEXEL_AVAILABLE: u32 = 0x100
```

the Image Operands the emitter adds to a coherent storage image's access under the
Vulkan memory model. no row admits them from a call yet

## val IMAGE_OPERANDS_MAKE_TEXEL_VISIBLE

```mach
pub val IMAGE_OPERANDS_MAKE_TEXEL_VISIBLE:   u32 = 0x200
```

## val IMAGE_OPERANDS_NON_PRIVATE_TEXEL

```mach
pub val IMAGE_OPERANDS_NON_PRIVATE_TEXEL:    u32 = 0x400
```

## val IMPLICIT_LOD_OPERANDS

```mach
pub val IMPLICIT_LOD_OPERANDS: isa.OpEnum = isa.OpEnum;
```

## val SAMPLE_LOD_OPERANDS

```mach
pub val SAMPLE_LOD_OPERANDS:   isa.OpEnum = isa.OpEnum;
```

## val FETCH_OPERANDS

```mach
pub val FETCH_OPERANDS:        isa.OpEnum = isa.OpEnum;
```

## val GATHER_OPERANDS

```mach
pub val GATHER_OPERANDS:       isa.OpEnum = isa.OpEnum;
```

## val STORAGE_OPERANDS

```mach
pub val STORAGE_OPERANDS:      isa.OpEnum = isa.OpEnum;
```

## val BALLOT_GROUP_OPERATION

```mach
pub val BALLOT_GROUP_OPERATION: isa.OpEnum = isa.OpEnum;
```

## val ATOMIC_INTEGER

```mach
pub val ATOMIC_INTEGER:       isa.OpTyping = isa.OpTyping;
```

## val ATOMIC_MEMORY

```mach
pub val ATOMIC_MEMORY:        isa.OpTyping = isa.OpTyping;
```

## val ATOMIC_FLOAT_ADD

```mach
pub val ATOMIC_FLOAT_ADD:     isa.OpTyping = isa.OpTyping;
```

## val ATOMIC_FLOAT_MIN_MAX

```mach
pub val ATOMIC_FLOAT_MIN_MAX: isa.OpTyping = isa.OpTyping;
```

## val FIRST_OPERAND

```mach
pub val FIRST_OPERAND:         isa.OpTyping = isa.OpTyping;
```

the operand an untyped row's relations are stated against: the first for an image's
handle, holding its type to nothing, the first for a math instruction, a scalar or vector
of floats, and the value for a subgroup operation, a scalar or vector of integers or floats

## val MATH_OPERAND

```mach
pub val MATH_OPERAND:          isa.OpTyping = isa.OpTyping;
```

## val SUBGROUP_VALUE

```mach
pub val SUBGROUP_VALUE:        isa.OpTyping = isa.OpTyping;
```

## val GROUP_OPERATION_VALUE

```mach
pub val GROUP_OPERATION_VALUE: isa.OpTyping = isa.OpTyping;
```

## val READ_TEXEL_COUNT

```mach
pub val READ_TEXEL_COUNT:   isa.OpTexelCount = isa.OpTexelCount;
```

how many components each image instruction's texel has. a read's result is a 4-vector in
Vulkan, a fetch's, a sample's and a gather's in every environment, and a write's texel needs at
least the components its image's format stores, which spirv-val cannot see: the format
is matched to a VkFormat only when the descriptor is bound

## val FETCH_TEXEL_COUNT

```mach
pub val FETCH_TEXEL_COUNT:  isa.OpTexelCount = isa.OpTexelCount;
```

## val SAMPLE_TEXEL_COUNT

```mach
pub val SAMPLE_TEXEL_COUNT: isa.OpTexelCount = isa.OpTexelCount;
```

## val GATHER_TEXEL_COUNT

```mach
pub val GATHER_TEXEL_COUNT: isa.OpTexelCount = isa.OpTexelCount;
```

## val DREF_TEXEL_COUNT

```mach
pub val DREF_TEXEL_COUNT:   isa.OpTexelCount = isa.OpTexelCount;
```

## val WRITE_TEXEL_COUNT

```mach
pub val WRITE_TEXEL_COUNT:  isa.OpTexelCount = isa.OpTexelCount;
```

## val DREF_REFERENCE

```mach
pub val DREF_REFERENCE: isa.OpOperandScalar = isa.OpOperandScalar;
```

the reference a depth comparison compares each texel against, a 32-bit float whatever the
image's sampled type

## val FREXP_EXPONENT

```mach
pub val FREXP_EXPONENT: isa.OpOperandScalar = isa.OpOperandScalar;
```

the exponent `Frexp` stores through its out-pointer, shaped like the value it splits

## rec DefStorage

```mach
pub rec DefStorage;
```

## fun set_import_name

```mach
pub fun set_import_name(set: u8) str;
```

## fun register_defs

```mach
pub fun register_defs(storage: *DefStorage) *isa.TargetDefs;
```

## fun is_image_operands

```mach
pub fun is_image_operands(en: *isa.OpEnum) bool;
```

whether `en` is an instruction's view of the Image Operands mask

## fun image_coordinate_components

```mach
pub fun image_coordinate_components(dim: u32) u32;
```

how many components an image of `dim` is addressed by, besides an array layer: what
an offset added to its coordinate, and each derivative of it, has

## fun image_operand_shape_refusal

```mach
pub fun image_operand_shape_refusal(bit: u32, dim: u32, float: bool, components: u32, elements: u32) str;
```

why the operands the Image Operands bit `bit` brings do not fit an image of `dim`, nil
when they do: each is a `float` (else integer) scalar or vector of `components`
components, or an array of `elements` of them (0 for none). an offset or a derivative
has one component per dimension of the image's coordinate, a least level of detail is
one float, and only a gather's four offsets are an array

## val NO_IMAGE_DIM

```mach
pub val NO_IMAGE_DIM: u32 = 0xFFFFFFFF
```

the dimensionality of an instruction's first operand when it is neither an image nor
a sampled image of one

## fun is_dref

```mach
pub fun is_dref(opcode: u32) bool;
```

whether `opcode` is one of the `OpImage*Dref*` depth comparisons

## fun image_use_refusal

```mach
pub fun image_use_refusal(opcode: u32, image: bool, ms: bool, storage: bool, dim: u32, depth: bool, mask: u32) str;
```

why an image instruction's use is invalid, nil when it is valid: `image` says whether
its first operand is an image, `ms` whether that image is multisampled, `storage`
whether it is a storage image, `dim` the dimensionality of the image it is or
samples (NO_IMAGE_DIM for neither) and `depth` whether that image is a depth image,
and `mask` is the Image Operands mask it passes, 0 when it passes none. a multisampled
image's texel is named by its sample, and only a multisampled image has samples

