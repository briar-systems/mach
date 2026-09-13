# mach.cli.args

## rec FlagSpec

```mach
pub rec FlagSpec;
```

one command-line option as the schema tables declare it

name: the spelling matched against an argv token, such as `--target` or `-o`
value: help placeholder for the option's value; non-empty means the option takes the next argv token, empty means a bare flag
doc: one-line help text; schema_valid rejects an empty one
default: help text for what applies when the option is absent; empty prints nothing
repeatable: rendered as `repeatable` by help; parse_invocation records every occurrence either way
conflicts: help text naming what this option cannot be combined with; nothing in this module enforces it
implies: help text naming the option this one also switches on
alias_of: the canonical spelling this option is an alias of, which must occur exactly once in the same schema; empty for a canonical option

## val SEL_N

```mach
pub val SEL_N: usize = 5
```

row count of SEL

## val SEL

```mach
pub val SEL: [SEL_N]FlagSpec = [SEL_N]FlagSpec;
```

the artifact-selection options `--target`, `--profile`, `-o`, `--bin`, `--lib`,
consumed by build, run, test, and doc; run hides `--lib` and doc hides all but
`--target` through visibility masks, and both still parse the hidden rows

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
pub val CGEN_N: usize = 6
```

row count of CGEN

## val CGEN

```mach
pub val CGEN: [CGEN_N]FlagSpec = [CGEN_N]FlagSpec;
```

the codegen options `--all-targets`, `--pie`, `--subsystem`, `-g`,
`--emit-asm`, `--emit-ir`, consumed by build and test; doc parses them with
every row hidden

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

## val OPT_O1_REMOVED_ERROR

```mach
pub val OPT_O1_REMOVED_ERROR: str = "-O1 was removed
```

the user error mach build prints when argv contains `-O1`, which no schema
table lists and which therefore parses as an unknown flag otherwise

## val EMIT_N

```mach
pub val EMIT_N: usize = 1
```

row count of EMIT

## val EMIT

```mach
pub val EMIT: [EMIT_N]FlagSpec = [EMIT_N]FlagSpec;
```

the `--emit <kind>` option, build only; its help text is request.EMIT_HELP

## val JOBS_N

```mach
pub val JOBS_N: usize = 1
```

row count of JOBS

## val JOBS

```mach
pub val JOBS: [JOBS_N]FlagSpec = [JOBS_N]FlagSpec;
```

the `--jobs <n>` option for build; test declares its own `--jobs` row in TEST_OWN

## val LINKIN_N

```mach
pub val LINKIN_N: usize = 2
```

row count of LINKIN

## val LINKIN

```mach
pub val LINKIN: [LINKIN_N]FlagSpec = [LINKIN_N]FlagSpec;
```

the repeatable link-input options `-L <dir>` and `-l <name>`, consumed by build and test

## val BUILD_OWN_N

```mach
pub val BUILD_OWN_N: usize = 3
```

row count of BUILD_OWN

## val BUILD_OWN

```mach
pub val BUILD_OWN: [BUILD_OWN_N]FlagSpec = [BUILD_OWN_N]FlagSpec;
```

the `--cache`, `--no-cache` and `--plan` options, build only

## val RUN_OWN_N

```mach
pub val RUN_OWN_N: usize = 2
```

row count of RUN_OWN

## val RUN_OWN

```mach
pub val RUN_OWN: [RUN_OWN_N]FlagSpec = [RUN_OWN_N]FlagSpec;
```

the `--runner <cmd>` and `--timeout_seconds <n>` options, run only

## val TEST_OWN_N

```mach
pub val TEST_OWN_N: usize = 9
```

row count of TEST_OWN

## val TEST_OWN

```mach
pub val TEST_OWN: [TEST_OWN_N]FlagSpec = [TEST_OWN_N]FlagSpec;
```

the test-only options `--cache`, `--no-cache`, `--jobs`, `--filter`,
`--include-deps`, `--list`, `--format`, `--runner`, `--timeout_seconds`

## val DOC_OWN_N

```mach
pub val DOC_OWN_N: usize = 1
```

row count of DOC_OWN

## val DOC_OWN

```mach
pub val DOC_OWN: [DOC_OWN_N]FlagSpec = [DOC_OWN_N]FlagSpec;
```

the `--out <dir>` option, doc only

## val INIT_N

```mach
pub val INIT_N: usize = 7
```

row count of INIT

## val INIT

```mach
pub val INIT: [INIT_N]FlagSpec = [INIT_N]FlagSpec;
```

the init options `--name`, `--force`, `--lib`, `--no-deps`, `--no-git`, `--quiet`, `-q`;
init does not use RDO, so its quiet rows are declared here

## val INFO_N

```mach
pub val INFO_N: usize = 1
```

row count of INFO

## val INFO

```mach
pub val INFO: [INFO_N]FlagSpec = [INFO_N]FlagSpec;
```

the `--version` option, info only

## val QUIET_N

```mach
pub val QUIET_N: usize = 2
```

row count of QUIET

## val QUIET

```mach
pub val QUIET: [QUIET_N]FlagSpec = [QUIET_N]FlagSpec;
```

the `--quiet` and `-q` options accepted by every dep action, through DEP_SCHEMA

## val DEP_ADD_N

```mach
pub val DEP_ADD_N: usize = 3
```

row count of DEP_ADD

## val DEP_ADD

```mach
pub val DEP_ADD: [DEP_ADD_N]FlagSpec = [DEP_ADD_N]FlagSpec;
```

the dep add options `--git <url>`, `--path <dir>`, `--ref <ref>`

## val DEP_REMOVE_N

```mach
pub val DEP_REMOVE_N: usize = 1
```

row count of DEP_REMOVE

## val DEP_REMOVE

```mach
pub val DEP_REMOVE: [DEP_REMOVE_N]FlagSpec = [DEP_REMOVE_N]FlagSpec;
```

the `--purge` option, dep remove only

## val DEP_UPDATE_N

```mach
pub val DEP_UPDATE_N: usize = 1
```

row count of DEP_UPDATE

## val DEP_UPDATE

```mach
pub val DEP_UPDATE: [DEP_UPDATE_N]FlagSpec = [DEP_UPDATE_N]FlagSpec;
```

the `--all` option, dep update only

## fun flag_in_table

```mach
pub fun flag_in_table(tok: *u8, table: *FlagSpec, n: usize) bool;
```

whether tok is the name of some row in table

tok: an argv token; nil is never in a table
table: first row of the FlagSpec table
n: row count of table
ret: true when a row's name equals tok

## def OptionArity

```mach
pub def OptionArity: u8
```

whether an option is a bare flag or takes a value

## val ARITY_FLAG

```mach
pub val ARITY_FLAG: OptionArity = 0
```

the option stands alone

## val ARITY_VALUE

```mach
pub val ARITY_VALUE: OptionArity = 1
```

the option consumes the next argv token as its value

## def SemanticKey

```mach
pub def SemanticKey: u32
```

identity of one schema row across a command's tables: the command id times
1000, plus the table's index within the schema times 100, plus the row's
index, plus 500 when the table belongs to a dep action rather than the
command. Two occurrences of the same option compare equal by key

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

## val DEPACT_SYNC

```mach
pub val DEPACT_SYNC: DepAction = 5
```

`mach dep sync`, the deprecated alias of pull; its own id, aliased through DepActionSpec.alias_of

## val DEPACT_VERIFY

```mach
pub val DEPACT_VERIFY: DepAction = 6
```

`mach dep verify`

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

## val BUILD_SCHEMA_N

```mach
pub val BUILD_SCHEMA_N: usize = 8
```

length of BUILD_SCHEMA

## val BUILD_SCHEMA

```mach
pub val BUILD_SCHEMA: [BUILD_SCHEMA_N]TableRef = [BUILD_SCHEMA_N]TableRef;
```

the option tables mach build accepts: SEL, RDO, CGEN, OPT, EMIT, JOBS, LINKIN, BUILD_OWN, all fully visible

## val CHECK_SEL_ACCEPTED

```mach
pub val CHECK_SEL_ACCEPTED: [SEL_N]bool = [SEL_N]bool;
```

SEL rows check accepts: every row but `-o`, since nothing is written

## val CHECK_SCHEMA_N

```mach
pub val CHECK_SCHEMA_N: usize = 2
```

length of CHECK_SCHEMA

## val CHECK_SCHEMA

```mach
pub val CHECK_SCHEMA: [CHECK_SCHEMA_N]TableRef = [CHECK_SCHEMA_N]TableRef;
```

the option tables mach check accepts: SEL masked by CHECK_SEL_ACCEPTED, and RDO

## val FMT_OWN_N

```mach
pub val FMT_OWN_N: usize = 1
```

length of FMT_OWN

## val FMT_OWN

```mach
pub val FMT_OWN: [FMT_OWN_N]FlagSpec = [FMT_OWN_N]FlagSpec;
```

the options only mach fmt accepts

## val FMT_SCHEMA_N

```mach
pub val FMT_SCHEMA_N: usize = 1
```

length of FMT_SCHEMA

## val FMT_SCHEMA

```mach
pub val FMT_SCHEMA: [FMT_SCHEMA_N]TableRef = [FMT_SCHEMA_N]TableRef;
```

the option tables mach fmt accepts: FMT_OWN alone

## val RUN_SEL_ACCEPTED

```mach
pub val RUN_SEL_ACCEPTED: [SEL_N]bool = [SEL_N]bool;
```

SEL rows run accepts: every row but `--lib`

## val RUN_SCHEMA_N

```mach
pub val RUN_SCHEMA_N: usize = 2
```

length of RUN_SCHEMA

## val RUN_SCHEMA

```mach
pub val RUN_SCHEMA: [RUN_SCHEMA_N]TableRef = [RUN_SCHEMA_N]TableRef;
```

the option tables mach run accepts: SEL masked by RUN_SEL_ACCEPTED, and RUN_OWN

## val TEST_SCHEMA_N

```mach
pub val TEST_SCHEMA_N: usize = 6
```

length of TEST_SCHEMA

## val TEST_SCHEMA

```mach
pub val TEST_SCHEMA: [TEST_SCHEMA_N]TableRef = [TEST_SCHEMA_N]TableRef;
```

the option tables mach test accepts: SEL, RDO, CGEN, OPT, LINKIN, TEST_OWN, all fully visible

## val DOC_SEL_ACCEPTED

```mach
pub val DOC_SEL_ACCEPTED: [SEL_N]bool = [SEL_N]bool;
```

SEL rows doc accepts: `--target`, `--bin`, `--lib`

## val DOC_RDO_ACCEPTED

```mach
pub val DOC_RDO_ACCEPTED: [RDO_N]bool = [RDO_N]bool;
```

RDO rows doc accepts: `--quiet` and `-q` only

## val DOC_CGEN_ACCEPTED

```mach
pub val DOC_CGEN_ACCEPTED: [CGEN_N]bool = [CGEN_N]bool;
```

CGEN rows doc accepts: none

## val DOC_SCHEMA_N

```mach
pub val DOC_SCHEMA_N: usize = 4
```

length of DOC_SCHEMA

## val DOC_SCHEMA

```mach
pub val DOC_SCHEMA: [DOC_SCHEMA_N]TableRef = [DOC_SCHEMA_N]TableRef;
```

the option tables mach doc accepts: SEL, RDO, CGEN under the DOC_*_ACCEPTED masks, and DOC_OWN

## val INFO_SCHEMA_N

```mach
pub val INFO_SCHEMA_N: usize = 1
```

length of INFO_SCHEMA

## val INFO_SCHEMA

```mach
pub val INFO_SCHEMA: [INFO_SCHEMA_N]TableRef = [INFO_SCHEMA_N]TableRef;
```

the option tables mach info accepts: INFO

## val INIT_SCHEMA_N

```mach
pub val INIT_SCHEMA_N: usize = 1
```

length of INIT_SCHEMA

## val INIT_SCHEMA

```mach
pub val INIT_SCHEMA: [INIT_SCHEMA_N]TableRef = [INIT_SCHEMA_N]TableRef;
```

the option tables mach init accepts: INIT

## val DEP_SCHEMA_N

```mach
pub val DEP_SCHEMA_N: usize = 1
```

length of DEP_SCHEMA

## val DEP_SCHEMA

```mach
pub val DEP_SCHEMA: [DEP_SCHEMA_N]TableRef = [DEP_SCHEMA_N]TableRef;
```

the option tables every mach dep action accepts: QUIET; an action's own tables come from DEP_ACTIONS

## val DEP_ADD_SCHEMA_N

```mach
pub val DEP_ADD_SCHEMA_N: usize = 1
```

length of DEP_ADD_SCHEMA

## val DEP_ADD_SCHEMA

```mach
pub val DEP_ADD_SCHEMA: [DEP_ADD_SCHEMA_N]TableRef = [DEP_ADD_SCHEMA_N]TableRef;
```

the option tables specific to dep add: DEP_ADD

## val DEP_REMOVE_SCHEMA_N

```mach
pub val DEP_REMOVE_SCHEMA_N: usize = 1
```

length of DEP_REMOVE_SCHEMA

## val DEP_REMOVE_SCHEMA

```mach
pub val DEP_REMOVE_SCHEMA: [DEP_REMOVE_SCHEMA_N]TableRef = [DEP_REMOVE_SCHEMA_N]TableRef;
```

the option tables specific to dep remove: DEP_REMOVE

## val DEP_UPDATE_SCHEMA_N

```mach
pub val DEP_UPDATE_SCHEMA_N: usize = 1
```

length of DEP_UPDATE_SCHEMA

## val DEP_UPDATE_SCHEMA

```mach
pub val DEP_UPDATE_SCHEMA: [DEP_UPDATE_SCHEMA_N]TableRef = [DEP_UPDATE_SCHEMA_N]TableRef;
```

the option tables specific to dep update: DEP_UPDATE

## val DEP_ADD_CONSTRAINT_N

```mach
pub val DEP_ADD_CONSTRAINT_N: usize = 2
```

length of DEP_ADD_CONSTRAINT

## val DEP_ADD_CONSTRAINT

```mach
pub val DEP_ADD_CONSTRAINT: [DEP_ADD_CONSTRAINT_N]str = [DEP_ADD_CONSTRAINT_N]str;
```

the constraint sentences help prints for dep add; the dep command enforces them, this module does not

## val DEP_UPDATE_CONSTRAINT_N

```mach
pub val DEP_UPDATE_CONSTRAINT_N: usize = 1
```

length of DEP_UPDATE_CONSTRAINT

## val DEP_UPDATE_CONSTRAINT

```mach
pub val DEP_UPDATE_CONSTRAINT: [DEP_UPDATE_CONSTRAINT_N]str = [DEP_UPDATE_CONSTRAINT_N]str;
```

the constraint sentence help prints for dep update; the dep command enforces it, this module does not

## rec DepActionSpec

```mach
pub rec DepActionSpec;
```

one dep action record as DEP_ACTIONS declares it; help renders every field

name: the action word after `dep`
alias_of: the canonical action this one is an alias of, which must itself be canonical; empty for a canonical action
id: the DepAction value
grammar: the argument grammar help appends after the action name, leading space included
effect: one-sentence description
exits: exit-code description
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

the seven dep action records: list, add, remove, update, pull, sync (alias
of pull), verify. Indexed by search, not by DepAction

## val BUILD_CONSTRAINT_N

```mach
pub val BUILD_CONSTRAINT_N: usize = 3
```

length of BUILD_CONSTRAINT

## val BUILD_CONSTRAINT

```mach
pub val BUILD_CONSTRAINT: [BUILD_CONSTRAINT_N]str = [BUILD_CONSTRAINT_N]str;
```

the constraint sentences help prints for build; the first is enforced by
build_cli_invocation, the second by selection_from_config, the third by the
build command

## val TEST_CONSTRAINT_N

```mach
pub val TEST_CONSTRAINT_N: usize = 2
```

length of TEST_CONSTRAINT

## val TEST_CONSTRAINT

```mach
pub val TEST_CONSTRAINT: [TEST_CONSTRAINT_N]str = [TEST_CONSTRAINT_N]str;
```

the constraint sentences help prints for test; enforced by
build_cli_invocation and selection_from_config

## val INFO_CONSTRAINT_N

```mach
pub val INFO_CONSTRAINT_N: usize = 1
```

length of INFO_CONSTRAINT

## val INFO_CONSTRAINT

```mach
pub val INFO_CONSTRAINT: [INFO_CONSTRAINT_N]str = [INFO_CONSTRAINT_N]str;
```

the constraint sentence help prints for info; the info command enforces it, this module does not

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
exits: exit-code description
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

## fun schema_for

```mach
pub fun schema_for(id: CommandId, action: DepAction, out: **TableRef, out_n: *usize, extra: **TableRef, extra_n: *usize);
```

the option tables a command and action accept. All four outputs are zeroed
first; an unknown id leaves them zeroed, and extra is filled only when the
command has actions, action is not DEPACT_NONE, and action has a record

id: the command
action: the dep action, or DEPACT_NONE
out: receives the command's tables
out_n: receives the length of out
extra: receives the action's own tables
extra_n: receives the length of extra

## fun table_accepts

```mach
pub fun table_accepts(t: *TableRef, j: usize) bool;
```

## fun schema_valid

```mach
pub fun schema_valid() bool;
```

whether COMMANDS and DEP_ACTIONS are well formed: every record has a name,
summary or effect, and exits; each count field is zero exactly when its
pointer is nil; every option row has a name and doc; ids and names are
unique; every alias resolves to its own record and collides with no name
or other alias; every option spelling and every alias_of target occurs
exactly once across a command's tables, and across the dep tables joined
with each action's tables; the dep record has has_action; an action's
alias_of names a canonical action. help refuses to render when this is
false

ret: true when every check passes

## rec SchemaHit

```mach
pub rec SchemaHit;
```

the result of looking one argv token up in a schema

found: the token names an option of the schema
arity: ARITY_VALUE when the row has a value placeholder, else ARITY_FLAG; ARITY_FLAG when not found
key: the row's SemanticKey; 0 when not found

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

## rec Occurrence

```mach
pub rec Occurrence;
```

one recognized option in argv

key: the row's SemanticKey
idx: argv index of the option token
has_value: a value token followed within the scanned range
value_idx: argv index of the value token, or idx when has_value is false

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

## fun build_cli_invocation

```mach
pub fun build_cli_invocation(cmd: CommandId, inv: *ParsedInvocation, argv: **u8) res[request.CliArgs, outcome.Fail];
```

the typed request.CliArgs of a build-shaped command: verbosity 0, 1, or 2
from `-v` and `-vv`, quiet from `--quiet` or `-q`, the CGEN flags, the SEL
values and `--jobs` as raw argv pointers or nil, opt_set and opt_release
from `-O0` and `-O2` with `-O0` winning when both occur, include_deps.
link_tokens and lib_dirs are left empty for collect_link_inputs

cmd: the command
inv: the parsed invocation
argv: the argument vector inv was parsed from
ret: the arguments, or err when `-v` or `-vv` is combined with `--quiet` or `-q`

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

## fun selection_from_config

```mach
pub fun selection_from_config(config: *request.CliArgs, out_sel: *manifest.Selection) err[outcome.Fail];
```

derive the manifest Selection from CliArgs: profile, target, and artifact
as strings that are empty when unset, want_lib when `--lib` was given,
has_artifact when either `--bin` or `--lib` was

config: the arguments
out_sel: receives the selection
ret: ok, or err when both `--bin` and `--lib` are set

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
pub fun build_init_invocation(inv: *ParsedInvocation, argv: **u8) InitInvocation;
```

