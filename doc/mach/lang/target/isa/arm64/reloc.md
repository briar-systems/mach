# mach.lang.target.isa.arm64.reloc

## fun is_local_got_kind

```mach
pub fun is_local_got_kind(kind: target_of.RelocKind) bool;
```

## fun branch_reach

```mach
pub fun branch_reach(kind: target_of.RelocKind) opt[target_of.BranchReach];
```

b and bl encode a signed 26-bit word displacement, +-128 MiB

## fun branch_thunk

```mach
pub fun branch_thunk() target_of.BranchThunk;
```

adrp x16 / add x16, x16, lo12 / br x16: ip0 is the register aapcs64 gives a
veneer to clobber, and adrp reaches +-4 GiB, the whole of the code

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

