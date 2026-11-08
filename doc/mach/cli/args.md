# mach.cli.args

## rec CliArgs

```mach
pub rec CliArgs;
```

the typed options of a build-shaped command: what it reports and selects, and
what it sets over the profile it builds

verbosity: 0, 1 or 2 from `-v` and `-vv`
quiet: `--quiet`
selectors: the `-a`, `-t` and `-p` patterns and `--all`
overrides: everything else the command sets, typed
settings: every `--set`, in argv order
emits: the EMIT_KIND_* bits `-e` names, 0 when it is absent
entry: what the command line states of an entry build

## rec EntryArgs

```mach
pub rec EntryArgs;
```

what an entry build's command line states: its entry, source roots, artifact kind, target
narrowing and the levers its profile takes

file: the entry operand, nil for a project build
sources: every `-s` value, `name=dir` as written
kind: the `-k` value, nil when absent
target: the `-t` value, the target as `<sys>-<arch>`, nil when absent
narrowing: the narrowing flags, each as written
exts: every `--ext` value, in argv order
optimize: the profile an entry build states optimizes
debug: the profile an entry build states emits debug information

## fun entry_stated

```mach
pub fun entry_stated(e: *EntryArgs) bool;
```

whether the command line states anything only an entry build takes

e: the entry arguments
ret: true when a source root, a kind or a narrowing flag is given

## val EMIT_KIND_ASM

```mach
pub val EMIT_KIND_ASM: u8 = 1
```

masc's listing of each module, beside its object

## val EMIT_KIND_IR

```mach
pub val EMIT_KIND_IR: u8 = 2
```

mirl's text of each module, beside its object

## val EMIT_KIND_OBJ

```mach
pub val EMIT_KIND_OBJ: u8 = 4
```

the objects, with no link

## val EMIT_KIND_EXE

```mach
pub val EMIT_KIND_EXE: u8 = 8
```

the linked artifact

## val EMIT_ERROR

```mach
pub val EMIT_ERROR: str = "-e expects asm, ir, obj or exe"
```

the refusal of an `-e` value no product is named by

## val TARGET

```mach
pub val TARGET: std_args.FlagSpec = std_args.FlagSpec;
```

`--target <pattern>`, a selector

## val ALL

```mach
pub val ALL: std_args.FlagSpec = std_args.FlagSpec;
```

`--all`, which fills every selector not given with `*`

## val OUTPUT

```mach
pub val OUTPUT: std_args.FlagSpec = std_args.FlagSpec;
```

`-o <path>`, the linked-binary path

## val SEL_N

```mach
pub val SEL_N: usize = 8
```

row count of SEL

## val SEL

```mach
pub val SEL: [SEL_N]*std_args.FlagSpec = [SEL_N]*std_args.FlagSpec;
```

the selection options, consumed by build, check, run, test and doc. each
selector takes an exact name or a glob and repeats. check hides `-o`, run
hides `--all`, and doc accepts only `--artifact` and `--target`

## val VERBOSITY_N

```mach
pub val VERBOSITY_N: usize = 1
```

row count of VERBOSITY

## val VERBOSITY

```mach
pub val VERBOSITY: [VERBOSITY_N]*std_args.FlagSpec = [VERBOSITY_N]*std_args.FlagSpec;
```

the readout levels `-v` and `-vv`, consumed by build, check and test

## val QUIET

```mach
pub val QUIET: std_args.FlagSpec = std_args.FlagSpec;
```

`--quiet`

## val QUIETNESS_N

```mach
pub val QUIETNESS_N: usize = 2
```

row count of QUIETNESS

## val QUIETNESS

```mach
pub val QUIETNESS: [QUIETNESS_N]*std_args.FlagSpec = [QUIETNESS_N]*std_args.FlagSpec;
```

`--quiet` and `-q`, consumed by build, check, test, doc, init and every dep action

## val CGEN_N

```mach
pub val CGEN_N: usize = 2
```

row count of CGEN

## val CGEN

