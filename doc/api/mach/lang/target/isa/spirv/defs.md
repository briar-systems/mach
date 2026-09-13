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

## val SPV_OP_NONE

```mach
pub val SPV_OP_NONE: u32 = 0xFFFFFFFF
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
pub val IMAGE_ARITY:      u32 = 6
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

## val OP_DEF_COUNT

```mach
pub val OP_DEF_COUNT:   usize = 38
```

## val TYPE_DEF_COUNT

```mach
pub val TYPE_DEF_COUNT: usize = 3
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

## fun opcode_of

```mach
pub fun opcode_of(definitions: *isa.TargetDefs, set: u8, name: str) u32;
```

## fun set_of

```mach
pub fun set_of(name: str) u8;
```

