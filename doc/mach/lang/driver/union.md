# mach.lang.driver.union

## fun count_target_gated

```mach
pub fun count_target_gated(p: *project.Project) u32;
```

## fun load_all_own_src

```mach
pub fun load_all_own_src(p: *project.Project) err[fail.Fail];
```

## fun populate_union_tuples

```mach
pub fun populate_union_tuples(p: *project.Project, m: *manifest.Manifest) err[fail.Fail];
```

## fun populate_union_entries

```mach
pub fun populate_union_entries(p: *project.Project, m: *manifest.Manifest) err[fail.Fail];
```

the artifact entries of this project, split by whether the selected target builds them:
an artifact this target does not declare is another target's, and the modules only it
reaches are not this build's to compile

## fun register_export_names

```mach
pub fun register_export_names(p: *project.Project) err[fail.Fail];
```

## fun register_import_libraries

```mach
pub fun register_import_libraries(p: *project.Project) err[fail.Fail];
```

## fun register_embed_inputs

```mach
pub fun register_embed_inputs(p: *project.Project) err[fail.Fail];
```

