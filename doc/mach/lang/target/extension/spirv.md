# mach.lang.target.extension.spirv

## val FLOAT16

```mach
pub val FLOAT16: u64 = 0x1
```

the spirv extension vocabulary: capabilities and device features an
environment guarantees beyond the core, or a consumer enables, which the
catalog's rows, the instruction rows and the emitter read. float16 is the
Float16 capability, so f16 is the native OpTypeFloat 16.
zero_init_workgroup is the shaderZeroInitializeWorkgroupMemory feature, so a
`#[shared]` variable takes an OpConstantNull initializer instead of the zeroing
the compiler inserts. the subgroup_ family are the VOTE, ARITHMETIC,
BALLOT, SHUFFLE, SHUFFLE_RELATIVE, CLUSTERED and QUAD bits of Vulkan's
subgroupSupportedOperations, which no Vulkan version guarantees, and
subgroup_graphics_stages is subgroupSupportedStages reaching the vertex and
fragment stages, where Vulkan guarantees only compute. storage_read_without_format and
storage_write_without_format are the shaderStorageImageReadWithoutFormat and
shaderStorageImageWriteWithoutFormat features, so a storage image of Unknown
format may be read or written. storage_image_multisample is the
shaderStorageImageMultisample feature, which enables both StorageImageMultisample
and ImageMSArray, so a storage image may be multisampled, arrayed or not.
resource_min_lod is the shaderResourceMinLod feature, which enables MinLod, so a
sample may name the least level of detail it reads. image_gather_extended is the
shaderImageGatherExtended feature, which enables ImageGatherExtended, so a gather
may take an offset computed at run time. maintenance8 is the
maintenance8 feature, under which a fetch or a sample takes a run-time offset too
(VUID-RuntimeSpirv-Offset-10213).
vulkan_memory_model is the vulkanMemoryModel feature, and holding it selects the
Vulkan memory model in place of GLSL450. vulkan_memory_model_device_scope is
vulkanMemoryModelDeviceScope, which the Device scope needs under that model.
int8, int16, int64, float16 and float64 are the shaderInt8, shaderInt16, shaderInt64,
shaderFloat16 and shaderFloat64 features, which no Vulkan version guarantees: an
environment's ceiling only admits their capabilities. under int8 and int16 an
integer of that width is computed at its own width rather than carried in a wider
one, and under float16 an f16 is the native OpTypeFloat 16.
buffer_device_address is the bufferDeviceAddress feature, under which a pointer held in
memory is a physical pointer into a buffer the host passes by address.
the storage features are Vulkan's 16- and 8-bit storage features, each the one
capability of its name: storage_buffer_16bit_access is storageBuffer16BitAccess,
so a storage buffer, or a buffer reached through a physical pointer, may hold a
16-bit member, uniform_and_storage_buffer_16bit_access
a uniform block, storage_push_constant16 a push block and storage_input_output16 a
stage interface, and the 8-bit three the same for an 8-bit member. no Vulkan version
guarantees one, and Vulkan has no 8-bit stage interface

## val ZERO_INIT_WORKGROUP

```mach
pub val ZERO_INIT_WORKGROUP: u64 = 0x2
```

## val SUBGROUP_ARITHMETIC

```mach
pub val SUBGROUP_ARITHMETIC: u64 = 0x4
```

## val SUBGROUP_CLUSTERED

```mach
pub val SUBGROUP_CLUSTERED: u64 = 0x8
```

## val STORAGE_READ_WITHOUT_FORMAT

```mach
pub val STORAGE_READ_WITHOUT_FORMAT: u64 = 0x10
```

## val STORAGE_WRITE_WITHOUT_FORMAT

```mach
pub val STORAGE_WRITE_WITHOUT_FORMAT: u64 = 0x20
```

## val SUBGROUP_VOTE

```mach
pub val SUBGROUP_VOTE: u64 = 0x40
```

## val SUBGROUP_BALLOT

```mach
pub val SUBGROUP_BALLOT: u64 = 0x80
```

## val SUBGROUP_SHUFFLE

```mach
pub val SUBGROUP_SHUFFLE: u64 = 0x100
```

## val SUBGROUP_SHUFFLE_RELATIVE

