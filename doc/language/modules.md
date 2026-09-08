# Modules

A Mach project is a tree of modules rooted at the project's `id`. Each
`.mach` file under the project's source directory is a module reachable by
a dotted path from the project root.

## Path structure

The path separator is `.`. A file at `src/foo/bar.mach` in a project with
`id = "myproj"` is reachable as `myproj.foo.bar`.

There is no `this.` self-prefix. Within a project, modules always reference
each other by their full project-rooted path. Every reference is
syntactically uniform regardless of where it appears.

## Bare project-id imports

A one-segment `use`/`fwd` path equal to a dependency's project id binds its
explicit public module. Declare a `static` or `shared` artifact with
`default = true` to select its `entry` as that module:

```toml
[artifact.api]
kind = "static"
default = true
entry = "api.mach"
out = "lib/api{artifact.suffix}"
targets = ["*"]
link = []
need = []
```

For a dependency with `id = "example"`, `use example;` now binds
`example.api`. Nondefault artifacts and executable entries do not select a
dependency's public module. If several library artifacts are explicitly
defaulted, they must share the same entry for a bare import to be unambiguous.

Without an explicit public entry, import full module paths such as
`use example.api;`. Source-only dependencies may omit artifacts entirely.
There is no implicit `lib.mach` entry, even when that file exists, and a sole
library artifact still needs `default = true` to expose a bare import.

A bare import of the current project's own id binds the selected artifact's
entry. Longer paths are unaffected by public entry selection.

## Shadow-module pattern

A file `foo.mach` may co-exist with a directory `foo/`. The file is the
**surface** module — the public face of `foo`. The directory's files are
**split** implementations that the surface loads and re-exports.

```
myproj/
├── foo.mach          # surface
└── foo/
    ├── a.mach        # split: myproj.foo.a
    └── b.mach        # split: myproj.foo.b
```

The surface loads each split with `use myproj.foo.a;` and re-exports its
public symbols with `fwd a.X;`. Consumers `use myproj.foo;` and access
symbols through the surface — they never name the split files directly.

Two common uses:

- **Topical splits** — organize a large module by topic; all splits
  forwarded unconditionally.
- **Multiplatform splits** — one impl per target, selected by `$if` on
  `$mach.build.os` or `$mach.build.arch`.

## See also

- [files.md](files.md) — how `mach.toml` declares the project root
- [use.md](use.md) — loading a module into another module's scope
- [fwd.md](fwd.md) — re-exporting from a surface module
