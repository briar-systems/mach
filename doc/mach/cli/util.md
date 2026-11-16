[mach](../README.md) / [cli](README.md) / **util**

# mach.cli.util

a command's project operand resolved to its root and manifest, and a path's parents made

<sub>source: [`src/cli/util.mach`](../../../src/cli/util.mach) · imported by 8 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ProjectLocation`](#projectlocation) | `rec` | a resolved project operand; every string is owned and freed by dnit_project_location |

| On `ProjectLocation` | |
| :--- | :--- |
| [`resolve_project_location`](#resolve_project_location) | turn a project operand into a root directory and a manifest path a directory operand must contain mach.toml; a file operand must itself be named mach.toml, and its parent becomes the root |

| Functions | |
| :--- | :--- |
| [`ensure_parents`](#ensure_parents) | create every missing ancestor of a path with mode 0755; the path itself is not created |

---

## ProjectLocation

```mach
pub rec ProjectLocation {
    root:     Path;
    manifest: Path;
}
```

a resolved project operand; every string is owned and freed by dnit_project_location

| Field | Type | |
| :--- | :--- | :--- |
| `root` | `Path` | the project directory |
| `manifest` | `Path` | the path of its mach.toml |

### resolve_project_location

```mach
pub fun resolve_project_location(a: *A.Allocator, arg: str) res[ProjectLocation, fail.Fail]
```

turn a project operand into a root directory and a manifest path
a directory operand must contain mach.toml; a file operand must itself be named mach.toml,
and its parent becomes the root

| Parameter | |
| :--- | :--- |
| `a` | allocator for the returned strings |
| `arg` | the operand; nil is an error asking for a path |
| **returns** | the location, freed with dnit_project_location, or a message naming what was wrong |

## Functions

### ensure_parents

```mach
pub fun ensure_parents(a: *A.Allocator, p: str) err[fail.Fail]
```

create every missing ancestor of a path with mode 0755; the path itself is not created

| Parameter | |
| :--- | :--- |
| `a` | allocator for the parent path |
| `p` | the path whose parents are wanted |
| **returns** | none on success, or the parent computation or creation error |