```mach
pub val SUBGROUP_SHUFFLE_RELATIVE: u64 = 0x200
```

## val SUBGROUP_QUAD

```mach
pub val SUBGROUP_QUAD: u64 = 0x400
```

## val SUBGROUP_GRAPHICS_STAGES

```mach
pub val SUBGROUP_GRAPHICS_STAGES: u64 = 0x800
```

## val BUFFER_INT64_ATOMICS

```mach
pub val BUFFER_INT64_ATOMICS: u64 = 0x1000
```

the atomic features of VkPhysicalDeviceShaderAtomicInt64Features,
VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT, VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
and VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT, each named for its shaderBuffer*,
shaderShared* or shaderImage* member: an atomic on a type beyond 32-bit integers needs the
one for its storage class, which no Vulkan version guarantees. image_int64_atomics is also
the feature that enables Int64ImageEXT, which any image of 64-bit texels declares

## val SHARED_INT64_ATOMICS

```mach
pub val SHARED_INT64_ATOMICS: u64 = 0x2000
```

## val BUFFER_FLOAT32_ATOMICS

```mach
pub val BUFFER_FLOAT32_ATOMICS: u64 = 0x4000
```

## val BUFFER_FLOAT32_ATOMIC_ADD

```mach
pub val BUFFER_FLOAT32_ATOMIC_ADD: u64 = 0x8000
```

## val BUFFER_FLOAT32_ATOMIC_MIN_MAX

```mach
pub val BUFFER_FLOAT32_ATOMIC_MIN_MAX: u64 = 0x10000
```

## val BUFFER_FLOAT64_ATOMICS

```mach
pub val BUFFER_FLOAT64_ATOMICS: u64 = 0x20000
```

## val BUFFER_FLOAT64_ATOMIC_ADD

```mach
pub val BUFFER_FLOAT64_ATOMIC_ADD: u64 = 0x40000
```

## val BUFFER_FLOAT64_ATOMIC_MIN_MAX

```mach
pub val BUFFER_FLOAT64_ATOMIC_MIN_MAX: u64 = 0x80000
```

## val SHARED_FLOAT32_ATOMICS

```mach
pub val SHARED_FLOAT32_ATOMICS: u64 = 0x100000
```

## val SHARED_FLOAT32_ATOMIC_ADD

```mach
pub val SHARED_FLOAT32_ATOMIC_ADD: u64 = 0x200000
```

## val SHARED_FLOAT32_ATOMIC_MIN_MAX

```mach
pub val SHARED_FLOAT32_ATOMIC_MIN_MAX: u64 = 0x400000
```

## val SHARED_FLOAT64_ATOMICS

```mach
pub val SHARED_FLOAT64_ATOMICS: u64 = 0x800000
```

## val SHARED_FLOAT64_ATOMIC_ADD

```mach
pub val SHARED_FLOAT64_ATOMIC_ADD: u64 = 0x1000000
```

## val SHARED_FLOAT64_ATOMIC_MIN_MAX

```mach
pub val SHARED_FLOAT64_ATOMIC_MIN_MAX: u64 = 0x2000000
```

## val STORAGE_IMAGE_MULTISAMPLE

```mach
pub val STORAGE_IMAGE_MULTISAMPLE: u64 = 0x4000000
```

## val RESOURCE_MIN_LOD

```mach
pub val RESOURCE_MIN_LOD: u64 = 0x8000000
```

## val IMAGE_GATHER_EXTENDED

```mach
pub val IMAGE_GATHER_EXTENDED: u64 = 0x10000000
```

## val MAINTENANCE8

```mach
pub val MAINTENANCE8: u64 = 0x20000000
```

## val IMAGE_INT64_ATOMICS

```mach
pub val IMAGE_INT64_ATOMICS: u64 = 0x40000000
```

## val IMAGE_FLOAT32_ATOMICS

```mach
pub val IMAGE_FLOAT32_ATOMICS: u64 = 0x80000000
```

## val IMAGE_FLOAT32_ATOMIC_ADD

```mach
pub val IMAGE_FLOAT32_ATOMIC_ADD: u64 = 0x100000000
```

## val IMAGE_FLOAT32_ATOMIC_MIN_MAX

```mach
pub val IMAGE_FLOAT32_ATOMIC_MIN_MAX: u64 = 0x200000000
```

