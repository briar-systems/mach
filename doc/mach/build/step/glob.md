[mach](../../README.md) / [build](../README.md) / [step](README.md) / **glob**

# mach.build.step.glob

the input patterns a build step names: a wildcard over one directory, or one
recursive `**` segment, expanded beneath the project without following a link

Source: [`src/build/step/glob.mach`](../../../../src/build/step/glob.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Functions | |
| :--- | :--- |
| [`shape_ok`](#shape_ok) | whether a step input pattern has an allowed shape: wildcards only in the last segment, no two adjacent there, and at most one `**` directory segment before it |
| [`expand`](#expand) | the files a step input pattern matches, appended to `out` as paths relative to `project_root`, in the order the directories list them |

---

## Functions

### shape_ok

```mach
pub fun shape_ok(pattern: str) bool
```

whether a step input pattern has an allowed shape: wildcards only in the last segment, no
two adjacent there, and at most one `**` directory segment before it

| Parameter | |
| :--- | :--- |
| `pattern` | the pattern, relative to its root |
| **returns** | true when the shape is allowed |

### expand

```mach
pub fun expand(alloc: *std.allocator.Allocator, project_root: str, pattern: str, out: *std.collections.vector.Vector[str]) err[mach.base.fail.Fail]
```

the files a step input pattern matches, appended to `out` as paths relative to
`project_root`, in the order the directories list them. a pattern with `**` walks every
directory beneath the part before it, skipping dot directories, and one without walks
only the directory its last segment is in

| Parameter | |
| :--- | :--- |
| `alloc` | owns the paths appended to `out` |
| `project_root` | the root the pattern is beneath |
| `pattern` | the pattern, its shape already checked by `shape_ok` |
| `out` | receives the matched paths, the caller's to free |
| **returns** | ok, or the refusal of a directory that cannot be read, a symlink or a special entry |

