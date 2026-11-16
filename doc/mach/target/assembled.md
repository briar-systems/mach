[mach](../README.md) / [target](README.md) / **assembled**

# mach.target.assembled

the target mirl assembles from a resolved target

mirl assembles a target from the target mink resolved, masc's check of its baseline and
the output kind under build, reading no text, and opens the build the target's added
extensions select over it. the tuple of the target's parts names the modules built for it

Source: [`src/target/assembled.mach`](../../../src/target/assembled.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.target.resolved`](resolved.md).

| Type | | |
| :--- | :--- | :--- |
| [`Assembled`](#assembled) | `rec` | the target mirl assembled for a resolved target and the build basis its added extensions opened, which lowering lays types out under and compilation adds the added names to |

| On `Assembled` | |
| :--- | :--- |
| [`assemble`](#assemble) | the target mirl assembles for `t`, with the build its added extensions select opened |
| [`added`](#added) | the names the target adds over its baseline |
| [`basis_of`](#basis_of) | the build basis the added extensions opened, none for a target that states no baseline |
| [`dnit`](#dnit) | |

| Functions | |
| :--- | :--- |
| [`tuple_of`](#tuple_of) | the tuple a resolved target names its owners' parts by |

---

## Assembled

```mach
pub rec Assembled {
    tuple:  Tuple;
    target: mirl.target.record.Target;
    basis:  mirl.target.basis.Basis;
    based:  bool;
    names:  [resolved.MAX_ADDED]str;
    count:  usize;
}
```

the target mirl assembled for a resolved target and the build basis its added
extensions opened, which lowering lays types out under and compilation adds the
added names to

| Field | Type | |
| :--- | :--- | :--- |
| `tuple` | `Tuple` | the five parts the target was assembled from, which name its modules |
| `names` | `[resolved.MAX_ADDED]str` | the names of masc's catalog the target adds over its baseline |

### assemble

```mach
pub fun assemble(a: *std.allocator.Allocator, t: *mach.target.resolved.Target) res[*Assembled, mach.base.fail.Fail]
```

the target mirl assembles for `t`, with the build its added extensions select
opened. given back by release

### added

```mach
pub fun added(x: *Assembled) Slice[str]
```

the names the target adds over its baseline

### basis_of

```mach
pub fun basis_of(x: *Assembled) opt[*mirl.target.basis.Basis]
```

the build basis the added extensions opened, none for a target that states no baseline

### dnit

```mach
pub fun dnit(a: *std.allocator.Allocator, x: *Assembled)
```

## Functions

### tuple_of

```mach
pub fun tuple_of(t: *mach.target.resolved.Target) Tuple
```

the tuple a resolved target names its owners' parts by