```mach
pub val CGEN: [CGEN_N]*std_args.FlagSpec = [CGEN_N]*std_args.FlagSpec;
```

the codegen options, consumed by build and test and refused by run

## val OPTIMIZE

```mach
pub val OPTIMIZE: std_args.FlagSpec = std_args.FlagSpec;
```

`--optimize`, the profile's `optimize = true` for this invocation, and `--no-optimize`, its
`optimize = false`

## val DEBUG

```mach
pub val DEBUG: std_args.FlagSpec = std_args.FlagSpec;
```

`--debug`, the profile's `debug = true` for this invocation, and `--no-debug`, its `debug = false`

## val LEVERS_N

```mach
pub val LEVERS_N: usize = 4
```

row count of LEVERS

## val LEVERS

```mach
pub val LEVERS: [LEVERS_N]*std_args.FlagSpec = [LEVERS_N]*std_args.FlagSpec;
```

the profile levers, each over the selected profile for this invocation only,
consumed by build and test

## val SOURCE

```mach
pub val SOURCE: std_args.FlagSpec = std_args.FlagSpec;
```

`--source <name=dir>`, a source root of an entry build

## val KIND

```mach
pub val KIND: std_args.FlagSpec = std_args.FlagSpec;
```

`--kind <kind>`, the kind of an entry build's artifact

## val ENTRY_N

```mach
pub val ENTRY_N: usize = 4
```

row count of ENTRY

## val ENTRY

```mach
pub val ENTRY: [ENTRY_N]*std_args.FlagSpec = [ENTRY_N]*std_args.FlagSpec;
```

the source roots and kind of an entry build, consumed by build

## val NARROWING_N

```mach
pub val NARROWING_N: usize = 5
```

row count of NARROWING

## val NARROWING

```mach
pub val NARROWING: [NARROWING_N]*std_args.FlagSpec = [NARROWING_N]*std_args.FlagSpec;
```

the flags that narrow an entry build's `-t <sys>-<arch>`, consumed by build; each value left
out is the target's own, as mink's catalog states it

## val WORKDIR_N

```mach
pub val WORKDIR_N: usize = 1
```

row count of WORKDIR

## val WORKDIR

```mach
pub val WORKDIR: [WORKDIR_N]*std_args.FlagSpec = [WORKDIR_N]*std_args.FlagSpec;
```

`-w`, consumed by build and test

## val REMOVED_N

```mach
pub val REMOVED_N: usize = 16
```

row count of REMOVED

## val REMOVED

```mach
pub val REMOVED: [REMOVED_N]std_args.Removed = [REMOVED_N]std_args.Removed;
```

the options a previous major took, each refused by name with what to write instead

## val EMIT

```mach
pub val EMIT: std_args.FlagSpec = std_args.FlagSpec;
```

`--emit <kind>`, once per product

## val EMITS_N

```mach
pub val EMITS_N: usize = 2
```

row count of EMITS

## val EMITS

```mach
pub val EMITS: [EMITS_N]*std_args.FlagSpec = [EMITS_N]*std_args.FlagSpec;
```

`--emit` and `-e`, consumed by build

## val WORKERS_N

```mach
pub val WORKERS_N: usize = 1
```

row count of WORKERS

## val WORKERS

```mach
pub val WORKERS: [WORKERS_N]*std_args.FlagSpec = [WORKERS_N]*std_args.FlagSpec;
```

`--threads`, consumed by build and test

## val LIB

```mach
pub val LIB: std_args.FlagSpec = std_args.FlagSpec;
```

`-l <name>`

## val LINKIN_N

```mach
pub val LINKIN_N: usize = 2
```

row count of LINKIN

## val LINKIN

```mach
pub val LINKIN: [LINKIN_N]*std_args.FlagSpec = [LINKIN_N]*std_args.FlagSpec;
```

the repeatable link inputs `-L` and `-l`, consumed by build and test

## val CACHE_N

```mach
pub val CACHE_N: usize = 1
```

row count of CACHE

## val CACHE

