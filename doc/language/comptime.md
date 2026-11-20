# Comptime channel

The `$` prefix opens the **comptime channel**, the compiler-owned namespace a
program reads at compile time. It is read-only: `$` *selects and expands*, it
never executes or mutates. Everything that touches comptime in Mach
(conditional compilation, intrinsics, target queries) uses one of the shapes on
this channel.

## The shapes

| Shape | Meaning | Direction |
|---|---|---|
| `$mach.*` / `$project.*` / `$bin.*` | Rooted compiler-owned read | compiler to developer |
| `$<name>(args)` | Comptime function call (intrinsic) | call |
| `$if`, `$or` | Comptime control flow | structural |

> Per-declaration codegen properties (symbol rename, library pin, inline,
> align, section) are written as **`#[...]` decorators**, not `$`-comptime
> shapes. See [decorators.md](decorators.md). A comptime directive takes no
> `=`, and a stray one is a parse error at the directive's terminator.

The parser distinguishes these by structure:

- `$<root>.<path>`, where `<root>` is one of the reserved roots `mach`,
  `project`, `bin`, is a read into a compiler-owned tree. The roots are reserved
  at the top of `$`, so user symbols cannot collide with them.
- `$<name>(args)` is a comptime call. The closed compiler-intrinsic set lives
  here.
- `$if` / `$or` are comptime branches, structurally distinct from runtime
  `if` / `or`.

## Compiler-owned roots

| Root | Reads | Source |
|---|---|---|
| `$mach.*` | resolved build sys/arch/abi/mode tags, pointer width, compiler identity | active build + compiler |
| `$mach.project.{id,version}` | the **owning** project's metadata | `[project]` in the `mach.toml` of the project that owns the module |
| `$mach.project.version.{major,minor,patch}` | its structured version components | that `[project].version` |
| `$mach.source.{file,line,module}` | the module's project-relative file, the line, the module name | the module |
| `$project.{id,version}` | the **root** project's metadata | `[project]` in the root `mach.toml` |
| `$project.version.{major,minor,patch}` | structured version components | the root's `[project].version` |
| `$project.target.{sys,arch,abi}` | the selected target's declared tuple, as strings | the selected `mach.toml` target |
| `$bin.name` | the artifact being built | the selected build unit (`[artifact.*]`) |

Two roots read a project's identity, and they differ in which project:

- `$project.*` is the **root**: the project being built, the same in every module
  of the build, dependencies' modules included. A dependency built as another
  artifact's requirement is the root of that build.
- `$mach.project.*` is the **owner**: the project whose source tree holds the
  module being compiled, read from its own manifest. It follows the module, as
  `{artifact.<id>.out}` does ([manifest.md](manifest.md)).

In a root module the two read the same manifest. In a dependency's module
`$project.version` is the consumer's version and `$mach.project.version` the
dependency's own, so a library that reports its version reads
`$mach.project.version`.

