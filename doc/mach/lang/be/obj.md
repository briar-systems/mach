# mach.lang.be.obj

## fun image_init

```mach
pub fun image_init(a: *A.Allocator, interner: *intern.Interner, name: intern.StrId) res[of.ObjectImage, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(o: *of.ObjectImage);
```

## fun symbol_index

```mach
pub fun symbol_index(o: *of.ObjectImage, name: intern.StrId) u32;
```

## rec DeferredReloc

```mach
pub rec DeferredReloc;
```

## rec DeferredRelocs

```mach
pub rec DeferredRelocs;
```

## fun deferred_init

```mach
pub fun deferred_init(a: *A.Allocator) DeferredRelocs;
```

## fun deferred_dnit

```mach
pub fun deferred_dnit(d: *DeferredRelocs);
```

## fun defer_relocation

```mach
pub fun defer_relocation(d: *DeferredRelocs, rec: DeferredReloc) err[fail.Fail];
```

## fun defer_relocation_for_target

```mach
pub fun defer_relocation_for_target(d: *DeferredRelocs, rec: DeferredReloc,
tgt: *target.Target, section_kind: of.SectionKind,
codegen_image: bool) err[fail.Fail];
```

## fun flush_deferred

```mach
pub fun flush_deferred(o: *of.ObjectImage, d: *DeferredRelocs) err[fail.Fail];
```

## fun rehome

```mach
pub fun rehome(dst_alloc: *A.Allocator, dst_interner: *intern.Interner,
src: *of.ObjectImage, remap: intern.ReinternMap) res[of.ObjectImage, fail.Fail];
```

## fun emit_image

```mach
pub fun emit_image(o: *of.ObjectImage, tgt: *target.Target, destination: str) err[fail.Fail];
```

