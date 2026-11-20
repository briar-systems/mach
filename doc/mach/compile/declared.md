[mach](../README.md) / [compile](README.md) / **declared**

# mach.compile.declared

what the loaded modules declare for the phases after the load: each
declaration's export and import link names, the strings its decorators
evaluate to, and the files it embeds, recorded once the load walk has bound
every constant a live declaration can name

Source: [`src/compile/declared.mach`](../../../src/compile/declared.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.compile.project`](project.md), [`mach.session.module`](../session/module.md).

| Functions | |
| :--- | :--- |
| [`register_export_names`](#register_export_names) | record the link name of every live ext declaration and every `#[symbol]`, `#[weak]` and `#[library]` string of each module |
| [`register_import_libraries`](#register_import_libraries) | bind each `#[library]` attributed ext import of the emitted modules to its library |
| [`decorator_strings_register`](#decorator_strings_register) | the string every decorator argument that takes one evaluates to, recorded for each later phase to read |
| [`register_embed_inputs`](#register_embed_inputs) | refresh the embed input of every file a live `#[embed]` of any loaded module reads |
| [`module_embeds`](#module_embeds) | the resolved paths of the files module mid embeds, each owned by the caller in p.s.alloc |

---

## Functions

### register_export_names

```mach
pub fun register_export_names(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

record the link name of every live ext declaration and every `#[symbol]`, `#[weak]` and `#[library]` string of each module

| Parameter | |
| :--- | :--- |
| `p` | project whose loaded modules are scanned |
| **returns** | ok, or the failure of interning or registering a name, or a nesting refusal already reported |

### register_import_libraries

```mach
pub fun register_import_libraries(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

bind each `#[library]` attributed ext import of the emitted modules to its library

| Parameter | |
| :--- | :--- |
| `p` | project whose emitted modules are walked in emit order |
| **returns** | ok, or a failure naming both declarations when two attribute one import to different libraries, a nesting refusal already reported, an attributed import with no link name, or the failure of interning or registering a name |

### decorator_strings_register

```mach
pub fun decorator_strings_register(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

the string every decorator argument that takes one evaluates to, recorded for
each later phase to read. it runs once the load walk has bound every constant
a live declaration can name; an argument that is not a constant string is
left out, and type checking reports it where it stands

### register_embed_inputs

```mach
pub fun register_embed_inputs(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

refresh the embed input of every file a live `#[embed]` of any loaded module reads

| Parameter | |
| :--- | :--- |
| `p` | project whose loaded modules are scanned |
| **returns** | ok, or the failure of collecting a path or refreshing its input |

### module_embeds

```mach
pub fun module_embeds(p: *mach.compile.project.Project, mid: mach.session.module.ModuleId, paths: *std.collections.vector.Vector[str]) err[mach.base.fail.Fail]
```

the resolved paths of the files module mid embeds, each owned by the caller in p.s.alloc