`$project.target.*` carries the manifest's declared **string** spellings
(`"linux"`, `"x86_64"`, `"sysv64"`), distinct from `$mach.build.*`'s numeric tags
used for `$mach.{sys,arch,abi}.*` comparison. The v6 spelling
`$project.target.os` is refused naming `$project.target.sys`, which
`mach migrate` writes. Flat `$project.version` is the whole
version **string** (`"2.0.0"`), and the structured `$project.version.{major,minor,
patch}` folds its integer components. Both are available. `[project]` has
exactly the keys `id`, `version`, `mach`, `src`, and `work`
([manifest.md](manifest.md#project)), and `$project.*` carries exactly `id`
and `version`. A path the root does not carry, `$project.name` and
`$project.description` included, reports `` unknown `$project.*` path ``
(`comptime.path_unknown`) at the path. See
[comptime-mach.md](comptime-mach.md) for the `$mach.*` subtree.

`$bin.name` is the table key of the artifact the module is compiled for. A build
compiles one artifact, and a test build tests one, so every module reads that
artifact's key, a dependency's modules included. An editor
session analyzes the project as the union of every artifact, and there a module
reads the artifact whose walk reached it first: the primary artifact when its
walk reaches the module, otherwise the first artifact in manifest order that
does. A module first reached behind a gate decided later reads its importer's.

```mach fragment
val ver: str = $project.version;                 # "2.0.0", from [project].version
$if ($project.target.sys == "windows") { ... }    # the declared sys string
```

## The bare `$ident` is rejected

A standalone bare `$ident` such as `$mode` or `$foo` is **none** of the shapes
above and is rejected with one teaching diagnostic, `comptime.path_root`, owned
by the comptime evaluator:

> comptime parameters are referenced without `$`. a comptime path starts at
> `$mach`, `$project` or `$bin`

A comptime **parameter** is referenced by its bare name (no `$`), and every
comptime **path** is rooted. The rule applies identically in a `$if` gate and in
value position. Sema and lowering both defer to the evaluator's single verdict
rather than each carrying their own.

## Which binding a name denotes

An identifier in the comptime channel denotes the declaration it resolves to,
under the ordinary scoping rules, never whichever binding happens to share its
spelling. A block-scoped binding shadows an outer one of the same name here
exactly as it does at runtime:

```mach error the array length must be a comptime constant
val N: i64 = 9;

fun f(k: i64) i64 {
    val N:  i64 = k; # shadows the module constant
    var xs: [N]i64; # error: the array length must be a comptime constant
    ret xs[0];
}
```

The inner `N` is decided at runtime, so it has no comptime value, and the
constant it shadows is not what the expression names. Reading such a name in a
comptime position is refused, here as `array.length_not_const` with the reason
in parentheses:

> identifier names a runtime binding, so it has no comptime value

A binding marked `$`, a comptime value parameter or an `$each` loop variable,
*is* a comptime binding, and shadows an outer name of its own spelling in the
same way. It holds a value only where its instance or iteration binds one, so a
comptime parameter sizes a type in each instance of its function's body (see
[fun.md](fun.md)), and a module constant read inside that body, or by another
constant's initializer, still reads the constant, whatever binding shares its
spelling. An `$each` loop variable over a constant array sizes a type in each
iteration the same way. Inside the `$each` below the name `N` is the element,
not the 9:

```mach
use std.print;
use std.runtime;

val N:  i64    = 9;
val ES: [2]i64 = [2]i64{1, 2};

fun g() i64 {
    var s: i64 = 0;
    $each N in ES { s = s + N; } # 3, not 18
    ret s;
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{}", g());
    ret 0;
}
```

## What's not in the channel

- No reflection-via-`$<Type>.*` subtree. Types are not first-class
  comptime values.
- No decl-attached prefix sugar (`$inline pub fun ...` does not exist). Use
  `#[...]` decorators (see [decorators.md](decorators.md)).
- No comptime function definitions, and no comptime *execution*. The channel
  selects and expands, it never runs a loop or mutates state. `$each` is not a
  counterexample: it splices its body once per element of a fixed comptime
  sequence (a variadic pack, `$fields(T)`, or a constant array `val`), a bounded
  structural expansion resolved at compile time, not an iterated computation.
- No compile-time evaluation of user functions, ever: a call in a local `val`
  initializer is a runtime call, and a call in a module-scope `val` initializer
  is refused as `global.init_not_constant` ("function calls are not
  comptime-evaluable"). A lookup table is committed as generator
  output next to its generator and held to it by a `test` (equality with the
  generator when it is deterministic, the property it searched for when it is
  randomised, and no startup fallback that repairs a bad constant before the
  test sees it), or built at startup into a module-private `var`. A
  module-scope `val` lands in read-only data.
- No bare `$ident`. See above.

## See also

- [comptime-mach.md](comptime-mach.md): the `$mach.*` namespace
- [decorators.md](decorators.md): `#[...]` codegen decorators
- [comptime-intrinsics.md](comptime-intrinsics.md): `$size_of`,
  `$is_record`, `$type_name`, ...
- [comptime-control.md](comptime-control.md): `$if` / `$or`
