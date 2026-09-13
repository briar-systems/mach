# mach.lang.target.abi

## val ABI_UNKNOWN

```mach
pub val ABI_UNKNOWN: u32 = 0
```

## val ABI_SYSV

```mach
pub val ABI_SYSV:    u32 = 1
```

## val ABI_WIN64

```mach
pub val ABI_WIN64:   u32 = 2
```

## val ABI_AAPCS64

```mach
pub val ABI_AAPCS64: u32 = 3
```

## val ABI_LP64

```mach
pub val ABI_LP64:    u32 = 4
```

## val ABI_LP64F

```mach
pub val ABI_LP64F:   u32 = 5
```

## val ABI_LP64D

```mach
pub val ABI_LP64D:   u32 = 6
```

## val ABI_ILP32

```mach
pub val ABI_ILP32:  u32 = 7
```

## val ABI_ILP32F

```mach
pub val ABI_ILP32F: u32 = 8
```

## val ABI_ILP32D

```mach
pub val ABI_ILP32D: u32 = 9
```

## val ABI_SPIRV

```mach
pub val ABI_SPIRV:  u32 = 10
```

## val ABI_CATALOG_VERSION

```mach
pub val ABI_CATALOG_VERSION: u8 = 1
```

## fun abi_id_for

```mach
pub fun abi_id_for(name: str) u32;
```

## fun abi_name_for

```mach
pub fun abi_name_for(id: u32) str;
```

## fun abi_catalog_len

```mach
pub fun abi_catalog_len() usize;
```

## fun abi_catalog_name

```mach
pub fun abi_catalog_name(index: usize) str;
```

## fun abi_fingerprint_tag

```mach
pub fun abi_fingerprint_tag(id: u32) u8;
```

## def ParamClass

```mach
pub def ParamClass: u8
```

## val CLASS_GP

```mach
pub val CLASS_GP:              ParamClass = 0
```

## val CLASS_FP

```mach
pub val CLASS_FP:              ParamClass = 1
```

## val CLASS_STACK

```mach
pub val CLASS_STACK:           ParamClass = 2
```

## val CLASS_SRET

```mach
pub val CLASS_SRET:            ParamClass = 3
```

## val CLASS_BYREF

```mach
pub val CLASS_BYREF:           ParamClass = 4
```

## val CLASS_STACK_BYREF

```mach
pub val CLASS_STACK_BYREF:     ParamClass = 5
```

## val CLASS_VEC_BYREF

```mach
pub val CLASS_VEC_BYREF:       ParamClass = 6
```

## val CLASS_VEC_STACK_BYREF

```mach
pub val CLASS_VEC_STACK_BYREF: ParamClass = 7
```

## val CLASS_VALUE

```mach
pub val CLASS_VALUE:           ParamClass = 8
```

## def PieceKind

```mach
pub def PieceKind: u8
```

## val PIECE_GP

```mach
pub val PIECE_GP:    PieceKind = 0
```

## val PIECE_FP

```mach
pub val PIECE_FP:    PieceKind = 1
```

## val PIECE_STACK

```mach
pub val PIECE_STACK: PieceKind = 2
```

## val PARAM_MAX_PIECES

```mach
pub val PARAM_MAX_PIECES: u32 = 4
```

## rec ParamPiece

```mach
pub rec ParamPiece;
```

## rec AggField

```mach
pub rec AggField;
```

## rec AggLayout

```mach
pub rec AggLayout;
```

## fun agg_none

```mach
pub fun agg_none() AggLayout;
```

## rec ParamSlot

```mach
pub rec ParamSlot;
```

reg is the physical register the slot rides, MIR_PREG_NIL for a stack slot;
the constructors take the isa regid the packs select (-1 for none)

## val EB_SSE_LO

```mach
pub val EB_SSE_LO:    u8 = 1
```

eightbyte class mask handed to a classifier: bit 0 and bit 1 mark eightbyte 0 and 1 as SSE, bit 2 and
bit 3 mark them as holding no leaf at all (padding only), and an unmarked eightbyte inside the object is INTEGER;
bit 4 marks an aggregate holding a field at an offset that is not a multiple of the field's own alignment
(a packed record), which System V classifies as MEMORY regardless of what its eightbytes hold

## val EB_SSE_HI

```mach
pub val EB_SSE_HI:    u8 = 2
```

## val EB_EMPTY_LO

```mach
pub val EB_EMPTY_LO:  u8 = 4
```

## val EB_EMPTY_HI

```mach
pub val EB_EMPTY_HI:  u8 = 8
```

## val EB_UNALIGNED

```mach
pub val EB_UNALIGNED: u8 = 16
```

## def ArgPassingFn

```mach
pub def ArgPassingFn: fun(i32, u64, u64, bool, u8, bool, bool, i32, i32, u8, u8, AggLayout) ParamSlot
```

## def RetPassingFn