read the init arguments out of a parsed invocation; the directory is the
first positional

inv: the parsed invocation
argv: the argument vector inv was parsed from; not read
ret: the arguments

## fun removed_option_message

```mach
pub fun removed_option_message(tok: str) opt[str];
```

the removal message for an option that used to exist, so every command refuses it the same way

tok: the flag-shaped token
ret: the message when `tok` names a removed option

## fun reject_unknown_from_invocation

```mach
pub fun reject_unknown_from_invocation(inv: *ParsedInvocation, argv: **u8, cmd: str) bool;
```

print `error: unknown flag` for the first unrecognized flag-shaped token, or the
removal message when the token names a removed option

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

the typed arguments of mach run; each value option is an index into argv

has_target: `--target` occurred with a value
target_idx: argv index of the target value
has_profile: `--profile` occurred with a value
profile_idx: argv index of the profile value
has_output: `-o` occurred with a value
output_idx: argv index of the output value
output_seen: `-o` occurred at all, with or without a value
has_bin: `--bin` occurred with a value
bin_idx: argv index of the bin value
has_runner: `--runner` occurred with a value
runner_idx: argv index of the runner value
has_timeout: `--timeout_seconds` occurred with a value
timeout_idx: argv index of the timeout value

## fun build_run_invocation

```mach
pub fun build_run_invocation(inv: *ParsedInvocation) RunInvocation;
```

read the run arguments out of a parsed invocation; the last occurrence of a
value option wins, and a trailing `-o` without a value sets output_seen only

inv: the parsed invocation
ret: the arguments

