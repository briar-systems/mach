# mach.lang.target.isa.riscv.reloc

## fun is_local_got_kind

```mach
pub fun is_local_got_kind(kind: of.RelocKind) bool;
```

## fun reloc_traits

```mach
pub fun reloc_traits(kind: of.RelocKind,
section_kind: of.SectionKind,
codegen_image: bool) res[rel.RelocTraits, rel.RelocError];
```

## fun apply_reloc

```mach
pub fun apply_reloc(kind: of.RelocKind, dst: *u8, patch_off: u32, sec_len: u32,
target: rel.RelocTarget, addend: i64, patch_va: u64,
image_base: u64) res[bool, rel.RelocError];
```

## fun resolve_riscv_pcrel_pairs

```mach
pub fun resolve_riscv_pcrel_pairs(alloc: *A.Allocator,
img: *of.ObjectImage) err[fail.Fail];
```

## fun resolve_riscv_reloc_operand

```mach
pub fun resolve_riscv_reloc_operand(img: *of.ObjectImage,
reloc_index: u32) res[of.RelocOperand, fail.Fail];
```

