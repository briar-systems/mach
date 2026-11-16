[mach](../README.md) / [cli](README.md) / **util**

# mach.cli.util

a command's project operand resolved to its root and manifest

Source: [`src/cli/util.mach`](../../../src/cli/util.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`ProjectLocation`](#projectlocation) | `rec` | a resolved project operand; both strings come from the allocator resolve_project_location was given, which every command passes as its arena (`args.Call.a`), so they live as long as the command |

| On `ProjectLocation` | |
| :--- | :--- |
| [`resolve_project_location`](#resolve_project_location) | turn a project operand into a root directory and a manifest path a directory operand must contain mach.toml; a file operand must itself be named mach.toml, and its parent becomes the root |

---

## ProjectLocation

```mach
pub rec ProjectLocation {
    root:     Path;
    manifest: Path;
}
```

a resolved project operand; both strings come from the allocator resolve_project_location was
given, which every command passes as its arena (`args.Call.a`), so they live as long as the command

| Field | Type | |
| :--- | :--- | :--- |
| `root` | `Path` | the project directory |
| `manifest` | `Path` | the path of its mach.toml |

### resolve_project_location

```mach
pub fun resolve_project_location(a: *std.allocator.Allocator, arg: str) res[ProjectLocation, mach.base.fail.Fail]
```

turn a project operand into a root directory and a manifest path
a directory operand must contain mach.toml; a file operand must itself be named mach.toml,
and its parent becomes the root

| Parameter | |
| :--- | :--- |
| `a` | allocator for the returned strings, which nothing frees one by one |
| `arg` | the operand; nil is an error asking for a path |
| **returns** | the location, or a message naming what was wrong |

