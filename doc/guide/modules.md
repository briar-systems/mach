# Modules

Every `.mach` file is a module. `use` imports a name from another module, `pub` decides what a module shows, and `fwd` re-exports.

## Module paths

```text
mach.toml             id = "app"
src/
  bin/main.mach       app.bin.main
  geometry.mach       app.geometry
  geometry/
    flat.mach         app.geometry.flat
    solid.mach        app.geometry.solid
```

A module's path is the project id, then the file's path under the source directory with `.` between the parts. The first segment names the project itself or a [dependency](dependencies.md). There is no relative form, so a module is spelled the same way everywhere.

## Imports

```mach
use std.print; # a module: print.println("hi")
use out: std.print; # the same module, named out
use std.types.size.usize; # a symbol: usize
use word: std.types.size.usize; # the same symbol, named word
```

- A path that ends at a module binds the module, and its members are reached through it. A path that ends at a symbol binds the symbol. Importing a module never brings its members in bare.
- One name per line. There are no globs and no brace lists.
- An import is private to its file. Each file imports every module it names.
- `use std;` is refused, because a path needs a module after the project id.

## Visibility

A declaration is private to its file unless it is marked `pub`.

```mach
# src/geometry/flat.mach
pub rec Rect {
    w: i64;
    h: i64;
}

pub fun area(r: Rect) i64 {
    ret side(r.w) * side(r.h);
}

fun side(n: i64) i64 {
    if (n < 0) { ret 0; }
    ret n;
}
```

`pub` goes before `fun`, `rec`, `uni`, `tag`, `def`, `val`, `var` and `ext`. Naming `flat.side` from another module is refused:

```text
error[visibility.not_exported] no symbol `side` exported by `app.geometry.flat`
```

## Re-exports

`fwd` publishes a name from another module as a member of this one. It takes the same paths and alias form as `use`, and it is always public.

```mach fragment
# src/geometry.mach
use app.geometry.flat;

fwd flat.Rect;
fwd flat.area;
fwd box: app.geometry.solid.volume;
```

- A path starts at a project id or at a name a `use` line bound, as `flat` is here.
- Only a `pub` symbol can be forwarded.
- A path that ends at a module forwards the whole module. After `fwd app.geometry.solid;` a consumer writes `geometry.solid.volume`.

## Surface modules

A file `geometry.mach` beside a directory `geometry/` is the surface of that directory. The files inside are splits, and the surface forwards what the rest of the program should see. Other modules import the surface alone:

```mach fragment
# src/bin/main.mach
use app.geometry;
use app.geometry.Rect;
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val r: Rect = Rect{w: 3, h: 4};
    print.printlnf("{} {}", geometry.area(r), geometry.box(3, 4, 5));
    ret 0;
}
```

This prints `12 60`. The layout is a convention, and a split is still an ordinary module that any file can import by its own path. A surface can also choose one split per target by putting its `use` lines inside [`$if`](comptime.md).

## Shared library exports

`pub` is visibility between Mach modules and exports nothing from a shared library. A `shared` artifact exports only the declarations marked `#[export]`, which must also be `pub`. See [decorators](decorators.md).

The full rules are in the reference for [modules](../language/modules.md), [`use`](../language/use.md), [`fwd`](../language/fwd.md) and [visibility](../language/visibility.md).
