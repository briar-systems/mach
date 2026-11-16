[mach](../README.md) / [manifest](README.md) / **dep**

# mach.manifest.dep

the decoding of the `[dep.<name>]` tables

a dependency names exactly one source, `git` or `path`. a git dependency selects by
`ref` or by a `version` range, never both, and no dependency may share the project's id.

<sub>source: [`src/manifest/dep.mach`](../../../src/manifest/dep.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `toml_value` is `toml.value` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`parse_deps`](#parse_deps) | |

---

## Functions

### parse_deps

```mach
pub fun parse_deps(alloc: *A.Allocator, itn: *intern.Interner, t: *toml_value.Table, m: *Manifest) err[fail.Fail]
```

