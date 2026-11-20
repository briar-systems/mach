[mach](../README.md) / [manifest](README.md) / **dep**

# mach.manifest.dep

the decoding of the `[dep.<name>]` tables

a dependency names exactly one source, `git` or `path`. a git dependency selects by
`ref` or by a `version` range, never both, and no dependency may share the project's id.

Source: [`src/manifest/dep.mach`](../../../src/manifest/dep.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md).

| Functions | |
| :--- | :--- |
| [`parse_deps`](#parse_deps) | decode every `[dep.<name>]` table of `t` into `m.deps`, nothing when there is none |

---

## Functions

### parse_deps

```mach
pub fun parse_deps(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

decode every `[dep.<name>]` table of `t` into `m.deps`, nothing when there is none

| Parameter | |
| :--- | :--- |
| `alloc` | owns the dependency array and a refusal's text |
| `itn` | interns the names, sources and selectors |
| `t` | the manifest's root table |
| `m` | the manifest the dependencies are set on, its `id` already decoded |
| **returns** | ok, or the refusal of the first dependency that does not decode |

