# mach.lang.target.isa.riscv.reloc

## fun is_local_got_kind

```mach
pub fun is_local_got_kind(kind: target_of.RelocKind) bool;
```

## fun reloc_traits

```mach
pub fun reloc_traits(kind: target_of.RelocKind,
section_kind: target_of.SectionKind,
codegen_image: bool) res[of_reloc.RelocTraits, of_reloc.RelocError];
```

## fun apply_reloc

```mach
pub fun apply_reloc(kind: target_of.RelocKind, dst: *u8, patch_off: u32, sec_len: u32,
target: of_reloc.RelocTarget, addend: i64, patch_va: u64,
image_base: u64) res[bool, of_reloc.RelocError];
```

## fun jal20_ok

```mach
pub fun jal20_ok(delta: i64) bool;
```

## fun set_jal20

```mach
pub fun set_jal20(word: u32, delta: i64) u32;
```

the j-type immediate: imm[20] at 31, imm[10:1] at 30:21, imm[11] at 20, imm[19:12] at 19:12

## fun resolve_riscv_pcrel_pairs

```mach
pub fun resolve_riscv_pcrel_pairs(alloc: *A.Allocator,
img: *target_of.ObjectImage) err[fail.Fail];
```

## fun resolve_riscv_reloc_operand

```mach
pub fun resolve_riscv_reloc_operand(img: *target_of.ObjectImage,
reloc_index: u32) res[target_of.RelocOperand, fail.Fail];
```

