# mach.lang.target.assembled

the target mirl assembles from a resolved target's tuple

a resolved target names the five parts of mirl's tuple as their owners spell
them: mink's architecture, system and environment, the member of the
architecture's convention axis and masc's baseline. mirl assembles the target
from them and opens the build the target's added extensions select, and every
fact of layout and float behaviour the front end reads is read from it

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

## fun multiplies_of

```mach
pub fun multiplies_of(a: *A.Allocator, x: *Assembled) res[ct.CtMulMask, fail.Fail];
```

the scalar multiplies of mach's catalog that mirl runs in time independent of their operands
on the assembled target, under the timing modes its build guarantees, as a hard constant-time
function holds them. only a register machine's selection classifies a multiply, so a target
another family serves holds none. a widening multiply is its low and its high half

## fun nans_of

```mach
pub fun nans_of(x: *Assembled) resolved.Nans;
```

the nan each float format's operations return on the assembled target, unstated
for a format it holds under no row

## fun va_list_of

```mach
pub fun va_list_of(a: *A.Allocator, x: *Assembled) resolved.VaList;
```

the `va_list` of the assembled target's own convention, as mirl's row of the
convention states it and its data layout lays it out

## fun registers_of

```mach
pub fun registers_of(x: *Assembled) opt[resolved.Registers];
```

the vector registers mirl fits a vector to under the build basis, none where it states no
register fit and a vector aligns to its element

## fun pointer_width_of

```mach
pub fun pointer_width_of(a: *A.Allocator, x: *Assembled) res[opt[u32], fail.Fail];
```

the width of an address of the default space, mirl's layout of a pointer under the build
basis. none for a pointer mirl holds logically, or under an addressing model that holds no
address of it

