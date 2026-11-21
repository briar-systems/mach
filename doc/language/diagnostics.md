# Diagnostics

Every diagnostic mach prints, from the compiler, the build, the manifest, the
dependency tooling, the test runner and the command line, names its kind by a
key, a stable code a tool can match instead of the wording:

```
error[name.unresolved] unresolved identifier `helpr`
  @ src/main.mach:4:9
4 |     ret helpr(x) * 2;
  |         ^^^^^
  help did you mean `helper`?
```

The key sits between the severity and the message: `error[<key>]` or
`warning[<key>]`. The message says what went wrong in words, and its wording
may improve from release to release. The key does not change. A tool reads the
key, the location and the rest as data with
[`--report json`](report-records.md) rather than from this text.

A diagnostic may name other places after its own, each as `@ path:line:column`
with a label that says what the place is, and its excerpt marked with `-`: the
previous definition of a name defined twice, or, for a name that does not
resolve because a `$if` branch this build does not take declares it, that
declaration and the condition that left it out (see
[comptime-control.md](comptime-control.md)). The
[json record](report-records.md) carries them as its `related` sites.

A note says what a command did or found and never fails it. Its human text is
`note <message>`, without the key, which the
[json record](report-records.md#notes) carries.

## Keys

A key is dotted and named by the subject the diagnostic is about, not by the
part of the compiler that raises it: `name.unresolved`, `call.arity`,
`secret.branch`, `vector.scalarize`. Its leading components name a family, so
`secret` covers every `secret.*` key. A family covers whole components only:
`vec` is not a family of `vector.scalarize`.

One key names one rule. Every place that reports the same rule reports it under
the same key, whatever the message's wording and whichever phase finds it: a
`naked` function whose body holds more than inline `asm` is `naked.body`
whether the type checker sees it in the source or code generation sees it in
the emitted instructions. A rule a phase checks on its own is its own key: the
constant-time validation of emitted code refuses what it cannot verify, a
secret branch included, under `secret.unverifiable`, while the type checker
refuses a secret branch in the source under `secret.branch`.

A failure the compiler does not blame on the program, a defect in mach itself,
is `compiler.internal`.

## Messages

A message is a sentence about the subject: what is wrong with it, in words a
reader of the program can act on. The key, the location and the origin say
where the diagnostic came from, so a message never starts with the part of the
compiler that raised it: `error[project.empty] the entry point loaded no
modules`, not `loader: the entry point ...`. A
format, a target or a flag is named where it is the subject, as in `COFF
object too small`.

A type in a message is spelled by its parts, with every alias expanded, since an
alias is never a type of its own (see [def.md](def.md)). The one exception is
the expected type of `type.mismatch` when an annotation wrote it: the type of a
`val` or `var`, a function's return type, a parameter of the function a call
names, or a field of the record or union a literal names. That type is spelled
as the annotation writes it, and the type it names follows once in parentheses
where the two spellings differ. The found type is always spelled by its parts:

```
error[type.mismatch] expected res[Vector[str], fs.FsError] (that is res[Vector[*u8], FsError]), found res[Vector[Listed], FsError]
```

## The registry

The keys are rows of one table in the compiler,
`src/base/diagnostic/kind.mach`. Each row gives the key and its severity, the
only place a diagnostic's severity is decided, and
every site that raises a diagnostic names its row. A diagnostic that names no
row, or a retired one, is refused before it is recorded, so every diagnostic the compiler can
print has a key. The same rows are what a profile's
[`allow`](manifest.md#silencing-warnings) and a declaration's
[`#[expect]`](decorators.md#expectkey--acknowledge-a-warning) select by, so a
key there is exactly the kind the diagnostic carries.

A build prints the warnings of the project being built, and a dependency's only
under `--dep-warnings`, where that dependency's own `allow` and `#[expect]`
govern them. Errors and notes print whoever owns them. See
[Whose warnings a build shows](manifest.md#whose-warnings-a-build-shows).

The table is append-only:

- **A key is never reused.** Once a key has named a kind, it names that kind and
  no other, for good.
- **A key is never removed.** A kind mach no longer raises is retired: its row
  stays, marked retired, so its key stays reserved and can never be given to a
  new kind.
- **A key never changes severity.** A warning key stays a warning, an error
  key stays an error and a note key stays a note.
- **A new kind gets a new key**, added at the end of the table.

These rules are stated in the table's own comment, and no test checks them: a
change to the table is held to them by whoever makes it.

A diagnostic is reported once. The same kind at the same place with the same
message, notes, related places and fixes is one report, however many phases,
modules or generic instances raise it.

## Families

| Family | Covers |
|---|---|
| `syntax` | source that does not parse: an expected token, name, type, expression or declaration is missing, or nesting is too deep |
| `source` | characters the source may not contain |
| `literal`, `float` | a literal that is malformed, unterminated or out of range for its type, and a float literal that is not exact at its type (`float.inexact`) |
| `name`, `use`, `module`, `visibility`, `import`, `fwd` | names that do not resolve, collide, take a built-in type's name (`name.builtin_type`) or are not exported, and `use` and `fwd` paths |
| `decl`, `def`, `binding`, `global`, `const` | declaration forms: bindings, globals, constants and type aliases |
| `type`, `cast`, `operator`, `condition`, `assign`, `address`, `ptr` | type checking: mismatches, conversions, operators, conditions, assignment and addresses |
| `call`, `variadic`, `pack`, `ret` | calls, C-variadic and pack parameters, and returned values |
| `jump` | a `brk` or `cnt` with no enclosing loop (`jump.no_loop`) |
| `field`, `index`, `range`, `shift`, `array`, `uni`, `tag`, `sel`, `guard` | records, arrays, unions, tags, `sel` and guarded places |
| `generic` | type arguments and instantiation |
| `comptime`, `gate`, `each` | compile-time evaluation, intrinsics and descriptors, `$if` gates, `$each`, and the user's own `$error` (`comptime.user_error`) |
| `decorator`, `expect`, `extension`, `embed`, `handle`, `abi_type`, `op`, `naked`, `fin`, `ext` | decorators and the declarations they shape |
| `test`, `testing` | `test` blocks and `#[testing]` declarations |
| `secret` | the secrecy rules in the source (`secret.branch`), and the constant-time validation of emitted code (`secret.unverifiable`) |
| `vector` | vector types and operations, including the scalar fallback (`vector.scalarize`) |
| `asm` | inline assembly: syntax, instructions, operands, extensions, labels and locals |
| `read` | reads the target forbids, such as a load from write-only storage (`read.writeonly_storage`) |
| `target`, `layout`, `stack`, `alloca`, `spirv` | what a target cannot realize: widths, operations, frame sizes, SPIR-V rules |
| `doc` | doc comments (`doc.lint`) |
| `manifest`, `toml`, `allow`, `selection`, `need`, `template`, `version` | `mach.toml`: its keys and values, profile `allow` lists, target, profile and artifact selection, `need` entries, path templates and version ranges |
| `project`, `artifact`, `output`, `source`, `path`, `glob`, `step`, `clean` | the build: finding the project, artifacts and their outputs, build steps, and `mach clean`, and the project `mach init` creates (`project.created`, a note) |
| `dep`, `mach`, `git` | dependencies: declaration, resolution, realization and pins, the compiler range the closure accepts, and the Git operations behind them, and the notes a dependency command writes (`dep.note`, `dep.effect`, `dep.realized`) |
| `test` | the test runner, alongside `test` blocks |
| `cli`, `editor` | command-line flags, commands and operands, and the editor analysis entry points |
| `fs`, `process`, `env`, `memory` | the machine: a file, process or environment operation that failed, or storage the allocator refused (`memory.refused`) |
| `link`, `resource` | the link: its declared inputs that are missing, unreadable or that mink refuses, sections and tables it cannot place, symbol claims that conflict, import libraries, resources that are malformed or over a format limit, and what a finished link reports of its inputs (`link.report`) |
| `readout` | the [timing readout](readout.md) `-v` turns on, when the clock refuses a read (`readout.clock_refused`, a warning) |
| `catalog` | a member of a closed catalog read from input that is malformed or that the target cannot honor |
| `compiler` | `compiler.internal`, a defect in mach |

The warnings are listed, with what raises each, under
[Silencing warnings](manifest.md#silencing-warnings). The full list is the table
itself.

## See also

- [report-records.md](report-records.md): `--report json` and `--report lines`, the same diagnostics as versioned records
- [manifest.md](manifest.md#silencing-warnings): silencing warnings with `allow`
- [decorators.md](decorators.md#expectkey--acknowledge-a-warning): acknowledging a warning with `#[expect]`
