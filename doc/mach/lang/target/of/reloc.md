# mach.lang.target.of.reloc

## def RelocError

```mach
pub def RelocError: target_of.RelocError
```

## val RELOC_OVERFLOW

```mach
pub val RELOC_OVERFLOW: RelocError = target_of.RELOC_OVERFLOW
```

## val RELOC_UNSUPPORTED

```mach
pub val RELOC_UNSUPPORTED: RelocError = target_of.RELOC_UNSUPPORTED
```

## val RELOC_INVALID_INSTRUCTION

```mach
pub val RELOC_INVALID_INSTRUCTION: RelocError = target_of.RELOC_INVALID_INSTRUCTION
```

## def RelocAddendMode

```mach
pub def RelocAddendMode: target_of.RelocAddendMode
```

## val RELOC_ADDEND_SYMBOL

```mach
pub val RELOC_ADDEND_SYMBOL: RelocAddendMode = target_of.RELOC_ADDEND_SYMBOL
```

## val RELOC_ADDEND_FIELD_BIAS

```mach
pub val RELOC_ADDEND_FIELD_BIAS: RelocAddendMode = target_of.RELOC_ADDEND_FIELD_BIAS
```

## val RELOC_ADDEND_IGNORED

```mach
pub val RELOC_ADDEND_IGNORED: RelocAddendMode = target_of.RELOC_ADDEND_IGNORED
```

## def RelocTraits

```mach
pub def RelocTraits: target_of.RelocTraits
```

## fun traits

```mach
pub fun traits(field_width: u32, addend_mode: RelocAddendMode,
text_bias_min: i32, text_bias_max: i32) RelocTraits;
```

## def RelocTarget

```mach
pub def RelocTarget: target_of.RelocTarget
```

## fun target

```mach
pub fun target(vaddr: u64, section_vaddr: u64, section_index: u32) RelocTarget;
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

## rec Site

```mach
pub rec Site;
```

one relocation to apply: the field it patches and the value it resolves to

## def ApplyFn

```mach
pub def ApplyFn: fun(*Site) res[bool, RelocError]
```

## rec Row

```mach
pub rec Row;
```

one relocation kind an instruction set applies: the width of its field, how
its addend is read, whether a pc-relative field in text carries its own width
as a bias, and how it is written

## fun row

```mach
pub fun row(rows: *Row, count: usize, kind: target_of.RelocKind) *Row;
```

the row of `kind` in an instruction set's table, nil for a kind it does not apply

## fun table_traits

```mach
pub fun table_traits(rows: *Row, count: usize, kind: target_of.RelocKind,
section_kind: target_of.SectionKind, codegen_image: bool) res[RelocTraits, RelocError];
```

the traits of `kind` as an instruction set's table states them

## fun table_apply

```mach
pub fun table_apply(rows: *Row, count: usize, kind: target_of.RelocKind, dst: *u8, patch_off: u32,
sec_len: u32, target: RelocTarget, addend: i64, patch_va: u64, image_base: u64) res[bool, RelocError];
```

`kind` applied through an instruction set's table

## fun site_abs64

```mach
pub fun site_abs64(s: *Site) res[bool, RelocError];
```

## fun site_abs32

```mach
pub fun site_abs32(s: *Site) res[bool, RelocError];
```

## fun site_pcrel32

```mach
pub fun site_pcrel32(s: *Site) res[bool, RelocError];
```

## fun site_pcrel64

```mach
pub fun site_pcrel64(s: *Site) res[bool, RelocError];
```

## fun normalize_image

```mach
pub fun normalize_image(tgt_isa: *isa.IsaVTable, alloc: *A.Allocator,
img: *target_of.ObjectImage) err[fail.Fail];
```

## fun resolve_operand

```mach
pub fun resolve_operand(tgt_isa: *isa.IsaVTable, img: *target_of.ObjectImage,
reloc_index: u32) res[target_of.RelocOperand, fail.Fail];
```

