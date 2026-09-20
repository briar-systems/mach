# mach.lang.target.of.macho.reloc

## rec MachoReloc

```mach
pub rec MachoReloc;
```

## fun macho_reloc_for

```mach
pub fun macho_reloc_for(itn: *intern.Interner, alloc: *A.Allocator, arch_id: u32, kind: of.RelocKind) res[MachoReloc, fail.Fail];
```

## fun reloc_kind_for

```mach
pub fun reloc_kind_for(itn: *intern.Interner, alloc: *A.Allocator, cputype: u32, r_type: u32, r_length: u32, insn: u32) res[of.RelocKind, fail.Fail];
```

## fun x64_is_signed

```mach
pub fun x64_is_signed(r_type: u32) bool;
```

## fun add_u64_mod

```mach
pub fun add_u64_mod(a: u64, b: u64) u64;
```

