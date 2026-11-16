[mach](../../README.md) / [build](../README.md) / [step](README.md) / **glob**

# mach.build.step.glob

the input patterns a build step names: a wildcard over one directory, or one
recursive `**` segment, expanded beneath the project without following a link

Source: [`src/build/step/glob.mach`](../../../../src/build/step/glob.mach)

Uses types from [`mach.base.fail`](../../base/fail.md).

| Functions | |
| :--- | :--- |
| [`shape_ok`](#shape_ok), [`expand`](#expand) | |

---

## Functions

### shape_ok

```mach
pub fun shape_ok(pattern: str) bool
```

### expand

```mach
pub fun expand(alloc: *std.allocator.Allocator, project_root: str, pattern: str, out: *std.collections.vector.Vector[str]) err[mach.base.fail.Fail]
```

