# Report records

Every command takes `--report <human|json|lines>`, or `--report=json`, and
`mach` takes it before the command word too, as every long option that takes a value accepts it
after `=` in its own word.
`human`, the default, is the text shown in [diagnostics.md](diagnostics.md) and
[readout.md](readout.md).
`json` writes everything the command reports as records, JSON objects one per line of
stderr (NDJSON), for editors, CI and other tools, and `lines` writes the same
records in a tab-separated encoding for line tools ([below](#the-lines-encoding)):
each diagnostic, a failure
raised outside the compiler's diagnostics (a manifest, dependency, build-step,
link or command-line refusal) included, each [note](#notes) a command writes, each test result under `mach test`, the build
readout under `-v` and `-vv`, the host and the targets `mach info` shows, and a closing summary. The records are the contract: the human text may change from
release to release, and a record changes only under the
[stability rule](#stability). The flag changes no exit code.

```
mach check . --report json
```

A bare `--report` or any other value is refused with
`error[cli.flag_value]`, in human text since no format has been chosen. `-v`
and `-vv` write the [readout](readout.md) as [readout records](#readout-records).

## The stream

- Records go to stderr, one per line, in the order the human rendering shows
  the same reports. The results the human rendering writes to stdout are not
  written. Stdout keeps the listings a command writes, such as `mach test
  --list`, `mach dep list` or the listing `mach check --declassified` writes,
  which stay plain lines. The page of `mach info` and the table of `mach info
  targets` are not kept: [info records](#info-records) are written in their
  place, and stdout stays empty.
- Every line mach writes to stderr is a record, a JSON object on a line of its
  own, and the last one is the [summary](#the-summary-record). Output a build
  step or a test writes itself is not mach's and is not a record.
- The text is ASCII. A non-ASCII character in a message, a label or a path is
  written as a `\u` escape, and a byte that is not valid UTF-8 as `\ufffd`,
  the escape of U+FFFD, so every line is valid UTF-8 and valid JSON.
- Under `json` and `lines` the live line is not drawn, and the counts a `failed`
  result gives are the summary's. Under `-v` a `unit` record opens every unit,
  as the text names the unit's target and profile on its result.

## A record

```json
{"schema":3,"record":"diagnostic","severity":"error","code":"name.unresolved","message":"unresolved identifier `helpr`","origin":"resolve","primary":{"file":"src/bin/main.mach","line":5,"column":9,"end_line":5,"end_column":14,"byte_start":84,"byte_end":89},"related":[],"notes":[],"help":["did you mean `helper`?"],"fixes":[{"label":"replace with `helper`","edits":[{"file":"src/bin/main.mach","line":5,"column":9,"end_line":5,"end_column":14,"byte_start":84,"byte_end":89,"replacement":"helper"}]}]}
```

A type error in a generic instance, whose primary span covers two lines and
whose related site is in another file:

```json
{"schema":3,"record":"diagnostic","severity":"error","code":"operator.operand_mismatch","message":"operands `u8` and `i64` have different types. cast one operand with `::` so both have one type","origin":"sema","primary":{"file":"src/bin/main.mach","line":6,"column":17,"end_line":7,"end_column":16,"byte_start":103,"byte_end":131},"related":[{"file":"src/lib.mach","line":2,"column":9,"end_line":2,"end_column":14,"byte_start":41,"byte_end":46,"label":"in this generic body, checked against this instance's type arguments"}],"notes":[],"help":[],"fixes":[]}
```

| Member | Type | Meaning |
|---|---|---|
| `schema` | integer | the schema version, `3` |
| `record` | string | the record type, `"diagnostic"`, a [failure](#failures) included |
| `severity` | string | `"error"`, `"warning"` or `"note"` |
| `code` | string | the diagnostic's key from the [registry](diagnostics.md#the-registry), such as `name.unresolved`; the stable identity of the kind |
| `message` | string | the primary text; its wording may change, the `code` does not |
| `origin` | string | the phase that produced the diagnostic, see [Origins](#origins); absent when mach cannot attribute it |
| `primary` | span or `null` | where the diagnostic points; `null` for a diagnostic with no location, such as `target.skipped` |
| `related` | array of span | other sites the diagnostic names, each span with a `label`: a string, or `null` for a site shown without one |
| `notes` | array of string | the notes, in order, including a related site's label when mach cannot locate that site |
| `help` | array of string | the help lines, in order |
| `fixes` | array of fix | the suggested fixes, in order |

A **fix** is an object with a `label` (string, what the fix does) and `edits`,
an array of spans each with a `replacement` string: applying a fix replaces
the bytes of every one of its edits with that edit's replacement. The edits of
one fix never overlap. A fix is carried here and offered by the language server,
and the human rendering does not print it.

### Spans

A span is an object:

| Member | Type | Meaning |
|---|---|---|
| `file` | string | the source file: relative to the project root for a file inside it, absolute for a file outside it, in the host's path spelling |
| `line` | integer | the line of the span's first byte, from 1 |
| `column` | integer | the column of the span's first byte, from 1, counted in UTF-8 bytes |
| `end_line` | integer | the line of the position just past the span's last byte |
| `end_column` | integer | the column of that position, from 1, in UTF-8 bytes |
| `byte_start` | integer | the byte offset of the span's first byte in the file, from 0 |
| `byte_end` | integer | the byte offset just past its last byte |

The end is exclusive, as in the Language Server Protocol: a span of `helpr` at
line 5 column 9 ends at line 5 column 14. An empty span has its end equal to
its start. A column counts bytes, not characters, so a tool that needs UTF-16
columns converts from the line's text. `line` and `column` are the position
the human rendering shows after `@`.

### Origins

| Origin | Produced by |
|---|---|
| `build` | build configuration: the manifest's target, profile and artifact selection, and build steps |
| `load` | loading and parsing the module closure, and `$if` gates decided while loading |
| `resolve` | name resolution, `use`, exports and visibility |
| `sema` | type checking, including a declaration's unfulfilled `#[expect]` |
| `lower` | lowering to IR and the optimizer |
| `codegen` | code generation and the constant-time validation of emitted code, and `vector.scalarize` |
| `link` | linking an executable or library |
| `test` | the test runner of `mach test`: running the collected tests |

A report takes the origin of the part of the compiler whose work raised it,
whichever part asked for that work, and a diagnostic and a failure follow the
same rule: a failure in lowering is `lower` even when code generation asked for
the lowering. A report raised outside that work, by the build driving a phase,
takes the phase the build was in.

A key names a rule, not a phase, so one key can arrive from two origins:
`secret.branch` is `sema` when the type checker finds it and `codegen` when
the constant-time validator does.

## Failures

A failure raised outside the compiler's diagnostics is a diagnostic record, as
the human rendering shows it as a diagnostic: the snippet at its place, its
related places and its notes. Its `severity` is `"error"`, its `code` the
failure's key, its `notes` the lines the failure carries, in order, and its
`help` is empty. A refusal of a dependency's manifest carries one note naming
the chain of projects from the root that reaches the dependency, such as
`"reached through app -> lib"`. A failure met while cleaning up after the
failure is a note on it, such as `"cleanup also failed: cannot remove the build
step staging area '...' (permission denied)"`, and so is a later check of a
build step that also failed, such as `"the step also failed: ..."`. The
`message` is always the primary failure's own text.

```json
{"schema":3,"record":"diagnostic","severity":"error","code":"link.input_invalid","message":"the link does not define the entry symbol `_start`. an image takes its entry symbol from a module its artifact's entry reaches, so write a `use` of the module that defines it in that entry","origin":"link","primary":{"file":"mach.toml","line":43,"column":9,"end_line":43,"end_column":24,"byte_start":622,"byte_end":637},"related":[],"notes":[],"help":[],"fixes":[]}
```

`primary` is the span in the file that caused the failure, or `null` when no
file did. A manifest refusal points into the `mach.toml` that made the claim it
refuses: a value it rejects, the key token of a name or key it rejects, the
table a required key is missing from, or the start of the file when a required
table is missing from it. A dependency's refusal points into the
dependency's own manifest, not the root that imported it. Its `file` follows
the rule for every span, so a dependency's manifest is `dep/libx/mach.toml`,
and `dep\libx\mach.toml` on Windows. The human rendering shows the same place
after the headline, as `@ <file>:<line>:<column>` and the snippet, naming the
file by the path it was read at, as it names a source file.

```json
{"schema":3,"record":"diagnostic","severity":"error","code":"version.invalid_range","message":"mach.toml: [project].mach = \"^x\" is not a valid range. expected a number (clause 1)","origin":"build","primary":{"file":"mach.toml","line":4,"column":8,"end_line":4,"end_column":12,"byte_start":46,"byte_end":50},"related":[],"notes":[],"help":[],"fixes":[]}
```

A refusal raised later from the parsed manifest, while a build is laid out or
its steps run, points at the entry that caused it the same way:

- a path template that does not expand, at its value: `[project].work`, an
  artifact's `out`, a local link's `path`, and a step's `argv`, `env`, `in` and
  `out` entries
- a step output under a directory the compiler owns, at the output
- a local link found neither among the step outputs nor on disk, at its `path`
- an artifact that does not build for the selected target, at its `targets`
- no artifact building for the selected target, at every artifact's `targets`

A refusal between several entries points at the first and lists the others in
`related`, each with a `null` label:

- two steps declaring one output, or two artifacts writing one path, at the
  first claim
- more than one `default = true` profile or artifact, at the first
  `default` value
- a selection several declarations could satisfy (`native` matching several
  targets, several profiles or artifacts with none marked default), at
  the first candidate's table key
- `native` matching no declared target, at the first declared target's table key,
  the other declared targets related
- a cycle among `need` entries, at the entry of its first edge, the entries of
  the other edges related

The human rendering shows each related place with its own `@` line and
snippet after the primary's. A target or profile named on the command line that the manifest does
not declare is caused by no entry in it, so its refusal has no `primary`.

```json
{"schema":3,"record":"diagnostic","severity":"error","code":"need.cycle","message":"mach.toml: step.a -> step.b -> step.a","origin":"build","primary":{"file":"mach.toml","line":67,"column":9,"end_line":67,"end_column":17,"byte_start":1007,"byte_end":1015},"related":[{"file":"mach.toml","line":73,"column":9,"end_line":73,"end_column":17,"byte_start":1075,"byte_end":1083,"label":null}],"notes":[],"help":[],"fixes":[]}
```

`origin` names the phase the failure came from, by the rule of
[Origins](#origins): `build` for the manifest, dependency resolution and build
steps, the phase whose work raised a failure inside the compiler, and `test` for
the test runner. A
refusal of the command line itself, such as an unknown flag or a project path
that names nothing, has no `origin`. A flag's value the build checks against
the project is refused by the build and carries `build`: a `-w` that is not a
relative path inside the project root (`cli.output_path`), or a `-t` or `-p`
the manifest does not declare (`selection.unknown`).

## Notes

A note says what a command did or found, without failing it, and is a diagnostic
record with `severity` `"note"`, its `code` the note's key, a `null`
`primary` and no `origin`. The human rendering writes a note that reports what a
command did as that command's result, as `mach init` writes `created` for
`project.created` and `realized` for `dep.realized`, and any other note as
`note <message>`.

| `code` | Written by |
|---|---|
| `dep.note` | a dependency command as it works: an overridden requirement, a directory left under `dep/`, a copy or checkout that stopped part way |
| `dep.effect` | a dependency command that failed, once for each change it made before the failure, after the failure's record |
| `project.created` | `mach init`, for the project it made, the id it chose, and under `--no-deps` the command that declares each dependency the entry uses |
| `dep.realized` | `mach init`, for each dependency it declared and realized, naming the commit it realized |

```json
{"schema":3,"record":"diagnostic","severity":"note","code":"project.created","message":"created project 'app'","primary":null,"related":[],"notes":[],"help":[],"fixes":[]}
{"schema":3,"record":"diagnostic","severity":"note","code":"dep.realized","message":"realized dependency 'std' @ a78d528f4b89","primary":null,"related":[],"notes":[],"help":[],"fixes":[]}
```

## Test records

Under `mach test` a run is records too, in place of its results and its
`test.failed` diagnostics.
Each (target, profile) whose tests run opens with a `run_start` record, writes
a `test` record per test in the order the tests finish, and closes with a
`run_end` record. A (target, profile) whose tests were built and not run is a
`skip` record, and under `--list` each collected test is a `case` record.

```json
{"schema":3,"record":"run_start","tests":4}
{"schema":3,"record":"test","name":"app.bin.main#passes","module":"app.bin.main","file":"src/bin/main.mach","line":9,"index":0,"target":"linux-x86_64","profile":"debug","outcome":"pass","code":0,"timeout_ns":0,"duration_ns":459535,"exe":"./out/linux-x86_64/debug/test/app/app","output":null,"origin":"test"}
{"schema":3,"record":"test","name":"app.bin.main#fails_on_purpose","module":"app.bin.main","file":"src/bin/main.mach","line":13,"index":1,"target":"linux-x86_64","profile":"debug","outcome":"exit","code":3,"timeout_ns":0,"duration_ns":641207,"exe":"./out/linux-x86_64/debug/test/app/app","output":"./out/linux-x86_64/debug/test/app/log/1.log","origin":"test"}
{"schema":3,"record":"test","name":"app.bin.main#skips_here","module":"app.bin.main","file":"src/bin/main.mach","line":17,"index":2,"target":"linux-x86_64","profile":"debug","outcome":"skip","code":77,"timeout_ns":0,"duration_ns":803249,"exe":"./out/linux-x86_64/debug/test/app/app","output":null,"origin":"test"}
{"schema":3,"record":"test","name":"app.parser#rejects_trailing_comma","module":"app.parser","file":"src/parser.mach","line":5,"index":3,"target":"linux-x86_64","profile":"debug","outcome":"pass","code":0,"timeout_ns":0,"duration_ns":964561,"exe":"./out/linux-x86_64/debug/test/app/app","output":null,"origin":"test"}
{"schema":3,"record":"run_end","passed":2,"failed":1,"skipped":1,"total":4,"duration_ns":1574518,"mismatch":1,"infrastructure_failure":0,"resource_exhaustion":0,"timed_out":0,"origin":"test"}
{"schema":3,"record":"skip","target":"linux-aarch64","profile":"debug","reason":"not native to this host"}
```

A `test` or `case` record names its test with these members:

| Member | Type | Meaning |
|---|---|---|
| `name` | string | the test's qualified name |
| `module` | string | the module that declares it |
| `file` | string | the source file, spelled as a span's `file` is |
| `line` | integer | the line of its declaration, from 1 |
| `index` | integer | its index in the dispatcher, the argument a rerun passes |
| `target` | string or `null` | the target it ran for |
| `profile` | string or `null` | the profile it ran under |

A `test` record adds how it ended:

| Member | Type | Meaning |
|---|---|---|
| `outcome` | string | `"pass"`, `"skip"`, `"exit"`, `"signal"`, `"timeout"`, `"spawn"` or `"other"` |
| `code` | integer | its exit code, or `0` when it has none |
| `timeout_ns` | integer | the bound it exceeded in nanoseconds when its outcome is `"timeout"`, otherwise `0` |
| `duration_ns` | integer | how long it ran, in nanoseconds |
| `exe` | string | the dispatcher that ran it |
| `output` | string or `null` | the file holding its captured output, kept for a test that failed |
| `origin` | string | `"test"` |

A `case` record adds `object`, the test object that holds the test. Unlike
`file`, the paths `exe`, `output` and `object` are spelled from the project
path the command was given, as a `built` object's `out` is: `mach test .`
writes `./out/...`, and `mach test app` from the parent writes `app/out/...`.
A `skip` record carries `target`, `profile` and `reason`. `run_start` carries `tests`,
the run's count, and `run_end` the run's totals: `passed`, `failed`, `skipped`,
`total`, `duration_ns`, `timed_out` and the count of each failure class:
`mismatch` for a test that exited non-zero or died of a signal,
`infrastructure_failure` for one the harness could not launch or judge, and
`resource_exhaustion` for one stopped at its time bound. A test
that skipped (exit 77, see [test.md](test.md#semantics)) has the outcome
`"skip"` and counts in `skipped`, neither passed nor failed.

None of these records carries a `severity` or counts in a summary total. The
summary's `outcome` and `exit_code` say whether a test failed.

## Info records

Under `mach info` the page is one `host` record, and under `mach info targets`
the table is one `target` record a row, in the table's order. The text is not
written. Both are read from the catalog the text is written from, so a record
says what its row of the table says, with no cell to cut and no mark to read.

```json
{"schema":3,"record":"host","mach":"7.2.0","name":"linux-x86_64","sys":"linux","arch":"x86_64","env":null,"abi":"sysv64","isa":"cmov,cx8,x87,fxsr,osfxsr,mmx,sce,sse,sse2","code_model":"small","linkage":"static","object":"elf"}
```

```
host	schema=3	mach=7.2.0	name=linux-x86_64	sys=linux	arch=x86_64	abi=sysv64	isa=cmov,cx8,x87,fxsr,osfxsr,mmx,sce,sse,sse2	code_model=small	linkage=static	object=elf
```

A **host** record is the compiler's version and the target the host resolves to,
each part as the target took it:

| Member | Type | Meaning |
|---|---|---|
| `mach` | string | the compiler's version |
| `name` | string | the host as `-t` takes it, `<sys>-<arch>` |
| `sys` | string | its system |
| `arch` | string | its architecture |
| `env` | string or `null` | its environment, `null` for a host that states none |
| `abi` | string or `null` | its calling convention, `null` where its architecture names none |
| `isa` | string or `null` | the instruction set it stands on, whole as `--isa` takes it, `null` where its system states none |
| `code_model` | string or `null` | its code model, `null` where the host offers none |
| `linkage` | string or `null` | how an executable is linked for it, `null` where the host runs none |
| `object` | string | its object format |

A **target** record is one target a build can be made for and what a target
spelled so accepts:

```json
{"schema":3,"record":"target","name":"linux-riscv32","sys":"linux","arch":"riscv32","envs":[{"default":true,"name":null,"linkages":[{"default":true,"name":"static","abi":null}]},{"default":false,"name":"gnu","linkages":[{"default":true,"name":"pie","abi":null},{"default":false,"name":"dynamic","abi":null},{"default":false,"name":"static","abi":null}]}],"abis":[{"default":false,"name":"ilp32"},{"default":false,"name":"ilp32f"},{"default":false,"name":"ilp32d"}],"code_models":[{"default":true,"name":"medany"}],"needs_isa":true}
```

| Member | Type | Meaning |
|---|---|---|
| `name` | string | the target as `-t` takes it, `<sys>-<arch>` |
| `sys` | string | its system, as a manifest's `sys` names it |
| `arch` | string | its architecture, as a manifest's `arch` names it |
| `envs` | array of choice | the environments a target of the row can take, each with its `linkages` |
| `abis` | array of choice | the calling conventions it accepts |
| `code_models` | array of choice | the code models it accepts, empty where it has none |
| `needs_isa` | boolean | whether the target must name its instruction set, with `--isa` or a manifest's `isa`: `true` where its system bounds the machine not at all |

A **choice** is an object, one value of a part of the target:

| Member | Type | Meaning |
|---|---|---|
| `default` | boolean | whether the target takes this choice when it names none. No choice of a list is the default where the target must name one, as `linux-riscv32` must name its calling convention |
| `name` | string or `null` | the choice as its flag and its manifest key take it: `--env` and `env`, `--abi` and `abi`, `--code-model` and `code_model`, `--linkage` and `linkage`. `null` only in `envs`, for the hosts that state no environment, which a target takes by naming none, so that element is always the default |
| `linkages` | array of choice | in `envs` only: how an executable may be linked under the environment, empty where the target runs no executable there, as `vulkan-spirv` runs none |
| `abi` | string or `null` | in `linkages` only: the calling convention the linkage holds under. `null` where the environment's linkages are the same whatever the convention. Where they follow the convention, each convention lists its own linkages and marks its own default, so a tool keeps the linkages whose `abi` is `null` or the convention it chose |

`abis` and `code_models` are the row's, read with nothing narrowed, and
`linkages` those of each environment, as the table lists them. A boolean is
`true` or `false` in both encodings. Under `lines` every choice opens with its
`default`, which is never left out, so each `envs.default` opens an environment
and each `envs.linkages.default` after it a linkage of that environment:

```
target	schema=3	name=linux-riscv32	sys=linux	arch=riscv32	envs.default=true	envs.linkages.default=true	envs.linkages.name=static	envs.default=false	envs.name=gnu	envs.linkages.default=true	envs.linkages.name=pie	envs.linkages.default=false	envs.linkages.name=dynamic	envs.linkages.default=false	envs.linkages.name=static	abis.default=false	abis.name=ilp32	abis.default=false	abis.name=ilp32f	abis.default=false	abis.name=ilp32d	code_models.default=true	code_models.name=medany	needs_isa=true
```

A target that runs no executable and has no code model lists neither:

```json
{"schema":3,"record":"target","name":"vulkan-spirv","sys":"vulkan","arch":"spirv","envs":[{"default":true,"name":null,"linkages":[]},{"default":false,"name":"vulkan1.0","linkages":[]},{"default":false,"name":"vulkan1.1","linkages":[]},{"default":false,"name":"vulkan1.2","linkages":[]},{"default":false,"name":"vulkan1.3","linkages":[]}],"abis":[{"default":true,"name":"shader"}],"code_models":[],"needs_isa":false}
```

## Readout records

Under `-v` the [readout](readout.md) is written as records, each when its event
arrives: a `phase` record for each log line the human text writes, and for each
part of code generation, which the human text shows only in the `-vv` recap, and
a `counter` record for each of the unit's totals. Under `-vv` the records of the
items, the target's facts and each module's counters are added. Times are
integer microseconds.

```json
{"schema":3,"record":"unit","verb":"building","artifact":"demo","target":"linux-x86_64","profile":"debug"}
{"schema":3,"record":"phase","name":"target","count":1,"unit":"target","time_us":14936}
{"schema":3,"record":"phase","name":"load","count":55,"unit":"modules","time_us":55735,"slowest":"std.types.string","slowest_us":1374}
{"schema":3,"record":"phase","name":"lower","parent":"codegen","count":1,"unit":"object","time_us":266,"threads":16}
{"schema":3,"record":"phase","name":"other","time_us":4347}
```

A **unit** record opens each unit, before its rows. It is written for every
unit, as the text names every unit's target and profile on its result. It names the target the unit resolved to
whether `-t` selected it or the host's default did, here a project with no
`[target]` table, whose one target is `native`:

```json
{"schema":3,"record":"unit","verb":"building","artifact":"records","target":"native","profile":"debug"}
```

```
unit	schema=3	verb=building	artifact=records	target=native	profile=debug
```

| Member | Type | Meaning |
|---|---|---|
| `verb` | string | what the unit does, `"building"` or `"checking"` |
| `artifact` | string | the artifact the unit builds |
| `target` | string | the target the unit builds for, by its declared name |
| `profile` | string | the profile, absent when the unit names none |

A **phase** record is a phase, or a part of one, closing:

| Member | Type | Meaning |
|---|---|---|
| `name` | string | the phase's label: `steps`, `target`, `load`, `cache`, `resolve`, `sema`, `codegen`, its parts `prepare`, `lower`, `verify`, `passes`, `select`, `alloc`, `frame`, `emit`, `layout`, `encode` and `write`, `link`, or `other` for the time no row accounts for |
| `parent` | string | the phase this one is a part of, `"codegen"` for its parts; absent for a phase of its own |
| `count` | integer | what the phase processed; absent for `other` |
| `unit` | string | the noun the count is in, singular for a count of one: `"steps"` for the `steps` row, `"modules"` for the front end's rows and `cache`, `"objects"` for `codegen` and its parts, `"target"` for the `target` row, and for the `link` row `"binary"` for an executable, `"archive"` for a static library and `"library"` for a shared one |
| `time_us` | integer | the phase's time |
| `bytes` | integer | the size of the `link` row's output, present when it linked one |
| `threads` | integer | the threads its time is added up over, present only when more than one; such a row is not part of the unit's wall time |
| `dropped` | integer | the modules it dropped as target-gated, present only when any |
| `slowest` | string | under `-vv`, the slowest of its items, present when it had two or more |
| `slowest_us` | integer | that item's time, present with `slowest` |

The slowest item is on the phase record rather than on an item, since it is
only known when the phase closes.

Under `-vv` a **phase_item** record is each item a phase processed, written as
it finishes, before its phase's record: a module, a build step under `steps`, or
a stage of the `cache` row such as `module keys`. A module's code generation is
an item of each part that timed it, which the human text adds up into one
`codegen` line for the module:

```json
{"schema":3,"record":"phase_item","phase":"load","name":"std.print","time_us":422}
```

| Member | Type | Meaning |
|---|---|---|
| `phase` | string | the label of the phase, or the part of one, that processed it |
| `name` | string | the module, file, step or stage |
| `time_us` | integer | its time |

Under `-v` a **counter** record is the unit's total of a counter, written once per unit
after the rows of the phase that counted it (a test build's after its test objects' rows,
the total covering its modules and test objects), and under `-vv` one is also written for each module,
named by it, as its step finishes. A counter whose total is zero, such as every
counter of a unit whose objects all came from the cache, writes no total record,
while a module's record is written with its value even when that is zero:

```json
{"schema":3,"record":"counter","counter":"ir instrs","name":"demo.bin.main","value":11}
{"schema":3,"record":"counter","counter":"ir instrs","value":11}
```

| Member | Type | Meaning |
|---|---|---|
| `counter` | string | the counter's label: `functions`, `ir instrs`, `verify runs`, `heap blocks`, `heap bytes`, `heap peak`, `selected instrs`, `allocated instrs`, `alloc rounds` |
| `name` | string | the module, absent for the unit's total |
| `value` | integer | its value; a total is the sum over modules, and for `heap peak` the most any module reached |

Under `-vv` a **fact** record is a fact of the unit's target, written after the
`target` phase's record, one per fact, as `-vv` lists them beneath its log line:

```json
{"schema":3,"record":"fact","key":"abi","value":"sysv64"}
```

| Member | Type | Meaning |
|---|---|---|
| `key` | string | the fact: `isa`, `sys`, `env`, `abi`, `code_model`, `linkage` or `object` |
| `value` | string | its value, as the target resolved it |

Under `-v` a **growth** record is a growing cycle of instances, written each time its
instance count doubles:

```json
{"schema":3,"record":"growth","module":"app.bin.main","members":"peel","instances":32}
```

| Member | Type | Meaning |
|---|---|---|
| `module` | string | the module that declares the templates on the cycle |
| `members` | string | the templates on the cycle, comma separated |
| `instances` | integer | the instances of the cycle the module being checked has made so far |

A unit's `built` line is not a record of its own: it is carried by the
[summary record](#the-summary-record).

## The summary record

Every run ends with one summary record, the last line on stderr:

```json
{"schema":3,"record":"summary","errors":1,"warnings":0,"notes":0,"outcome":"failure","exit_code":1}
```

| Member | Type | Meaning |
|---|---|---|
| `errors` | integer | the diagnostic records before it with severity `"error"` |
| `warnings` | integer | those with severity `"warning"` |
| `notes` | integer | those with severity `"note"` |
| `outcome` | string | `"success"` when the command exits 0, `"interrupted"` when an interrupt stopped it, otherwise `"failure"` |
| `event` | string | when the outcome is `"interrupted"`, what stopped it: `"SIGINT"`, `"SIGTERM"` or `"SIGHUP"`, or on Windows `"Ctrl+C"`, `"Ctrl+Break"`, `"console close"` or `"service stop"`; absent otherwise |
| `exit_code` | integer | the command's exit code |
| `time_us` | integer | the wall time of every unit that built, added up; absent when none did |
| `built` | array | one object per unit that built, in order; absent when none did |

An interrupted command writes nothing after the interrupt arrives, neither the
records its stopped operations would make of it nor any output on stdout, and
ends with the summary record alone (the counts are of the records written
before it):

```json
{"schema":3,"record":"summary","errors":0,"warnings":0,"notes":0,"outcome":"interrupted","event":"SIGINT","exit_code":130}
```

The human text ends the same way, with the single line `interrupted by SIGINT`.

A `built` object has the unit's `out` (string, absent when the unit wrote no
output, as a `--list` run's), `modules` (integer), `rebuilt` (integer, the units
the build compiled code for rather than taking from the object cache), `bytes`
(integer, present when the output is one file: an executable, a static or shared
library or a finished module, never an `object` artifact's objects) and `time_us`
(integer).

```json
{"schema":3,"record":"summary","errors":0,"warnings":0,"notes":0,"outcome":"success","exit_code":0,"time_us":262071,"built":[{"out":"demo/out/linux-x86_64/debug/bin/demo","modules":51,"rebuilt":1,"bytes":52040,"time_us":262071}]}
```

A static library, and `mach test -a archive --list`, which writes nothing:

```json
{"schema":3,"record":"summary","errors":0,"warnings":0,"notes":0,"outcome":"success","exit_code":0,"time_us":48814,"built":[{"out":"./out/j2/lib/calc.a","modules":1,"rebuilt":1,"bytes":3640,"time_us":48814}]}
{"schema":3,"record":"summary","errors":0,"warnings":0,"notes":0,"outcome":"success","exit_code":0,"time_us":48183,"built":[{"modules":1,"rebuilt":1,"time_us":48183}]}
```

```
summary	schema=3	errors=0	warnings=0	notes=0	outcome=success	exit_code=0	time_us=48409	built.out=./out/l2/lib/calc.a	built.modules=1	built.rebuilt=1	built.bytes=3640	built.time_us=48409
summary	schema=3	errors=0	warnings=0	notes=0	outcome=success	exit_code=0	time_us=47618	built.modules=1	built.rebuilt=1	built.time_us=47618
```

## The lines encoding

`--report lines` writes exactly the records `json` writes, in the same order on
the same stream, in another encoding; the fields of each record are the ones
this page lists. A record is one line: its `record` value bare, then every
other member as `key=value` in the record's order, each part separated by one
tab. A member of a nested object is keyed by the path to it joined with `.`
(`primary.line=12`), each element of an array repeats the array's key
(`notes=first` then `notes=second`, `related.file=a` per related site), so an
empty array writes nothing, and a `null` member is left out. A boolean is written `true` or `false`. In a value a tab, a newline and a backslash are
written `\t`, `\n` and `\\`, so every tab on a line is a separator. Nothing is
quoted, padded or aligned, and `awk -F'\t' '$1=="test"'` selects every test
record.

```
diagnostic	schema=3	severity=error	code=name.unresolved	message=unresolved identifier `helpr`	origin=resolve	primary.file=src/bin/main.mach	primary.line=5	primary.column=9	primary.end_line=5	primary.end_column=14	primary.byte_start=84	primary.byte_end=89	help=did you mean `helper`?	fixes.label=replace with `helper`	fixes.edits.file=src/bin/main.mach	fixes.edits.line=5	fixes.edits.column=9	fixes.edits.end_line=5	fixes.edits.end_column=14	fixes.edits.byte_start=84	fixes.edits.byte_end=89	fixes.edits.replacement=helper
run_end	schema=3	passed=2	failed=1	skipped=1	total=4	duration_ns=1374916	mismatch=1	infrastructure_failure=0	resource_exhaustion=0	timed_out=0	origin=test
summary	schema=3	errors=0	warnings=0	notes=0	outcome=failure	exit_code=1	time_us=102176	built.out=./out/linux-x86_64/debug/test/app/app	built.modules=28	built.rebuilt=0	built.bytes=36408	built.time_us=102176
```

## Stability

- Every record carries `"schema": 3`. Schema 3 drops the `run_end` members
  `unsupported`, `target_unavailable` and `invalid_injection`, which no run ever
  counted. Schema 2 wrote a failure as a diagnostic record, where schema 1 wrote
  it as a `"failure"` record.
- `rebuilt` in a `built` object is a compatible addition under schema 3.
- The `host` and `target` records of `mach info`, and the boolean members they
  carry, are compatible additions under schema 3.
- **Compatible changes keep the version:** a new member in a record or a span,
  a new record type, and a new value of an enumerated member (`severity`,
  `origin`, `record`). A tool ignores members it does not know, skips records
  whose `record` it does not know, and accepts an `origin` it does not know.
- **Any other change bumps the version:** removing or renaming a member,
  changing its type or meaning, or changing the span convention.
- A diagnostic or failure `code` keeps its meaning for good, under the rules of the
  [registry](diagnostics.md#the-registry).

## See also

- [diagnostics.md](diagnostics.md): keys, the registry and its never-reused rule
- [readout.md](readout.md): what `-v` and `-vv` show, on which stream, and when
- [test.md](test.md#json-output): the records of a test run
