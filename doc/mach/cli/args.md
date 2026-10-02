# mach.cli.args

## fwd request.CliArgs

```mach
fwd request.CliArgs
```

forwards [`mach.lang.build.request.CliArgs`](../lang/build/request.md#rec-cliargs)

## fwd request.is_object_path

```mach
fwd request.is_object_path
```

forwards [`mach.lang.build.request.is_object_path`](../lang/build/request.md#fun-is_object_path)

## val ARITY_ATTACHED

```mach
pub val ARITY_ATTACHED: OptionArity = 2
```

the option carries an optional value in its own token after `=`; it never
consumes the next one, so `--emit-ir` and `--emit-ir=listing` both parse

## rec FlagSpec

```mach
pub rec FlagSpec;
```

one command-line option as the schema tables declare it

name: the spelling matched against an argv token, such as `--target` or `-o`
value: help placeholder for the option's value; empty for ARITY_FLAG
arity: how the option takes its value; the ARITY_* constants
doc: one-line help text; schema_valid rejects an empty one
default: help text for what applies when the option is absent; empty prints nothing
repeatable: rendered as `repeatable` by help; parse_invocation records every occurrence either way
conflicts: help text naming what this option cannot be combined with; nothing in this module enforces it
implies: help text naming the option this one also switches on
alias_of: the canonical spelling this option is an alias of, which must occur exactly once in the same schema; empty for a canonical option

## val RDO_N

```mach
pub val RDO_N: usize = 4
```

row count of RDO

## val RDO

```mach
pub val RDO: [RDO_N]FlagSpec = [RDO_N]FlagSpec;
```

the readout options `-v`, `-vv`, `--quiet`, `-q`, consumed by build, test,
and doc; doc hides `-v` and `-vv`. `-q` is the alias of `--quiet`

## val CGEN_N

```mach
pub val CGEN_N: usize = 5
```

row count of CGEN

## val CGEN

```mach
pub val CGEN: [CGEN_N]FlagSpec = [CGEN_N]FlagSpec;
```

the codegen options `--pie`, `--subsystem`, `-g`,
`--emit-asm`, `--emit-ir`, consumed by build and test; doc parses them with
every row hidden. `--emit-ir` is the one attached-value row: bare it writes
the ir-debug dump, `--emit-ir=<form>` selects a row of printer.IR_FORMS

## val OPT_N

```mach
pub val OPT_N: usize = 2
```

row count of OPT

## val OPT

```mach
pub val OPT: [OPT_N]FlagSpec = [OPT_N]FlagSpec;
```

the pipeline overrides `-O0` and `-O2`, consumed by build and test; there is
no `-O1` row

## val DEP_ADD

```mach
pub val DEP_ADD: [DEP_ADD_N]FlagSpec = [DEP_ADD_N]FlagSpec;
```

the dep add options `--git <url>`, `--path <dir>`, `--ref <ref>`, `--version <range>`, `--offline`

## fun flag_in_table

```mach
pub fun flag_in_table(tok: *u8, table: *FlagSpec, n: usize) bool;
```

whether tok is the name of some row in table

tok: an argv token; nil is never in a table
table: first row of the FlagSpec table
n: row count of table
ret: true when a row's name equals tok

## def CommandId

```mach
pub def CommandId: u8
```

which command argv[1] named; the CMD_* constants

## val CMD_BUILD

```mach
pub val CMD_BUILD: CommandId = 0
```

`mach build`

## val CMD_RUN

```mach
pub val CMD_RUN: CommandId = 1
```

`mach run`

## val CMD_TEST

```mach
pub val CMD_TEST: CommandId = 2
```

`mach test`

## val CMD_CLEAN

```mach
pub val CMD_CLEAN: CommandId = 3
```

`mach clean`

## val CMD_DEP

```mach
pub val CMD_DEP: CommandId = 4
```

`mach dep`, the one command with actions

## val CMD_INIT

```mach
pub val CMD_INIT: CommandId = 5
```

`mach init`

## val CMD_DOC

```mach
pub val CMD_DOC: CommandId = 6
```

`mach doc`

## val CMD_INFO

```mach
pub val CMD_INFO: CommandId = 7
```

`mach info`

## val CMD_HELP

```mach
pub val CMD_HELP: CommandId = 8
```

`mach help`

## val CMD_CHECK

```mach
pub val CMD_CHECK: CommandId = 9
```

`mach check`

## val CMD_FMT

```mach
pub val CMD_FMT: CommandId = 10
```

`mach fmt`

## val CMD_N

```mach
pub val CMD_N: usize = 11
```

number of commands, the length of COMMANDS

## val CMD_NONE

```mach
pub val CMD_NONE: CommandId = 255
```

no command recognized; the value of ParsedInvocation.command when has_command is false

## def DepAction

```mach
pub def DepAction: u8
```

which dep action argv[2] named; the DEPACT_* constants

## val DEPACT_LIST

```mach
pub val DEPACT_LIST: DepAction = 0
```

`mach dep list`

## val DEPACT_ADD

```mach
pub val DEPACT_ADD: DepAction = 1
```

`mach dep add`

## val DEPACT_REMOVE

```mach
pub val DEPACT_REMOVE: DepAction = 2
```

`mach dep remove`

## val DEPACT_UPDATE

```mach
pub val DEPACT_UPDATE: DepAction = 3
```

`mach dep update`

## val DEPACT_PULL

```mach
pub val DEPACT_PULL: DepAction = 4
```

`mach dep pull`

## val DEPACT_VERIFY

```mach
pub val DEPACT_VERIFY: DepAction = 5
```

`mach dep verify`

## val DEPACT_OUTDATED

```mach
pub val DEPACT_OUTDATED: DepAction = 6
```

`mach dep outdated`

## val DEPACT_N

```mach
pub val DEPACT_N: usize = 7
```

number of dep actions, the length of DEP_ACTIONS

## val DEPACT_NONE

```mach
pub val DEPACT_NONE: DepAction = 255
```

no action recognized, and the value passed to schema_lookup to search a command's own tables only

## rec TableRef

```mach
pub rec TableRef;
```

one option table as a command or action schema references it

table: first row of the FlagSpec table
n: row count of table
accepted: per-row acceptance parallel to table, or nil to accept every row; an unaccepted row is unknown to the parser and absent from help

## rec DepActionSpec

```mach
pub rec DepActionSpec;
```

one dep action record as DEP_ACTIONS declares it; help renders every field

name: the action word after `dep`
id: the DepAction value
grammar: the argument grammar help appends after the action name, leading space included
effect: one-sentence description
exits: the action's own exit notes beside exit.SHARED; nil when exit_n is 0
exit_n: length of exits
tables: the action's own option tables, searched after DEP_SCHEMA; nil when table_n is 0
table_n: length of tables
constraints: constraint sentences for help; nil when constraint_n is 0
constraint_n: length of constraints
examples: example lines for help; nil when example_n is 0
example_n: length of examples

## val DEP_ACTIONS

```mach
pub val DEP_ACTIONS: [DEPACT_N]DepActionSpec = [DEPACT_N]DepActionSpec;
```

the seven dep action records: list, add, remove, update, outdated, pull, verify.
Indexed by search, not by DepAction

## rec CommandSpec

```mach
pub rec CommandSpec;
```

one command record as COMMANDS declares it; parse_invocation reads
pos_start, truncate_at_sep, has_action, and tables, help renders the rest

name: the command word at argv[1]
aliases: alternative command words; nil when alias_n is 0
alias_n: length of aliases
id: the CommandId value
grammar: the argument grammar help appends after the name, leading space included
summary: one-line description for the overview
effect: one-sentence description for the command page
exits: the command's own exit notes beside exit.SHARED; nil when exit_n is 0
exit_n: length of exits
terminator: help text for what follows `--`; empty when the command has none
tables: the option tables the command accepts; nil when table_n is 0
table_n: length of tables
constraints: constraint sentences for help; nil when constraint_n is 0
constraint_n: length of constraints
pos_start: first argv index parse_invocation scans: 2 after the command word, 3 after a dep action word
truncate_at_sep: stop scanning at the first `--`, so later tokens are neither options nor positionals
has_action: argv[2] names a DepAction
examples: example lines for help; nil when example_n is 0
example_n: length of examples

## val COMMANDS

```mach
pub val COMMANDS: [CMD_N]CommandSpec = [CMD_N]CommandSpec;
```

the ten command records in help order: build, check, run, test, clean, dep,
init, doc, info, help. Indexed by search, not by CommandId

## fun command_lookup

```mach
pub fun command_lookup(name: str) opt[CommandId];
```

the CommandId whose record has name as its name or one of its aliases

name: a command word
ret: some id, or none when no record matches

## fun command_spec

```mach
pub fun command_spec(id: CommandId) *CommandSpec;
```

the COMMANDS record carrying id

id: a CommandId
ret: the record, or nil when no record carries id (CMD_NONE always)

## fun dep_action_lookup

```mach
pub fun dep_action_lookup(name: str) opt[DepAction];
```

the DepAction whose record has name as its name; aliases are separate records with their own name

name: an action word
ret: some action, or none when no record matches

## fun dep_action_spec

```mach
pub fun dep_action_spec(id: DepAction) *DepActionSpec;
```

the DEP_ACTIONS record carrying id

id: a DepAction
ret: the record, or nil when no record carries id (DEPACT_NONE always)

## fun schema_valid

```mach
pub fun schema_valid() bool;
```

whether COMMANDS and DEP_ACTIONS are well formed: every record has a name,
summary or effect, and exit notes that keep to exit.notes_valid; each count
field is zero exactly when its pointer is nil; every option row has a name
and doc; ids and names are unique; every alias resolves to its own record
and collides with no name or other alias; every option spelling and every
alias_of target occurs exactly once across a command's tables, and across
the dep tables joined with each action's tables; the dep record has has_action. help refuses to
render when this is false

ret: true when every check passes

## fun schema_lookup

```mach
pub fun schema_lookup(id: CommandId, action: DepAction, tok: str) SchemaHit;
```

look tok up in the tables of id and action, the command's own tables before
the action's

id: the command
action: the dep action, or DEPACT_NONE to search the command's tables only
tok: an argv token
ret: the hit; found false with arity ARITY_FLAG and key 0 when tok is not an option there

## rec ParsedInvocation

```mach
pub rec ParsedInvocation;
```

argv classified against the command schema by parse_invocation. The three
arrays are argc long and owned by the invocation; dnit_invocation frees
them

argc: length of argv
command: the recognized command, or CMD_NONE
has_command: argv[1] named a command
action: the recognized dep action, or DEPACT_NONE
has_action: argv[2] named a dep action
sep_idx: index of the first `--` when the command truncates there, else argc
marks: true at every recognized option token and its value token; nil when parsing stopped before scanning
occ: recognized options in argv order; nil when parsing stopped before scanning
occ_len: length of occ in use
positional_idx: argv index of the first positional, or argc when none
positional_count: number of positionals
positional_idxs: argv indices of the positionals in order; nil when parsing stopped before scanning
has_positional: at least one positional
unknown_idx: argv index of the first unrecognized token that starts with `-`, or argc when none
has_unknown: at least one unrecognized flag-shaped token

## fun dnit_invocation

```mach
pub fun dnit_invocation(a: *A.Allocator, inv: *ParsedInvocation);
```

free the arrays parse_invocation allocated; nil arrays are skipped

a: the allocator parse_invocation was given
inv: the invocation

## fun parse_invocation

```mach
pub fun parse_invocation(a: *A.Allocator, argc: usize, argv: **u8) res[ParsedInvocation, outcome.Fail];
```

classify argv against the command schema: recognize the command at argv[1]
and, for dep, the action at argv[2], then scan from the command's pos_start
to the end or to the first `--` when the command truncates there. A
recognized value option claims the next token as its value when one is
in range. Every other token starting with `-` is unknown; the rest are
positionals. `-O1` is not in any table and so parses as unknown

a: allocator for the invocation's three arrays
argc: length of argv
argv: the argument vector, program name at argv[0]
ret: the invocation. argc below 2, an unrecognized command word, or an unrecognized dep action word return early with has_command or has_action false and every array nil. err on allocation failure, or when the command has no record

## fun invocation_flag

```mach
pub fun invocation_flag(cmd: CommandId, inv: *ParsedInvocation, flag: str) bool;
```

whether flag occurred, looked up in cmd's own tables only, never an action's

cmd: the command
inv: the parsed invocation
flag: the option spelling
ret: true when some occurrence carries the flag's key

## fun invocation_value

```mach
pub fun invocation_value(cmd: CommandId, inv: *ParsedInvocation, argv: **u8, flag: str) opt[*u8];
```

## fun diagnostics_format

```mach
pub fun diagnostics_format(cmd: CommandId, inv: *ParsedInvocation, argv: **u8) res[cli_diagnostic.Format, outcome.Fail];
```

the diagnostics format `--diagnostics=<human|json>` selects for a command
that takes the DIAG table; human when the option is absent

cmd: the command
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the format, or a user failure for a bare `--diagnostics` or any other value

## fun readout_allowed

```mach
pub fun readout_allowed(c: *request.CliArgs, format: cli_diagnostic.Format) err[outcome.Fail];
```

`-v` and `-vv` render the phase readout to stderr as text, which a json run
keeps to records

c: the command's arguments
format: the diagnostics format
ret: ok, or a user failure when a readout is asked for under json

## fun collect_selectors

```mach
pub fun collect_selectors(a: *A.Allocator, cmd: CommandId, inv: *ParsedInvocation, argv: **u8) res[manifest.Selectors, outcome.Fail];
```

the `-a`, `-t` and `-p` patterns and `--all` a command was given

a: backs the pattern vectors
cmd: the command
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the selectors; err when a selector has no value

## fun build_cli_invocation

```mach
pub fun build_cli_invocation(a: *A.Allocator, cmd: CommandId, inv: *ParsedInvocation, argv: **u8) res[request.CliArgs, outcome.Fail];
```

the typed request.CliArgs of a build-shaped command: verbosity 0, 1, or 2
from `-v` and `-vv`, quiet from `--quiet` or `-q`, the CGEN flags, the
selectors, `-o` and `--jobs` as raw argv pointers or nil, opt_set and
opt_release from `-O0` and `-O2` with `-O0` winning when both occur,
include_deps. link_tokens and lib_dirs are left empty for collect_link_inputs

a: backs the selector patterns
cmd: the command
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the arguments, or err when `-v` or `-vv` is combined with `--quiet` or `-q`
      or a selector has no value

## fun collect_link_inputs

```mach
pub fun collect_link_inputs(a: *A.Allocator, c: *request.CliArgs, cmd: CommandId, inv: *ParsedInvocation, argv: **u8) err[outcome.Fail];
```

fill c.link_tokens and c.lib_dirs in command-line order: every `-l` value and
every bare positional after the first that request.is_object_path accepts go
to link_tokens, every `-L` value to lib_dirs. Both vectors are reinitialised
with a first

a: allocator for the two vectors
c: the arguments to fill
cmd: the command
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: ok, or err when a push fails

## rec InitInvocation

```mach
pub rec InitInvocation;
```

the typed arguments of mach init

has_dir: a positional directory was given
dir_idx: argv index of that directory
has_name: `--name` occurred with a value
name_idx: argv index of the name value
force: `--force`
is_lib: `--lib`
no_deps: `--no-deps`
no_git: `--no-git`
quiet: `--quiet` or `-q`

## fun build_init_invocation

```mach
pub fun build_init_invocation(inv: *ParsedInvocation) InitInvocation;
```

read the init arguments out of a parsed invocation; the directory is the
first positional

inv: the parsed invocation
ret: the arguments

## fun unknown_flag

```mach
pub fun unknown_flag(a: *A.Allocator, inv: *ParsedInvocation, argv: **u8, cmd: str) opt[outcome.Fail];
```

the refusal of the first unrecognized flag-shaped token: `cli.flag_unknown`, or
`cli.flag_removed` with the removal message when the token names a removed option

a: owns the message
inv: the parsed invocation
argv: the argument vector inv was parsed from
cmd: the command name the message names
ret: the refusal, or none when every token was recognized

## fun reject_unknown_from_invocation

```mach
pub fun reject_unknown_from_invocation(inv: *ParsedInvocation, argv: **u8, cmd: str) bool;
```

print the refusal `unknown_flag` names for the first unrecognized flag-shaped token

inv: the parsed invocation
argv: the argument vector inv was parsed from
cmd: the command name printed in the message
ret: true when an unknown token was reported, false when there was none

## rec InfoInvocation

```mach
pub rec InfoInvocation;
```

the typed arguments of mach info

show_version: `--version` occurred
show_targets: the sole positional was `targets`
invalid_verb: a positional other than `targets` was given, or more than one positional

## fun build_info_invocation

```mach
pub fun build_info_invocation(inv: *ParsedInvocation, argv: **u8) InfoInvocation;
```

read the info arguments out of a parsed invocation

inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the arguments; `--version` together with `targets` sets both flags and is left for the command to refuse

## rec CleanInvocation

```mach
pub rec CleanInvocation;
```

the typed arguments of mach clean

has_project: a positional project path was given
project_idx: argv index of that path

## fun build_clean_invocation

```mach
pub fun build_clean_invocation(inv: *ParsedInvocation) CleanInvocation;
```

read the clean arguments out of a parsed invocation; the project is the first positional

inv: the parsed invocation
ret: the arguments

## rec DocInvocation

```mach
pub rec DocInvocation;
```

the typed arguments of mach doc

has_out: `--out` occurred with a value
out_idx: argv index of the out value
quiet: `--quiet` or `-q`
has_project: a positional project path was given
project_idx: argv index of that path

## fun build_doc_invocation

```mach
pub fun build_doc_invocation(inv: *ParsedInvocation) DocInvocation;
```

read the doc arguments out of a parsed invocation; the project is the first
positional, and the last occurrence of a value option wins

inv: the parsed invocation
ret: the arguments

## rec DepInvocation

```mach
pub rec DepInvocation;
```

argv-backed dependency operands and options from the shared parser.

## fun build_dep_invocation

```mach
pub fun build_dep_invocation(inv: *ParsedInvocation, argv: **u8) DepInvocation;
```

## rec RunInvocation

```mach
pub rec RunInvocation;
```

the typed arguments of mach run beside its selectors; each value option is an index into argv

has_output: `-o` occurred with a value
output_idx: argv index of the output value
output_seen: `-o` occurred at all, with or without a value
has_runner: `--runner` occurred with a value
runner_idx: argv index of the runner value
has_timeout: `--timeout` occurred with a value
timeout_idx: argv index of the timeout value

## fun build_run_invocation

```mach
pub fun build_run_invocation(inv: *ParsedInvocation) RunInvocation;
```

read the run arguments out of a parsed invocation; the last occurrence of a
value option wins, and a trailing `-o` without a value sets output_seen only

inv: the parsed invocation
ret: the arguments

