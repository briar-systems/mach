# mach.lang.target.assembled

the target mirl assembles from a resolved target

mirl assembles a target from the target mink resolved, masc's check of its baseline and
the product under build, reading no text, and opens the build the target's added
extensions select over it. the tuple of the target's parts names the modules built for it

## fun tuple_of

```mach
pub fun tuple_of(t: *resolved.Target) Tuple;
```

the tuple a resolved target names its owners' parts by

## rec Assembled

```mach
pub rec Assembled;
```

the target mirl assembled for a resolved target and the build basis its added
extensions opened, which lowering lays types out under and compilation adds the
added names to

tuple: the five parts the target was assembled from, which name its modules
names: the names of masc's catalog the target adds over its baseline

## fun assemble

```mach
pub fun assemble(a: *A.Allocator, t: *resolved.Target) res[*Assembled, fail.Fail];
```

the target mirl assembles for `t`, with the build its added extensions select
opened. given back by release

## fun added

```mach
pub fun added(x: *Assembled) Slice[str];
```

the names the target adds over its baseline

## fun basis_of

```mach
pub fun basis_of(x: *Assembled) opt[*mirl_target.Basis];
```

the build basis the added extensions opened, none for a target that states no baseline

## fun release

```mach
pub fun release(a: *A.Allocator, x: *Assembled);
```

