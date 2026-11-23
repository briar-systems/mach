[mach](../README.md) / [package](README.md) / **agreement**

# mach.package.agreement

agreement: one package pinned at one commit across a whole dependency closure. every
project of the closure records the commits of the dependencies it declares, so two
projects, one reached through a path dependency, can pin the same package differently
and a build would mix two versions of it with nothing saying so. identity is the
dependency key, which the closure already holds equal to the dependency's project id.
pins are compared among declarations that select the same source and ref: declarations
that select differently are the root's override or a conflict, which the closure walk
already notes or refuses, and a version range is reconciled by resolution, so a
requirer's pin of it is only a seed

Source: [`src/package/agreement.mach`](../../../src/package/agreement.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.package.closure`](closure.md).

| Functions | |
| :--- | :--- |
| [`disagreements`](#disagreements) | one line per pin that differs from the first one seen for its identity, in closure order |
| [`refused`](#refused) | the refusal of a closure that pins a package at more than one commit, ok when `lines` is empty: every line past the first is reported to the operation's report and the first is the failure, so every command that walks a closure refuses a disagreement alike |
| [`lines_dnit`](#lines_dnit) | free every line in a vector and then the vector itself |

---

## Functions

### disagreements

```mach
pub fun disagreements(op: *mach.package.closure.Operation, reqs: *Vector[mach.package.closure.Request], out: *Vector[str]) err[mach.base.fail.Fail]
```

one line per pin that differs from the first one seen for its identity, in closure order

| Parameter | |
| :--- | :--- |
| `op` | the operation |
| `reqs` | the realized closure |
| `out` | receives the lines, owned by `op.a`; free with `lines_dnit` |
| **returns** | err naming the first failure to read a manifest or a pin |

### refused

```mach
pub fun refused(op: *mach.package.closure.Operation, lines: *Vector[str]) err[mach.base.fail.Fail]
```

the refusal of a closure that pins a package at more than one commit, ok when `lines` is
empty: every line past the first is reported to the operation's report and the first is the
failure, so every command that walks a closure refuses a disagreement alike

### lines_dnit

```mach
pub fun lines_dnit(a: *std.allocator.Allocator, lines: *Vector[str])
```

free every line in a vector and then the vector itself

| Parameter | |
| :--- | :--- |
| `a` | the allocator that owns the lines |
| `lines` | the vector to release |

