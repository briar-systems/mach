[mach](../README.md) / [package](README.md) / **list**

# mach.package.list

list: what `mach dep list` shows: each root declaration with its winning selection, its
pin and its state, and under it every requirement of the identity that selection overrides

Source: [`src/package/list.mach`](../../../src/package/list.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.package.closure`](closure.md).

| Type | | |
| :--- | :--- | :--- |
| [`Row`](#row) | `rec` | one row of `mach dep list`: a root declaration's name and what is listed of it, or under it, with no name, a requirement it overrides |

| On `Row` | |
| :--- | :--- |
| [`lines`](#lines) | the rows `mach dep list` shows, appended to `out`, none when the manifest declares no dependency |

---

## Row

```mach
pub rec Row {
    name: str;
    text: str;
}
```

one row of `mach dep list`: a root declaration's name and what is listed of it, or under it,
with no name, a requirement it overrides

### lines

```mach
pub fun lines(op: *mach.package.closure.Operation, out: *Vector[Row]) err[mach.base.fail.Fail]
```

the rows `mach dep list` shows, appended to `out`, none when the manifest declares no
dependency

| Parameter | |
| :--- | :--- |
| `op` | the operation |
| `out` | receives one row per root declaration and one per requirement it overrides |
| **returns** | err naming the failure |

