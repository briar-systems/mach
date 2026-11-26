[mach](../README.md) / [cli](README.md) / **args**

# mach.cli.args

the flags commands share, the policy a line is held to, and a build's typed arguments

a build-shaped command's line becomes `CliArgs` here, each value typed as the request
reads it. `run` reports a refused line or flag combination in the format `--report`
selects, or runs the command's handler, and an option a previous major took is refused
by name with what replaced it.

Source: [`src/cli/args.mach`](../../../src/cli/args.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.cli.diagnostic`](diagnostic.md), [`mach.compile.entry`](../compile/entry.md), [`mach.compile.request`](../compile/request.md), [`mach.manifest.select`](../manifest/select.md), [`mach.manifest.setting`](../manifest/setting.md).

| Type | | |
| :--- | :--- | :--- |
| [`CliArgs`](#cliargs) | `rec` | the typed options of a build-shaped command: what it reports and selects, and what it sets over the profile it builds |
| [`EntryArgs`](#entryargs) | `rec` | what an entry build's command line states: its entry, source roots, artifact kind, target narrowing and the levers its profile takes |
| [`Removed`](#removed) | `rec` | a spelling an earlier version took, refused by name with what replaced it |
| [`Conflict`](#conflict) | `rec` | two flags a command refuses together |
| [`Policy`](#policy) | `rec` | what mach holds a command's line to beyond what std.args parses |
| [`Call`](#call) | `rec` | what dispatch hands a command's handler; dispatch owns every member and outlives the call |

| On `CliArgs` | |
| :--- | :--- |
| [`build_cli_invocation`](#build_cli_invocation) | the typed CliArgs of a build-shaped command: verbosity, quiet, the selectors, and the overrides, each typed as the request reads it: `-o` and `-w` as raw argument pointers or nil, the LEVERS as given, `--threads` as a count or 0 when absent |
| [`collect_link_inputs`](#collect_link_inputs) | fill c.overrides.link_tokens and c.overrides.lib_dirs in command-line order: every `-l` value and every operand after the first, the project, go to link_tokens, every `-L` value to lib_dirs |

| On `EntryArgs` | |
| :--- | :--- |
| [`entry_stated`](#entry_stated) | whether the command line states anything only an entry build takes |

| On `Removed` | |
| :--- | :--- |
| [`removed_in`](#removed_in) | whether the removed spelling `row` stands in an invocation whose leading words after `mach` are `path`, as `dep add . regex`: a row for every command does, and a row for one command only within that command |

| On `Policy` | |
| :--- | :--- |
| [`refusal`](#refusal) | a parsed line's refusal as a failure, in mach's words where mach knows more of the option than the parser and the parser's words otherwise |
| [`run`](#run) | run a parsed line and report how it ended through one report, in the format `--report` selects, human when that format is itself malformed: its refusal when it holds one, then a flag combination the command refuses, otherwise the handler, given an arena this call owns |
| [`refuse`](#refuse) | report a parsed line's refusal through one report, in the format `--report` selects, human when that format is itself malformed |

| Functions | |
| :--- | :--- |
| [`operand`](#operand) | the operand at a position among a line's operands |
| [`value`](#value) | the value a flag was last given |
| [`values`](#values) | every value a flag was given, in argument order |
| [`serve`](#serve) | run a handler over a parsed line whatever it is refused for, reporting through one report in the format `--report` selects, as `--help` does in place of the command it follows |
| [`collect_selectors`](#collect_selectors) | the `-a`, `-t` and `-p` patterns and `--all` an invocation was given |
| [`selection_hint`](#selection_hint) | a selection refusal whose unknown value names a path in the working directory, restated as the shell's expansion of an unquoted wildcard it almost always is; any other failure is returned as given |
| [`verbosity`](#verbosity) | the readout level `-v` and `-vv` ask for: 0 without them, 1 for one, 2 for more, and 0 for a command that takes no `-v` |

| Constants | |
| :--- | :--- |
| [`EMIT_KIND_OBJ`](#constants) | the objects, with no link |
| [`EMIT_KIND_EXE`](#constants) | the linked artifact |
| [`EMIT_ERROR`](#constants) | the refusal of an `--emit` value no product is named by |
| [`ARTIFACT`](#constants) | `--artifact <pattern>`, `-a`, a selector |
| [`TARGET`](#constants) | `--target <pattern>`, `-t`, a selector, or an entry build's target |
| [`PROFILE`](#constants) | `--profile <pattern>`, `-p`, a selector |
| [`ALL`](#constants) | `--all`, which fills every selector not given with `*` |
| [`OUTPUT`](#constants) | `-o <path>`, the linked artifact's path |
| [`VERBOSE`](#constants) | `--verbose`, `-v`, once for each build phase's row and twice, as `-vv`, for each module or file as well; what each readout level shows is the readout contract, doc/language/readout.md |
| [`QUIET`](#constants) | `--quiet`, `-q` |
| [`SET`](#constants) | `--set <key=value>`, a manifest key over every table of its section |
| [`VERIFY`](#constants) | `--verify`, mach's and mirl's verifiers over every lowered module |
| [`OPTIMIZE`](#constants) | `--optimize`, `-O`, the profile's `optimize = true` for this invocation, and `--no-optimize`, its `optimize = false` |
| [`DEBUG`](#constants) | `--debug`, the profile's `debug = true` for this invocation, and `--no-debug`, its `debug = false` |
| [`SOURCE`](#constants) | `--source <name=dir>`, a source root of an entry build |
| [`KIND`](#constants) | `--kind <product>`, `-k`, the product of an entry build's artifact, by mink's name for it |
| [`LINKAGE`](#constants) | `--linkage <linkage>`, how an entry build's executable is linked, by mink's name for it |
| [`ENV`](#constants) | `--env <name>` |
| [`ABI`](#constants) | `--abi <name>` |
| [`ISA`](#constants) | `--isa <selection>` |
| [`EXT`](#constants) | `--ext <name>` |
| [`CODE_MODEL`](#constants) | `--code-model <name>` |
| [`WORK`](#constants) | `-w <path>`, the work directory |
| [`EMIT`](#constants) | `--emit <kind>`, once per product |
| [`THREADS`](#constants) | `--threads <n>` |
| [`LIB_DIR`](#constants) | `--library-dir <dir>`, `-L` |
| [`LIB`](#constants) | `--library <name>`, `-l` |
| [`DEP_WARNINGS`](#constants) | `--dep-warnings` |
| [`NO_CACHE`](#constants) | `--no-cache` |
| [`REPORT`](#constants) | `--report <human\|json\|lines>`: how everything a command reports reaches stderr: human text, or its records |
| [`PASSES`](#constants) | `--passes`: each module's report of the optimisation passes that changed each of its functions, which mirl writes |
| [`PLAN`](#constants) | `--plan` |
| [`HELP`](#constants) | `--help`, `-h`: the invocation asks for its command's page instead of running it, which takes precedence over every refusal but an unknown command |
| [`VERSION`](#constants) | `--version`, the compiler's version alone on one line |
| [`RUNNER`](#constants) | `--runner <cmd>` |
| [`TIMEOUT`](#constants) | `--timeout <duration>` |
| [`ALLOW_UNRUNNABLE`](#constants) | `--allow-unrunnable` |
| [`OFFLINE`](#constants) | `--offline` |
| [`REMOVED_N`](#constants) | row count of REMOVED |
| [`REMOVED`](#constants) | the options a previous major took, each refused by name with what to write instead |
| [`NO_WORDS`](#constants) | no words, for a command with no aliases |
| [`NO_COMMANDS`](#constants) | no commands, for a command nothing is nested under |
| [`NO_FLAGS`](#constants) | no flags |
| [`NO_CONFLICTS`](#constants) | no conflicts of a command's own |

---

## CliArgs

```mach
pub rec CliArgs {
    verbosity: u8;
    quiet:     bool;
    selectors: mach.manifest.select.Selectors;
    overrides: mach.compile.request.Overrides;
    settings:  Vector[mach.manifest.setting.Assignment];
    emits:     u8;
    entry:     EntryArgs;
}
```

the typed options of a build-shaped command: what it reports and selects, and
what it sets over the profile it builds

| Field | Type | |
| :--- | :--- | :--- |
| `verbosity` | `u8` | 0, 1 or 2 from `-v` and `-vv` |
| `quiet` | `bool` | `--quiet` |
| `selectors` | `mach.manifest.select.Selectors` | the `-a`, `-t` and `-p` patterns and `--all` |
| `overrides` | `mach.compile.request.Overrides` | everything else the command sets, typed |
| `settings` | `Vector[mach.manifest.setting.Assignment]` | every `--set`, in argv order |
| `emits` | `u8` | the EMIT_KIND_* bits `--emit` names besides the views, 0 when it names none |
| `entry` | `EntryArgs` | what the command line states of an entry build |

### build_cli_invocation

```mach
pub fun build_cli_invocation(a: *std.allocator.Allocator, inv: *std.args.Parsed) res[CliArgs, mach.base.fail.Fail]
```

the typed CliArgs of a build-shaped command: verbosity, quiet, the selectors,
and the overrides, each typed as the request reads it: `-o` and `-w` as raw argument
pointers or nil, the LEVERS as given, `--threads` as a count or 0 when absent.
include_deps is false for the command to set, and link_tokens and lib_dirs are
left empty for collect_link_inputs

| Parameter | |
| :--- | :--- |
| `a` | backs the selector patterns and the name lists |
| `inv` | the invocation |
| **returns** | the arguments; err when a vector cannot grow or a value names nothing its flag takes |

### collect_link_inputs

```mach
pub fun collect_link_inputs(a: *std.allocator.Allocator, c: *CliArgs, inv: *std.args.Parsed) err[mach.base.fail.Fail]
```

fill c.overrides.link_tokens and c.overrides.lib_dirs in command-line order: every `-l` value and every
operand after the first, the project, go to link_tokens, every `-L` value to lib_dirs. an operand
input.token_is_path does not accept names no link input and is refused. both vectors are
reinitialised first

| Parameter | |
| :--- | :--- |
| `a` | allocator for the two vectors |
| `c` | the arguments to fill |
| `inv` | the invocation |
| **returns** | ok, or err when an operand names no link input or a push fails |

## EntryArgs

```mach
pub rec EntryArgs {
    file:      str;
    sources:   Vector[str];
    kind:      str;
    linkage:   str;
    target:    str;
    narrowing: mach.compile.entry.Narrowing;
    exts:      Vector[str];
    optimize:  bool;
    debug:     bool;
}
```

what an entry build's command line states: its entry, source roots, artifact kind, target
narrowing and the levers its profile takes

| Field | Type | |
| :--- | :--- | :--- |
| `file` | `str` | the entry operand, nil for a project build |
| `sources` | `Vector[str]` | every `--source` value, `name=dir` as written |
| `kind` | `str` | the `--kind` value, nil when absent |
| `target` | `str` | the `-t` value, the target as `<sys>-<arch>`, nil when absent |
| `narrowing` | `mach.compile.entry.Narrowing` | the narrowing flags, each as written |
| `exts` | `Vector[str]` | every `--ext` value, in argv order |
| `optimize` | `bool` | the profile an entry build states optimizes |
| `debug` | `bool` | the profile an entry build states emits debug information |

### entry_stated

```mach
pub fun entry_stated(e: *EntryArgs) bool
```

whether the command line states anything only an entry build takes

| Parameter | |
| :--- | :--- |
| `e` | the entry arguments |
| **returns** | true when a source root, a kind, a linkage or a narrowing flag is given |

## Removed

```mach
pub rec Removed {
    command: str;
    name:    str;
    use:     str;
}
```

a spelling an earlier version took, refused by name with what replaced it

| Field | Type | |
| :--- | :--- | :--- |
| `command` | `str` | the command it was removed from, its words after `mach` as `dep add`, "" for every command |
| `name` | `str` | the spelling, `--name` or `-c` |
| `use` | `str` | what to write instead |

### removed_in

```mach
pub fun removed_in(row: *Removed, path: str) bool
```

whether the removed spelling `row` stands in an invocation whose leading words after `mach` are
`path`, as `dep add . regex`: a row for every command does, and a row for one command only
within that command

## Conflict

```mach
pub rec Conflict {
    flag:  *std.args.Flag;
    other: *std.args.Flag;
}
```

two flags a command refuses together

## Policy

```mach
pub rec Policy {
    refused:   Slice[*std.args.Flag];
    refusal:   str;
    conflicts: Slice[Conflict];
}
```

what mach holds a command's line to beyond what std.args parses

| Field | Type | |
| :--- | :--- | :--- |
| `refused` | `Slice[*std.args.Flag]` | flags it recognizes only to refuse, by refusal |
| `refusal` | `str` | why it refuses them |
| `conflicts` | `Slice[Conflict]` | pairs of its flags it refuses together |

### refusal

```mach
pub fun refusal(a: *std.allocator.Allocator, inv: *std.args.Parsed, policy: *Policy) mach.base.fail.Fail
```

a parsed line's refusal as a failure, in mach's words where mach knows more of the option than
the parser and the parser's words otherwise

| Parameter | |
| :--- | :--- |
| `a` | owns the text |
| `inv` | the parsed line, which holds a refusal |
| `policy` | the matched command's policy |
| **returns** | a user failure of the refusal's kind |

### run

```mach
pub fun run(a: *std.allocator.Allocator, inv: *std.args.Parsed, policy: *Policy, handler: fun(*Call) res[i64, mach.base.fail.Fail]) i64
```

run a parsed line and report how it ended through one report, in the format `--report`
selects, human when that format is itself malformed: its refusal when it holds one, then a flag
combination the command refuses, otherwise the handler, given an arena this call owns. a
failure the handler returns is reported here

| Parameter | |
| :--- | :--- |
| `a` | owns the refusal's text |
| `inv` | the parsed line |
| `policy` | the matched command's policy |
| `handler` | what runs |
| **returns** | the exit code |

### refuse

```mach
pub fun refuse(a: *std.allocator.Allocator, inv: *std.args.Parsed, policy: *Policy) i64
```

report a parsed line's refusal through one report, in the format `--report` selects, human
when that format is itself malformed

| Parameter | |
| :--- | :--- |
| `a` | owns the refusal's text |
| `inv` | the parsed line, which holds a refusal |
| `policy` | the matched command's policy |
| **returns** | the exit code |

## Call

```mach
pub rec Call {
    a:       *std.allocator.Allocator;
    backing: *std.allocator.Allocator;
    spans:   *std.allocator.heap.Pool;
    inv:     *std.args.Parsed;
    report:  *mach.cli.diagnostic.Report;
    environ: std.process.exec.Environment;
    # the texts the command's sessions read, which the failure it returns is located in
    sources: *mach.base.source.SourceMap;
}
```

what dispatch hands a command's handler; dispatch owns every member and outlives the call

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*std.allocator.Allocator` | an arena dispatch releases once the handler returns, for what lives as long as the command |
| `backing` | `*std.allocator.Allocator` | the heap under a, for what the command frees itself before it returns |
| `spans` | `*std.allocator.heap.Pool` | the spans every heap of the command draws from and gives back to |
| `inv` | `*std.args.Parsed` | the parsed command line |
| `report` | `*mach.cli.diagnostic.Report` | where the command reports a failure it goes on past or that carries an origin, in the format `--report` selected |
| `environ` | `std.process.exec.Environment` | the environment the command runs in, which the processes it starts inherit |

## Functions

### operand

```mach
pub fun operand(inv: *std.args.Parsed, n: usize) *u8
```

the operand at a position among a line's operands

| Parameter | |
| :--- | :--- |
| `inv` | the parsed line |
| `n` | the position, 0 the first |
| **returns** | the operand, or nil when there are not that many |

### value

```mach
pub fun value(inv: *std.args.Parsed, flag: *std.args.Flag) opt[*u8]
```

the value a flag was last given

| Parameter | |
| :--- | :--- |
| `inv` | the parsed line |
| `flag` | a flag that takes a value |
| **returns** | the value, or none when the flag did not occur or occurred without one |

### values

```mach
pub fun values(a: *std.allocator.Allocator, inv: *std.args.Parsed, flag: *std.args.Flag) res[Vector[str], mach.base.fail.Fail]
```

every value a flag was given, in argument order

| Parameter | |
| :--- | :--- |
| `a` | backs the vector |
| `inv` | the parsed line |
| `flag` | the flag |
| **returns** | the values, empty for a flag the command does not take; err when the vector cannot grow |

### serve

```mach
pub fun serve(inv: *std.args.Parsed, handler: fun(*Call) res[i64, mach.base.fail.Fail]) i64
```

run a handler over a parsed line whatever it is refused for, reporting through one report in
the format `--report` selects, as `--help` does in place of the command it follows

| Parameter | |
| :--- | :--- |
| `inv` | the parsed line |
| `handler` | what runs |
| **returns** | the exit code |

### collect_selectors

```mach
pub fun collect_selectors(a: *std.allocator.Allocator, inv: *std.args.Parsed) res[mach.manifest.select.Selectors, mach.base.fail.Fail]
```

the `-a`, `-t` and `-p` patterns and `--all` an invocation was given

| Parameter | |
| :--- | :--- |
| `a` | backs the pattern vectors |
| `inv` | the invocation |
| **returns** | the selectors; err when a vector cannot grow |

### selection_hint

```mach
pub fun selection_hint(a: *std.allocator.Allocator, selectors: *mach.manifest.select.Selectors, f: mach.base.fail.Fail) mach.base.fail.Fail
```

a selection refusal whose unknown value names a path in the working directory,
restated as the shell's expansion of an unquoted wildcard it almost always is;
any other failure is returned as given

| Parameter | |
| :--- | :--- |
| `a` | owns the restated message |
| `selectors` | the patterns the command was given |
| `f` | the failure resolving them |

### verbosity

```mach
pub fun verbosity(inv: *std.args.Parsed) u8
```

the readout level `-v` and `-vv` ask for: 0 without them, 1 for one, 2 for more, and 0 for a
command that takes no `-v`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `EMIT_KIND_OBJ` | `u8` | `1` | the objects, with no link |
| `EMIT_KIND_EXE` | `u8` | `2` | the linked artifact |
| `EMIT_ERROR` | `str` | `"--emit expects asm, ir, ir-opt, obj or exe"` | the refusal of an `--emit` value no product is named by |
| `ARTIFACT` | `std.args.Flag` | `std_args.Flag{long: "artifact", short: 'a', value: "pattern", repeat: true, negatable: false, help: "select artifacts by name or glob (default: the default selection)"}` | `--artifact <pattern>`, `-a`, a selector |
| `TARGET` | `std.args.Flag` | `std_args.Flag{long: "target", short: 't', value: "pattern", repeat: true, negatable: false, help: "which targets: a name or glob of the manifest's targets in a project, a <sys>-<arch> in an entry build (default: the host's)"}` | `--target <pattern>`, `-t`, a selector, or an entry build's target |
| `PROFILE` | `std.args.Flag` | `std_args.Flag{long: "profile", short: 'p', value: "pattern", repeat: true, negatable: false, help: "select profiles by name or glob (default: the default profile)"}` | `--profile <pattern>`, `-p`, a selector |
| `ALL` | `std.args.Flag` | `std_args.Flag{long: "all", short: 0, value: nil, repeat: false, negatable: false, help: "select every artifact, target and profile no selector narrows"}` | `--all`, which fills every selector not given with `*` |
| `OUTPUT` | `std.args.Flag` | `std_args.Flag{long: "output", short: 'o', value: "path", repeat: false, negatable: false, help: "write the linked artifact to this path"}` | `-o <path>`, the linked artifact's path |
| `VERBOSE` | `std.args.Flag` | `std_args.Flag{long: "verbose", short: 'v', value: nil, repeat: true, negatable: false, help: "show each phase as it closes, -vv each module too"}` | `--verbose`, `-v`, once for each build phase's row and twice, as `-vv`, for each module or file as well; what each readout level shows is the readout contract, doc/language/readout.md |
| `QUIET` | `std.args.Flag` | `std_args.Flag{long: "quiet", short: 'q', value: nil, repeat: false, negatable: false, help: "suppress non-error output"}` | `--quiet`, `-q` |
| `SET` | `std.args.Flag` | `std_args.Flag{long: "set", short: 0, value: "key=value", repeat: true, negatable: false, help: "set a manifest key for this invocation"}` | `--set <key=value>`, a manifest key over every table of its section |
| `VERIFY` | `std.args.Flag` | `std_args.Flag{long: "verify", short: 0, value: nil, repeat: false, negatable: false, help: "run the module and code generation verifiers"}` | `--verify`, mach's and mirl's verifiers over every lowered module |
| `OPTIMIZE` | `std.args.Flag` | `std_args.Flag{long: "optimize", short: 'O', value: nil, repeat: false, negatable: true, help: "optimize, or not with --no-optimize (default: the profile's)"}` | `--optimize`, `-O`, the profile's `optimize = true` for this invocation, and `--no-optimize`, its `optimize = false` |
| `DEBUG` | `std.args.Flag` | `std_args.Flag{long: "debug", short: 0, value: nil, repeat: false, negatable: true, help: "emit debug information, or not with --no-debug (default: the profile's)"}` | `--debug`, the profile's `debug = true` for this invocation, and `--no-debug`, its `debug = false` |
| `SOURCE` | `std.args.Flag` | `std_args.Flag{long: "source", short: 0, value: "name=dir", repeat: true, negatable: false, help: "bind a module prefix to a directory (default: the entry's directory)"}` | `--source <name=dir>`, a source root of an entry build |
| `KIND` | `std.args.Flag` | `std_args.Flag{long: "kind", short: 'k', value: "bin\|static\|shared\|object\|module", repeat: false, negatable: false, help: "the product to make (default: bin)"}` | `--kind <product>`, `-k`, the product of an entry build's artifact, by mink's name for it |
| `LINKAGE` | `std.args.Flag` | `std_args.Flag{long: "linkage", short: 0, value: "pie\|dynamic\|static", repeat: false, negatable: false, help: "how the executable links (default: the target's)"}` | `--linkage <linkage>`, how an entry build's executable is linked, by mink's name for it |
| `ENV` | `std.args.Flag` | `std_args.Flag{long: "env", short: 0, value: "name", repeat: false, negatable: false, help: "the environment (default: the target's)"}` | `--env <name>` |
| `ABI` | `std.args.Flag` | `std_args.Flag{long: "abi", short: 0, value: "name", repeat: false, negatable: false, help: "the calling convention (default: the target's)"}` | `--abi <name>` |
| `ISA` | `std.args.Flag` | `std_args.Flag{long: "isa", short: 0, value: "selection", repeat: false, negatable: false, help: "the instruction set (default: the target's)"}` | `--isa <selection>` |
| `EXT` | `std.args.Flag` | `std_args.Flag{long: "ext", short: 0, value: "name", repeat: true, negatable: false, help: "add an instruction set extension, once each"}` | `--ext <name>` |
| `CODE_MODEL` | `std.args.Flag` | `std_args.Flag{long: "code-model", short: 0, value: "name", repeat: false, negatable: false, help: "the code model (default: the target's)"}` | `--code-model <name>` |
| `WORK` | `std.args.Flag` | `std_args.Flag{long: "work-dir", short: 'w', value: "path", repeat: false, negatable: false, help: "the work directory (default: the project's)"}` | `-w <path>`, the work directory |
| `EMIT` | `std.args.Flag` | `std_args.Flag{long: "emit", short: 0, value: "asm\|ir\|ir-opt\|obj\|exe", repeat: true, negatable: false, help: "a product to write, once each (default: exe)"}` | `--emit <kind>`, once per product |
| `THREADS` | `std.args.Flag` | `std_args.Flag{long: "threads", short: 0, value: "n", repeat: false, negatable: false, help: "how many threads a build compiles and links on, and test processes run at once (default: host CPUs)"}` | `--threads <n>` |
| `LIB_DIR` | `std.args.Flag` | `std_args.Flag{long: "library-dir", short: 'L', value: "dir", repeat: true, negatable: false, help: "add a library search directory"}` | `--library-dir <dir>`, `-L` |
| `LIB` | `std.args.Flag` | `std_args.Flag{long: "library", short: 'l', value: "name", repeat: true, negatable: false, help: "link a library, object or archive by name"}` | `--library <name>`, `-l` |
| `DEP_WARNINGS` | `std.args.Flag` | `std_args.Flag{long: "dep-warnings", short: 0, value: nil, repeat: false, negatable: false, help: "also show dependencies' warnings, each under its own allow and #[expect]"}` | `--dep-warnings` |
| `NO_CACHE` | `std.args.Flag` | `std_args.Flag{long: "no-cache", short: 0, value: nil, repeat: false, negatable: false, help: "rebuild without reusing cached objects or steps"}` | `--no-cache` |
| `REPORT` | `std.args.Flag` | `std_args.Flag{long: "report", short: 0, value: "human\|json\|lines", repeat: false, negatable: false, help: "report as human text, NDJSON records or tab-separated record lines (default: human)"}` | `--report <human\|json\|lines>`: how everything a command reports reaches stderr: human text, or its records |
| `PASSES` | `std.args.Flag` | `std_args.Flag{long: "passes", short: 0, value: nil, repeat: false, negatable: false, help: "print each optimization pass that changed each function, and what it did"}` | `--passes`: each module's report of the optimisation passes that changed each of its functions, which mirl writes |
| `PLAN` | `std.args.Flag` | `std_args.Flag{long: "plan", short: 0, value: nil, repeat: false, negatable: false, help: "print the build plan without building"}` | `--plan` |
| `HELP` | `std.args.Flag` | `std_args.Flag{long: "help", short: 'h', value: nil, repeat: false, negatable: false, help: "show help"}` | `--help`, `-h`: the invocation asks for its command's page instead of running it, which takes precedence over every refusal but an unknown command |
| `VERSION` | `std.args.Flag` | `std_args.Flag{long: "version", short: 0, value: nil, repeat: false, negatable: false, help: "print the version"}` | `--version`, the compiler's version alone on one line |
| `RUNNER` | `std.args.Flag` | `std_args.Flag{long: "runner", short: 0, value: "cmd", repeat: false, negatable: false, help: "run through this command (default: the target's)"}` | `--runner <cmd>` |
| `TIMEOUT` | `std.args.Flag` | `std_args.Flag{long: "timeout", short: 0, value: "duration", repeat: false, negatable: false, help: "stop after a duration such as 30s"}` | `--timeout <duration>` |
| `ALLOW_UNRUNNABLE` | `std.args.Flag` | `std_args.Flag{long: "allow-unrunnable", short: 0, value: nil, repeat: false, negatable: false, help: "succeed when nothing selected can run on this host"}` | `--allow-unrunnable` |
| `OFFLINE` | `std.args.Flag` | `std_args.Flag{long: "offline", short: 0, value: nil, repeat: false, negatable: false, help: "resolve from releases already fetched"}` | `--offline` |
| `REMOVED_N` | `usize` | `20` | row count of REMOVED |
| `REMOVED` | `[REMOVED_N]Removed` | `[REMOVED_N]Removed{ Removed{command: "", name: "-g", use: "--debug"}, Removed{command: "", name: "-d", use: "--debug"}, Removed{command: "", name: "-s", use: "--source"}, Removed{command: "", name: "-e", use: "--emit"}, Removed{command: "", name: "-O0", use: "--no-optimize"}, Removed{command: "", name: "-O2", use: "-O or --optimize"}, Removed{command: "", name: "--emit-asm", use: "--emit asm"}, Removed{command: "", name: "--emit-ir", use: "--emit ir"}, Removed{command: "", name: "--jobs", use: "--threads"}, Removed{command: "", name: "--pie", use: "--linkage pie, or the linkage key of a bin artifact"}, Removed{command: "", name: "--simd", use: "--set simd=<mode>"}, Removed{command: "", name: "--arch", use: "-t <sys>-<arch>"}, Removed{command: "", name: "--sys", use: "-t <sys>-<arch>"}, Removed{command: "", name: "--of", use: "the target's system, which implies the object format"}, Removed{command: "", name: "--platform", use: "--set platform=<name>"}, Removed{command: "", name: "--subsystem", use: "--set subsystem=<name>"}, Removed{command: "", name: "--base", use: "--set base=<address>"}, Removed{command: "", name: "--stack-reserve", use: "--set stack_reserve=<bytes>"}, Removed{command: "", name: "--stack-commit", use: "--set stack_commit=<bytes>"}, Removed{command: "dep add", name: "--version", use: "--range"}, }` | the options a previous major took, each refused by name with what to write instead |
| `NO_WORDS` | `Slice[str]` | `Slice[str]{data: nil, size: 0}` | no words, for a command with no aliases |
| `NO_COMMANDS` | `Slice[*std.args.Command]` | `Slice[*std_args.Command]{data: nil, size: 0}` | no commands, for a command nothing is nested under |
| `NO_FLAGS` | `Slice[*std.args.Flag]` | `Slice[*std_args.Flag]{data: nil, size: 0}` | no flags |
| `NO_CONFLICTS` | `Slice[Conflict]` | `Slice[Conflict]{data: nil, size: 0}` | no conflicts of a command's own |

