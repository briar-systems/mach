# mach.lang.be.linker.atoms

## rec AtomPlan

```mach
pub rec AtomPlan;
```

## val ATOM_OWNS

```mach
pub val ATOM_OWNS: u32 = 0xFFFFFFFF
```

## fun plan_atom_coalescing

```mach
pub fun plan_atom_coalescing(alloc: *A.Allocator, modules: *of.ObjectImage, module_count: u32,
sec_base: *u32, sec_total: u32,
atoms: *AtomPlan) res[*u32, fail.Fail];
```

## fun init_atom_plan

```mach
pub fun init_atom_plan(plan: *AtomPlan);
```

## fun free_atom_plan

```mach
pub fun free_atom_plan(alloc: *A.Allocator, plan: *AtomPlan);
```

## fun atom_offset_dead

```mach
pub fun atom_offset_dead(plan: *AtomPlan, section: u32, off: u32) bool;
```

## fun atom_live_offset

```mach
pub fun atom_live_offset(plan: *AtomPlan, section: u32, off: u32) u32;
```

## fun atom_live_len

```mach
pub fun atom_live_len(plan: *AtomPlan, section: u32, original_len: u32) u32;
```

## fun copy_live_section

```mach
pub fun copy_live_section(dst: *u8, src: *u8, src_len: u32, plan: *AtomPlan, section: u32);
```

## fun build_atom_plan

```mach
pub fun build_atom_plan(s: *session.Session, modules: *of.ObjectImage, module_count: u32,
codegen_count: u32, sec_base: *u32, sec_total: u32, enabled: bool,
roots: *LinkRoots, arch: *isa.IsaVTable, plan: *AtomPlan) err[fail.Fail];
```

the atom plan of a final image: losing weak duplicates and every range no
root reaches become drops

## fun symbol_is_defined

```mach
pub fun symbol_is_defined(modules: *of.ObjectImage, m: u32, sy: u32) bool;
```

## fun definition_ref_is_dead

```mach
pub fun definition_ref_is_dead(modules: *of.ObjectImage, m: u32, sy: u32,
addend: i64, sec_base: *u32, atoms: *AtomPlan) bool;
```

whether the bytes a reference to definition sy of module m plus addend
addresses were dropped from the image

## fun reloc_source_live

```mach
pub fun reloc_source_live(modules: *of.ObjectImage, m: u32, r: *of.Relocation,
sec_base: *u32, atoms: *AtomPlan) bool;
```

## fun import_symbol_live

```mach
pub fun import_symbol_live(modules: *of.ObjectImage, module_count: u32,
symbol_name: intern.StrId, sec_base: *u32,
atoms: *AtomPlan) bool;
```

whether a live relocation still names symbol_name; a reference from debug
information describes code rather than needing the symbol, so it never counts

## fun has_function_branch_reloc

```mach
pub fun has_function_branch_reloc(modules: *of.ObjectImage, module_count: u32,
symbol_name: intern.StrId, sec_base: *u32,
atoms: *AtomPlan) bool;
```

## fun lt_isa

```mach
pub fun lt_isa(reg: *target.TargetRegistry, name: str) *isa.IsaVTable;
```

