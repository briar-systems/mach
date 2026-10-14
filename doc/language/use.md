# `use` — imports

`use` brings an external symbol or module into the current scope under a
local name. It is a private import; the imported name is not exposed to
consumers of this module.

## Grammar

```mach fragment
use PATH;                   # binds the leaf component
use ALIAS: PATH;            # binds ALIAS
```

- The alias defaults to the path's leaf component when not given.
- One name per line. No splat (`use foo.*` does not exist) and no
  combined forms (`use foo.{a, b, c}` does not exist).

The resolver binds whatever the path points to. A path ending at a **module**
binds the module — access its members with qualified `module.member`. A path
ending at a **symbol** binds the symbol for bare use. Importing a module does
**not** pull its members into scope unqualified; to use `usize` bare, import
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

```mach fragment
use mylib;                       # bare project id: binds mylib's public module
```

A one-segment path equal to a resolvable project id binds that project's
public module, the `entry` of its library artifact marked `export = true`
(see [modules.md](modules.md#bare-project-id-imports)). A dependency that
exports none has no public module and is imported by full path only; std is
one (`use std;` is refused naming the rule, while `use std.print;` needs no
export).

## Paths through re-exports

A path follows `fwd` re-exports. Once a path reaches a module, each further
segment but the last may name a module that module re-exports with `fwd`, at
any depth of re-export, and the path goes on in the module it names. A path
reaches a member through a module alias the same way it does through the
module itself:

```mach fragment
# src/deep/alpha.mach
pub fun answer() i64 { ret 42; }

# src/lib/lib.mach
fwd beta: example.deep.alpha;

# src/top.mach
fwd delta: example.lib.lib.beta;

# another module of project 'example'
use example.lib.lib.beta.answer;  # binds alpha's 'answer'
use a: example.top.delta.answer;  # through an alias of an alias
```

A segment that names no module re-exported there ends the path, and the use
is refused naming the module and the segment. Only a one-segment project id
reaches a project's public module: a longer path names the project's source
tree, so `use example.alpha;` names `src/alpha.mach` and does not look in the
public module for a `fwd` named `alpha`.

## Design rule

A Mach module imports every dependency it directly names — including any
dependency reached only through a re-export. There is no shortcut for
"my surface uses these transitively; just give me the leaf." The dependency
graph is visible at the top of every file.

## See also

- [fwd.md](fwd.md) — the public-re-export counterpart
- [modules.md](modules.md) — how paths form module identifiers
