[mach](../README.md) / [package](README.md) / **list**

# mach.package.list

list: what `mach dep list` shows: each root declaration with its winning selection, its
pin, its state and its source, and under it every requirement of the identity that selection
overrides. why a pin or a state cannot be read goes to the report as a note, so the rows
stay one table

Source: [`src/package/list.mach`](../../../src/package/list.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.package.closure`](closure.md).

| Type | | |
| :--- | :--- | :--- |
| [`Row`](#row) | `rec` | one row of `mach dep list`: a root declaration's cells, or under it, with no name, a requirement it overrides |

| On `Row` | |
| :--- | :--- |
| [`lines`](#lines) | the rows `mach dep list` shows, appended to `out`, none when the manifest declares no dependency |

---

## Row

```mach
pub rec Row {
    name:    str;
    version: str;
    pin:     str;
    state:   str;
    source:  str;
    text:    str;
}
```

one row of `mach dep list`: a root declaration's cells, or under it, with no name, a
requirement it overrides

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | the declared id, nil for an override |
| `version` | `str` | the range or the ref it selects by, `-` for a path |
| `pin` | `str` | the pinned commit, short, `-` for none, or `unreadable` |
| `state` | `str` | realized, missing, out of range, release unknown, unreadable or invalid |
| `source` | `str` | its git url or its path |
| `text` | `str` | an override's line, nil for a declaration |

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

