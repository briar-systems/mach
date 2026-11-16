[mach](../README.md) / [manifest](README.md) / **project**

# mach.manifest.project

the decoding of the `[project]` table, and the expansion of its `work` directory

<sub>source: [`src/manifest/project.mach`](../../../src/manifest/project.mach) · imported by 4 modules</sub>

<sub>`A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `toml_value` is `toml.value` · `fail` is [`mach.base.fail`](../base/fail.md) · `template` is [`mach.manifest.template`](template.md)</sub>

| Functions | |
| :--- | :--- |
| [`parse_project`](#parse_project) | |
| [`expand_project_work`](#expand_project_work) | expand `m`'s `[project].work` with `v`; a refusal points at the `work` value |

---

## Functions

### parse_project

```mach
pub fun parse_project(alloc: *A.Allocator, itn: *intern.Interner, t: *toml_value.Table, m: *Manifest) err[fail.Fail]
```

### expand_project_work

```mach
pub fun expand_project_work(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest, v: *template.Values) res[str, fail.Fail]
```

expand `m`'s `[project].work` with `v`; a refusal points at the `work` value

