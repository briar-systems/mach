[mach](../README.md) / [compile](README.md) / **union**

# mach.compile.union

the union build's targets, under which every gate is decided, and every artifact's entry

a target whose facts cannot be assembled is skipped with a warning. an artifact the
selected target does not build is foreign: its entry loads so the modules only it
reaches are known and left out of the compiled set.

<sub>source: [`src/compile/union.mach`](../../../src/compile/union.mach) · imported by 1 module</sub>

<sub>`project` is [`mach.compile.project`](project.md) · `manifest_model` is [`mach.manifest.model`](../manifest/model.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`populate_union_tuples`](#populate_union_tuples) | |
| [`populate_union_entries`](#populate_union_entries) | the artifact entries of this project, split by whether the selected target builds them: an artifact this target does not declare is another target's, and the modules only it reaches are not this build's to compile |

---

## Functions

### populate_union_tuples

```mach
pub fun populate_union_tuples(p: *project.Project, m: *manifest_model.Manifest) err[fail.Fail]
```

### populate_union_entries

```mach
pub fun populate_union_entries(p: *project.Project, m: *manifest_model.Manifest) err[fail.Fail]
```

the artifact entries of this project, split by whether the selected target builds them:
an artifact this target does not declare is another target's, and the modules only it
reaches are not this build's to compile. each entry carries its artifact, the one the
modules its walk first reaches are attributed to; two artifacts sharing an entry keep
the first in manifest order

