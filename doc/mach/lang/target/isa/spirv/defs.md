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

## val GLSL450_PACK_HALF_2X16

```mach
pub val GLSL450_PACK_HALF_2X16:   u32 = 58
```

the GLSL.std.450 instructions the emitter itself moves an f16's bits through

## val GLSL450_UNPACK_HALF_2X16

```mach
pub val GLSL450_UNPACK_HALF_2X16: u32 = 62
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

## val IMAGE_OPERANDS_LOD

```mach
pub val IMAGE_OPERANDS_LOD:           u32 = 0x2
```

## val IMAGE_OPERANDS_SAMPLE

```mach
pub val IMAGE_OPERANDS_SAMPLE:        u32 = 0x40
```

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