```mach
pub def RetPassingFn: fun(u64, u64, bool, u8, bool, bool, u8, u8, AggLayout) ParamSlot
```

## def RegFileFn

```mach
pub def RegFileFn: isa.RegFileFn
```

## def VaModelFn

```mach
pub def VaModelFn: fun() VaModel
```

## def PassingModel

```mach
pub def PassingModel: u8
```

## val PASSING_CARRIERS

```mach
pub val PASSING_CARRIERS: PassingModel = 0
```

## val PASSING_VALUES

```mach
pub val PASSING_VALUES:   PassingModel = 1
```

## rec AbiVTable

```mach
pub rec AbiVTable;
```

## fun abi_vtable

```mach
pub fun abi_vtable(id: u32, name: str, arch_id: u32,
arg_passing: ArgPassingFn, ret_passing: RetPassingFn,
gp_arg_regs: RegFileFn, callee_saved: RegFileFn,
stack_align: u32, red_zone: u32, shadow_space: u32,
indirect_result_reg: i32, indirect_result_in_argfile: bool,
consumes_agg_layout: bool, fp_callee_saved_full_vector: bool,
float_arg_bits: u32, arg_slot_granularity: u32,
va_model: VaModelFn) AbiVTable;
```

## fun value_abi_vtable

```mach
pub fun value_abi_vtable(id: u32, name: str, arch_id: u32,
arg_passing: ArgPassingFn, ret_passing: RetPassingFn) AbiVTable;
```

## rec AbiRegistry

```mach
pub rec AbiRegistry;
```

## rec SigLayout

```mach
pub rec SigLayout;
```

## val VA_MODEL_REG_SAVE

```mach
pub val VA_MODEL_REG_SAVE: u8 = 0
```

## val VA_MODEL_HOME

```mach
pub val VA_MODEL_HOME:     u8 = 1
```

## rec VaModel

```mach
pub rec VaModel;
```

## fun va_model_make

```mach
pub fun va_model_make(kind: u8, float_dup_gp: bool, vector_count_reg: i32,
variadic_float_bits: u32) VaModel;
```

## fun variadic_float_in_fp_bank

```mach
pub fun variadic_float_in_fp_bank(m: *VaModel, width: u64) bool;
```

## fun registry_init_with_allocator

```mach
pub fun registry_init_with_allocator(alloc: *A.Allocator) AbiRegistry;
```

## fun registry_init

```mach
pub fun registry_init() AbiRegistry;
```

## fun registry_dnit

```mach
pub fun registry_dnit(reg: *AbiRegistry);
```

## fun registry_validate

```mach
pub fun registry_validate(reg: *AbiRegistry) err[fail.Fail];
```

## fun register

```mach
pub fun register(reg: *AbiRegistry, vt: *AbiVTable) err[fail.Fail];
```

## fun lookup

```mach
pub fun lookup(reg: *AbiRegistry, name: str) opt[*AbiVTable];
```

## fun registered_count

```mach
pub fun registered_count(reg: *AbiRegistry) u32;
```

## fun registered

```mach
pub fun registered(reg: *AbiRegistry, idx: u32) opt[*AbiVTable];
```

## fun covers_isa

```mach
pub fun covers_isa(vt: *AbiVTable, arch_id: u32) bool;
```

## fun make_slot

```mach
pub fun make_slot(class: ParamClass, reg: i32, offset: i64, size: u64, carrier_width: u8) ParamSlot;
```

## fun make_slot_indirect

```mach
pub fun make_slot_indirect(class: ParamClass, reg: i32, offset: i64, size: u64,
indirect_size: u64, indirect_align: u32) ParamSlot;
```

## fun make_slot_value

```mach
pub fun make_slot_value(size: u64) ParamSlot;
```

## fun make_slot_pair

```mach
pub fun make_slot_pair(class: ParamClass, reg: i32, reg2: i32, offset: i64, size: u64,
word: u32) ParamSlot;
```

## fun make_slot_hfa

```mach
pub fun make_slot_hfa(base_reg: i32, count: u8, elem: u8, size: u64) ParamSlot;
```

## fun make_piece

```mach
pub fun make_piece(kind: PieceKind, reg: i32, src_off: i64, stk_off: i64, width: u8) ParamPiece;
```

## fun make_slot_pieces

```mach
pub fun make_slot_pieces(class: ParamClass, pieces: *ParamPiece, count: u32, size: u64) ParamSlot;
```

## fun piece_memory_width

```mach
pub fun piece_memory_width(slot: *ParamSlot, piece: *ParamPiece) res[u8, fail.Fail];
```

## fun carrier_storage_extent

```mach
pub fun carrier_storage_extent(slot: *ParamSlot) res[u64, fail.Fail];
```

## fun carrier_storage_alignment

```mach
pub fun carrier_storage_alignment(slot: *ParamSlot) res[u32, fail.Fail];
```

