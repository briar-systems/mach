[mach](../../README.md) / [build](../README.md) / [step](README.md) / **glob**

# mach.build.step.glob

the input patterns a build step names: a wildcard over one directory, or one
recursive `**` segment, expanded beneath the project without following a link

<sub>source: [`src/build/step/glob.mach`](../../../../src/build/step/glob.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `collections_vector` is `std.collections.vector` · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`shape_ok`](#shape_ok) · [`expand`](#expand) | |

---

## Functions

### shape_ok

```mach
pub fun shape_ok(pattern: str) bool
```

### expand

```mach
pub fun expand(alloc: *A.Allocator, project_root: str, pattern: str, out: *collections_vector.Vector[str]) err[fail.Fail]
```

