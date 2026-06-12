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

A one-segment `use`/`fwd` path equal to a resolvable project id — a dependency's
id or the current project's own id — resolves to that project's declared
`[project].module` (the manifest names the surface file). So a library `glfw`
that declares `module = "glfw.mach"` is imported as `use glfw;`, equivalent to
naming the full path to its surface module. A bare import of a project that
declares no module is an error directing you to a full path or to add the key;
longer paths are unaffected. See
[manifest.md](../manifest.md#bare-project-id-imports).

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
  `$mach.target.os` or `$mach.target.arch`.

## See also

- [files.md](files.md) — how `mach.toml` declares the project root
- [use.md](use.md) — loading a module into another module's scope
- [fwd.md](fwd.md) — re-exporting from a surface module
