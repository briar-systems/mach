# mach.lang.target.isa.x64.reloc

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