```mach
pub val CACHE: [CACHE_N]*std_args.FlagSpec = [CACHE_N]*std_args.FlagSpec;
```

`--no-cache`, consumed by build and test

## val DIAG_N

```mach
pub val DIAG_N: usize = 1
```

row count of DIAG

## val DIAG

```mach
pub val DIAG: [DIAG_N]*std_args.FlagSpec = [DIAG_N]*std_args.FlagSpec;
```

`--diagnostics`, consumed by build, check and test

## val PLAN

```mach
pub val PLAN: std_args.FlagSpec = std_args.FlagSpec;
```

`--plan`

## val PLANS_N

```mach
pub val PLANS_N: usize = 1
```

row count of PLANS

## val PLANS

```mach
pub val PLANS: [PLANS_N]*std_args.FlagSpec = [PLANS_N]*std_args.FlagSpec;
```

`--plan`, consumed by build and refused by run

## val HELP

```mach
pub val HELP: std_args.FlagSpec = std_args.FlagSpec;
```

`--help`: the invocation asks for its command's page, or its action's, instead of running it.
it is the set's help flag, so every command accepts it and it takes precedence over every
refusal but an unknown action

## val HELP_TABLE

```mach
pub val HELP_TABLE: std_args.TableRef = std_args.TableRef;
```

the table every command accepts before its own

## val RUNNER

```mach
pub val RUNNER: std_args.FlagSpec = std_args.FlagSpec;
```

`--runner <cmd>`

## val RUNNERS_N

```mach
pub val RUNNERS_N: usize = 1
```

row count of RUNNERS

## val RUNNERS

```mach
pub val RUNNERS: [RUNNERS_N]*std_args.FlagSpec = [RUNNERS_N]*std_args.FlagSpec;
```

`--runner`, consumed by run and test

## val TIMEOUT

```mach
pub val TIMEOUT: std_args.FlagSpec = std_args.FlagSpec;
```

`--timeout <duration>`

## val TIMEOUTS_N

```mach
pub val TIMEOUTS_N: usize = 1
```

row count of TIMEOUTS

## val TIMEOUTS

```mach
pub val TIMEOUTS: [TIMEOUTS_N]*std_args.FlagSpec = [TIMEOUTS_N]*std_args.FlagSpec;
```

`--timeout`, consumed by run and test

## val ALLOW_UNRUNNABLE

```mach
pub val ALLOW_UNRUNNABLE: std_args.FlagSpec = std_args.FlagSpec;
```

`--allow-unrunnable`

## val ALLOW_UNRUNNABLES_N

```mach
pub val ALLOW_UNRUNNABLES_N: usize = 1
```

row count of ALLOW_UNRUNNABLES

## val ALLOW_UNRUNNABLES

```mach
pub val ALLOW_UNRUNNABLES: [ALLOW_UNRUNNABLES_N]*std_args.FlagSpec = [ALLOW_UNRUNNABLES_N]*std_args.FlagSpec;
```

`--allow-unrunnable`, consumed by test

## val OFFLINE

```mach
pub val OFFLINE: std_args.FlagSpec = std_args.FlagSpec;
```

`--offline`

## val OFFLINES_N

```mach
pub val OFFLINES_N: usize = 1
```

row count of OFFLINES

## val OFFLINES

```mach
pub val OFFLINES: [OFFLINES_N]*std_args.FlagSpec = [OFFLINES_N]*std_args.FlagSpec;
```

`--offline`, consumed by the dep actions that resolve releases

## val RELATIONS_N

```mach
pub val RELATIONS_N: usize = 1
```

length of RELATIONS

## val RELATIONS

```mach
pub val RELATIONS: [RELATIONS_N]std_args.Relation = [RELATIONS_N]std_args.Relation;
```

the relations among the shared rows, in force in every schema that accepts both rows

## val SELECTOR_CONSTRAINT

```mach
pub val SELECTOR_CONSTRAINT: str = "-a, -t and -p take an exact name, which must be declared, or a glob with * and ?, which must match
```

the rule every selector keeps, which the help of each command with selectors prints

