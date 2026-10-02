# mach.lang.driver.union

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
reaches are not this build's to compile. each entry carries its artifact, the one the
modules its walk first reaches are attributed to; two artifacts sharing an entry keep
the first in manifest order

## fun register_export_names

```mach
pub fun register_export_names(p: *project.Project) err[fail.Fail];
```

## fun register_import_libraries

```mach
pub fun register_import_libraries(p: *project.Project) err[fail.Fail];
```

## fun register_attribute_strings

```mach
pub fun register_attribute_strings(p: *project.Project) err[fail.Fail];
```

the string every attribute argument that takes one evaluates to, recorded for
each later phase to read. it runs once the load walk has bound every constant
a live declaration can name; an argument that is not a constant string is
left out, and type checking reports it where it stands

## fun register_embed_inputs

```mach
pub fun register_embed_inputs(p: *project.Project) err[fail.Fail];
```

## fun module_embeds

```mach
pub fun module_embeds(p: *project.Project, mid: session.ModuleId, paths: *collections_vector.Vector[str]) err[fail.Fail];
```

the resolved paths of the files module m embeds, each owned by the caller in p.s.alloc

