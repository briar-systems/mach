# mach.lang.target.isa.arm64.reloc

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

