# mach.lang.driver.declared

what the loaded modules declare for the phases after the load: each
declaration's export and import link names, the strings its decorators
evaluate to, and the files it embeds, recorded once the load walk has bound
every constant a live declaration can name

## fun register_export_names

```mach
pub fun register_export_names(p: *project.Project) err[fail.Fail];
```

## fun register_import_libraries

```mach
pub fun register_import_libraries(p: *project.Project) err[fail.Fail];
```

## fun decorator_strings_register

```mach
pub fun decorator_strings_register(p: *project.Project) err[fail.Fail];
```

the string every decorator argument that takes one evaluates to, recorded for
each later phase to read. it runs once the load walk has bound every constant
a live declaration can name; an argument that is not a constant string is
left out, and type checking reports it where it stands

## fun register_embed_inputs

```mach
pub fun register_embed_inputs(p: *project.Project) err[fail.Fail];
```

## fun module_embeds

```mach
pub fun module_embeds(p: *project.Project, mid: lang_module.ModuleId, paths: *collections_vector.Vector[str]) err[fail.Fail];
```

the resolved paths of the files module m embeds, each owned by the caller in p.s.alloc

