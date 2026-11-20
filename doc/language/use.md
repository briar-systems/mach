# `use`: imports

`use` brings an external symbol or module into the current scope under a
local name. It is a private import. The imported name is not exposed to
consumers of this module: a consumer that imports it through this module is
refused with `visibility.not_exported`, and one that writes it qualified
through this module, as `module.name`, gets `name.unresolved`.

## Grammar

```mach fragment
use PATH;                   # binds the leaf component
use ALIAS: PATH;            # binds ALIAS
```

- The alias defaults to the path's leaf component when not given.
- One name per line. No splat (`use foo.*` does not exist) and no
  combined forms (`use foo.{a, b, c}` does not exist). Either is a syntax
  error, `expected ';' after 'use' declaration`.
- A bound name must be new in its scope. Binding a name twice, by leaf or by
  alias, is refused with `name.duplicate`.

The resolver binds whatever the path points to. A path ending at a **module**
binds the module. Access its members with qualified `module.member`. A path
ending at a **symbol** binds the symbol for bare use. Importing a module does
**not** pull its members into scope unqualified. To use `usize` bare, import
the symbol, not its module.

## Examples

```mach
use std.types.size; # binds module 'size'; use as `size.usize`
use sz: std.types.size; # binds module under 'sz'; use as `sz.usize`
use std.types.size.usize; # binds symbol 'usize'; use bare as `usize`
use my_usize: std.types.size.usize; # binds symbol under 'my_usize'

val a: size.usize = 1;
val b: sz.usize   = 2;
val c: usize      = 3;
val d: my_usize   = 4;
```

A path is always `<project id>.<source path>`. A one-segment path such as
`use std;` names a project and no module, so it is refused like any path that
names nothing, with `use.unresolved` (see
[modules.md](modules.md#a-use-path-is-a-project-id-and-a-source-path)).

## Unused imports

A symbol import that nothing in the module names is reported with the
warning `import.unused`. A module import is never reported, because it pulls
its module into the build, which is an effect of its own. Only the root
project's modules are checked, so a dependency's imports are never reported.

## Design rule

A Mach module imports every dependency it directly names, including any
dependency reached only through a re-export. There is no shortcut for
"my surface uses these transitively, just give me the leaf." The dependency
graph is visible at the top of every file.

## See also

- [fwd.md](fwd.md): the public re-export counterpart
- [modules.md](modules.md): how paths form module identifiers
