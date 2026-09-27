# mach.lang.target.isa.arm64.reloc

## fun is_local_got_kind

```mach
pub fun is_local_got_kind(kind: of.RelocKind) bool;
```

## fun branch_reach

```mach
pub fun branch_reach(kind: of.RelocKind) opt[of.BranchReach];
```

b and bl encode a signed 26-bit word displacement, +-128 MiB

## fun branch_thunk

```mach
pub fun branch_thunk() of.BranchThunk;
```

adrp x16 / add x16, x16, lo12 / br x16: ip0 is the register aapcs64 gives a
veneer to clobber, and adrp reaches +-4 GiB, the whole of the code

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

