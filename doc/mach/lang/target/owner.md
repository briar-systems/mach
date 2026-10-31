# mach.lang.target.owner

the owners' target each mach target cell means

a mach target cell, an operating system, an instruction set and a calling
convention, names one target of mirl's, assembled from the five parts of its
tuple the lower layers own: mink's architecture, system and environment, the
architecture's convention and masc's baseline. the cell states every part as
its owners spell it and mirl assembles it, so mach states which target it
means and none of its facts. a cell no owner assembles yet keeps mach's own
facts until one does (#4779)

## rec Owned

```mach
pub rec Owned;
```

what the owners state of one cell for mach's front end, read once when the
target resolves

scalars: the extents of every primitive and the vector registers
nans: the nan each float format's operations return

## rec Assembled

```mach
pub rec Assembled;
```

the target a cell means as mirl assembled it, and the build basis its selection opened,
which lowering lays types out under and compilation adds the selection's names to

tuple: the five parts the target was assembled from, which name its modules
names: the names of masc's catalog the selection adds over the baseline

## fun assemble_for

```mach
pub fun assemble_for(a: *A.Allocator, os_id: u32, arch_id: u32, abi_id: u32, stated_env: str, spelled: opt[str], t: *extension.Table, bits: u64) res[opt[*Assembled], fail.Fail];
```

the target of the cell of `os_id`, `arch_id` and `abi_id` standing on the selection the
target spells, `spelled`, else the cell's own, under the selection `bits` of mach's
extension table `t`, none for a cell no owner assembles yet. given back by release

## fun assemble_resolved

```mach
pub fun assemble_resolved(a: *A.Allocator, t: *resolved.Target) res[opt[*Assembled], fail.Fail];
```

the target a resolved cell means, assembled over the selection it stands on

## fun added

```mach
pub fun added(x: *Assembled) Slice[str];
```

the names the selection adds over the baseline

## fun basis_of

```mach
pub fun basis_of(x: *Assembled) opt[*mirl_target.Basis];
```

the build basis the selection opened, none for a target that states no baseline

## fun release

```mach
pub fun release(a: *A.Allocator, x: *Assembled);
```

## fun owned_for

```mach
pub fun owned_for(a: *A.Allocator, os_id: u32, arch_id: u32, abi_id: u32, env: str, spelled: opt[str], t: *extension.Table, bits: u64) res[opt[Owned], fail.Fail];
```

what the owners state of the cell of `os_id`, `arch_id` and `abi_id` standing on
the selection the target spells, `spelled`, else the cell's own, under the
selection `bits` of mach's extension table `t`, none for a cell no owner
assembles yet

