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
| `$bin.name` | the artifact being built | the selected build unit (`[artifact.*]`) |

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

## Which binding a name denotes

An identifier in the comptime channel denotes the declaration it resolves to,
under the ordinary scoping rules — never whichever binding happens to share its
spelling. A block-scoped binding shadows an outer one of the same name here
exactly as it does at runtime:

```mach
val N: i64 = 9;

fun f(k: i64) i64 {
    val N: i64 = k;      # shadows the module constant
    var xs: [N]i64;      # error: array length is not a comptime constant
    ret xs[0];
}
```

The inner `N` is decided at runtime, so it has no comptime value, and the
constant it shadows is not what the expression names. Reading such a name in a
comptime position is refused:

> identifier names a runtime binding, so it has no comptime value

A binding marked `$` — a comptime value parameter, an `$each` loop variable —
*is* a comptime binding, and shadows an outer name of its own spelling in the
same way. Inside the `$each` below the name `N` is the element, not the 9:

```mach
val N:  i64    = 9;
val ES: [2]i64 = [2]i64{1, 2};

fun g() i64 {
    var s: i64 = 0;
    $each N in ES { s = s + N; }   # 3, not 18
    ret s;
}
```

## What's not in the channel

- No reflection-via-`$<Type>.*` subtree. Types are not first-class
  comptime values.
- No decl-attached prefix sugar (`$inline pub fun ...` does not exist) —
  use `#[...]` decorators (see [decorators.md](decorators.md)).
- No comptime function definitions, and no comptime *execution* — the channel
  selects and expands, it never runs a loop or mutates state. `$each` is not a
  counterexample: it splices its body once per element of a fixed comptime
  sequence (a variadic pack, `$fields(T)`, or a constant array `val`), a bounded
  structural expansion resolved at compile time, not an iterated computation.
- No bare `$ident` — see above.

## See also

- [comptime-mach.md](comptime-mach.md) — the `$mach.*` namespace
- [decorators.md](decorators.md) — `#[...]` codegen decorators
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`,
  `$is_record`, `$type_name`, …
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
