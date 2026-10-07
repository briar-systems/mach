# mach.lang.be.linker.resolution

the layout a link resolves, as the phases that patch, thunk and unwind the
image read it: the inputs, where each of their sections landed, the merged
sections and the image built from them, the symbols and the dynamic imports.
the link fills it phase by phase and releases what it holds with one dnit

## rec Resolution

```mach
pub rec Resolution;
```

modules and module_count are borrowed from the link's inputs, and format and
arch from its target, and the record owns everything else

## fun resolution_init

```mach
pub fun resolution_init(r: *Resolution, alloc: *A.Allocator, itn: *intern.Interner,
format: *target_of.OfVTable, arch: *isa.IsaVTable, image_base: u64);
```

## fun resolution_dnit

```mach
pub fun resolution_dnit(r: *Resolution, alloc: *A.Allocator);
```

