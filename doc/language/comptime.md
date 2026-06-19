# Comptime channel

The `$` prefix opens the **comptime channel** — the compiler-owned namespace a
program reads at compile time. It is read-only: `$` *selects and expands*, it
never executes or mutates. Everything that touches comptime in Mach —
conditional compilation, intrinsics, target queries — uses one of the shapes on
this channel.

## The shapes

| Shape | Meaning | Direction |
|---|---|---|
| `$mach.*` / `$project.*` / `$bin.*` | Rooted compiler-owned read | compiler → developer |
| `$sym(args)` | Comptime function call (intrinsic) | call |
| `$if`, `$or` | Comptime control flow | structural |

> Per-declaration codegen attributes (symbol rename, library pin, inline,
> align, section) are written as **`#[...]` decorators**, not `$`-comptime
> shapes — see [decorators.md](decorators.md). The legacy `$sym.attr = value`
> attribute setters were removed in v2.0.0.

The parser distinguishes these by structure:

- `$<root>.<path>`, where `<root>` is one of the reserved roots `mach`,
  `project`, `bin` — a read into a compiler-owned tree. The roots are reserved
  at the top of `$`; user symbols cannot collide with them.
- `$ident(args)` — comptime call; the closed compiler-intrinsic set lives
  here.
- `$if` / `$or` — comptime branches, structurally distinct from runtime
  `if` / `or`.

## Compiler-owned roots

| Root | Reads | Source |
|---|---|---|
| `$mach.*` | resolved build os/arch/abi/mode tags, pointer width, compiler identity | active build + compiler |
| `$project.{id,version,name,description}` | project metadata | `[project]` in `mach.toml` |
| `$project.version.{major,minor,patch}` | structured version components | `[project].version` |
| `$project.target.{os,arch,abi}` | the selected target's declared tuple, as strings | the selected `mach.toml` target |
| `$bin.name` | the artifact being built | the selected build unit (`[bin.*]` / `[lib.*]`) |

`$project.target.*` carries the manifest's declared **string** spellings
(`"linux"`, `"x86_64"`, `"sysv64"`), distinct from `$mach.build.*`'s numeric tags
used for `$mach.{os,arch,abi}.*` comparison. Flat `$project.version` is the whole
version **string** (`"2.0.0"`); the structured `$project.version.{major,minor,
patch}` folds its integer components — both are available. A root field the
manifest does not declare (`$project.description` on a v1 manifest, `$bin.name`
on a v1 manifest with no artifact stanza) reports the read as unavailable rather
than folding to an empty string. See [comptime-mach.md](comptime-mach.md) for the
`$mach.*` subtree.

```mach
val ver: str = $project.version;                 # "2.0.0", from [project].version
$if ($project.target.os == "windows") { ... }    # the declared os string
```

## The bare `$ident` is rejected

A standalone bare `$ident` — `$mode`, `$foo` — is **none** of the shapes above
and is rejected with one teaching diagnostic, owned by the comptime evaluator:

> comptime parameters are referenced without `$`; comptime paths are rooted:
> `` $mach ``, `` $project ``, `` $bin ``

A comptime **parameter** is referenced by its bare name (no `$`); every comptime
**path** is rooted. The rule applies identically in a `$if` gate and in value
position — sema and lowering both defer to the evaluator's single verdict rather
than each carrying their own.

## What's not in the channel

- No reflection-via-`$<Type>.*` subtree. Types are not first-class
  comptime values.
- No decl-attached prefix sugar (`$inline pub fun ...` does not exist) —
  use `#[...]` decorators (see [decorators.md](decorators.md)).
- No comptime function definitions, no comptime loops.
- No bare `$ident` — see above.

## See also

- [comptime-mach.md](comptime-mach.md) — the `$mach.*` namespace
- [decorators.md](decorators.md) — `#[...]` codegen decorators
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`,
  `$assert`, …
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
