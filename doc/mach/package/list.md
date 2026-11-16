[mach](../README.md) / [package](README.md) / **list**

# mach.package.list

list: what `mach dep list` shows: each root declaration with its winning selection, its
pin and its state, and under it every requirement of the identity that selection overrides

<sub>source: [`src/package/list.mach`](../../../src/package/list.mach) · imported by 1 module</sub>

<sub>`package_closure` is [`mach.package.closure`](closure.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`lines`](#lines) | the lines `mach dep list` shows, appended to `out` |

---

## Functions

### lines

```mach
pub fun lines(op: *package_closure.Operation, out: *Vector[str]) err[fail.Fail]
```

the lines `mach dep list` shows, appended to `out`

| Parameter | |
| :--- | :--- |
| `op` | the operation |
| `out` | receives one line per root declaration and one per requirement it overrides |
| **returns** | err naming the failure |

