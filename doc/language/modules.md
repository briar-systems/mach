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

## A use path is a project id and a source path

A `use` or `fwd` path is always `<project id>.<source path>`: its first segment
names a project, the current one or a declared dependency, and the rest names a
file under that project's source directory. `use std.print;` binds
`src/print.mach` of the project `std`. A one-segment path names a project and
no module, so `use std;` is refused with the ordinary unresolved-path error,
`use path 'std' does not name a module`, whatever files the project holds. A
module whose file is named for its project, such as `src/std.mach`, is
imported as `use std.std;`.

A library artifact's `entry` and `export = true` describe the prebuilt library
the project ships (see [manifest.md](manifest.md#artifactname)) and play no part in
name resolution. A consumer imports a library's modules by their source paths
whether or not it links that library prebuilt.

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

```mach
# file: src/foo/a.mach
pub fun one() i64 {
    ret 1;
}

# file: src/foo/b.mach
pub fun two() i64 {
    ret 2;
}

# file: src/foo.mach
use example.foo.a;
use example.foo.b;
fwd a.one;
fwd b.two;

# file: src/main.mach
use example.foo;
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{}", foo.one() + foo.two());
    ret 0;
}
```

Two common uses:

- **Topical splits** — organize a large module by topic; all splits
  forwarded unconditionally.
- **Multiplatform splits** — one impl per target, selected by `$if` on
  `$mach.build.sys` or `$mach.build.arch`.

## See also

- [files.md](files.md) — how `mach.toml` declares the project root
- [use.md](use.md) — loading a module into another module's scope
- [fwd.md](fwd.md) — re-exporting from a surface module
