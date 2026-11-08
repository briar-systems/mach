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
emits: the EMIT_KIND_* bits `--emit` names, 0 when it is absent
entry: what the command line states of an entry build

## rec EntryArgs

```mach
pub rec EntryArgs;
```

what an entry build's command line states: its entry, source roots, artifact kind, target
narrowing and the levers its profile takes

file: the entry operand, nil for a project build
sources: every `--source` value, `name=dir` as written
kind: the `--kind` value, nil when absent
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
pub val EMIT_ERROR: str = "--emit expects asm, ir, obj or exe"
```

the refusal of an `--emit` value no product is named by

## val ARTIFACT

```mach
pub val ARTIFACT: std_args.Flag = std_args.Flag;
```

`--artifact <pattern>`, `-a`, a selector

## val TARGET

```mach
pub val TARGET: std_args.Flag = std_args.Flag;
```

`--target <pattern>`, `-t`, a selector, or an entry build's target

## val PROFILE

```mach
pub val PROFILE: std_args.Flag = std_args.Flag;
```

`--profile <pattern>`, `-p`, a selector

## val ALL

```mach
pub val ALL: std_args.Flag = std_args.Flag;
```

`--all`, which fills every selector not given with `*`

## val OUTPUT

```mach
pub val OUTPUT: std_args.Flag = std_args.Flag;
```

`-o <path>`, the linked-binary path

## val VERBOSE

```mach
pub val VERBOSE: std_args.Flag = std_args.Flag;
```

`-v`, once for each build phase's row and twice, as `-vv`, for each module or file as well; what
each readout level shows is the readout contract, doc/language/readout.md

## val QUIET

```mach
pub val QUIET: std_args.Flag = std_args.Flag;
```

`--quiet`, `-q`

## val SET

```mach
pub val SET: std_args.Flag = std_args.Flag;
```

`--set <key=value>`, a manifest key over every table of its section

## val VERIFY

```mach
pub val VERIFY: std_args.Flag = std_args.Flag;
```

`--verify`, mach's and mirl's verifiers over every lowered module

## val OPTIMIZE

```mach
pub val OPTIMIZE: std_args.Flag = std_args.Flag;
```

`--optimize`, `-O`, the profile's `optimize = true` for this invocation, and `--no-optimize`,
its `optimize = false`

## val DEBUG

```mach
pub val DEBUG: std_args.Flag = std_args.Flag;
```

`--debug`, the profile's `debug = true` for this invocation, and `--no-debug`, its `debug = false`

## val SOURCE

```mach
pub val SOURCE: std_args.Flag = std_args.Flag;
```

`--source <name=dir>`, a source root of an entry build

## val KIND

```mach
pub val KIND: std_args.Flag = std_args.Flag;
```

`--kind <kind>`, the kind of an entry build's artifact

## val ENV

```mach
pub val ENV: std_args.Flag = std_args.Flag;
```

`--env <name>`

## val ABI

```mach
pub val ABI: std_args.Flag = std_args.Flag;
```

`--abi <name>`

## val ISA

```mach
pub val ISA: std_args.Flag = std_args.Flag;
```

`--isa <selection>`

## val EXT

```mach
pub val EXT: std_args.Flag = std_args.Flag;
```

`--ext <name>`

## val CODE_MODEL

```mach
pub val CODE_MODEL: std_args.Flag = std_args.Flag;
```

`--code-model <name>`

## val WORK

```mach
pub val WORK: std_args.Flag = std_args.Flag;
```

`-w <path>`, the work directory

## val EMIT

```mach
pub val EMIT: std_args.Flag = std_args.Flag;
```

`--emit <kind>`, once per product

## val THREADS

```mach
pub val THREADS: std_args.Flag = std_args.Flag;
```

`--threads <n>`

## val LIB_DIR

```mach
pub val LIB_DIR: std_args.Flag = std_args.Flag;
```

`-L <dir>`

## val LIB

```mach
pub val LIB: std_args.Flag = std_args.Flag;
```

`-l <name>`

## val NO_CACHE

```mach
pub val NO_CACHE: std_args.Flag = std_args.Flag;
```

`--no-cache`. a build reads `obj/` as the object cache by default: a module whose object there
carries the build's key is reused instead of lowered and generated

## val DIAGNOSTICS

```mach
pub val DIAGNOSTICS: std_args.Flag = std_args.Flag;
```

`--diagnostics <human|json>`: how diagnostics, failures and a test run's results reach stderr

## val PLAN

```mach
pub val PLAN: std_args.Flag = std_args.Flag;
```

`--plan`

## val HELP

```mach
pub val HELP: std_args.Flag = std_args.Flag;
```

`--help`, `-h`: the invocation asks for its command's page instead of running it, which takes
precedence over every refusal but an unknown command

## val VERSION

```mach
pub val VERSION: std_args.Flag = std_args.Flag;
```

`--version`, the compiler's version alone on one line

## val RUNNER

```mach
pub val RUNNER: std_args.Flag = std_args.Flag;
```

`--runner <cmd>`

## val TIMEOUT

```mach
pub val TIMEOUT: std_args.Flag = std_args.Flag;
```

`--timeout <duration>`

## val ALLOW_UNRUNNABLE

```mach
pub val ALLOW_UNRUNNABLE: std_args.Flag = std_args.Flag;
```

`--allow-unrunnable`

## val OFFLINE

```mach
pub val OFFLINE: std_args.Flag = std_args.Flag;
```

`--offline`

## rec Removed

```mach
pub rec Removed;
```

a spelling an earlier version took, refused by name with what replaced it

name: the spelling, `--name` or `-c`
use: what to write instead

## val REMOVED_N

```mach
pub val REMOVED_N: usize = 20
```

row count of REMOVED

## val REMOVED

```mach
pub val REMOVED: [REMOVED_N]Removed = [REMOVED_N]Removed;
```

the options a previous major took, each refused by name with what to write instead

## rec Conflict

```mach
pub rec Conflict;
```

two flags a command refuses together

## rec Policy

```mach
pub rec Policy;
```

what mach holds a command's line to beyond what std.args parses, and what its page adds

summary: the command's one line in the overview
exits: its own exit codes, beside the shared ones
constraints: the rules its page prints after its options
refused: flags it recognizes only to refuse, by refusal
refusal: why it refuses them
conflicts: pairs of its flags it refuses together

## val NO_WORDS

```mach
pub val NO_WORDS: Slice[str] = Slice[str];
```

no words, for a command with no aliases or no constraints

## val NO_COMMANDS

```mach
pub val NO_COMMANDS: Slice[*std_args.Command] = Slice[*std_args.Command];
```

no commands, for a command nothing is nested under

## val NO_FLAGS

```mach
pub val NO_FLAGS: Slice[*std_args.Flag] = Slice[*std_args.Flag];
```

no flags

## val NO_EXITS

```mach
pub val NO_EXITS: Slice[exit.Exit] = Slice[exit.Exit];
```

no exit codes of a command's own

## val NO_CONFLICTS

```mach
pub val NO_CONFLICTS: Slice[Conflict] = Slice[Conflict];
```

no conflicts of a command's own

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
inv: the parsed command line
report: where the command reports a failure it goes on past or that carries an origin, in the
         format `--diagnostics` selected
environ: the environment the command runs in, which the processes it starts inherit

## fun operand

```mach
pub fun operand(inv: *std_args.Parsed, n: usize) *u8;
```

the operand at a position among a line's operands

inv: the parsed line
n: the position, 0 the first
ret: the operand, or nil when there are not that many

## fun value

```mach
pub fun value(inv: *std_args.Parsed, flag: *std_args.Flag) opt[*u8];
```

the value a flag was last given

inv: the parsed line
flag: a flag that takes a value
ret: the value, or none when the flag did not occur or occurred without one

## fun values

```mach
pub fun values(a: *A.Allocator, inv: *std_args.Parsed, flag: *std_args.Flag) res[Vector[str], fail.Fail];
```

every value a flag was given, in argument order

a: backs the vector
inv: the parsed line
flag: the flag
ret: the values, empty for a flag the command does not take; err when the vector cannot grow

## fun refusal

```mach
pub fun refusal(a: *A.Allocator, inv: *std_args.Parsed, policy: *Policy) fail.Fail;
```

a parsed line's refusal as a failure, in mach's words where mach knows more of the option than
the parser and the parser's words otherwise

a: owns the text
inv: the parsed line, which holds a refusal
policy: the matched command's policy
ret: a user failure of the refusal's kind

## fun run

```mach
pub fun run(a: *A.Allocator, inv: *std_args.Parsed, policy: *Policy, handler: fun(*Call) res[i64, fail.Fail]) i64;
```

run a parsed line and report how it ended through one report, in the format `--diagnostics`
selects, human when that format is itself malformed: its refusal when it holds one, then a flag
combination the command refuses, otherwise the handler, given an arena this call owns. a
failure the handler returns is reported here

a: owns the refusal's text
inv: the parsed line
policy: the matched command's policy
handler: what runs
ret: the exit code

## fun serve

```mach
pub fun serve(inv: *std_args.Parsed, handler: fun(*Call) res[i64, fail.Fail]) i64;
```

run a handler over a parsed line whatever it is refused for, reporting through one report in
the format `--diagnostics` selects, as `--help` does in place of the command it follows

inv: the parsed line
handler: what runs
ret: the exit code

## fun collect_selectors

```mach
pub fun collect_selectors(a: *A.Allocator, inv: *std_args.Parsed) res[manifest.Selectors, fail.Fail];
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
pub fun build_cli_invocation(a: *A.Allocator, inv: *std_args.Parsed) res[CliArgs, fail.Fail];
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
pub fun collect_link_inputs(a: *A.Allocator, c: *CliArgs, inv: *std_args.Parsed) err[fail.Fail];
```

fill c.overrides.link_tokens and c.overrides.lib_dirs in command-line order: every `-l` value and every
operand after the first that input.token_is_path accepts go to link_tokens, every `-L` value to
lib_dirs. Both vectors are reinitialised with a first

a: allocator for the two vectors
c: the arguments to fill
inv: the invocation
ret: ok, or err when a push fails

