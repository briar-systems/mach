# Selection and the build matrix

A build cell is one artifact × one target × one profile.

Every command that builds, checks, tests, runs or documents cells selects them
with three options, one per axis:

- `-a, --artifact <pattern>` selects `[artifact.<name>]` entries;
- `-t, --target <pattern>` selects `[target.<name>]` entries;
- `-p, --profile <pattern>` selects `[profile.<name>]` entries.

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
  whole default selection. `mach test` and `mach doc` need one artifact and take the
  default selection when it holds one; several with none marked are refused.
  `mach run` takes the sole `bin` the target builds.

No default is chosen by table order: several candidates with none marked are
refused, naming them.

- `mach build <path>` and `mach check <path>` build and check every selected cell.
  A selection that spans several profiles plans and runs one profile after another.
- `mach test <path>` builds a test dispatcher for every selected cell as
  `mach build` would build the cell, its closure, its `link` entries, its `need` and
  exported dependency entries, and links the dispatcher in place of its entry. Tests
  then run once per (target, profile): the tests every selected artifact reaches
  there are combined, each qualified name running once. Only a target whose `os` and
  `isa` are the host's runs; every other (target, profile) is built, reported on a
  `skip` line, and not run, whatever emulation the host has. `--runner <cmd>` runs a
  foreign target's tests through a command and needs the selection to resolve to one
  cell. A run in which nothing was runnable exits `1`, so a green run always ran
  something.
- `mach run <path>` and `mach doc <path>` consume exactly one cell and refuse a
  selection that resolves to several, naming them. `mach run` takes no `--all`, and
  `mach doc` selects with `-a` and `-t` only.

## Enumerated cells are filtered; named ones are not

A cell whose artifact does not list the cell's target is a cell the manifest never
declared, so a selection that reaches it through a glob skips it. Naming both halves
of that pair exactly is a different act: `-a kernel -t linux-x86_64` is refused by
name, because you asked for a cell that does not exist. `-a kernel -t '*'` globs
the target axis and so filters back to the targets `kernel` declares.

If a selection is well-formed but holds no cell — a `-t` no artifact lists, or
globs that only pair unsupported cells — it fails naming what it selected, rather
than succeeding with an empty plan.

## A named artifact can settle the target

`-a <name>` with no `-t` lets the artifact decide, since its `targets` list may
already leave only one answer:

- exactly one declared target: that target is used, and `-t` would only
  repeat what the manifest already said. A hosted target that does not match the
  host is refused instead (see [`native` target resolution](#native-target-resolution))
- several, one of which matches the host: the host target, as before
- several, none matching the host: refused, naming the targets the artifact does
  declare so the choice is visible without opening `mach.toml`

An explicit `-t` always wins, including when it names a target the artifact
does not list — that pair is still refused by name. This only applies to a named
artifact: an artifact axis left to the default keeps the target fixed for the whole
matrix, so a bare `mach build <path>` never widens into a target it was not asked
for.

## `-o` names one output

`-o` is accepted exactly when the selection resolves to a single (artifact, target,
profile), and refused otherwise, naming the cells it resolved to. Two artifacts
collide on one output path the same way two targets or two profiles do: each would
link over the previous, leaving only the last with no warning. Narrow with `-a`, `-t`
and `-p`.

`-o` names a canonical path inside the project root, as an artifact's `out` does:
relative, `/`-separated, with no `.` or `..` component and no empty one.
`-o ../mach`, `-o ./mach` and `-o /tmp/mach` are refused with `-o must name a
canonical path inside the project root`, so a build never writes outside the
tree it was asked to build.

## When one cell fails

Every cell is attempted; a failure does not abandon the ones after it. Each cell's
diagnostics are reported under its own heading as it happens, and every cell that
succeeded leaves its artifact on disk at its own path — nothing is rolled back. The
exit code is `0` when all cells succeeded, and otherwise the code of the worst failure
among them: `2` when any cell failed internally, else `3` when any failed for the
environment, and `1` otherwise.

Artifacts cannot share an output path: a manifest whose expanded `out` templates
collide is rejected before the build starts, and so is a selection spanning
profiles whose `[project].out` has no `{profile.name}` to keep them apart.

## `native` target resolution

`native` resolves the host's `(isa, os)` against the **declared** targets only —
never a synthesized tuple, and never a target the host cannot run. Exactly one host
match is chosen; several matching tuples is an ambiguity error naming the candidates.
With no match `native` is an error, however many targets are declared and however
they are marked, and it is raised before any step runs. A declared target that does
not match the host is built only when `-t` names it:

```
error[selection.no_host_target]: mach.toml: 'native' matches no declared target: the host is linux-aarch64 and the declared targets are linux-x86_64 (linux-x86_64), windows-x86_64 (windows-x86_64); declare a [target.<name>] for the host or select one with -t
```

A cross-only project whose targets are hosted (`linux`, `darwin`, `windows`)
selects its target with `-t`.

With no `-t`, [an artifact can settle the target](#a-named-artifact-can-settle-the-target)
when its `targets` list leaves one answer. The same rule holds there. An artifact whose
only target is hosted and does not match the host is refused with
`selection.no_host_target`, naming the artifact, because building it would be the same
fallback. An artifact whose only target no host runs as `native` (a `freestanding`
target, including a finished-module target such as `spirv`) is still pinned to it:
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


## See also

- [manifest.md](manifest.md) — the `[target]`, `[profile]` and `[artifact]` tables a selection reads
- [dependencies.md](dependencies.md) — the dependency closure a build compiles
