[mach](../README.md) / [manifest](README.md) / **dep**

# mach.manifest.dep

the decoding of the `[dep.<name>]` tables

a dependency names exactly one source, `git` or `path`. a git dependency selects by
`ref` or by a `version` range, never both, and no dependency may share the project's id.

Source: [`src/manifest/dep.mach`](../../../src/manifest/dep.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`parse_deps`](#parse_deps) | |

---

## Functions

### parse_deps

```mach
pub fun parse_deps(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

