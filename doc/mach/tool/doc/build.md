[mach](../../README.md) / [tool](../README.md) / [doc](README.md) / **build**

# mach.tool.doc.build

the document model of a loaded project, built from its syntax trees, its name
resolution and its module graph

every `$if` arm is read from the syntax tree, so the entries and their signatures do
not depend on the target analyzed; only what resolution decides (what a `fwd` in a
selected arm forwards, which modules some artifact reaches) does. nothing here reads
source text except the spans the syntax tree gives

<sub>source: [`src/tool/doc/build.mach`](../../../../src/tool/doc/build.mach) · imported by 1 module</sub>

<sub>`A` is `std.allocator` · `project` is [`mach.compile.project`](../../compile/project.md) · `manifest_model` is [`mach.manifest.model`](../../manifest/model.md) · `model` is [`mach.tool.doc.model`](model.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`build`](#build) | the model of `p`, every module of its own that its artifacts reach |

---

## Functions

### build

```mach
pub fun build(a: *A.Allocator, p: *project.Project, m: *manifest_model.Manifest, out: *model.Project) err[fail.Fail]
```

the model of `p`, every module of its own that its artifacts reach

| Parameter | |
| :--- | :--- |
| `a` | the arena the model lives in |
| `p` | the analyzed project, through name resolution |
| `m` | its manifest |
| `out` | the model |
| **returns** | ok, or the allocator's refusal |

