[mach](../README.md) / [compile](README.md) / **union**

# mach.compile.union

the union build's targets, under which every gate is decided, and every artifact's entry

a target whose facts cannot be assembled is skipped with a warning. an artifact the
selected target does not build is foreign: its entry loads so the modules only it
reaches are known and left out of the compiled set.

Source: [`src/compile/union.mach`](../../../src/compile/union.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.compile.project`](project.md), [`mach.manifest.model`](../manifest/model.md).

| Functions | |
| :--- | :--- |
| [`populate_union_tuples`](#populate_union_tuples) | make the project's loader a union build over every target the manifest declares a target whose facts cannot be resolved or assembled is skipped with a TARGET_SKIPPED warning, and the loader takes ownership of the tuple array |
| [`populate_union_entries`](#populate_union_entries) | the artifact entries of this project, split by whether the selected target builds them: an artifact this target does not declare is another target's, and the modules only it reaches are not this build's to compile |

---

## Functions

### populate_union_tuples

```mach
pub fun populate_union_tuples(p: *mach.compile.project.Project, m: *mach.manifest.model.Manifest) err[mach.base.fail.Fail]
```

make the project's loader a union build over every target the manifest declares
a target whose facts cannot be resolved or assembled is skipped with a TARGET_SKIPPED warning,
and the loader takes ownership of the tuple array. a manifest with no targets leaves the loader as it is

| Parameter | |
| :--- | :--- |
| `p` | the project whose loader takes the tuples |
| `m` | the manifest whose targets are read |
| **returns** | ok, or the failure when the tuple array cannot be allocated |

### populate_union_entries

```mach
pub fun populate_union_entries(p: *mach.compile.project.Project, m: *mach.manifest.model.Manifest) err[mach.base.fail.Fail]
```

the artifact entries of this project, split by whether the selected target builds them:
an artifact this target does not declare is another target's, and the modules only it
reaches are not this build's to compile. each entry carries its artifact, the one the
modules its walk first reaches are attributed to; two artifacts sharing an entry keep
the first in manifest order

