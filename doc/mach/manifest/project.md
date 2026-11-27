[mach](../README.md) / [manifest](README.md) / **project**

# mach.manifest.project

the decoding of the `[project]` table, and the expansion of its `work` directory

Source: [`src/manifest/project.mach`](../../../src/manifest/project.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.manifest.template`](template.md).

| Functions | |
| :--- | :--- |
| [`parse_project`](#parse_project) | decode the `[project]` table of `t` into `m`: its id, version, src, work and `mach` compiler range |
| [`expand_project_work`](#expand_project_work) | expand `m`'s `[project].work` with `v`; a refusal points at the `work` value |

---

## Functions

### parse_project

```mach
pub fun parse_project(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

decode the `[project]` table of `t` into `m`: its id, version, src, work and
`mach` compiler range

| Parameter | |
| :--- | :--- |
| `alloc` | owns a refusal's text |
| `itn` | interns the values |
| `t` | the manifest's root table, its `[project]` already held to the schema |
| `m` | the manifest the fields are set on |
| **returns** | ok, or the refusal of a path, of an id that is not a project id, of a version that is not a version, or of a `mach` value that is not a valid range |

### expand_project_work

```mach
pub fun expand_project_work(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest, v: *mach.manifest.template.Values) res[str, mach.base.fail.Fail]
```

expand `m`'s `[project].work` with `v`; a refusal points at the `work` value

