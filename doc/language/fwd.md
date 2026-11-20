# `fwd`: re-exports

`fwd` re-exports a symbol or module from another module under this
module's public surface. It is the public counterpart to `use`.

## Grammar

```mach fragment
fwd PATH;                   # re-export under the path leaf
fwd ALIAS: PATH;            # re-export with rename
```

- `fwd` always publishes. There is no `pub fwd` form, and writing one is
  refused with `fwd.pub`, `` `pub fwd` is not allowed, because `fwd` always
  publishes ``.
- One name per line. No splat and no combined forms. Mirrors `use` grammar
  exactly, so `fwd foo.*;` or `fwd foo.{a, b};` is a syntax error,
  `expected ';' after 'fwd' declaration`.
- A re-exported name must be new in its module. Re-exporting a name twice, by
  leaf or by alias, is refused with `name.duplicate`.
- Only a `pub` symbol can be re-exported. A `fwd` of a private symbol is
  refused with `visibility.not_exported`, `re-exported symbol is not exported
  by its module`.
- A path that names a project and no module, such as `fwd std;`, is refused
  with `fwd.not_symbol`, `` `fwd` path must name a symbol of a module ``.
- A path that names no module or member of one is refused with `use.unresolved`,
  as a `use` is, with a message that reads `fwd path ...`. A path whose root is
  not this project or a declared dependency names that root and says to declare
  it in mach.toml.

## Examples

```mach fragment
use impl: full.core.data;

fwd impl.Point;             # re-exports as 'Point'
fwd Pt: impl.Point;         # re-exports as 'Pt'
```

A path may start at a name this file's `use` lines bind, as `impl` does here,
as well as at a project id.

The surface file in a shadow-module pattern is typically a long list of
`fwd` lines, one per symbol the surface exposes.

## Module re-exports

A `fwd` path that ends at a **module** re-exports the whole module as a
public module alias, mirroring `use`'s module binding:

```mach fragment
fwd demo.alpha;             # re-exports module 'alpha'
fwd deep: demo.deep.beta;   # re-exports module under 'deep'
```

A consumer reaches the alias's members with qualified access, chaining
through any depth of re-export, including a `fwd` of another library's
`fwd`. In a project whose `[project] id` is `example`:

```mach
# file: src/alpha.mach
pub fun answer() i64 {
    ret 42;
}

# file: src/lib.mach
fwd example.alpha; # re-exports module 'alpha'

# file: src/main.mach
use example.lib; # lib.mach contains `fwd example.alpha;`
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{}", lib.alpha.answer()); # resolves through the module re-export
    ret 0;
}
```

As with `use`, a module alias is not a value. Only its members can be
named, and naming the alias alone where a value is expected is refused with
`name.not_value`.

A `use` or `fwd` path may end at a module alias, as `use a: example.lib.alpha;`
or `fwd example.lib.alpha;` does, but it does not continue past one:
`use example.lib.alpha.answer;` is refused with `use.unresolved`. Reach a
member of a re-exported module with qualified access, or by a path to the
module that declares it.

## When to use

- Composing a public surface from split implementation files.
- Aliasing platform-specific impls under a stable name for consumers.

## See also

- [use.md](use.md): the private-import counterpart
- [modules.md](modules.md): shadow-module pattern
