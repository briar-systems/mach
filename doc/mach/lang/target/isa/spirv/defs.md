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
pub val OP_DEF_COUNT:   usize = 101
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

## val IMAGE_OPERANDS_LOD

```mach
pub val IMAGE_OPERANDS_LOD:    u32 = 0x2
```

## val IMAGE_OPERANDS_SAMPLE

```mach
pub val IMAGE_OPERANDS_SAMPLE: u32 = 0x40
```

## val SAMPLE_LOD_OPERANDS

```mach
pub val SAMPLE_LOD_OPERANDS: isa.OpEnum = isa.OpEnum;
```

## val FETCH_OPERANDS

```mach
pub val FETCH_OPERANDS:      isa.OpEnum = isa.OpEnum;
```

## val STORAGE_OPERANDS

```mach
pub val STORAGE_OPERANDS:    isa.OpEnum = isa.OpEnum;
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

## fun image_use_refusal

```mach
pub fun image_use_refusal(opcode: u32, image: bool, ms: bool, storage: bool, dim: u32, mask: u32) str;
```

why an image instruction's use is invalid, nil when it is valid: `image` says whether
its first operand is an image, `ms` whether that image is multisampled, `storage`
whether it is a storage image and `dim` its dimensionality, and `mask` is the Image
Operands mask it passes, 0 when it passes none. a multisampled image's texel is named
by its sample, and only a multisampled image has samples