## val VULKAN_MEMORY_MODEL

```mach
pub val VULKAN_MEMORY_MODEL: u64 = 0x400000000
```

## val VULKAN_MEMORY_MODEL_DEVICE_SCOPE

```mach
pub val VULKAN_MEMORY_MODEL_DEVICE_SCOPE: u64 = 0x800000000
```

## val INT8

```mach
pub val INT8: u64 = 0x1000000000
```

## val INT16

```mach
pub val INT16: u64 = 0x2000000000
```

## val BUFFER_DEVICE_ADDRESS

```mach
pub val BUFFER_DEVICE_ADDRESS: u64 = 0x4000000000
```

## val INT64

```mach
pub val INT64: u64 = 0x8000000000
```

## val FLOAT64

```mach
pub val FLOAT64: u64 = 0x10000000000
```

## val BUFFER_FLOAT16_ATOMICS

```mach
pub val BUFFER_FLOAT16_ATOMICS: u64 = 0x20000000000
```

## val BUFFER_FLOAT16_ATOMIC_ADD

```mach
pub val BUFFER_FLOAT16_ATOMIC_ADD: u64 = 0x40000000000
```

## val BUFFER_FLOAT16_ATOMIC_MIN_MAX

```mach
pub val BUFFER_FLOAT16_ATOMIC_MIN_MAX: u64 = 0x80000000000
```

## val SHARED_FLOAT16_ATOMICS

```mach
pub val SHARED_FLOAT16_ATOMICS: u64 = 0x100000000000
```

## val SHARED_FLOAT16_ATOMIC_ADD

```mach
pub val SHARED_FLOAT16_ATOMIC_ADD: u64 = 0x200000000000
```

## val SHARED_FLOAT16_ATOMIC_MIN_MAX

```mach
pub val SHARED_FLOAT16_ATOMIC_MIN_MAX: u64 = 0x400000000000
```

## val STORAGE_BUFFER_16BIT_ACCESS

```mach
pub val STORAGE_BUFFER_16BIT_ACCESS: u64 = 0x800000000000
```

## val UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS

```mach
pub val UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS: u64 = 0x1000000000000
```

## val STORAGE_PUSH_CONSTANT16

```mach
pub val STORAGE_PUSH_CONSTANT16: u64 = 0x2000000000000
```

## val STORAGE_INPUT_OUTPUT16

```mach
pub val STORAGE_INPUT_OUTPUT16: u64 = 0x4000000000000
```

## val STORAGE_BUFFER_8BIT_ACCESS

```mach
pub val STORAGE_BUFFER_8BIT_ACCESS: u64 = 0x8000000000000
```

## val UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS

```mach
pub val UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS: u64 = 0x10000000000000
```

## val STORAGE_PUSH_CONSTANT8

```mach
pub val STORAGE_PUSH_CONSTANT8: u64 = 0x20000000000000
```

## val NAME_COUNT

```mach
pub val NAME_COUNT: u32 = 54
```

## val NAMES

```mach
pub val NAMES: [NAME_COUNT]extension.Extension = [NAME_COUNT]extension.Extension;
```

## val OPEN

```mach
pub val OPEN: u64 = FLOAT16 | ZERO_INIT_WORKGROUP | SUBGROUP_ARITHMETIC | SUBGROUP_CLUSTERED
| STORAGE_READ_WITHOUT_FORMAT | STORAGE_WRITE_WITHOUT_FORMAT
| SUBGROUP_VOTE | SUBGROUP_BALLOT | SUBGROUP_SHUFFLE | SUBGROUP_SHUFFLE_RELATIVE | SUBGROUP_QUAD
| SUBGROUP_GRAPHICS_STAGES | ATOMICS | STORAGE_IMAGE_MULTISAMPLE | RESOURCE_MIN_LOD
| IMAGE_GATHER_EXTENDED | MAINTENANCE8 | VULKAN_MEMORY_MODEL | VULKAN_MEMORY_MODEL_DEVICE_SCOPE
| INT8 | INT16 | BUFFER_DEVICE_ADDRESS | INT64 | FLOAT64 | STORAGE_ACCESS
```

every extension of the vocabulary, which a module naming no environment holds

