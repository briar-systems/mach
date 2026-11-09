# mach.lang.target.assembled

the target mirl assembles from a resolved target

mirl assembles a target from the five parts of its tuple, spelled as their owners
spell them: mink's architecture, system and environment, the calling convention and
masc's baseline, each read from the target mink resolved. mirl takes the code model
mink resolved and the product under build beside them, and opens the build the
target's added extensions select

## fun tuple_of

```mach
pub fun tuple_of(t: *resolved.Target) Tuple;
```

the tuple a resolved target names its owners' parts by

## fun made

```mach
pub fun made(a: *A.Allocator, tuple: Tuple, code: opt[*CodeModel], kind: opt[*Product]) res[mirl_target.Target, fail.Fail];
```

the target mirl assembles from `tuple` under the code model `code`, mink's row of it, none
for the host's default, for an artifact of the product `kind`, none where no artifact is
under build. refused when it assembles none: a baseline that does not hold what the
convention requires, a code model the target does not offer, or parts the owners rule out.
given back by mirl_target.release

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

