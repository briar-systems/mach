# mach.lang.target.of.reloc

## def RelocError

```mach
pub def RelocError: of.RelocError
```

## val RELOC_OVERFLOW

```mach
pub val RELOC_OVERFLOW: RelocError = of.RELOC_OVERFLOW
```

## val RELOC_UNSUPPORTED

```mach
pub val RELOC_UNSUPPORTED: RelocError = of.RELOC_UNSUPPORTED
```

## val RELOC_INVALID_INSTRUCTION

```mach
pub val RELOC_INVALID_INSTRUCTION: RelocError = of.RELOC_INVALID_INSTRUCTION
```

## def RelocAddendMode

```mach
pub def RelocAddendMode: of.RelocAddendMode
```

## val RELOC_ADDEND_SYMBOL

```mach
pub val RELOC_ADDEND_SYMBOL: RelocAddendMode = of.RELOC_ADDEND_SYMBOL
```

## val RELOC_ADDEND_FIELD_BIAS

```mach
pub val RELOC_ADDEND_FIELD_BIAS: RelocAddendMode = of.RELOC_ADDEND_FIELD_BIAS
```

## val RELOC_ADDEND_IGNORED

```mach
pub val RELOC_ADDEND_IGNORED: RelocAddendMode = of.RELOC_ADDEND_IGNORED
```

## def RelocTraits

```mach
pub def RelocTraits: of.RelocTraits
```

## fun traits

```mach
pub fun traits(field_width: u32, addend_mode: RelocAddendMode,
text_bias_min: i32, text_bias_max: i32) RelocTraits;
```

## def RelocTarget

```mach
pub def RelocTarget: of.RelocTarget
```

## fun target

```mach
pub fun target(vaddr: u64, section_vaddr: u64, section_index: u32) RelocTarget;
```

## def RelocTraitsFn

```mach
pub def RelocTraitsFn: of.RelocTraitsFn
```

## def ApplyRelocFn

```mach
pub def ApplyRelocFn: of.ApplyRelocFn
```

## fun add_signed_to_u64

```mach
pub fun add_signed_to_u64(base: u64, addend: i64) res[u64, RelocError];
```

## fun add_u64_to_i64

```mach
pub fun add_u64_to_i64(base: u64, addend: i64) res[i64, RelocError];
```

## fun subtract_u64_from_i64

```mach
pub fun subtract_u64_from_i64(base: i64, subtrahend: u64) res[i64, RelocError];
```

## fun displacement

```mach
pub fun displacement(target_vaddr: u64, addend: i64,
patch_vaddr: u64) res[i64, RelocError];
```

## fun apply_abs64

```mach
pub fun apply_abs64(dst: *u8, patch_off: u32, sec_len: u32,
sym_va: u64, addend: i64) res[bool, RelocError];
```

## fun apply_abs32

```mach
pub fun apply_abs32(dst: *u8, patch_off: u32, sec_len: u32,
sym_va: u64, addend: i64) res[bool, RelocError];
```

## fun apply_abs16

```mach
pub fun apply_abs16(dst: *u8, patch_off: u32, sec_len: u32,
sym_va: u64, addend: i64) res[bool, RelocError];
```

## fun apply_pcrel32

```mach
pub fun apply_pcrel32(dst: *u8, patch_off: u32, sec_len: u32,
sym_va: u64, addend: i64, patch_va: u64) res[bool, RelocError];
```

## fun apply_pcrel64

```mach
pub fun apply_pcrel64(dst: *u8, patch_off: u32, sec_len: u32,
sym_va: u64, addend: i64, patch_va: u64) res[bool, RelocError];
```

## def NormalizeImageFn

```mach
pub def NormalizeImageFn: of.NormalizeImageFn
```

## fun normalize_image

```mach
pub fun normalize_image(tgt_isa: *isa.IsaVTable, alloc: *A.Allocator,
img: *of.ObjectImage) err[fail.Fail];
```

## fun resolve_operand

```mach
pub fun resolve_operand(tgt_isa: *isa.IsaVTable, img: *of.ObjectImage,
reloc_index: u32) res[of.RelocOperand, fail.Fail];
```

