# Diagnostics

Every diagnostic mach prints, from the compiler, the build, the manifest, the
dependency tooling, the test runner and the command line, names its kind by a
key, a stable code a tool can match instead of the wording:

```
error[name.unresolved]: unresolved identifier `helpr`
 --> src/main.mach:4:9
  |
4 |     ret helpr(x) * 2;
  |         ^^^^^
  |
  = help: did you mean `helper`?
```

The key sits between the severity and the message: `error[<key>]` or
`warning[<key>]`. The message says what went wrong in words, and its wording
may improve from release to release. The key does not change.

## Keys

A key is dotted and named by the subject the diagnostic is about, not by the
part of the compiler that raises it: `name.unresolved`, `call.arity`,
`secret.branch`, `vector.scalarize`. Its leading components name a family, so
`secret` covers every `secret.*` key. A family covers whole components only:
`vec` is not a family of `vector.scalarize`.

One key names one rule. Every place that reports the same rule reports it under
the same key, whatever the message's wording and whichever phase finds it: a
secret branch is `secret.branch` whether the type checker sees it in the source
or the constant-time validator sees it in the emitted instructions.

A failure the compiler does not blame on the program, a defect in mach itself,
is `compiler.internal`.

## The registry

The keys are rows of one table in the compiler,
`src/lang/diagnostic/kind.mach`. Each row gives the key and its severity, and
every site that raises a diagnostic names its row. A diagnostic that names no
row is refused before it is recorded, so every diagnostic the compiler can
print has a key. The same rows are what a profile's
[`allow`](manifest.md#silencing-warnings) and a declaration's
[`#[expect]`](decorators.md#expectkey--acknowledge-a-warning) select by, so a
key there is exactly the kind the diagnostic carries.

The table is append-only:

- **A key is never reused.** Once a key has named a kind, it names that kind and
  no other, for good.
- **A key is never removed.** A kind mach no longer raises is retired: its row
  stays, marked retired, so its key stays reserved and can never be given to a
  new kind.
- **A key never changes severity.** A warning key stays a warning and an error
  key stays an error.
- **A new kind gets a new key**, added at the end of the table.

A test in the compiler holds the table to these rules: it fails when a key
disappears, moves or changes severity.

## Families

| Family | Covers |
|---|---|
| `syntax` | source that does not parse: an expected token, name, type, expression or declaration is missing, or nesting is too deep |
| `source` | characters the source may not contain |
| `literal` | a literal that is malformed, unterminated or out of range for its type |
| `name`, `use`, `module`, `visibility`, `import`, `fwd` | names that do not resolve, collide or are not exported; `use` and `fwd` paths |
| `decl`, `binding`, `global`, `const` | declaration forms: bindings, globals and constants |
| `type`, `cast`, `operator`, `condition`, `assign`, `address`, `ptr` | type checking: mismatches, conversions, operators, conditions, assignment and addresses |
| `call`, `variadic`, `pack`, `ret` | calls, C-variadic and pack parameters, and returned values |
| `field`, `index`, `range`, `shift`, `array`, `uni`, `tag`, `sel`, `guard` | records, arrays, unions, tags, `sel` and guarded places |
| `generic` | type arguments and instantiation |
| `comptime`, `gate`, `each` | compile-time evaluation, intrinsics and descriptors, `$if` gates, `$each`, and the user's own `$error` (`comptime.user_error`) |
| `decorator`, `expect`, `extension`, `embed`, `handle`, `abi_type`, `op`, `naked`, `fin`, `ext` | decorators and the declarations they shape |
| `test`, `testing` | `test` blocks and `#[testing]` declarations |
| `secret` | the secrecy rules, in the source and in the constant-time validation of emitted code (`secret.not_oblivious`) |
| `vector` | vector types and operations, including the scalar fallback (`vector.scalarize`) |
| `asm` | inline assembly: syntax, instructions, operands, extensions, labels and locals |
| `target`, `layout`, `stack`, `alloca`, `spirv` | what a target cannot realize: widths, operations, frame sizes, SPIR-V rules |
| `import.unused`, `decl.deprecated`, `doc.lint`, `float.inexact`, `fwd.instances`, `debug.dropped`, `target.skipped`, `target.native_fallback`, `expect.unfulfilled` | the warnings, listed with what raises them under [Silencing warnings](manifest.md#silencing-warnings) |
| `manifest`, `toml`, `allow`, `selection`, `need`, `template`, `version` | `mach.toml`: its keys and values, profile `allow` lists, target, profile and artifact selection, `need` entries, path templates and version ranges |
| `project`, `artifact`, `output`, `source`, `path`, `glob`, `step`, `clean` | the build: finding the project, artifacts and their outputs, build steps, and `mach clean` |
| `dep`, `mach`, `git` | dependencies: declaration, resolution, realization and pins, the compiler range the closure accepts, and the Git operations behind them |
| `test` | the test runner, alongside `test` blocks |
| `cli`, `editor` | command-line flags, commands and operands, and the editor analysis entry points |
| `fs`, `process`, `env` | the machine: a file, process or environment operation that failed |
| `link`, `object`, `resource` | the link: its declared inputs, undefined and duplicate symbols, relocations that overflow or are unsupported, images over a format limit, and input objects, archives, libraries or resources that are malformed, unsupported or of another format |
| `catalog` | a member of a closed catalog read from input that is malformed or that the target cannot honor |
| `compiler` | `compiler.internal`, a defect in mach |

The full list is the table itself.

## See also

- [manifest.md](manifest.md#silencing-warnings) — silencing warnings with `allow`
- [decorators.md](decorators.md#expectkey--acknowledge-a-warning) — acknowledging a warning with `#[expect]`
