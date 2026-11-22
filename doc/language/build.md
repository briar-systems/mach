# Selection and the build matrix

A build cell is one artifact by one target by one profile.

Every command that builds, checks, tests, runs or documents cells selects them
with three options, one per axis:

- `-a, --artifact <pattern>` selects `[artifact.<name>]` entries;
- `-t, --target <pattern>` selects `[target.<name>]` entries;
- `-p, --profile <pattern>` selects `[profile.<name>]` entries.

`-t` names which targets: in a project, a name or glob over the manifest's target
names, and in an [entry build](#entry-builds), a `<sys>-<arch>`.

A pattern is an exact name, which must be declared, or a glob in which `*`
matches any run of characters and `?` any one character, which must match at
least one entry. Each option repeats, and the axis takes every entry any of its
patterns names, in declaration order. Artifact names are unique table keys, so
an artifact's kind never needs naming. `-t native` names the declared target
matching the host, as an unnamed target axis does. Quote a glob so the shell
leaves it alone: `-a '*'`. When a value names no entry but does name a file or
directory in the working directory, the command line reports it as the shell's
expansion of an unquoted wildcard, with a hint to quote it.

`--all` fills every axis no option names with `*`: `mach build . --all` builds
every artifact on every target it supports in every profile, and
`mach test . --all -p debug` does the same in `debug` only.

An axis no option names, without `--all`, takes the manifest's default:

- the target is the [`native` target](#native-target-resolution), or the one a
  named artifact settles (see [below](#a-named-artifact-can-settle-the-target));
- the profile is the sole declared one, or the one marked `default = true`;
- the artifacts are the **default selection** for each selected target: of the
  artifacts whose `targets` includes it, those marked `default = true` when any is
  marked, and every one of them when none is. `mach build` and `mach check` take the
  whole default selection. `mach test` needs one artifact and takes the
  default selection when it holds one. Several with none marked are refused.
  `mach doc` selects nothing: it documents every source file, on no target.
  `mach run` takes the sole `bin` the target builds, or the one marked
  `default = true` when several do.

No default is chosen by table order: several candidates with none marked are
refused with `selection.ambiguous`, whose fix is to mark one `default = true` or
to select one. `mach test` names the artifacts it found.

- `mach build <path>` and `mach check <path>` build and check every selected cell.
  A selection that spans several profiles plans and runs one profile after another.
- `mach test <path>` builds a test dispatcher for every selected cell as
  `mach build` would build the cell, its closure, its `link` entries, its `need` and
  exported dependency entries, and links the dispatcher in place of its entry. Tests
  then run once per (target, profile): the tests every selected artifact reaches
  there are combined, each qualified name running once. A target that declares a
  [`runner`](manifest.md#targetname) runs through it, and a target whose `sys` and
  `arch` are the host's and that declares none runs directly. Every other (target,
  profile) is built, reported on a `skip` line, and not run, whatever emulation the
  host has. A cell on a finished-module target links no dispatcher: it is built as its
  artifact and its (target, profile) is reported on a `skip` line naming why (see
  [test.md](test.md#which-tests-run)). `--runner <cmd>` replaces the target's runner for one invocation and needs
  the selection to resolve to one cell. A run in which nothing was runnable exits `1`, so a green run always ran
  something, unless `--allow-unrunnable` accepts it.
- `mach run <path>` consumes exactly one cell and refuses a selection that resolves
  to several, naming them, and takes no `--all`. It launches a program a build
  already wrote and never builds one: with nothing at the cell's output path it is
  refused with `artifact.not_built`, which names `mach build`. `mach doc <path>` refuses `-a`
  and `-t`, since one run documents every source file and runs no target's analysis
  (see [documentation.md](documentation.md#what-mach-doc-writes)). `mach run` launches the built program
  through the target's `runner` when it declares one, the expanded runner followed by
  the arguments after `--`, and `--runner <cmd>` replaces it with `<cmd> <program> <args>`
  for one invocation. A target that is not the host's and declares no runner is
  refused, naming the target, and the program's exit status, or the runner's, is
  `mach run`'s.

## Operands

`mach build <path> [inputs...]` and `mach test <path> [inputs...]` take the
project first: a directory whose `mach.toml` is read, or the manifest file itself,
and for `mach build` an entry file (see [Entry builds](#entry-builds)). Each
operand after it is a link input, a file named by its path: it holds a `/`, or it
is named as an object, an archive or a shared library (`x/libfoo.a`, `foo.o`,
`foo.dll`). A relative input is read against the working directory, where it was
typed, never against the project, so `mach build ../app x/libfoo.a` links
`x/libfoo.a` under the working directory, not under `../app`. Any other operand is
refused as `cli.operand`, since a library named without a path is `-l <name>`
(see [ext-fun.md](ext-fun.md#command-line)). `mach check` takes the project
alone.

## Enumerated cells are filtered, named ones are not

A cell whose artifact does not list the cell's target is a cell the manifest never
declared, so a selection that reaches it through a glob skips it. Naming both halves
of that pair exactly is a different act: `-a kernel -t linux-x86_64` is refused by
name, because you asked for a cell that does not exist. `-a kernel -t '*'` globs
the target axis and so filters back to the targets `kernel` declares.

If a selection is well-formed but holds no cell, it fails with
`target.no_artifacts` rather than succeeding with an empty plan. A `-t` that names
exactly a target no artifact lists is refused naming that target and the declared
artifacts, and globs that only pair unsupported cells are refused with the fix to
widen `-a` or `-t`.

## A named artifact can settle the target

`-a <name>` with no `-t` lets the artifact decide, since its `targets` list may
already leave only one answer:

- exactly one declared target: that target is used, and `-t` would only
  repeat what the manifest already said. A hosted target that does not match the
  host is refused instead (see [`native` target resolution](#native-target-resolution))
- several, one of which matches the host: the host target, as before
- several, none matching the host: refused. When the manifest declares a target
  for the host that the artifact does not list, the refusal is
  `selection.unsupported_target`, naming the artifact and that default target, with
  the fix to select one of the artifact's targets with `-t`. When the manifest
  declares no target for the host, it is `native`'s own `selection.no_host_target`

A glob over the artifact axis settles each artifact it names the same way, and
leaves out one that cannot be settled. When the glob selects nothing else, the
refusal of an artifact the manifest declares no host target for stands, and so
does the `selection.no_host_target` of an artifact whose only target is hosted
and not the host's, while an artifact that lists several targets, none of them
the host target, gives `target.no_artifacts`.

An explicit `-t` always wins, including when it names a target the artifact
does not list, and that pair is still refused by name. This only applies to a named
artifact: an artifact axis left to the default keeps the target fixed for the whole
matrix, so a bare `mach build <path>` never widens into a target it was not asked
for.

## `-o` names one output

`-o` names the build's output, and `mach run -o <path>` names the same output, the
program it launches.

The products `--emit` names are the products made. An `--emit` set without `exe`
stops at the objects, with their `ir/` and `asm/` text when it names those, and
links nothing. `-o` names the linked artifact, so it is refused as
`cli.flag_inapplicable` when `--emit` names no `exe`.

`-o` is accepted exactly when the selection resolves to a single (artifact, target,
profile), and refused otherwise, naming the cells it resolved to. Two artifacts
collide on one output path the same way two targets or two profiles do: each would
link over the previous, leaving only the last with no warning. Narrow with `-a`, `-t`
and `-p`.

`-o` names a canonical path inside the project root, as an artifact's `out` does:
relative, `/`-separated, with no `.` or `..` component and no empty one.
`-o ../mach`, `-o ./mach` and `-o /tmp/mach` are refused with `-o must name a
canonical path inside the project root, relative, with no . or .. component`, so
a build never writes outside the tree it was asked to build.

## When one cell fails

Every cell is attempted, and a failure does not abandon the ones after it. Each cell's
diagnostics are reported as they happen, and when the selection has several cells
they follow a line naming the cell, its target, artifact and profile as the units
table spells them (under `-v`, the banner over the cell's table). One tally of
every cell's errors and warnings closes the run:

```
one  broken  debug
error[name.unresolved] unresolved identifier `missing`
  @ src/broken.mach:2:13
2 |     ret x + missing;
  |             ^^^^^^^

two  broken  debug
error[name.unresolved] unresolved identifier `missing`
  @ src/broken.mach:2:13
2 |     ret x + missing;
  |             ^^^^^^^

2 errors / 0 warnings
```

Every cell that succeeded leaves its artifact on disk at its own path.
Nothing is rolled back.

A cell that fails, or is not built because an artifact it needs failed, leaves
nothing at its output path that could be taken for its product: the artifact
there is removed, whether this build or an earlier one wrote it, and so are the
`ir/` and `asm/` trees of its work directory when `--emit ir` or `--emit asm`
asked for them, since a refusal can come after a module's text was written. Its
objects in `obj/` stay, as the object cache keys each one by everything that
made it. The
exit code is `0` when all cells succeeded, and otherwise the code of the worst failure
among them: `2` when any cell failed internally, else `3` when any failed for the
environment, and `1` otherwise.

Artifacts cannot share an output path: a manifest whose expanded `out` templates
collide is rejected before the build starts, and so is a selection spanning
profiles whose `[project].work` has no `{profile.name}` to keep them apart.

## `native` target resolution

`native` resolves the host's `(arch, sys)` against the **declared** targets only,
never a synthesized tuple, and never a target the host cannot run. Exactly one host
match is chosen, and several matching tuples is an ambiguity error naming the candidates.
With no match `native` is an error, however many targets are declared and however
they are marked, and it is raised before any step runs. A declared target that does
not match the host is built only when `-t` names it:

```
error[selection.no_host_target] mach.toml: 'native' matches no declared target for the host linux-x86_64, and the declared targets are linux-aarch64 (linux-aarch64), windows-x86_64 (windows-x86_64), board (freestanding-x86_64). declare a [target.<name>] for the host or select one with -t
   @ ./mach.toml:8:9
 8 | [target.linux-aarch64]
   |         ^^^^^^^^^^^^^
   @ ./mach.toml:13:9
13 | [target.windows-x86_64]
   |         --------------
   @ ./mach.toml:25:9
25 | [target.board]
   |         -----
```

A cross-only project whose targets are hosted (`linux`, `darwin`, `windows`)
selects its target with `-t`.

With no `-t`, [an artifact can settle the target](#a-named-artifact-can-settle-the-target)
when its `targets` list leaves one answer. The same rule holds there. An artifact whose
only target is hosted and does not match the host is refused with
`selection.no_host_target`, naming the artifact, because building it would be the same
fallback. An artifact whose only target no host runs as `native` (a `freestanding`,
`uefi` or `wasi` target, or a finished-module target on `vulkan`) is still pinned to it:
such a target is never `native`, so naming the artifact selects it explicitly, and a
host artifact that `need`s it builds it on any host. This path is taken whenever an
artifact is settled before its target: `mach build` and `mach check` with
`-a`, `mach run` and `mach test` (which also settle on a sole artifact), and
editor analysis. A plain `mach build` or `mach check` resolves `native` first and never
reaches it. A manifest with no `[target.*]` table has the synthesized host
target. `[target.*] default` is a removed key and is refused: declare a target
for each host the project builds on, or pass `-t`.

The same rule applies to `[profile.*]` and to `[artifact.*]` when a command
needs one artifact.


## Entry builds

`mach build <entry>.mach` builds one entry file that no manifest declares. The
command line states the project: `--source <name>=<dir>` binds a source root,
`-k`/`--kind` names what it makes, and `-t <sys>-<arch>` names the one target,
spelled as `mach info targets` prints it. The target is the host's when `-t` is left out.
`-a`, `-p` and `--all` select nothing in an entry build and are refused.

The entry's own project is the one a `--source` binds over the entry's
directory, else a project named by the entry's stem, so `mach build tool.mach`
builds project `tool`. Both names must be project ids, which a `use` path can
spell: a letter or `_`, then letters, digits and `_`. A stem such as `my-tool`
is refused, and the entry is renamed or bound with `--source my_tool=.`.

A directory a `--source` binds may be a project, holding a `mach.toml`. Its `id`
must be the name it is bound to, it may declare no dependencies, and its sources
are its `src`. Its exported `[link]` entries then apply to the entry build as they
apply to a project that depends on it, so `--source std=<std checkout>` links the
libraries std's bindings name on every target. A directory with no `mach.toml` is
a source root alone and brings no link entry. The entry build states no link entry
of its own: the libraries it links are the ones its bound projects export.

`-k` takes mink's product names, the values of an artifact's `kind`: `bin` (the
default), `static`, `shared`, `object` or `module`. `--linkage` says how a `bin` is
linked, by mink's linkage names: `pie`, `dynamic` or `static`. Left out, it is the
target's default, which `mach info targets` marks `*` under `linkage=` for each
environment. It is
refused with any other `-k`, naming the kind, and a linkage the target's loader
does not run is refused in mink's words, listing those it does:

```
mach build main.mach -k static                    # a static library
mach build main.mach -k bin --linkage static      # a fully static executable
mach build main.mach -k shared -t linux-x86_64    # a shared library
```

These flags narrow the target, each value left out taking the default of
mink's catalog row, which `mach info targets` lists first and marks `*`:

- `--env <name>` the environment;
- `--abi <name>` the calling convention;
- `--isa <selection>` the instruction set, which a target marked `needs --isa`
  must name;
- `--ext <name>`, once per extension, an extension the instruction set adds;
- `--code-model <name>` the code model.

mink's catalog parses and resolves the target and refuses a bad part by name,
so `mach build main.mach -t freestanding-riscv32` is refused for naming no
instruction set. A bare `linux-<arch>` is the raw-syscall host with no
environment, so an entry build that links the C library names `--env gnu`. The
previous major's `--arch` and `--sys` are refused naming `-t`, and `--of` is
refused because the system implies the object format.
`--set` writes a manifest key over the entry build's target, artifact or profile
as over a project's, so `mach build main.mach --set simd=require -o strict` builds
the file under `simd = "require"`, and
`mach build main.mach -O --set passes.licm=false -o fast` optimizes it with every
optimization pass but loop invariant motion. A `--set` changes the build's profile,
so it names its output with `-o` or `-w` (see
[Overriding a key for one build](manifest.md#overriding-a-key-for-one-build)).

## See also

- [manifest.md](manifest.md): the `[target]`, `[profile]` and `[artifact]` tables a selection reads
- [dependencies.md](dependencies.md): the dependency closure a build compiles
