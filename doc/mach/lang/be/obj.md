# mach.lang.be.obj

## fun image_init

```mach
pub fun image_init(a: *A.Allocator, interner: *intern.Interner, name: intern.StrId) res[target_of.ObjectImage, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(o: *target_of.ObjectImage);
```

## fun symbol_index

```mach
pub fun symbol_index(o: *target_of.ObjectImage, name: intern.StrId) u32;
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
tgt: *lang_target.Target, section_kind: target_of.SectionKind,
codegen_image: bool) err[fail.Fail];
```

## fun flush_deferred

```mach
pub fun flush_deferred(o: *target_of.ObjectImage, d: *DeferredRelocs) err[fail.Fail];
```

## fun rehome

```mach
pub fun rehome(dst_alloc: *A.Allocator, dst_interner: *intern.Interner,
src: *target_of.ObjectImage, remap: intern.ReinternMap) res[target_of.ObjectImage, fail.Fail];
```

## fun emit_image

```mach
pub fun emit_image(o: *target_of.ObjectImage, tgt: *lang_target.Target, destination: str) err[fail.Fail];
```

