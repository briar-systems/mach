[mach](../README.md) / [package](README.md) / **list**

# mach.package.list

list: what `mach dep list` shows: each root declaration with its winning selection, its
pin and its state, and under it every requirement of the identity that selection overrides

Source: [`src/package/list.mach`](../../../src/package/list.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.package.closure`](closure.md).

| Functions | |
| :--- | :--- |
| [`lines`](#lines) | the lines `mach dep list` shows, appended to `out` |

---

## Functions

### lines

```mach
pub fun lines(op: *mach.package.closure.Operation, out: *Vector[str]) err[mach.base.fail.Fail]
```

the lines `mach dep list` shows, appended to `out`

| Parameter | |
| :--- | :--- |
| `op` | the operation |
| `out` | receives one line per root declaration and one per requirement it overrides |
| **returns** | err naming the failure |

