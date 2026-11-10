# mach.lang.driver.union

## fun populate_union_tuples

```mach
pub fun populate_union_tuples(p: *project.Project, m: *manifest_model.Manifest) err[fail.Fail];
```

## fun populate_union_entries

```mach
pub fun populate_union_entries(p: *project.Project, m: *manifest_model.Manifest) err[fail.Fail];
```

the artifact entries of this project, split by whether the selected target builds them:
an artifact this target does not declare is another target's, and the modules only it
reaches are not this build's to compile. each entry carries its artifact, the one the
modules its walk first reaches are attributed to; two artifacts sharing an entry keep
the first in manifest order