## rec Call

```mach
pub rec Call;
```

what dispatch hands a command's handler; dispatch owns every member and outlives the call

a: an arena dispatch releases once the handler returns, for what lives as long as the command
backing: the heap under a, for what the command frees itself before it returns
inv: the invocation
report: where the command reports a failure it goes on past or that carries an origin, in the
         format `--diagnostics` selected
environ: the environment the command runs in, which the processes it starts inherit

## fun operand

```mach
pub fun operand(inv: *std_args.Invocation, n: usize) *u8;
```

the operand at a position among an invocation's operands

inv: the invocation
n: the position, 0 the first
ret: the operand, or nil when there are not that many

## fun value

```mach
pub fun value(inv: *std_args.Invocation, row: *std_args.FlagSpec) opt[*u8];
```

the value a row was given, which a row of a schema that does not list it never has

inv: the invocation
row: a canonical row that does not collect
ret: the value, or none when the row did not occur or occurred without one

## fun values

```mach
pub fun values(a: *A.Allocator, inv: *std_args.Invocation, row: *std_args.FlagSpec) res[Vector[str], fail.Fail];
```

every value a row was given, in argument order

a: backs the vector
inv: the invocation
row: a canonical row
ret: the values, empty for a row its schema does not list; err when the vector cannot grow

## fun refusal

```mach
pub fun refusal(a: *A.Allocator, inv: *std_args.Invocation) fail.Fail;
```

an invocation's refusal as a failure, in the engine's words

a: owns the text
inv: the invocation
ret: the failure: a user failure of the refusal's kind, or an internal one for a malformed set

## fun run

```mach
pub fun run(a: *A.Allocator, inv: *std_args.Invocation, handler: fun(*Call) res[i64, fail.Fail]) i64;
```

run an invocation and report how it ended through one report, in the format `--diagnostics`
selects, human when that format is itself malformed: its refusal when it holds one, otherwise
the handler, given an arena this call owns. a failure the handler returns is reported here

a: owns the refusal's text
inv: the invocation; its command is not nil
handler: what runs
ret: the exit code

## fun collect_selectors

```mach
pub fun collect_selectors(a: *A.Allocator, inv: *std_args.Invocation) res[manifest.Selectors, fail.Fail];
```

the `-a`, `-t` and `-p` patterns and `--all` an invocation was given

a: backs the pattern vectors
inv: the invocation
ret: the selectors; err when a vector cannot grow

## fun selection_hint

```mach
pub fun selection_hint(a: *A.Allocator, selectors: *manifest.Selectors, f: fail.Fail) fail.Fail;
```

a selection refusal whose unknown value names a path in the working directory,
restated as the shell's expansion of an unquoted wildcard it almost always is;
any other failure is returned as given

a: owns the restated message
selectors: the patterns the command was given
f: the failure resolving them

## fun build_cli_invocation

```mach
pub fun build_cli_invocation(a: *A.Allocator, inv: *std_args.Invocation) res[CliArgs, fail.Fail];
```

the typed CliArgs of a build-shaped command: verbosity, quiet, the selectors,
and the overrides, each typed as the request reads it: `-o` and `-w` as raw argument
pointers or nil, the LEVERS as given, `--threads` as a count or 0 when absent.
include_deps is false for the command to set, and link_tokens and lib_dirs are
left empty for collect_link_inputs

a: backs the selector patterns and the name lists
inv: the invocation
ret: the arguments; err when a vector cannot grow or a value names nothing its flag takes

## fun collect_link_inputs

```mach
pub fun collect_link_inputs(a: *A.Allocator, c: *CliArgs, inv: *std_args.Invocation) err[fail.Fail];
```

fill c.overrides.link_tokens and c.overrides.lib_dirs in command-line order: every `-l` value and every
operand after the first that input.token_is_path accepts go to link_tokens, every `-L` value to
lib_dirs. Both vectors are reinitialised with a first

a: allocator for the two vectors
c: the arguments to fill
inv: the invocation
ret: ok, or err when a push fails

